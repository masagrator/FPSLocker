#ifdef __SWITCH__
#include <switch.h>
#else
#include <cstdint>
typedef uint32_t Result;
#define R_FAILED(res)      ((res)!=0)
#define R_SUCCEEDED(res)   ((res)==0)
#endif
#include "asmjit/a32.h"
#include <string>
#include <array>
#include <vector>
#include <cstring>
#include "c4/yml/node.hpp"
#include "c4/std/string.hpp"
#include <unordered_map>

namespace LOCK {

	struct declare_var {
		ptrdiff_t cave_offset;
		uint8_t value_type;
		std::string evaluate;
		uint64_t default_value;
	};

	struct declare_code {
		ptrdiff_t cave_offset;
		size_t instructions_num;
		uint32_t* code_buffer;
		uint8_t* adjust_types_buffer;
	};

	extern std::unordered_map<uint32_t, declare_var> declared_variables;
	extern std::unordered_map<uint32_t, uint64_t> declared_consts;
	extern std::vector<std::pair<uint32_t, declare_code>> declared_codes;
}

namespace ASM {

// Everything is in an anonymous namespace to not clash with asmA64.cpp symbols (same names are used there).
namespace {

	using namespace asmjit;
	using namespace asmjit::a32;

	/*
		Adjust types (same meaning as asmA64):
		0 - none
		1 - Branch_Direct:
		      B/BL to `_code()`: imm24 = -(cave_offset + 0x100) / 4 (always <= -64)
		2 - "ADRP" to code cave:    ADD Rd, PC, _code()  -> placeholder ADD Rd, PC, #0
		3 - "ADRP" to variables:    ADD Rd, PC, $var     -> placeholder ADD Rd, PC, #0
		4 - "ADRP" to main from cave: ADD Rd, PC, #imm   -> imm is encoded as provided
		5 - Branch_Relative:        B/BL/BLX to absolute main offset: imm24 = (target - (pc + 8)) / 4, where pc is the
		                            offset of the branch in its own region (main offset or cave offset). In main it's
		                            already correct (FPSLocker masks it to 0), in a code cave Core relocates it to main.

		6 - MOVW Rd, $var     -> placeholder imm16 = cave_offset, Core writes low 16 bits of (variables_start + cave_offset)
		7 - MOVT Rd, $var     -> placeholder imm16 = cave_offset, Core writes high 16 bits of (variables_start + cave_offset)
		8 - MOVW Rd, _code()  -> placeholder imm16 = cave_offset, Core writes low 16 bits of (codeCave_start + cave_offset)
		9 - MOVT Rd, _code()  -> placeholder imm16 = cave_offset, Core writes high 16 bits of (codeCave_start + cave_offset)
		    (both halves keep the whole offset, so Core can compute the carry into the high half)

		Offsets inside regions (A64 equivalent of `add x0, x0, $var` / `ldr s0, [x0, $var]`) are encoded directly:
		  ADD/SUB Rd, Rn, $var|_code(), memory offsets [Rn, $var] -> cave_offset
	*/

	uintptr_t m_pc_address = 0;
	uintptr_t m_pc_start = 0;
	uint8_t adjust_type = 0;

	constexpr uint32_t hash32(const char* str) {
		uint32_t FNV1_INIT = 0x811C9DC5;
		const uint32_t FNV1_PRIME = 0x1000193;
		for (size_t x = 0; str[x]; x++) {
			uint8_t byte = str[x];
			if ((byte - 65) < 26)
				byte += 32;
			FNV1_INIT = (FNV1_PRIME * FNV1_INIT) ^ byte;
		}
		return FNV1_INIT;
	}

	std::string toLower(std::string s) {
		for (auto& c : s) if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
		return s;
	}

	template <typename T> bool getInteger(std::string var, T* out) {
		char* end = 0;
		if (var.c_str()[0] == '#')
			var = var.substr(1, std::string::npos);
		if (var.empty()) return false;
		int64_t value = std::strtoll(var.c_str(), &end, 0);
		if (end == var.c_str() || *end != 0) {
			// Values above INT64_MAX (e.g. 0xFFFFFFFFFFFFFFFF).
			uint64_t uvalue = std::strtoull(var.c_str(), &end, 0);
			if (end == var.c_str() || *end != 0) return false;
			memcpy(&value, &uvalue, 8);
		}
		*out = (T)value;
		return true;
	}

	bool isFloatString(const std::string& s) {
		std::string v = s[0] == '#' ? s.substr(1) : s;
		if (v.size() > 1 && (v[0] == '0' || v[1] == '0') && (v.find('x') != std::string::npos || v.find('X') != std::string::npos))
			return false;
		return v.find('.') != std::string::npos || v.find('e') != std::string::npos || v.find('E') != std::string::npos ||
		       !toLower(v).compare("inf") || !toLower(v).compare("-inf") || !toLower(v).compare("nan");
	}

	bool getFloat(std::string var, double* out) {
		char* end = 0;
		if (var.c_str()[0] == '#')
			var = var.substr(1, std::string::npos);
		double value = std::strtod(var.c_str(), &end);
		if (end == var.c_str() || *end != 0) return false;
		*out = value;
		return true;
	}

	// Registers
	// ---------

	bool getGenRegister(std::string name, Gp* out) {
		name = toLower(name);
		switch(hash32(name.c_str())) {
			case hash32("sp"): *out = sp; return true;
			case hash32("lr"): *out = lr; return true;
			case hash32("pc"): *out = pc; return true;
			case hash32("ip"): *out = ip; return true;
			case hash32("fp"): *out = fp; return true;
			case hash32("sb"): *out = sb; return true;
			case hash32("apsr_nzcv"): *out = apsr_nzcv; return true;
		}
		if (name.size() < 2 || name[0] != 'r') return false;
		uint32_t id = 0;
		if (!getInteger(name.substr(1), &id) || id > 15) return false;
		*out = Gp::make_r32(id);
		return true;
	}

	// s0-s31, d0-d31, q0-q15, d0[1] (scalar), d0[] (all lanes).
	bool getFpRegister(std::string name, Vec* out) {
		name = toLower(name);
		if (name.size() < 2) return false;
		char type = name[0];
		if (type != 's' && type != 'd' && type != 'q') return false;
		std::string rest = name.substr(1);
		std::string index;
		size_t bracket = rest.find('[');
		if (bracket != std::string::npos) {
			if (type != 'd' || rest.back() != ']') return false;
			index = rest.substr(bracket + 1, rest.size() - bracket - 2);
			rest = rest.substr(0, bracket);
		}
		uint32_t id = 0;
		if (!getInteger(rest, &id)) return false;
		if (id > ((type == 'q') ? 15u : 31u)) return false;
		Vec reg = (type == 's') ? Vec::make_s(id) : (type == 'd') ? Vec::make_d(id) : Vec::make_q(id);
		if (bracket != std::string::npos) {
			if (index.empty()) reg = reg.all();
			else {
				uint32_t x = 0;
				if (!getInteger(index, &x) || x > 7) return false;
				reg = reg.at(x);
			}
		}
		*out = reg;
		return true;
	}

	bool getShiftOp(const std::string& name, ShiftOp* out) {
		switch(hash32(name.c_str())) {
			case hash32("lsl"): *out = ShiftOp::kLSL; return true;
			case hash32("lsr"): *out = ShiftOp::kLSR; return true;
			case hash32("asr"): *out = ShiftOp::kASR; return true;
			case hash32("ror"): *out = ShiftOp::kROR; return true;
		}
		return false;
	}

	bool getFpSysReg(const std::string& name, uint32_t* out) {
		switch(hash32(name.c_str())) {
			case hash32("fpsid"): *out = uint32_t(FpSysReg::kFPSID); return true;
			case hash32("fpscr"): *out = uint32_t(FpSysReg::kFPSCR); return true;
			case hash32("mvfr2"): *out = uint32_t(FpSysReg::kMVFR2); return true;
			case hash32("mvfr1"): *out = uint32_t(FpSysReg::kMVFR1); return true;
			case hash32("mvfr0"): *out = uint32_t(FpSysReg::kMVFR0); return true;
			case hash32("fpexc"): *out = uint32_t(FpSysReg::kFPEXC); return true;
		}
		return false;
	}

	// Mnemonics
	// ---------

	enum Kind : uint8_t {
		kGeneric = 0,
		kBranch,        // B, BL, BLX (target) / BLX, BX (register)
		kRegList,       // PUSH, POP, VPUSH, VPOP - first operand is a list
		kMovW,          // MOVW - symbols use low 16 bits
		kMovT           // MOVT - symbols use high 16 bits
	};

	struct MnemonicInfo {
		const char* name;
		uint32_t instId;
		Kind kind;
	};

	constexpr MnemonicInfo mnemonics[] = {
		{"adc", Inst::kIdAdc, kGeneric}, {"adcs", Inst::kIdAdcs, kGeneric},
		{"add", Inst::kIdAdd, kGeneric}, {"adds", Inst::kIdAdds, kGeneric},
		{"and", Inst::kIdAnd, kGeneric}, {"ands", Inst::kIdAnds, kGeneric},
		{"asr", Inst::kIdAsr, kGeneric}, {"asrs", Inst::kIdAsrs, kGeneric},
		{"b", Inst::kIdB, kBranch},
		{"bic", Inst::kIdBic, kGeneric}, {"bics", Inst::kIdBics, kGeneric},
		{"bl", Inst::kIdBl, kBranch},
		{"blx", Inst::kIdBlx, kBranch},
		{"bx", Inst::kIdBx, kBranch},
		{"cmn", Inst::kIdCmn, kGeneric},
		{"cmp", Inst::kIdCmp, kGeneric},
		{"eor", Inst::kIdEor, kGeneric}, {"eors", Inst::kIdEors, kGeneric},
		{"ldr", Inst::kIdLdr, kGeneric},
		{"ldrb", Inst::kIdLdrb, kGeneric},
		{"ldrd", Inst::kIdLdrd, kGeneric},
		{"ldrh", Inst::kIdLdrh, kGeneric},
		{"ldrsb", Inst::kIdLdrsb, kGeneric},
		{"ldrsh", Inst::kIdLdrsh, kGeneric},
		{"lsl", Inst::kIdLsl, kGeneric}, {"lsls", Inst::kIdLsls, kGeneric},
		{"lsr", Inst::kIdLsr, kGeneric}, {"lsrs", Inst::kIdLsrs, kGeneric},
		{"mla", Inst::kIdMla, kGeneric}, {"mlas", Inst::kIdMlas, kGeneric},
		{"mls", Inst::kIdMls, kGeneric},
		{"mov", Inst::kIdMov, kGeneric}, {"movs", Inst::kIdMovs, kGeneric},
		{"movt", Inst::kIdMovt, kMovT},
		{"movw", Inst::kIdMovw, kMovW},
		{"mul", Inst::kIdMul, kGeneric}, {"muls", Inst::kIdMuls, kGeneric},
		{"mvn", Inst::kIdMvn, kGeneric}, {"mvns", Inst::kIdMvns, kGeneric},
		{"nop", Inst::kIdNop, kGeneric},
		{"orr", Inst::kIdOrr, kGeneric}, {"orrs", Inst::kIdOrrs, kGeneric},
		{"pop", Inst::kIdPop, kRegList},
		{"push", Inst::kIdPush, kRegList},
		{"ror", Inst::kIdRor, kGeneric}, {"rors", Inst::kIdRors, kGeneric},
		{"rsb", Inst::kIdRsb, kGeneric}, {"rsbs", Inst::kIdRsbs, kGeneric},
		{"sbc", Inst::kIdSbc, kGeneric}, {"sbcs", Inst::kIdSbcs, kGeneric},
		{"sdiv", Inst::kIdSdiv, kGeneric},
		{"smull", Inst::kIdSmull, kGeneric},
		{"str", Inst::kIdStr, kGeneric},
		{"strb", Inst::kIdStrb, kGeneric},
		{"strd", Inst::kIdStrd, kGeneric},
		{"strh", Inst::kIdStrh, kGeneric},
		{"sub", Inst::kIdSub, kGeneric}, {"subs", Inst::kIdSubs, kGeneric},
		{"svc", Inst::kIdSvc, kGeneric},
		{"sxtb", Inst::kIdSxtb, kGeneric},
		{"sxth", Inst::kIdSxth, kGeneric},
		{"teq", Inst::kIdTeq, kGeneric},
		{"tst", Inst::kIdTst, kGeneric},
		{"udiv", Inst::kIdUdiv, kGeneric},
		{"umull", Inst::kIdUmull, kGeneric},
		{"uxtb", Inst::kIdUxtb, kGeneric},
		{"uxth", Inst::kIdUxth, kGeneric},
		{"vabs", Inst::kIdVabs, kGeneric},
		{"vadd", Inst::kIdVadd, kGeneric},
		{"vcmp", Inst::kIdVcmp, kGeneric},
		{"vcmpe", Inst::kIdVcmpe, kGeneric},
		{"vcvt", Inst::kIdVcvt, kGeneric},
		{"vcvtr", Inst::kIdVcvtr, kGeneric},
		{"vdiv", Inst::kIdVdiv, kGeneric},
		{"vfma", Inst::kIdVfma, kGeneric},
		{"vfms", Inst::kIdVfms, kGeneric},
		{"vldr", Inst::kIdVldr, kGeneric},
		{"vmaxnm", Inst::kIdVmaxnm, kGeneric},
		{"vminnm", Inst::kIdVminnm, kGeneric},
		{"vmla", Inst::kIdVmla, kGeneric},
		{"vmls", Inst::kIdVmls, kGeneric},
		{"vmov", Inst::kIdVmov, kGeneric},
		{"vmrs", Inst::kIdVmrs, kGeneric},
		{"vmsr", Inst::kIdVmsr, kGeneric},
		{"vmul", Inst::kIdVmul, kGeneric},
		{"vneg", Inst::kIdVneg, kGeneric},
		{"vnmul", Inst::kIdVnmul, kGeneric},
		{"vpop", Inst::kIdVpop, kRegList},
		{"vpush", Inst::kIdVpush, kRegList},
		{"vsqrt", Inst::kIdVsqrt, kGeneric},
		{"vstr", Inst::kIdVstr, kGeneric},
		{"vsub", Inst::kIdVsub, kGeneric},
	};

	template <typename T, size_t N> constexpr bool has_duplicates(const T (&array)[N]) {
		for (size_t i = 1; i < N; i++)
			for (size_t j = 0; j < i; j++)
				if (hash32(array[i].name) == hash32(array[j].name))
					return true;
		return false;
	}

	static_assert(!has_duplicates(mnemonics), "Detected repeated hash!");

	constexpr const char* condNames[] = {"eq", "ne", "cs", "hs", "cc", "lo", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "al"};
	constexpr CondCode condCodes[] = {
		CondCode::kEQ, CondCode::kNE, CondCode::kCS, CondCode::kHS, CondCode::kCC, CondCode::kLO, CondCode::kMI, CondCode::kPL,
		CondCode::kVS, CondCode::kVC, CondCode::kHI, CondCode::kLS, CondCode::kGE, CondCode::kLT, CondCode::kGT, CondCode::kLE, CondCode::kAL
	};

	bool getCondition(const std::string& s, CondCode* out) {
		for (size_t i = 0; i < std::size(condNames); i++) {
			if (!s.compare(condNames[i])) {*out = condCodes[i]; return true;}
		}
		return false;
	}

	bool getDataType(const std::string& s, DataType* out) {
		switch(hash32(s.c_str())) {
			case hash32("f16"): *out = DataType::kF16; return true;
			case hash32("f32"): *out = DataType::kF32; return true;
			case hash32("f64"): *out = DataType::kF64; return true;
			case hash32("s8"): *out = DataType::kS8; return true;
			case hash32("s16"): *out = DataType::kS16; return true;
			case hash32("s32"): *out = DataType::kS32; return true;
			case hash32("u8"): *out = DataType::kU8; return true;
			case hash32("u16"): *out = DataType::kU16; return true;
			case hash32("u32"): *out = DataType::kU32; return true;
			case hash32("i8"): case hash32("8"): *out = DataType::kI8; return true;
			case hash32("i16"): case hash32("16"): *out = DataType::kI16; return true;
			case hash32("i32"): case hash32("32"): *out = DataType::kI32; return true;
		}
		return false;
	}

	struct ParsedMnemonic {
		const MnemonicInfo* info = nullptr;
		CondCode cc = CondCode::kAL;
		DataType dt = DataType::kNone;
		DataType dt2 = DataType::kNone;
	};

	// Accepts `add`, `addeq`, `adds`, `addseq`, `addeqs`, `b.eq`, `vadd.f32`, `vaddeq.f32`, `vcvt.s32.f32`, ...
	bool parseMnemonic(std::string text, ParsedMnemonic* out) {
		text = toLower(text);
		std::vector<std::string> parts;
		size_t start = 0;
		while (true) {
			size_t dot = text.find('.', start);
			parts.push_back(text.substr(start, dot == std::string::npos ? std::string::npos : dot - start));
			if (dot == std::string::npos) break;
			start = dot + 1;
		}

		ParsedMnemonic result;
		bool hasCond = false;
		int dtCount = 0;
		for (size_t i = 1; i < parts.size(); i++) {
			CondCode cc;
			DataType dt;
			if (!hasCond && getCondition(parts[i], &cc)) {result.cc = cc; hasCond = true;}
			else if (dtCount < 2 && getDataType(parts[i], &dt)) {
				if (dtCount++ == 0) result.dt = dt;
				else result.dt2 = dt;
			}
			else return false;
		}

		const std::string& base = parts[0];
		// Exact match has priority, then try `base + cond` and `base + cond + s` (pre-UAL) splits.
		for (const auto& m : mnemonics) {
			if (!base.compare(m.name)) {
				result.info = &m;
				*out = result;
				return true;
			}
		}
		if (hasCond) return false;
		for (const auto& m : mnemonics) {
			size_t len = strlen(m.name);
			if (base.size() <= len || base.compare(0, len, m.name)) continue;
			std::string rest = base.substr(len);
			CondCode cc;
			if (rest.size() == 2 && getCondition(rest, &cc)) {
				result.info = &m;
				result.cc = cc;
				*out = result;
				return true;
			}
			if (rest.size() == 3 && rest[2] == 's' && getCondition(rest.substr(0, 2), &cc)) {
				std::string sName = std::string(m.name) + "s";
				for (const auto& ms : mnemonics) {
					if (!sName.compare(ms.name)) {
						result.info = &ms;
						result.cc = cc;
						*out = result;
						return true;
					}
				}
			}
		}
		return false;
	}

	// Symbols
	// -------

	enum SymbolType : uint8_t {
		kSymbolNone = 0,
		kSymbolConst,
		kSymbolVariable,
		kSymbolCode
	};

	// Resolves `$const`, `$variable`, `_code()`. Returns error code or 0.
	Result resolveSymbol(const std::string& s, SymbolType* type, uint64_t* value) {
		if (s.c_str()[0] == '$') {
			uint32_t hash = hash32(&s.c_str()[1]);
			auto var = LOCK::declared_variables.find(hash);
			if (var != LOCK::declared_variables.end()) {
				*type = kSymbolVariable;
				*value = var->second.cave_offset;
				return 0;
			}
			auto cst = LOCK::declared_consts.find(hash);
			if (cst != LOCK::declared_consts.end()) {
				*type = kSymbolConst;
				*value = cst->second;
				return 0;
			}
			return 0xFF3004;
		}
		if (s.c_str()[0] == '_') {
			uint32_t hash = hash32(s.c_str());
			auto it = std::find_if(LOCK::declared_codes.begin(), LOCK::declared_codes.end(), [hash](auto& pair){return pair.first == hash;});
			if (it == LOCK::declared_codes.end()) return 0xFF3005;
			*type = kSymbolCode;
			*value = it->second.cave_offset;
			return 0;
		}
		*type = kSymbolNone;
		return 0;
	}

	// Branches
	// --------

	// B/BL/BLX target: `_code()`, `:goto`, `+rel`, `-rel` or absolute offset in the current region.
	template <typename T> Result BRANCH(T entry_impl, const ParsedMnemonic& pm, const std::unordered_map<std::string, uint32_t>& gotos, Assembler& a) {
		if (entry_impl.num_children() != 2)
			return 0xFF3010;
		std::string inst;
		entry_impl[1] >> inst;
		uint32_t id = pm.info->instId;
		InstId instId = BaseInst::composeARMInstId(id, pm.cc);

		// BX Rm, BLX Rm.
		Gp reg;
		if (getGenRegister(inst, &reg)) {
			if (id != Inst::kIdBx && id != Inst::kIdBlx) return 0xFF3011;
			Error err = a.emit(instId, reg);
			return err ? (0xFF3200 | err) : 0;
		}
		if (id == Inst::kIdBx) return 0xFF3011;

		int64_t address = 0;
		if (inst.c_str()[0] == '_') {
			if (!inst.compare("_convertTickToTimeSpan()") || !inst.compare("_setUserInactivityDetectionTimeExtended()"))
				return 0xFF3012;
			if (id == Inst::kIdBlx) return 0xFF3015; // BLX <imm> switches to Thumb state, use BL.
			SymbolType type;
			uint64_t value = 0;
			Result rc = resolveSymbol(inst, &type, &value);
			if (R_FAILED(rc)) return rc;
			// imm24 = -(cave_offset + 0x100) / 4 (PC reads as current instruction + 8).
			address = (int64_t)m_pc_address + 8 - (int64_t)(value + 0x100);
			adjust_type = 1;
		}
		else if (inst.c_str()[0] == ':') {
			auto it = gotos.find(inst);
			if (it == gotos.end()) return 0xFF3013;
			address = it->second;
		}
		else {
			bool relative = (inst.c_str()[0] == '+' || inst.c_str()[0] == '-');
			if (!getInteger(inst, &address)) return 0xFF3014;
			if (relative) address += m_pc_address;
			else adjust_type = 5; // Relative inside main, relocated only when it's used in a code cave.
		}
		Error err = a.emit(instId, Imm(address));
		return err ? (0xFF3200 | err) : 0;
	}

	// Operands
	// --------

	struct OperandBuilder {
		const ParsedMnemonic& pm;
		std::vector<Operand> ops;
		SymbolType lastSymbol = kSymbolNone;

		explicit OperandBuilder(const ParsedMnemonic& p) : pm(p) {}

		// Converts a symbol value according to the instruction (MOVW/MOVT use low/high halves of constants,
		// variables and codes keep the whole cave offset as a placeholder that is relocated by Core).
		Result symbolValue(SymbolType type, uint64_t value, int64_t* out) const {
			if (pm.info->kind == kMovW || pm.info->kind == kMovT) {
				if (type == kSymbolVariable || type == kSymbolCode) {
					if (value > 0xFFFF) return 0xFF3007;
					*out = (int64_t)value;
				}
				else if (pm.info->kind == kMovW) *out = (int64_t)(value & 0xFFFF);
				else *out = (int64_t)((value >> 16) & 0xFFFF);
				return 0;
			}
			*out = (int64_t)value;
			return 0;
		}

		Result immediate(const std::string& s, Imm* out) {
			SymbolType type;
			uint64_t value = 0;
			Result rc = resolveSymbol(s, &type, &value);
			if (R_FAILED(rc)) return rc;
			if (type != kSymbolNone) {
				lastSymbol = type;
				int64_t v = 0;
				rc = symbolValue(type, value, &v);
				if (R_FAILED(rc)) return rc;
				*out = Imm(v);
				return 0;
			}
			if (isFloatString(s)) {
				double d = 0;
				if (!getFloat(s, &d)) return 0xFF3006;
				*out = Imm(d);
				return 0;
			}
			int64_t v = 0;
			if (!getInteger(s, &v)) return 0xFF3006;
			*out = Imm(v);
			return 0;
		}

		// Memory operand: [Rn], [Rn, imm|$var], [Rn, Rm], [Rn, -Rm], [Rn, Rm, shift, amount].
		template <typename T> Result memory(T node, Mem* out) {
			size_t n = node.num_children();
			if (n == 0 || n > 4) return 0xFF3020;
			std::string s;
			node[0] >> s;
			Gp base;
			if (!getGenRegister(s, &base)) return 0xFF3021;
			if (n == 1) {*out = ptr(base); return 0;}
			node[1] >> s;
			std::string idxName = s;
			bool neg = false;
			if (idxName[0] == '-' || idxName[0] == '+') {neg = idxName[0] == '-'; idxName = idxName.substr(1);}
			Gp index;
			if (getGenRegister(idxName, &index)) {
				Mem m = ptr(base, index);
				if (n == 4) {
					std::string shName, shAmount;
					node[2] >> shName;
					node[3] >> shAmount;
					ShiftOp op;
					uint32_t amount = 0;
					if (!getShiftOp(toLower(shName), &op) || !getInteger(shAmount, &amount)) return 0xFF3022;
					m = ptr(base, index, Shift(op, amount));
				}
				else if (n == 3) {
					node[2] >> s;
					if (toLower(s).compare("rrx")) return 0xFF3022;
					m = ptr(base, index, rrx());
				}
				*out = neg ? m.neg() : m;
				return 0;
			}
			if (n != 2) return 0xFF3023;
			Imm imm;
			Result rc = immediate(s, &imm);
			if (R_FAILED(rc)) return rc;
			*out = ptr(base, (int32_t)imm.value());
			return 0;
		}

		// Register list: [r4, r5, lr], ["r4-r11", lr], [d8, d9], ["d8-d15"].
		template <typename T> Result regList(T node) {
			uint32_t mask = 0;
			int type = -1; // 0 = gp, 1 = s, 2 = d, 3 = q
			for (size_t i = 0; i < node.num_children(); i++) {
				std::string s;
				node[i] >> s;
				std::string first = s, last = s;
				size_t dash = s.find('-');
				if (dash != std::string::npos) {first = s.substr(0, dash); last = s.substr(dash + 1);}
				Gp g1, g2;
				Vec v1, v2;
				uint32_t lo, hi;
				int t;
				if (getGenRegister(first, &g1) && getGenRegister(last, &g2)) {t = 0; lo = g1.id(); hi = g2.id();}
				else if (getFpRegister(first, &v1) && getFpRegister(last, &v2) && v1.regType() == v2.regType() && v1.isPlain() && v2.isPlain()) {
					t = v1.isS() ? 1 : v1.isD() ? 2 : 3;
					lo = v1.id(); hi = v2.id();
				}
				else return 0xFF3030;
				if (type != -1 && type != t) return 0xFF3031;
				if (hi < lo) return 0xFF3032;
				type = t;
				for (uint32_t r = lo; r <= hi; r++) mask |= 1u << r;
			}
			if (type == -1) return 0xFF3030;
			if (type == 0) ops.push_back(GpList(mask));
			else {
				RegType rt = type == 1 ? RegType::kVec32 : type == 2 ? RegType::kVec64 : RegType::kVec128;
				uint32_t lo = 0;
				while (!(mask & (1u << lo))) lo++;
				uint32_t count = 0;
				while (lo + count < 32 && (mask & (1u << (lo + count)))) count++;
				if ((mask >> lo) != ((count == 32) ? 0xFFFFFFFFu : ((1u << count) - 1))) return 0xFF3033;
				Vec first = rt == RegType::kVec32 ? Vec::make_s(lo) : rt == RegType::kVec64 ? Vec::make_d(lo) : Vec::make_q(lo);
				ops.push_back(VecList(first, count));
			}
			return 0;
		}

		template <typename T> Result build(T entry_impl) {
			size_t count = entry_impl.num_children();
			for (size_t i = 1; i < count; i++) {
				auto node = entry_impl[i];
				if (node.is_seq()) {
					if (pm.info->kind == kRegList && ops.empty()) {
						Result rc = regList(node);
						if (R_FAILED(rc)) return rc;
						continue;
					}
					Mem m;
					Result rc = memory(node, &m);
					if (R_FAILED(rc)) return rc;
					ops.push_back(m);
					continue;
				}

				std::string s;
				node >> s;
				if (s.empty()) return 0xFF3100 + i;
				std::string ls = toLower(s);

				// Pre-index / write-back `!` (must be quoted in YAML).
				if (!s.compare("!")) {
					if (ops.empty()) return 0xFF3100 + i;
					Operand& prev = ops.back();
					if (prev.isMem()) prev.as<Mem>().makePreIndex();
					else if (prev.isReg(RegType::kGp32)) prev = ptr_pre(prev.as<Gp>());
					else return 0xFF3100 + i;
					continue;
				}

				// Post-index: [Rn], imm | [Rn], Rm {, shift, amount} | [Rn], -Rm
				if (!ops.empty() && ops.back().isMem() && ops.back().as<Mem>().isFixedOffset() &&
				    !ops.back().as<Mem>().hasIndex() && ops.back().as<Mem>().offsetLo32() == 0 &&
				    !ops.back().as<Mem>().hasBaseLabel()) {
					Gp base = Gp::make_r32(ops.back().as<Mem>().baseId());
					std::string idxName = s;
					bool neg = false;
					if (idxName[0] == '-' || idxName[0] == '+') {neg = idxName[0] == '-'; idxName = idxName.substr(1);}
					Gp index;
					if (getGenRegister(idxName, &index)) {
						Mem m = ptr_post(base, index);
						if (i + 2 < count && !entry_impl[i + 1].is_seq()) {
							std::string shName, shAmount;
							entry_impl[i + 1] >> shName;
							entry_impl[i + 2] >> shAmount;
							ShiftOp op;
							uint32_t amount = 0;
							if (getShiftOp(toLower(shName), &op)) {
								if (!getInteger(shAmount, &amount)) return 0xFF3100 + i + 2;
								m = ptr_post(base, index, Shift(op, amount));
								i += 2;
							}
						}
						ops.back() = neg ? m.neg() : m;
						continue;
					}
					Imm imm;
					if (R_SUCCEEDED(immediate(s, &imm)) && imm.isInt()) {
						ops.back() = ptr_post(base, (int32_t)imm.value());
						continue;
					}
				}

				// Register with write-back used by LDM-like syntax `r0!`.
				Gp gp;
				if (s.back() == '!' && getGenRegister(s.substr(0, s.size() - 1), &gp)) {
					ops.push_back(ptr_pre(gp));
					continue;
				}

				if (getGenRegister(s, &gp)) {
					ops.push_back(gp);
					continue;
				}

				Vec vec;
				if (getFpRegister(s, &vec)) {
					ops.push_back(vec);
					continue;
				}

				// Shift: `lsl, 3` / `lsl, r3` / `rrx` / `ror, 8` (extend rotation).
				ShiftOp sop;
				if (getShiftOp(ls, &sop)) {
					if (i + 1 >= count || entry_impl[i + 1].is_seq()) return 0xFF3100 + i;
					std::string amount;
					entry_impl[++i] >> amount;
					Gp rs;
					if (getGenRegister(amount, &rs)) ops.push_back(rs.withShiftOp(sop));
					else {
						uint32_t v = 0;
						if (!getInteger(amount, &v)) return 0xFF3100 + i;
						ops.push_back(Imm(Shift(sop, v)));
					}
					continue;
				}
				if (!ls.compare("rrx")) {
					ops.push_back(Imm(rrx()));
					continue;
				}

				uint32_t sysReg = 0;
				if (getFpSysReg(ls, &sysReg)) {
					ops.push_back(Imm(sysReg));
					continue;
				}

				Imm imm;
				Result rc = immediate(s, &imm);
				if (R_FAILED(rc)) return (rc == 0xFF3006) ? (0xFF3100 + i) : rc;
				ops.push_back(imm);
			}
			if (ops.size() > Globals::kMaxOpCount) return 0xFF3001;
			return 0;
		}
	};

	template <typename T> Result GENERIC(T entry_impl, const ParsedMnemonic& pm, Assembler& a) {
		OperandBuilder b(pm);
		Result rc = b.build(entry_impl);
		if (R_FAILED(rc)) return rc;

		uint32_t id = pm.info->instId;

		// `ADD Rd, PC, <target>` is the AArch32 counterpart of ADRP.
		if ((id == Inst::kIdAdd || id == Inst::kIdSub) && b.ops.size() == 3 &&
		    b.ops[1].isReg(RegType::kGp32) && b.ops[1].as<Gp>().isPc() && b.ops[2].isImm()) {
			switch (b.lastSymbol) {
				case kSymbolCode: adjust_type = 2; b.ops[2] = Imm(0); break;
				case kSymbolVariable: adjust_type = 3; b.ops[2] = Imm(0); break;
				case kSymbolNone: adjust_type = 4; break;
				default: break;
			}
		}

		// MOVW/MOVT of a variable or code address - relocated by Core.
		if (pm.info->kind == kMovW || pm.info->kind == kMovT) {
			bool isMovt = pm.info->kind == kMovT;
			if (b.lastSymbol == kSymbolVariable) adjust_type = isMovt ? 7 : 6;
			else if (b.lastSymbol == kSymbolCode) adjust_type = isMovt ? 9 : 8;
		}

		InstId instId = BaseInst::composeARMInstId(id, pm.dt, pm.dt2, pm.cc);
		Error err = a.emitOpArray(instId, b.ops.data(), b.ops.size());
		return err ? (0xFF3200 | err) : 0;
	}

} // {anonymous}

	Result processArm32(c4::yml::NodeRef entry, uint32_t* out, uint8_t* adjust_type_arg, uintptr_t pc_address, uintptr_t start_address, const std::unordered_map<std::string, uint32_t> gotos) {
		std::string inst;
		entry[0] >> inst;

		ParsedMnemonic pm;
		if (!parseMnemonic(inst, &pm)) return 0xFFFFFE;

		Environment env(Arch::kARM);
		CodeHolder code;
		code.init(env, pc_address);
		Assembler a(&code);

		m_pc_address = pc_address;
		m_pc_start = start_address;
		adjust_type = 0;

		Result rc = 0;
		switch (pm.info->kind) {
			case kBranch: {rc = BRANCH(entry, pm, gotos, a); break;}
			default: {rc = GENERIC(entry, pm, a); break;}
		}
		if (R_FAILED(rc)) {
			return rc;
		}
		size_t codeSize = code.codeSize();
		if (codeSize != 4) {
			return 0xFFFFFD;
		}
		code.copyFlattenedData(out, 4);
		if (adjust_type_arg) *adjust_type_arg = adjust_type;
		return 0;
	}

}
