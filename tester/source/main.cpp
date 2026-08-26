#include <cstdio>
#include <cstring>
#include <cstdlib>

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

	void usage(const char* argv0) {
		printf("Usage: %s [-v] <path to yaml file>\n", argv0);
	}
}

int main(int argc, char *argv[]) {
	const char* path = nullptr;
	bool verbose = false;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
			verbose = true;
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
		printf("OK: %s\n", path);

	return 0;
}
