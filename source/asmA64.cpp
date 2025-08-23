#include <switch.h>
#include "asmjit/a64.h"
#include <string>
#include <array>
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
	extern std::unordered_map<uint32_t, declare_code> declared_codes;
}

namespace ASM {

	asmjit::Environment customEnv(asmjit::Arch::kAArch64);
	asmjit::CodeHolder code;

	#define GP_REG_ERROR asmjit::a64::Gp::make_r32(asmjit::a64::Gp::Id::kIdSp)
	#define FP_REG_ERROR asmjit::a64::Vec::make_v128(31)
	#define COND_ERROR (asmjit::a64::CondCode)0xFF

	uintptr_t m_pc_address = 0;
	uintptr_t m_pc_start = 0;
	uint8_t adjust_type = 0; //1 - B/BL Codes, 2 - ADRP Variables, 3 - ADRP Codes

	constexpr uint32_t hash32(const char* str) {
		uint32_t FNV1_INIT = 0x811C9DC5;
		uint32_t FNV1_PRIME = 0x1000193;
		for (size_t x = 0; x < strlen(str); x++) {
			uint8_t byte = str[x];
			if ((byte - 65) < 26)
				byte += 32;
			FNV1_INIT = (FNV1_PRIME * FNV1_INIT) ^ byte;
		}
		return FNV1_INIT;
	}

	asmjit::a64::Gp getGenRegister(std::string register_name, bool includeW = false, bool includeSP = false, bool includeZR = false, bool includeX = true) {
		bool isW = false;
		if (includeW && (register_name.c_str()[0] == 'W' || register_name.c_str()[0] == 'w'))
			isW = true;
		else if (includeSP && (hash32("SP") == hash32(register_name.c_str()))) {
			return asmjit::a64::Gp::make_r64(asmjit::a64::Gp::Id::kIdSp);
		}
		else if (!includeX || (register_name.c_str()[0] != 'X' && register_name.c_str()[0] != 'x'))
			return GP_REG_ERROR; //Error code WSP
		uint8_t reg_value = 0;
		register_name = register_name.substr(1, std::string::npos);
		if (hash32("ZR") == hash32(register_name.c_str())) {
			if (!includeZR) return GP_REG_ERROR;
			reg_value = asmjit::a64::Gp::Id::kIdZr;
		}
		else reg_value = std::stoi(register_name);
		return isW ? asmjit::a64::Gp::make_r32(reg_value) : asmjit::a64::Gp::make_r64(reg_value);
	}

	asmjit::a64::Vec getFpRegister(std::string register_name, bool isB = true, bool isH = true, bool isS = true, bool isD = true, bool isQ = true) {
		uint8_t type;
		unsigned char register_type = register_name.c_str()[0];
		if (isB && (register_type == 'B' || register_type == 'b'))
			type = 0;
		else if (isH && (register_type == 'H' || register_type == 'h'))
			type = 1;
		else if (isS && (register_type == 'S' || register_type == 's'))
			type = 2;
		else if (isD && (register_type == 'D' || register_type == 'd'))
			type = 3;
		else if (isQ && (register_type == 'Q' || register_type == 'q'))
			type = 4;
		else return FP_REG_ERROR;
		register_name = register_name.substr(1, std::string::npos);
		uint8_t reg_value = std::stoi(register_name);
		switch(type) {
			case 0:
				return asmjit::a64::Vec::make_v8(reg_value);
			case 1:
				return asmjit::a64::Vec::make_v16(reg_value);
			case 2:
				return asmjit::a64::Vec::make_v32(reg_value);
			case 3:
				return asmjit::a64::Vec::make_v64(reg_value);
			case 4:
				return asmjit::a64::Vec::make_v128(reg_value);
			default:
				return FP_REG_ERROR;
		}
	}

	constexpr uint32_t hashes2[] = {
		hash32("EQ"),
		hash32("NE"),
		hash32("CS"),
		hash32("HS"),
		hash32("CC"),
		hash32("LO"),
		hash32("MI"),
		hash32("PL"),
		hash32("VS"),
		hash32("VC"),
		hash32("HI"),
		hash32("LS"),
		hash32("GE"),
		hash32("LT"),
		hash32("GT"),
		hash32("LE"),
		hash32("AL"),
		hash32("NV")
	};

	asmjit::a64::CondCode getConditionValue(std::string cond) {
		switch(hash32(cond.c_str())) {
			case hash32("EQ"): return asmjit::a64::CondCode::kEQ;
			case hash32("NE"): return asmjit::a64::CondCode::kNE;
			case hash32("CS"): return asmjit::a64::CondCode::kCS;
			case hash32("HS"): return asmjit::a64::CondCode::kHS;
			case hash32("CC"): return asmjit::a64::CondCode::kCC;
			case hash32("LO"): return asmjit::a64::CondCode::kLO;
			case hash32("MI"): return asmjit::a64::CondCode::kMI;
			case hash32("PL"): return asmjit::a64::CondCode::kPL;
			case hash32("VS"): return asmjit::a64::CondCode::kVS;
			case hash32("VC"): return asmjit::a64::CondCode::kVC;
			case hash32("HI"): return asmjit::a64::CondCode::kHI;
			case hash32("LS"): return asmjit::a64::CondCode::kLS;
			case hash32("GE"): return asmjit::a64::CondCode::kGE;
			case hash32("LT"): return asmjit::a64::CondCode::kLT;
			case hash32("GT"): return asmjit::a64::CondCode::kGT;
			case hash32("LE"): return asmjit::a64::CondCode::kLE;
			case hash32("AL"): return asmjit::a64::CondCode::kAL;
			case hash32("NV"): return asmjit::a64::CondCode::kNA;
		}
		return COND_ERROR;
	}

	template <typename T> bool getInteger(std::string var, T* out) {
		char* end = 0;
		if (var.c_str()[0] == '#')
			var = var.substr(1, std::string::npos);
		intptr_t value = 0;
		if (sizeof(intptr_t) == 8) value = std::strtoll(var.c_str(), &end, 0);
		else value = std::strtol(var.c_str(), &end, 0);
		if (end == var.c_str()) return false;
		*out = (T)value;
		return true;
	}

	template <typename T> bool getFloat(std::string var, T* out) {
		char* end = 0;
		if (var.c_str()[0] == '#')
			var = var.substr(1, std::string::npos);
		double value = std::strtod(var.c_str(), &end);
		if (end == var.c_str()) return false;
		*out = (T)value;
		return true;
	}

	template <typename T> Result ADRP(T entry_impl) {
		if (entry_impl.num_children() != 3)
			return 0xFF0000;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		int64_t address = 0;
		std::string var;
		entry_impl[2] >> var;
		if (var.c_str()[0] == '_') {
			if (LOCK::declared_codes.find(hash32(var.c_str())) != LOCK::declared_codes.end()) {
				adjust_type = 2;
				address = (m_pc_address & ~0xFFF) - 0x10000000;
			} 
			else return 0xFF0003;
		}
		else if (var.c_str()[0] == '$') {
			if (LOCK::declared_variables.find(hash32(&var.c_str()[1])) != LOCK::declared_variables.end()) {
				adjust_type = 3;
				address = (m_pc_address & ~0xFFF) - 0xFFFFF000;
			}
			else return 0xFF0004;
		}
		else {
			bool passed = getInteger(var, &address);
			if (!passed) return 0xFF0002;
		}
		asmjit::a64::Gp reg = getGenRegister(inst);
		if (reg == GP_REG_ERROR) return 0xFF0001;
		a.adrp(reg, address);
		return 0;
	}

	template <typename T> Result ADD(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 4)
			return 0xFF0010;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (type == 0 || type == 2) {
			asmjit::a64::Gp regs[3];
			for (size_t i = 1; i < 3; i++) {
				entry_impl[i] >> inst;
				regs[i-1] = getGenRegister(inst, true, true, true);
				if (regs[i-1] == GP_REG_ERROR) return 0xFF0010 + i;
			}
			entry_impl[3] >> inst;
			regs[2] = getGenRegister(inst, true, true, true);
			if (regs[2] == GP_REG_ERROR) {
				int64_t value = 0;
				if (inst.c_str()[0] == '_') {
					if (type != 0) return 0xFF0015;
					if (LOCK::declared_codes.find(hash32(inst.c_str())) != LOCK::declared_codes.end()) {
						value = LOCK::declared_codes[hash32(inst.c_str())].cave_offset;
					} 
					else return 0xFF0013;
				}
				else if (inst.c_str()[0] == '$') {
					if (type != 0) return 0xFF0016;
					if (LOCK::declared_variables.find(hash32(&inst.c_str()[1])) != LOCK::declared_variables.end()) {
						value = LOCK::declared_variables[hash32(&inst.c_str()[1])].cave_offset;
					}
					else return 0xFF0014;
				}
				else {
					bool passed = getInteger(inst, &value);
					if (!passed) return 0xFF0013;
				}
				if (type == 0) a.add(regs[0], regs[1], value);
				else if (type == 2) a.sub(regs[0], regs[1], value);
			}
			else {
				if (type == 0) a.add(regs[0], regs[1], regs[2]);
				else if (type == 2) a.sub(regs[0], regs[1], regs[2]);
			}
		}
		else if (type == 1 || type == 3) {
			asmjit::a64::Vec regs[3];
			for (size_t i = 1; i < 4; i++) {
				entry_impl[i] >> inst;
				regs[i-1] = getFpRegister(inst, false, true, true, true, false);
				if (regs[i-1] == FP_REG_ERROR) return 0xFF0010 + i;
			}
			if (type == 1) a.fadd(regs[0], regs[1], regs[2]);
			else if (type == 3) a.fsub(regs[0], regs[1], regs[2]);
		}
		return 0;
	}


	template <typename T> Result LDR (T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (entry_impl.num_children() < 3 || entry_impl.num_children() > 4)
			return 0xFF0020;
		if (!entry_impl[2].is_seq()) return 0xFF0021;
		if (entry_impl[2].num_children() == 0 || entry_impl[2].num_children() > 2) return 0xFF0022;

		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) {
			auto regfp = getFpRegister(inst);
			if (regfp == FP_REG_ERROR) return 0xFF0023;
			entry_impl[2][0] >> inst;
			auto reg1 = getGenRegister(inst, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0024;
			if (entry_impl[2].num_children() == 2) {
				entry_impl[2][1] >> inst;
				auto reg2 = getGenRegister(inst, false, false, true);
				if (reg2 == GP_REG_ERROR) {
					int32_t value = 0;
					if (inst.c_str()[0] == '$') {
						if (type != 0) return 0xFF0025;
						if (LOCK::declared_variables.find(hash32(&inst.c_str()[1])) != LOCK::declared_variables.end()) {
							value = LOCK::declared_variables[hash32(&inst.c_str()[1])].cave_offset;
						}
						else return 0xFF0026;
					}
					else {
						bool passed = getInteger(inst, &value);
						if (!passed) return 0xFF0027;
					}
					switch(type) {
						case 0:
							if (entry_impl.num_children() == 3) a.ldr(regfp, asmjit::a64::Mem(reg1, value));
							else a.ldr(regfp, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 3:
							a.ldur(regfp, asmjit::a64::Mem(reg1, value));
							break;
					}
				}
				else {
					switch(type) {
						case 0:
							a.ldr(regfp, asmjit::a64::Mem(reg1, reg2));
							break;
					}
				}
			}
			else {
				switch(type) {
					case 0:
						if (entry_impl.num_children() == 3) a.ldr(regfp, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (!inst.compare("!")) a.ldr(regfp, asmjit::a64::ptr_pre(reg1));
							else {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0058;
								a.ldr(regfp, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 3:
						a.ldur(regfp, asmjit::a64::Mem(reg1));
						break;
				}
			}
		}
		else {
			entry_impl[2][0] >> inst;
			auto reg1 = getGenRegister(inst, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0024;
			if (entry_impl[2].num_children() == 2) {
				entry_impl[2][1] >> inst;
				auto reg2 = getGenRegister(inst);
				if (reg2 == GP_REG_ERROR) {
					int32_t value = 0;
					if (inst.c_str()[0] == '$') {
						if (type != 0) return 0xFF0025;
						if (LOCK::declared_variables.find(hash32(&inst.c_str()[1])) != LOCK::declared_variables.end()) {
							value = LOCK::declared_variables[hash32(&inst.c_str()[1])].cave_offset;
						}
						else return 0xFF0026;
					}
					else {
						bool passed = getInteger(inst, &value);
						if (!passed) return 0xFF0027;
					}
					switch(type) {
						case 0:
							if (entry_impl.num_children() == 3) a.ldr(reg0, asmjit::a64::Mem(reg1, value));
							else a.ldr(reg0, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 1:
							if (entry_impl.num_children() == 3) a.ldrb(reg0, asmjit::a64::Mem(reg1, value));
							else a.ldrb(reg0, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 2:
							if (entry_impl.num_children() == 3) a.ldrh(reg0, asmjit::a64::Mem(reg1, value));
							else a.ldrh(reg0, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 3:
							a.ldur(reg0, asmjit::a64::Mem(reg1, value));
							break;
						case 4:
							a.ldurh(reg0, asmjit::a64::Mem(reg1, value));
							break;
					}
				}
				else {
					switch(type) {
						case 0:
							a.ldr(reg0, asmjit::a64::Mem(reg1, reg2));
							break;
						case 1:
							a.ldrb(reg0, asmjit::a64::Mem(reg1, reg2));
							break;
						case 2:
							a.ldrh(reg0, asmjit::a64::Mem(reg1, reg2));
							break;
					}
				}
			}
			else {
				switch(type) {
					case 0:
						if (entry_impl.num_children() == 3) a.ldr(reg0, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0028;
								a.ldr(reg0, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 1:
						if (entry_impl.num_children() == 3) a.ldrb(reg0, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0028;
								a.ldrb(reg0, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 2:
						if (entry_impl.num_children() == 3) a.ldrh(reg0, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0028;
								a.ldrh(reg0, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 3:
						a.ldur(reg0, asmjit::a64::Mem(reg1));
						break;
					case 4:
						a.ldurh(reg0, asmjit::a64::Mem(reg1));
						break;
				}
			}
		}
		return 0;
	}

	void NOP() {
		asmjit::a64::Assembler a(&code);
		a.nop();
	}

	template <typename T> Result MOV(T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (type == 1) {
			if (entry_impl.num_children() != 4)
				return 0xFF0030;
		}
		else if (entry_impl.num_children() != 3)
			return 0xFF0030;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, true, true);
		if (type == 0) {
			if (reg0 == GP_REG_ERROR) return 0xFF0031;
			entry_impl[2] >> inst;
			auto reg1 = getGenRegister(inst, true, true, true);
			if (reg1 == GP_REG_ERROR) {
				uint64_t value = 0;
				if (inst.c_str()[0] == '$') {
					value = LOCK::declared_consts[hash32(&inst.c_str()[1])] & 0xFFFF;
				}
				else {
					bool passed = getInteger(inst, &value);
					if (!passed) return 0xFF0035;
				}
				a.mov(reg0, value);
			}
			else a.mov(reg0, reg1);
		}
		else if (type == 1) {
			if (reg0 == GP_REG_ERROR) return 0xFF0032;
			uint8_t shift = 0;
			entry_impl[3] >> inst;
			bool passed = getInteger(inst, &shift);
			if (!passed) return 0xFF0037;
			if ((shift > 64) || (shift % 16 != 0)) return 0xFF0036;
			uint64_t value = 0;
			entry_impl[2] >> inst;
			if (inst.c_str()[0] == '$') {
				value = LOCK::declared_consts[hash32(&inst.c_str()[1])];
				value = (value >> shift) & 0xFFFF;
			}
			else {
				bool passed = getInteger(inst, &value);
				if (!passed) return 0xFF0035;
			}
			a.movk(reg0, value, shift);
		}
		else if (type == 2) {
			if (reg0 == GP_REG_ERROR) {
				auto regfp = getFpRegister(inst, false, true, true, true, false);
				if (regfp == FP_REG_ERROR) return 0xFF0033;
				entry_impl[2] >> inst;
				auto reg1 = getGenRegister(inst, true, false, true);
				if (reg1 == GP_REG_ERROR) {
					auto regfp2 = getFpRegister(inst, false, true, true, true, false);
					if (regfp2 == FP_REG_ERROR) {
						double value = 0.d;
						bool passed = getFloat(inst, &value);
						if (!passed) return 0xFF0035;
						a.fmov(regfp, value);
					}
					else a.fmov(regfp, regfp2);
				}
				else a.fmov(regfp, reg1);
			}
			else {
				auto reg1 = getFpRegister(inst, true, false, true);
				if (reg1 == FP_REG_ERROR) return 0xFF0034;
				a.fmov(reg0, reg1);
			}
		}
		return 0;
	}

	template <typename T> Result FMUL(T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (entry_impl.num_children() != 4)
			return 0xFF0040;
		asmjit::a64::Vec regs[3];
		for (size_t i = 1; i < 4; i++) {
			entry_impl[i] >> inst;
			regs[i-1] = getFpRegister(inst, false, true, true, true, false);
			if (regs[i-1] == FP_REG_ERROR) return 0xFF0040 + i;
		}
		if (type == 0)
			a.fmul(regs[0], regs[1], regs[2]);
		else if (type == 1)
			a.fdiv(regs[0], regs[1], regs[2]);
		return 0;
	}


	void RET() {
		asmjit::a64::Assembler a(&code);
		a.ret(asmjit::a64::x30);
	}

	template <typename T> Result STR (T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (entry_impl.num_children() < 3 || entry_impl.num_children() > 4)
			return 0xFF0050;
		if (!entry_impl[2].is_seq()) return 0xFF0051;
		if (entry_impl[2].num_children() == 0 || entry_impl[2].num_children() > 2) return 0xFF0052;

		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) {
			auto regfp = getFpRegister(inst);
			if (regfp == FP_REG_ERROR) return 0xFF0053;
			entry_impl[2][0] >> inst;
			auto reg1 = getGenRegister(inst, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0054;
			if (entry_impl[2].num_children() == 2) {
				entry_impl[2][1] >> inst;
				auto reg2 = getGenRegister(inst, false, false, true);
				if (reg2 == GP_REG_ERROR) {
					int32_t value = 0;
					if (inst.c_str()[0] == '$') {
						if (type != 0) return 0xFF0057;
						if (LOCK::declared_variables.find(hash32(&inst.c_str()[1])) != LOCK::declared_variables.end()) {
							value = LOCK::declared_variables[hash32(&inst.c_str()[1])].cave_offset;
						}
						else return 0xFF0056;
					}
					else {
						bool passed = getInteger(inst, &value);
						if (!passed) return 0xFF0055;
					}
					switch (type) {
						case 0:
							if (entry_impl.num_children() == 3) a.str(regfp, asmjit::a64::Mem(reg1, value));
							else a.str(regfp, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 3:
							a.stur(regfp, asmjit::a64::Mem(reg1, value));
					}
				}
				else {
					switch(type) {
						case 0:
							a.str(regfp, asmjit::a64::Mem(reg1, reg2));
							break;
					}
				}
			}
			else {
				switch(type) {
					case 0:
						if (entry_impl.num_children() == 3) a.str(regfp, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0058;
								a.str(regfp, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 3:
						a.stur(regfp, asmjit::a64::Mem(reg1));
				}
			}
		}
		else {
			entry_impl[2][0] >> inst;
			auto reg1 = getGenRegister(inst, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0054;
			if (entry_impl[2].num_children() == 2) {
				entry_impl[2][1] >> inst;
				auto reg2 = getGenRegister(inst);
				if (reg2 == GP_REG_ERROR) {
					int32_t value = 0;
					if (inst.c_str()[0] == '$') {
						if (type > 2) return 0xFF0059;
						if (LOCK::declared_variables.find(hash32(&inst.c_str()[1])) != LOCK::declared_variables.end()) {
							value = LOCK::declared_variables[hash32(&inst.c_str()[1])].cave_offset;
						}
						else return 0xFF0056;
					}
					else {
						bool passed = getInteger(inst, &value);
						if (!passed) return 0xFF0055;
					}
					switch(type) {
						case 0:
							if (entry_impl.num_children() == 3) a.str(reg0, asmjit::a64::Mem(reg1, value));
							else a.str(reg0, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 1:
							if (entry_impl.num_children() == 3) a.strb(reg0, asmjit::a64::Mem(reg1, value));
							else a.strb(reg0, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 2:
							if (entry_impl.num_children() == 3) a.strh(reg0, asmjit::a64::Mem(reg1, value));
							else a.strh(reg0, asmjit::a64::ptr_pre(reg1, value));
							break;
						case 3:
							a.stur(reg0, asmjit::a64::Mem(reg1, value));
							break;
						case 4:
							a.sturh(reg0, asmjit::a64::Mem(reg1, value));
							break;
					}
				}
				else {
					switch(type) {
						case 0:
							a.str(reg0, asmjit::a64::Mem(reg1, reg2));
							break;
						case 1:
							a.strb(reg0, asmjit::a64::Mem(reg1, reg2));
							break;
						case 2:
							a.strh(reg0, asmjit::a64::Mem(reg1, reg2));
							break;
					}
				}
			}
			else {
				switch(type) {
					case 0:
						if (entry_impl.num_children() == 3) a.str(reg0, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0058;
								a.str(reg0, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 1:
						if (entry_impl.num_children() == 3) a.strb(reg0, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (!inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0058;
								a.strb(reg0, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 2:
						if (entry_impl.num_children() == 3) a.strh(reg0, asmjit::a64::Mem(reg1));
						else {
							std::string inst;
							entry_impl[3] >> inst;
							if (!inst.compare("!")) {
								uint16_t value = 0;
								bool passed = getInteger(inst, &value);
								if (!passed) return 0xFF0058;
								a.strh(reg0, asmjit::a64::ptr_post(reg1, value));
							}
						}
						break;
					case 3:
						a.stur(reg0, asmjit::a64::Mem(reg1));
						break;
					case 4:
						a.sturh(reg0, asmjit::a64::Mem(reg1));
						break;
				}
			}
		}
		return 0;
	}

	template <typename T> Result B(T entry_impl, uint8_t type, uint32_t subtype, const std::unordered_map<std::string, uint32_t> gotos = {}) {
		if (entry_impl.num_children() != 2)
			return 0xFF0060;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		if (type == 0) {
			int64_t address = 0;
			if (inst.c_str()[0] == '_') {
				if (!inst.compare("_convertTickToTimeSpan()")) a.b(m_pc_address - 4);
				else if (!inst.compare("_setUserInactivityDetectionTimeExtended()")) a.b(m_pc_address - 8);
				else {
					if (LOCK::declared_codes.size() == 0) return 0xFF0065;
					uint32_t hash = hash32(inst.c_str());
					auto it = LOCK::declared_codes.find(hash);
					if (it == LOCK::declared_codes.end()) return 0xFF0066;
					a.b(m_pc_address - (it->second.cave_offset + 0x100));
				}
				adjust_type = 1;
				return 0;
			}
			bool relative = false;
			if (inst.c_str()[0] == ':') {
				auto it = gotos.find(inst);
				if (it == gotos.end()) return 0xFF0069;
				address = m_pc_start + it->second;
				relative = true;
			}
			else {
				if (inst.c_str()[0] == '+' || inst.c_str()[0] == '-') {
					relative = true;
				}
				bool passed = getInteger(inst, &address);
				if (!passed) return 0xFF0063;
				address += m_pc_address;
				if (!relative) adjust_type = 1;
			}
			switch(subtype) {
				case 0xFF: {a.b(address); break;}
				case hash32("LE"): {a.b_le(address); break;}
				case hash32("GE"): {a.b_ge(address); break;}
				case hash32("NE"): {a.b_ne(address); break;}
				case hash32("GT"): {a.b_gt(address); break;}
				case hash32("LT"): {a.b_lt(address); break;}
				case hash32("HI"): {a.b_hi(address); break;}
				case hash32("EQ"): {a.b_eq(address); break;}
				default: return 0xFF0064;
			}
		}
		else if (type == 1) {
			if (inst.c_str()[0] == '_') {
				if (!inst.compare("_convertTickToTimeSpan()")) a.bl(m_pc_address - 4);
				else if (!inst.compare("_setUserInactivityDetectionTimeExtended()")) a.bl(m_pc_address - 8);
				else {
					if (LOCK::declared_codes.size() == 0) return 0xFF0065;
					uint32_t hash = hash32(inst.c_str());
					auto it = LOCK::declared_codes.find(hash);
					if (it == LOCK::declared_codes.end()) return 0xFF0066;
					a.bl(m_pc_address - (it->second.cave_offset + 0x100));
				}
				adjust_type = 1;
				return 0;
			}
			int64_t address = 0;
			bool relative = false;
			if (inst.c_str()[0] == '+' || inst.c_str()[0] == '-') {
				relative = true;
			}
			else if (!adjust_type) adjust_type = 4;
			bool passed = getInteger(inst, &address);
			if (!passed) return 0xFF0067;
			if (relative && adjust_type == 4) address += m_pc_address;
			a.bl(address);

		}
		else if (type == 2 || type == 3) {
			auto reg = getGenRegister(inst);
			if (reg == GP_REG_ERROR) return 0xFF0068;
			if (type == 2) a.blr(reg);
			else if (type == 3) a.br(reg);
		}
		return 0;
	}

	template <typename T> Result CMP(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 3)
			return 0xFF0080;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		if (type == 0) {
			auto reg0 = getGenRegister(inst, true, true, true);
			if (reg0 == GP_REG_ERROR) return 0xFF0081;
			entry_impl[2] >> inst;
			auto reg1 = getGenRegister(inst, true, false, true);
			if (reg1 == GP_REG_ERROR) {
				uint16_t value = 0;
				entry_impl[2] >> value;
				a.cmp(reg0, value);
			}
			else a.cmp(reg0, reg1);
		}
		else if (type == 1 || type == 2) {
			auto regfp = getFpRegister(inst, false, true, true, true, false);
			if (regfp == FP_REG_ERROR) return 0xFF0082;
			entry_impl[2] >> inst;
			auto regfp2 = getFpRegister(inst, false, true, true, true, false);
			if (regfp2 == FP_REG_ERROR) {
				float value = 0;
				bool passed = getFloat(inst, &value);
				if (!passed) return 0xFF0083;
				switch(type) {
					case 1:
						a.fcmp(regfp, value);
						break;
					case 2:
						a.fcmpe(regfp, value);
						break;
				}
			}
			else {
				switch(type) {
					case 1:
						a.fcmp(regfp, regfp2);
						break;
					case 2:
						a.fcmpe(regfp, regfp2);
						break;
				}
			}
		}
		return 0;
	}

	template <typename T> Result UCVTF(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 3)
			return 0xFF0090;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto regfp = getFpRegister(inst, false, true, true, true, false);
		if (regfp == FP_REG_ERROR) return 0xFF0091;
		entry_impl[2] >> inst;
		auto regfp2 = getFpRegister(inst, false, true, true, true, false);
		if (regfp2 == FP_REG_ERROR) {
			auto reg1 = getGenRegister(inst, true, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0092;
			if (type == 0)
				a.ucvtf(regfp, reg1);
			else if (type == 1)
				a.scvtf(regfp, reg1);
		}
		else {
			if (type == 0)
				a.ucvtf(regfp, regfp2);
			else if (type == 1)
				a.scvtf(regfp, regfp2);
		}
		return 0;
	}

	template <typename T> Result FCVT(T entry_impl) {
		if (entry_impl.num_children() != 3)
			return 0xFF00A0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto regfp = getFpRegister(inst, false, true, true, true, false);
		if (regfp == FP_REG_ERROR) return 0xFF00A1;
		entry_impl[2] >> inst;
		auto regfp2 = getFpRegister(inst, false, true, true, true, false);
		if (regfp2 == FP_REG_ERROR) return 0xFF00A2;
		else a.fcvt(regfp, regfp2);
		return 0;
	}

	template <typename T> Result CBZ(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 3)
			return 0xFF00B0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) return 0xFF00B1;
		entry_impl[2] >> inst;
		bool relative = false;
		if (inst.c_str()[0] == '+' || inst.c_str()[0] == '-') {
			relative = true;
		}
		int64_t address = 0;
		bool passed = getInteger(inst, &address);
		if (!passed) return 0xFF00B2;
		if (relative) address += m_pc_address;
		if (type == 0) a.cbz(reg0, address);
		if (type == 1) a.cbnz(reg0, address);
		return 0;
	}

	template <typename T> Result TBZ(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 4)
			return 0xFF00C0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) return 0xFF00C1;
		entry_impl[2] >> inst;
		uint8_t shift = 0;
		bool passed = getInteger(inst, &shift);
		if (!passed) return 0xFF00C3;
		entry_impl[3] >> inst;
		bool relative = false;
		if (inst.c_str()[0] == '+' || inst.c_str()[0] == '-') {
			relative = true;
		}
		int64_t address = 0;
		passed = getInteger(inst, &address);
		if (!passed) return 0xFF00C2;
		if (relative) address += m_pc_address;
		if (type == 0) a.tbz(reg0, shift, address);
		if (type == 1) a.tbnz(reg0, shift, address);
		return 0;
	}


	template <typename T> Result CSEL(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 5)
			return 0xFF00C0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (type == 0) {
			asmjit::a64::Gp regs[3];
			for (size_t i = 1; i < 4; i++) {
				entry_impl[i] >> inst;
				regs[i-1] = getGenRegister(inst, true, false, true);
				if (regs[i-1] == GP_REG_ERROR) return 0xFF00C0 + i;
			}
			entry_impl[4] >> inst;
			auto cond = getConditionValue(inst);
			if (cond == COND_ERROR) return 0xFF00C4;
			a.csel(regs[0], regs[1], regs[2], cond);
		}
		else if (type == 1) {
			asmjit::a64::Vec regs[3];
			for (size_t i = 1; i < 4; i++) {
				entry_impl[i] >> inst;
				regs[i-1] = getFpRegister(inst, false, true, true, true, false);
				if (regs[i-1] == FP_REG_ERROR) return 0xFF00C4 + i;
			}
			entry_impl[4] >> inst;
			auto cond = getConditionValue(inst);
			if (cond == COND_ERROR) return 0xFF00C4;
			a.fcsel(regs[0], regs[1], regs[2], cond);
		}
		return 0;
	}

	template <typename T> Result FCVTZU(T entry_impl) {
		if (entry_impl.num_children() != 3)
			return 0xFF00D0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) return 0xFF00D1;
		entry_impl[2] >> inst;
		auto regfp = getFpRegister(inst, false, true, true, true, false);
		if (regfp == FP_REG_ERROR) return 0xFF00D2;
		else a.fcvtzu(reg0, regfp);
		return 0;
	}

	template <typename T> Result MADD(T entry_impl, uint8_t type = 0) {
		if (entry_impl.num_children() != 5)
			return 0xFF00E0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (type == 0) {
			asmjit::a64::Gp regs[4];
			for (size_t i = 1; i < 5; i++) {
				entry_impl[i] >> inst;
				regs[i-1] = getGenRegister(inst, true, false, true);
				if (regs[i-1] == GP_REG_ERROR) return 0xFF00E0 + i;
			}
			a.madd(regs[0], regs[1], regs[2], regs[3]);
		}
		else if (type == 1) {
			asmjit::a64::Vec regs[4];
			for (size_t i = 1; i < 5; i++) {
				entry_impl[i] >> inst;
				regs[i-1] = getFpRegister(inst, false, true, true, true, false);
				if (regs[i-1] == FP_REG_ERROR) return 0xFF00E5 + i;
			}
			a.fmadd(regs[0], regs[1], regs[2], regs[3]);
		}
		return 0;
	}

	template <typename T> Result FNEG(T entry_impl) {
		if (entry_impl.num_children() != 3)
			return 0xFF00F0;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto regfp = getFpRegister(inst, false, true, true, true, false);
		if (regfp == FP_REG_ERROR) return 0xFF00F1;
		entry_impl[2] >> inst;
		auto regfp2 = getFpRegister(inst, false, true, true, true, false);
		if (regfp2 == FP_REG_ERROR) return 0xFF00F2;
		else a.fneg(regfp, regfp2);
		return 0;
	}

	template <typename T> Result FSQRT(T entry_impl) {
		if (entry_impl.num_children() != 3)
			return 0xFF0100;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto regfp = getFpRegister(inst, false, true, true, true, false);
		if (regfp == FP_REG_ERROR) return 0xFF0101;
		entry_impl[2] >> inst;
		auto regfp2 = getFpRegister(inst, false, true, true, true, false);
		if (regfp2 == FP_REG_ERROR) return 0xFF0102;
		else a.fsqrt(regfp, regfp2);
		return 0;
	}

	template <typename T> Result MRS(T entry_impl) {
		if (entry_impl.num_children() != 3)
			return 0xFF0110;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) return 0xFF0111;
		entry_impl[2] >> inst;
		asmjit::a64::Predicate::SysReg::Id value;
		switch(hash32(inst.c_str())) {
			case hash32("cntpct_el0"):
				value = asmjit::a64::Predicate::SysReg::Id::kCNTPCT_EL0;
				break;
			case hash32("cntfrq_el0"):
				value = asmjit::a64::Predicate::SysReg::Id::kCNTFRQ_EL0;
				break;
			default:
				return 0xFF0112;
		}
		a.mrs(reg0, value);
		return 0;
	}

	template <typename T> Result MUL(T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (entry_impl.num_children() != 4)
			return 0xFF0120;
		asmjit::a64::Gp regs[3];
		for (size_t i = 1; i < 4; i++) {
			entry_impl[i] >> inst;
			regs[i-1] = getGenRegister(inst, true, false, true);
			if (regs[i-1] == GP_REG_ERROR) return 0xFF0120 + i;
		}
		if (type == 0)
			a.mul(regs[0], regs[1], regs[2]);
		else if (type == 1)
			a.sdiv(regs[0], regs[1], regs[2]);
		else if (type == 2)
			a.udiv(regs[0], regs[1], regs[2]);
		return 0;
	}

	template <typename T> Result LDP (T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (entry_impl.num_children() < 4 || entry_impl.num_children() > 5)
			return 0xFF0130;
		if (!entry_impl[3].is_seq()) return 0xFF0021;
		if (entry_impl[3].num_children() == 0 || entry_impl[3].num_children() > 2) return 0xFF0132;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) {
			auto regfp = getFpRegister(inst, false, false, true, true, true);
			if (regfp == FP_REG_ERROR) return 0xFF0133;
			entry_impl[2] >> inst;
			auto regfp2 = getFpRegister(inst, false, false, true, true, true);
			if (regfp2 == FP_REG_ERROR) return 0xFF0134;
			entry_impl[3][0] >> inst;
			auto reg1 = getGenRegister(inst, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0135;
			if (entry_impl[3].num_children() == 2) {
				int32_t value = 0;
				entry_impl[3][1] >> inst;
				bool passed = getInteger(inst, &value);
				if (!passed) return 0xFF0138;
				if (type == 0) {
					if (entry_impl.num_children() == 4) a.ldp(regfp, regfp2, asmjit::a64::Mem(reg1, value));
					else a.ldp(regfp, regfp2, asmjit::a64::ptr_pre(reg1, value));
				}
				else if (type == 1) {
					if (entry_impl.num_children() == 4) a.stp(regfp, regfp2, asmjit::a64::Mem(reg1, value));
					else a.stp(regfp, regfp2, asmjit::a64::ptr_pre(reg1, value));
				}
			}
			else {
				if (type == 0) {
					if (entry_impl.num_children() == 4) a.ldp(regfp, regfp2, asmjit::a64::Mem(reg1));
					else {
						std::string inst;
						entry_impl[4] >> inst;
						if (inst.compare("!")) {
							uint16_t value = 0;
							bool passed = getInteger(inst, &value);
							if (!passed) return 0xFF0139;
							a.ldp(regfp, regfp2, asmjit::a64::ptr_post(reg1, value));
						}
					}
				}
				else if (type == 1) {
					if (entry_impl.num_children() == 4) a.stp(regfp, regfp2, asmjit::a64::Mem(reg1));
					else {
						std::string inst;
						entry_impl[4] >> inst;
						if (inst.compare("!")) {
							uint16_t value = 0;
							bool passed = getInteger(inst, &value);
							if (!passed) return 0xFF013A;
							a.stp(regfp, regfp2, asmjit::a64::ptr_post(reg1, value));
						}
					}
				}
			}
		}
		else {
			entry_impl[2] >> inst;
			auto reg1 = getGenRegister(inst, true, false, true);
			if (reg1 == GP_REG_ERROR) return 0xFF0136;
			entry_impl[3][0] >> inst;
			auto reg2 = getGenRegister(inst, false, true);
			if (reg2 == GP_REG_ERROR) return 0xFF0137;
			if (entry_impl[3].num_children() == 2) {
				int32_t value = 0;
				entry_impl[3][1] >> inst;
				bool passed = getInteger(inst, &value);
				if (!passed) return 0xFF013B;
				if (type == 0) {
					if (entry_impl.num_children() == 4) a.ldp(reg0, reg1, asmjit::a64::Mem(reg2, value));
					else a.ldp(reg0, reg1, asmjit::a64::ptr_pre(reg2, value));
				}
				else if (type == 1) {
					if (entry_impl.num_children() == 4) a.stp(reg0, reg1, asmjit::a64::Mem(reg2, value));
					else a.stp(reg0, reg1, asmjit::a64::ptr_pre(reg2, value));
				}
			}
			else {
				if (type == 0) {
					if (entry_impl.num_children() == 4) a.ldp(reg0, reg1, asmjit::a64::Mem(reg2));
					else {
						std::string inst;
						entry_impl[4] >> inst;
						if (inst.compare("!")) {
							uint16_t value = 0;
							bool passed = getInteger(inst, &value);
							if (!passed) return 0xFF013D;
							a.ldp(reg0, reg1, asmjit::a64::ptr_post(reg2, value));
						}
					}
				}
				else if (type == 1) {
					if (entry_impl.num_children() == 4) a.stp(reg0, reg1, asmjit::a64::Mem(reg2));
					else {
						std::string inst;
						entry_impl[4] >> inst;
						if (inst.compare("!")) {
							uint16_t value = 0;
							bool passed = getInteger(inst, &value);
							if (!passed) return 0xFF013C;
							a.stp(reg0, reg1, asmjit::a64::ptr_post(reg2, value));
						}
					}
				}
			}
		}
		return 0;
	}

	template <typename T> Result LSL(T entry_impl) {
		if (entry_impl.num_children() != 4)
			return 0xFF0140;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		auto reg0 = getGenRegister(inst, true, false, true);
		if (reg0 == GP_REG_ERROR) return 0xFF0141;
		entry_impl[2] >> inst;
		auto reg1 = getGenRegister(inst, true, false, true);
		if (reg1 == GP_REG_ERROR) return 0xFF0142;
		entry_impl[3] >> inst;
		uint8_t shift = 0;
		bool passed = getInteger(inst, &shift);
		if (!passed) return 0xFF0143;
		a.lsl(reg0, reg1, shift);
		return 0;
	}

	template <typename T> Result SVC(T entry_impl) {
		if (entry_impl.num_children() != 2)
			return 0xFF0150;
		asmjit::a64::Assembler a(&code);
		std::string inst;
		entry_impl[1] >> inst;
		uint8_t value = 0;
		bool passed = getInteger(inst, &value);
		if (!passed) return 0xFF0151;
		a.svc(value);
		return 0;
	}

	template <typename T> Result FMINNM(T entry_impl, uint8_t type = 0) {
		asmjit::a64::Assembler a(&code);
		std::string inst;
		if (entry_impl.num_children() != 4)
			return 0xFF0160;
		asmjit::a64::Vec regs[3];
		for (size_t i = 1; i < 4; i++) {
			entry_impl[i] >> inst;
			regs[i-1] = getFpRegister(inst, false, true, true, true, false);
			if (regs[i-1] == FP_REG_ERROR) return 0xFF0160 + i;
		}
		a.fminnm(regs[0], regs[1], regs[2]);
		return 0;
	}

	constexpr uint32_t hashes[] = {
		hash32("ADRP"),
		hash32("ADD"),
		hash32("FADD"),
		hash32("LDR"),
		hash32("LDRB"),
		hash32("LDRH"),
		hash32("LDUR"),
		hash32("LDURH"),
		hash32("NOP"),
		hash32("MOV"),
		hash32("MOVK"),
		hash32("FMUL"),
		hash32("FDIV"),
		hash32("RET"),
		hash32("STR"),
		hash32("STRB"),
		hash32("STRH"),
		hash32("STUR"),
		hash32("STURH"),
		hash32("FMOV"),
		hash32("B"),
		hash32("B.EQ"),
		hash32("B.LE"),
		hash32("B.GE"),
		hash32("B.NE"),
		hash32("B.GT"),
		hash32("B.LT"),
		hash32("B.HI"),
		hash32("BL"),
		hash32("BLR"),
		hash32("BR"),
		hash32("SUB"),
		hash32("CMP"),
		hash32("FCMP"),
		hash32("FCMPE"),
		hash32("UCVTF"),
		hash32("SCVTF"),
		hash32("FCVT"),
		hash32("CBZ"),
		hash32("CBNZ"),
		hash32("TBZ"),
		hash32("TBNZ"),
		hash32("CSEL"),
		hash32("FCSEL"),
		hash32("FCVTZU"),
		hash32("MADD"),
		hash32("FMADD"),
		hash32("MRS"),
		hash32("MUL"),
		hash32("UDIV"),
		hash32("SDIV"),
		hash32("LDP"),
		hash32("FNEG"),
		hash32("FSQRT"),
		hash32("STP"),
		hash32("FSUB"),
		hash32("LSL"),
		hash32("SVC"),
		hash32("FMINNM"),
	};

	template <typename T> constexpr bool has_duplicates(const T *array, std::size_t size)
	{
		for (std::size_t i = 1; i < size; i++)
			for (std::size_t j = 0; j < i; j++)
				if (array[i] == array[j]) {
					return true;
				}
		return false;
	}

	static_assert(!has_duplicates(hashes, std::size(hashes)), "Detected repeated hash!");
	static_assert(!has_duplicates(hashes2, std::size(hashes2)), "Detected repeated hash!");

	Result processArm64(c4::yml::NodeRef entry, uint32_t* out, uint8_t* adjust_type_arg, uintptr_t pc_address, uintptr_t start_address, const std::unordered_map<std::string, uint32_t> gotos) {
		std::string inst;
		entry[0] >> inst;
		code.init(customEnv, pc_address);
		m_pc_address = pc_address;
		m_pc_start = start_address;
		Result rc = 0;
		adjust_type = 0;
		switch(hash32(inst.c_str())) {
			case hash32("ADRP"): {rc = ADRP(entry); break;}
			case hash32("ADD"): {rc = ADD(entry); break;}
			case hash32("FADD"): {rc = ADD(entry, 1); break;}
			case hash32("LDR"): {rc = LDR(entry); break;}
			case hash32("LDRB"): {rc = LDR(entry, 1); break;}
			case hash32("LDRH"): {rc = LDR(entry, 2); break;}
			case hash32("LDUR"): {rc = LDR(entry, 3); break;}
			case hash32("LDURH"): {rc = LDR(entry, 4); break;}
			case hash32("NOP"): {NOP(); break;}
			case hash32("MOV"): {rc = MOV(entry); break;}
			case hash32("MOVK"): {rc = MOV(entry, 1); break;}
			case hash32("FMUL"): {rc = FMUL(entry); break;}
			case hash32("FDIV"): {rc = FMUL(entry, 1); break;}
			case hash32("RET"): {RET(); break;}
			case hash32("STR"): {rc = STR(entry); break;}
			case hash32("STRB"): {rc = STR(entry, 1); break;}
			case hash32("STRH"): {rc = STR(entry, 2); break;}
			case hash32("STUR"): {rc = STR(entry, 3); break;}
			case hash32("STURH"): {rc = STR(entry, 4); break;}
			case hash32("FMOV"): {rc = MOV(entry, 2); break;}
			case hash32("B"): {rc = B(entry, 0, 0xFF, gotos); break;}
			case hash32("B.EQ"): {rc = B(entry, 0, hash32("EQ"), gotos); break;}
			case hash32("B.LE"): {rc = B(entry, 0, hash32("LE"), gotos); break;}
			case hash32("B.GE"): {rc = B(entry, 0, hash32("GE"), gotos); break;}
			case hash32("B.NE"): {rc = B(entry, 0, hash32("NE"), gotos); break;}
			case hash32("B.GT"): {rc = B(entry, 0, hash32("GT"), gotos); break;}
			case hash32("B.LT"): {rc = B(entry, 0, hash32("LT"), gotos); break;}
			case hash32("B.HI"): {rc = B(entry, 0, hash32("HI"), gotos); break;}
			case hash32("BL"): {rc = B(entry, 1, 0xFF); break;}
			case hash32("BLR"): {rc = B(entry, 2, 0xFF); break;}
			case hash32("BR"): {rc = B(entry, 3, 0xFF); break;}
			case hash32("SUB"): {rc = ADD(entry, 2); break;}
			case hash32("FSUB"): {rc = ADD(entry, 3); break;}
			case hash32("CMP"): {rc = CMP(entry); break;}
			case hash32("FCMP"): {rc = CMP(entry, 1); break;}
			case hash32("FCMPE"): {rc = CMP(entry, 2); break;}
			case hash32("UCVTF"): {rc = UCVTF(entry); break;}
			case hash32("SCVTF"): {rc = UCVTF(entry, 1); break;}
			case hash32("FCVT"): {rc = FCVT(entry); break;}
			case hash32("CBZ"): {rc = CBZ(entry); break;}
			case hash32("CBNZ"): {rc = CBZ(entry, 1); break;}
			case hash32("TBZ"): {rc = TBZ(entry); break;}
			case hash32("TBNZ"): {rc = TBZ(entry, 1); break;}
			case hash32("CSEL"): {rc = CSEL(entry); break;}
			case hash32("FCSEL"): {rc = CSEL(entry, 1); break;}
			case hash32("FCVTZU"): {rc = FCVTZU(entry); break;}
			case hash32("MADD"): {rc = MADD(entry); break;}
			case hash32("FMADD"): {rc = MADD(entry, 1); break;}
			case hash32("MRS"): {rc = MRS(entry); break;}
			case hash32("MUL"): {rc = MUL(entry); break;}
			case hash32("SDIV"): {rc = MUL(entry, 1); break;}
			case hash32("UDIV"): {rc = MUL(entry, 2); break;}
			case hash32("LDP"): {rc = LDP(entry); break;}
			case hash32("FNEG"): {rc = FNEG(entry); break;}
			case hash32("FSQRT"): {rc = FSQRT(entry); break;}
			case hash32("STP"): {rc = LDP(entry, 1); break;}
			case hash32("LSL"): {rc = LSL(entry); break;}
			case hash32("SVC"): {rc = SVC(entry); break;}
			case hash32("FMINNM"): {rc = FMINNM(entry); break;}
			default: return 0xFFFFFE;
		}
		if (R_FAILED(rc)) {
			return rc;
		}
		size_t codeSize = code.codeSize();
		if (codeSize != 4) {
			return 0xFFFFFD;
		}
		code.copyFlattenedData(out, 4);
		code.reset();
		if (adjust_type_arg) *adjust_type_arg = adjust_type;
		return 0;
	}
}