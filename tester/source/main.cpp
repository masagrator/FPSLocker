#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "Lock.hpp"          // FPSLocker - the patch compiler
#include "saltynx/lock.hpp"  // SaltyNX  - the patch runtime

namespace LOCK {
	constinit Patcher patcher;
}

namespace {
	constexpr const char* INTERMEDIATE = "test.bin";

	struct Combination { uint8_t fps; uint8_t refreshRate; };

	constexpr Combination COMBINATIONS[] = {
		{ 60, 60 },
		{ 30, 60 },
		{ 40, 60 },
		{ 25, 50 },
		{ 45, 40 },
		{ 120, 120 },
		{ 1, 60 },
	};

#ifdef HOST_ABI32
	constexpr bool IS_A32_VALIDATOR = true;
#else
	constexpr bool IS_A32_VALIDATOR = false;
#endif

	void usage(const char* argv0) {
		printf("Usage: %s [-v] [--a32 | --a64] <path to yaml file>\n", argv0);
		printf("  --a32  validate as AArch32 game (default if the patch has asm_a32 entries)\n");
		printf("  --a64  validate as AArch64 game\n");
	}

	// Returns true if the yaml has an `asm_a32` entry (comments are ignored).
	bool hasAsmA32(const char* path) {
		FILE* file = fopen(path, "r");
		if (!file) return false;
		char line[1024];
		bool found = false;
		while (!found && fgets(line, sizeof(line), file)) {
			std::string text = line;
			size_t comment = text.find('#');
			if (comment != std::string::npos) text.resize(comment);
			found = text.find("asm_a32") != std::string::npos;
		}
		fclose(file);
		return found;
	}

	// Runs the AArch32 validator (my_program32 next to this executable) with the same arguments.
	int forwardToA32(int argc, char* argv[]) {
		char self[4096] = {0};
		ssize_t len = readlink("/proc/self/exe", self, sizeof(self) - 1);
		std::string exe = (len > 0) ? std::string(self, len) : std::string(argv[0]);
		exe += "32";
		char** args = (char**)calloc(argc + 2, sizeof(char*));
		args[0] = exe.data();
		for (int i = 1; i < argc; i++) args[i] = argv[i];
		fflush(stdout);
		execv(exe.c_str(), args);
		printf("Could not run AArch32 validator: %s\n", exe.c_str());
		free(args);
		return 1;
	}
}

int main(int argc, char *argv[]) {
	const char* path = nullptr;
	bool verbose = false;
	int forced_arch = 0; // 32 or 64

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			verbose = true;
		}
		else if (strcmp(argv[i], "--a32") == 0) {
			forced_arch = 32;
		}
		else if (strcmp(argv[i], "--a64") == 0) {
			forced_arch = 64;
		}
		else if (argv[i][0] == '-') {
			printf("Unknown option: %s\n", argv[i]);
			usage(argv[0]);
			return 1;
		}
		else if (!path) {
			path = argv[i];
		}
		else {
			printf("Garbage arguments detected!\n");
			return 1;
		}
	}

	if (!path) {
		printf("No path to yaml file was provided!\n");
		return 1;
	}

	bool want_a32 = forced_arch ? (forced_arch == 32) : hasAsmA32(path);
	if (want_a32 != IS_A32_VALIDATOR) {
		if (want_a32) return forwardToA32(argc, argv);
		printf("This is the AArch32 validator, use my_program for AArch64 patches.\n");
		return 1;
	}

	Host::setVerbose(verbose);

	// ---- 1. compile the yaml -------------------------------------------
	Result ret = LOCK::readConfig(path);
	if (ret) {
		printf("readConfig failed: 0x%X\n", ret);
		return ret;
	}

	ret = LOCK::createPatch(INTERMEDIATE);
	if (ret) {
		printf("createPatch failed: 0x%X\n", ret);
		return ret;
	}

	// ---- 2. run it through the SaltyNX patcher -------------------------
	if (!Host::createSandbox()) {
		printf("Could not reserve the emulated address space (mmap failed).\n");
		return 1;
	}

	LOCK::patcher.bindMainRegion(Host::mainRegion());
	LOCK::patcher.bindDynamicRegions(Host::aliasRegion(), Host::heapRegion());

	ret = LOCK::patcher.loadFromFile(INTERMEDIATE);
	if (ret) {
		printf("loadFromFile failed: 0x%X\n", ret);
		Host::printErrors(stdout, "  - ");
		return ret;
	}

	if (LOCK::patcher.hasMasterWrite() && !LOCK::patcher.masterWriteApplied()) {
		printf("MASTER_WRITE was declared but never applied.\n");
		return 1;
	}

	for (const auto& c : COMBINATIONS) {
		ret = LOCK::patcher.applyPatch(c.fps, c.refreshRate);
		if (ret) {
			printf("applyPatch(%u fps, %u Hz) failed: 0x%X\n",
			       (unsigned)c.fps, (unsigned)c.refreshRate, ret);
			Host::printErrors(stdout, "  - ");
			return ret;
		}
	}

	if (Host::errorCount()) {
		printf("Patch applied, but %zu problem(s) were found:\n", Host::errorCount());
		Host::printErrors(stdout, "  - ");
		return 1;
	}

	if (verbose)
		printf("OK (%s): %s\n", IS_A32_VALIDATOR ? "AArch32" : "AArch64", path);

	return 0;
}
