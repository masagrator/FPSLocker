#define NOINLINE __attribute__ ((noinline))

#include "Lock.hpp"
#include "c4/std/string.hpp"
#include "asmA64.hpp"
#include <cmath>
#include <vector>
#include <unordered_map>

namespace LOCK {

	ryml::Tree tree;
	char configBuffer[32770] = "";
	uint8_t gen = 3;
	bool master_write = false;

	struct buffer_data {
		size_t size;
		void* buffer_ptr;
	};
	std::vector<buffer_data*> buffers;

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

	std::unordered_map<uint32_t, declare_var> declared_variables;
	std::unordered_map<uint32_t, uint64_t> declared_consts;
	std::unordered_map<uint32_t, declare_code> declared_codes;

	void freeBuffers() {
		for (int i = (buffers.size() - 1); i >= 0; i--) {
			if (buffers[i] -> buffer_ptr) {
				free(buffers[i] -> buffer_ptr);
				free(buffers[i]);
			}
			buffers.pop_back();
		}
	}

	void freeDeclares() {
		for (const auto& [key, data] : declared_codes) {
			if (data.code_buffer) free(data.code_buffer);
			if (data.adjust_types_buffer) free(data.adjust_types_buffer);
		}
	}

	const char compare_types[6][3] = {">", ">=", "<", "<=", "==", "!="};
	uint8_t NOINLINE getCompareType(std::string compare_type) {
		for (size_t i = 0; i < std::size(compare_types); i++)
			if (!compare_type.compare(compare_types[i]))
				return i + 1;
		return 0;
	}

	const char regions[5][9] = {"MAIN", "HEAP", "ALIAS", "VARIABLE", "CODE"};
	uint8_t NOINLINE getAddressRegion(std::string region) {
		for (size_t i = 0; i < std::size(regions); i++)
			if (!region.compare(regions[i]))
				return i + 1;
		return 0;		
	}

	constexpr uint32_t NOINLINE hash32(const char* str) {
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

	constexpr uint8_t NOINLINE getValueType(std::string value_type) {
		if (!value_type.compare("uint8"))
			return 0x1;
		else if (!value_type.compare("uint16"))
			return 0x2;
		else if (!value_type.compare("uint32"))
			return 0x4;
		else if (!value_type.compare("uint64"))
			return 0x8;
		else if (!value_type.compare("int8"))
			return 0x11;
		else if (!value_type.compare("int16"))
			return 0x12;
		else if (!value_type.compare("int32"))
			return 0x14;
		else if (!value_type.compare("int64"))
			return 0x18;
		else if (!value_type.compare("float"))
			return 0x24;
		else if (!value_type.compare("double"))
			return 0x28;
		else if (!value_type.compare("refresh_rate"))
			return 0x38;
		else return 0;
	}

	constexpr size_t NOINLINE getTypeSize(std::string value_type) {
		if (!value_type.compare("int8") || !value_type.compare("uint8"))
			return sizeof(uint8_t);
		else if (!value_type.compare("int16") || !value_type.compare("uint16"))
			return sizeof(uint16_t);	
		else if (!value_type.compare("int32") || !value_type.compare("uint32") || !value_type.compare("float"))
			return sizeof(uint32_t);
		else if (!value_type.compare("int64") || !value_type.compare("uint64") || !value_type.compare("double") || !value_type.compare("refresh_rate"))
			return sizeof(uint64_t);
		else return 0;
	}

	template <typename T>
	Result writeEntryTo(T value, uint8_t* buffer, size_t* offset, uint8_t value_type) {
		switch(value_type) {
			case 1:
				value >> buffer[*offset];
				break;
			case 2:
				value >> *(uint16_t*)(&buffer[*offset]);
				break;
			case 4:
				value >> *(uint32_t*)(&buffer[*offset]);
				break;
			case 8:
			case 0x48:
				value >> *(uint64_t*)(&buffer[*offset]);
				break;
			case 0x11:
				value >> *(int8_t*)(&buffer[*offset]);
				break;
			case 0x12:
				value >> *(int16_t*)(&buffer[*offset]);
				break;
			case 0x14:
				value >> *(int32_t*)(&buffer[*offset]);
				break;
			case 0x18:
				value >> *(int64_t*)(&buffer[*offset]);
				break;
			case 0x24:
				value >> *(float*)(&buffer[*offset]);
				break;
			case 0x28:
			case 0x38:
				value >> *(double*)(&buffer[*offset]);
				break;
			default:
				return 4;
		}
		*offset += value_type % 0x10;
		return 0;
	}

	template <typename T>
	Result NOINLINE calculateSize(T entry, size_t* out, bool masterWrite = false, bool compiled = false) {
		std::string string_check = "";
		size_t temp_size = 0;

		//calculate bytes size

		auto num_children = entry.num_children();

		if (masterWrite) {
			for (size_t i = 0; i < num_children; i++) {
				temp_size++;
				entry[i]["type"] >> string_check;

				if (!string_check.compare("bytes")) {
					temp_size += 4; // main_offset
					temp_size++; // value_type
					temp_size++; // value count
					entry[i]["value_type"] >> string_check;
					if (entry[i]["value"].is_seq()) {
						temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
					}
					else temp_size += getTypeSize(string_check);
				}
				else if (!string_check.compare("asm_a64")) {
					temp_size++; // address_region
					temp_size += 4; // main_offset
					temp_size++; // value_type
					temp_size++; // value count
					if (entry[i]["instructions"].is_seq()) {
						temp_size += entry[i]["instructions"].num_children(); //adjustment
						temp_size += (getTypeSize("uint32") * entry[i]["instructions"].num_children());
					}
					else return 0x4096;
				}
				else return 2;
			}
			if (declared_variables.size() > 0) {
				for (const auto& [key, data] : declared_variables) {
					temp_size++; // type
					temp_size += 4;//main_offset
					temp_size++;//value_type
					temp_size++;//value_count
					temp_size += data.value_type % 0x10;
				}
			}
			if (declared_codes.size() > 0) {
				for (const auto& [key, data] : declared_codes) {
					temp_size++; // type
					temp_size++; // address_region
					temp_size += 4;//main_offset
					temp_size++;//value_type
					temp_size++;//value_count
					temp_size += data.instructions_num; //adjustment
					temp_size += getTypeSize("uint32") * data.instructions_num;
				}
			}
			temp_size++;
			*out = temp_size;
			return 0;
		}
		
		for (size_t i = 0; i < num_children; i++) {

			entry[i]["type"] >> string_check;
			
			temp_size++;
			if (!string_check.compare("write") || !string_check.compare("evaluate_write")) {
				temp_size++; // address count
				temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				temp_size++; // value_type
				temp_size++; // value count
				if (!string_check.compare("write") || compiled) {
					entry[i]["address"][0] >> string_check;
					uint8_t address_region = getAddressRegion(string_check);
					if (address_region != 4) {
						entry[i]["value_type"] >> string_check;
						if (entry[i]["value"].is_seq()) {
							temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
						}
						else temp_size += getTypeSize(string_check);
					}
					else {
						temp_size += declared_variables[hash32(string_check.c_str())].value_type % 0x10;
					}
				}
				else {
					if (entry[i]["value"].is_seq()) {
						for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
							entry[i]["value"][x] >> string_check;
							temp_size += string_check.size() + 1;
						}
					}
					else {
						entry[i]["value"] >> string_check;
						temp_size += string_check.size() + 1;
					}
				}
				
			}
			else if (!string_check.compare("compare") || !string_check.compare("evaluate_compare")) {
				bool evaluate_compare = false;
				if (!string_check.compare("evaluate_compare"))
					evaluate_compare = true;
				temp_size++; // compare_address count
				temp_size += ((entry[i]["compare_address"].num_children() - 1) * 4) + 1; // address array
				temp_size++; // compare_type
				temp_size++; // compare_value_type
				entry[i]["compare_address"][0] >> string_check;
				uint8_t address_region = getAddressRegion(string_check);
				if (address_region != 4) {
					entry[i]["compare_value_type"] >> string_check;
					temp_size += getTypeSize(string_check);
				}
				else {
					entry[i]["compare_address"][1] >> string_check;
					temp_size += declared_variables[hash32(string_check.c_str())].value_type % 0x10;
				}
				temp_size++; // address count
				if (entry[i].has_child("address"))
					temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				else temp_size++;
				temp_size++; // value_type
				temp_size++; // value count
				if (entry[i].has_child("address") == false) {
					entry[i]["value_type"] >> string_check;
					temp_size += getTypeSize(string_check);
				}
				else if (!evaluate_compare || compiled) {
					entry[i]["address"][0] >> string_check;
					address_region = getAddressRegion(string_check);
					if (address_region != 4) {
						entry[i]["value_type"] >> string_check;
						if (entry[i]["value"].is_seq()) {
							temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
						}
						else temp_size += getTypeSize(string_check);
					}
					else {
						entry[i]["address"][1] >> string_check;
						temp_size += declared_variables[hash32(string_check.c_str())].value_type % 0x10;
					}
				}
				else {
					if (entry[i]["value"].is_seq()) {
						for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
							entry[i]["value"][x] >> string_check;
							temp_size += string_check.size() + 1;
						}
					}
					else {
						entry[i]["value"] >> string_check;
						temp_size += string_check.size() + 1;
					}
				}
			}
			else if (!string_check.compare("block")) {
				temp_size++;
			}
			else return 0x12;
		}

		
		if (declared_variables.size() > 0) {
			for (const auto& [key, data] : declared_variables) {
				if (!data.evaluate.compare("")) continue;
				temp_size++; // type
				temp_size++; //address_count
				temp_size++; //address region
				temp_size += 4;//offset
				temp_size++;//value_type
				temp_size++;//value_count
				if (compiled) temp_size += data.value_type % 0x10;
				else temp_size += data.evaluate.length() + 1;
			}
		}		
		temp_size++;
		*out = temp_size;
		return 0;
	}

	template <typename T>
	Result NOINLINE processEntryImpl(T entry, uint8_t* buffer, size_t* out_size, bool masterWrite = false) {
		std::string string_check = "";
		size_t temp_size = 0;
		auto num_children = entry.num_children();
		if (masterWrite) {
			for (size_t i = 0; i < num_children; i++) {
				entry[i]["type"] >> string_check;
				if (!string_check.compare("bytes")) {
					buffer[temp_size++] = 1; // type
					entry[i]["main_offset"] >> *(uint32_t*)(&buffer[temp_size]);
					temp_size += 4;
					entry[i]["value_type"] >> string_check;
					uint8_t value_type = getValueType(string_check);
					buffer[temp_size++] = value_type;
					if (entry[i]["value"].is_seq()) {
						buffer[temp_size++] = entry[i]["value"].num_children(); //value_count
						for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
							Result rc = writeEntryTo(entry[i]["value"][x], buffer, &temp_size, value_type);
							if (R_FAILED(rc)) return rc;
						}
					}
					else {
						buffer[temp_size++] = 1;
						Result rc = writeEntryTo(entry[i]["value"], buffer, &temp_size, value_type);
						if (R_FAILED(rc)) return rc;
					}					
				}
				else if (!string_check.compare("asm_a64")) {
					buffer[temp_size++] = 3; // type
					uint32_t main_offset = 0;
					entry[i]["main_offset"] >> main_offset;
					buffer[temp_size++] = getAddressRegion("MAIN");
					*(uint32_t*)(&buffer[temp_size]) = main_offset;
					uint32_t start_main_offset = main_offset;
					temp_size += 4;
					buffer[temp_size++] = getValueType("uint32");
					if (entry[i]["instructions"].is_seq()) {
						buffer[temp_size++] = entry[i]["instructions"].num_children(); //value_count
						for (size_t x = 0; x < entry[i]["instructions"].num_children(); x++) {
							uint32_t inst = 0;
							Result rc = 1;
							uint8_t adjust_type = 0;
							if (entry[i]["instructions"][x].is_seq()) {
								rc = ASM::processArm64(entry[i]["instructions"][x], &inst, &adjust_type, main_offset, start_main_offset);
								if (R_FAILED(rc)) return rc;
							}
							else entry[i]["instructions"][x] >> inst;
							main_offset += 4;
							buffer[temp_size++] = adjust_type;
							*(uint32_t*)(&buffer[temp_size]) = inst;
							temp_size += 4;
						}
					}
					else return 0x4096;
				}
				else return 0x22;
			}
			if (declared_variables.size() > 0) {
				for (const auto& [key, data] : declared_variables) {
					buffer[temp_size++] = 2; // type
					*(uint32_t*)(&buffer[temp_size]) = data.cave_offset;
					temp_size += 4;
					uint8_t value_type = data.value_type;
					buffer[temp_size++] = value_type;
					buffer[temp_size++] = 1;
					memcpy(&buffer[temp_size], &data.default_value, value_type % 0x10);
					temp_size += value_type % 0x10;
				}
			}
			if (declared_codes.size() > 0) {
				for (const auto& [key, data] : declared_codes) {
					buffer[temp_size++] = 3; // type
					buffer[temp_size++] = getAddressRegion("CODE");
					*(uint32_t*)(&buffer[temp_size]) = data.cave_offset;
					temp_size += 4;
					buffer[temp_size++] = 4; //value_type
					buffer[temp_size++] = data.instructions_num;
					for (size_t i = 0; i < data.instructions_num; i++) {
						buffer[temp_size++] = data.adjust_types_buffer[i];
						memcpy(&buffer[temp_size], &data.code_buffer[i], 4);
						temp_size += 4;
					}
				}
			}
			buffer[temp_size++] = 0xFF;
			*out_size = temp_size;
			return 0;
		}
		for (size_t i = 0; i < num_children; i++) {
		
			entry[i]["type"] >> string_check;
			if (!string_check.compare("write") || !string_check.compare("evaluate_write")) {
				
				bool evaluate_write = false;
				if (!string_check.compare("evaluate_write"))
					evaluate_write = true;

				buffer[temp_size++] = (evaluate_write ? 0x81 : 1); // type
				buffer[temp_size++] = entry[i]["address"].num_children(); // address count
				entry[i]["address"][0] >> string_check;
				uint8_t address_region = getAddressRegion(string_check);
				buffer[temp_size++] = address_region;
				uint8_t value_type = 0;
				if (address_region != 4) {
					entry[i]["value_type"] >> string_check;
					value_type = getValueType(string_check);
					for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
						entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
						temp_size += 4;
					}
				}
				else {
					entry[i]["address"][1] >> string_check;
					*(int32_t*)(&buffer[temp_size]) = declared_variables[hash32(string_check.c_str())].cave_offset;
					temp_size += 4;
					value_type = declared_variables[hash32(string_check.c_str())].value_type;
				}
				buffer[temp_size++] = value_type;
				if (entry[i]["value"].is_seq()) {
					buffer[temp_size++] = entry[i]["value"].num_children(); //value_count
					for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
						Result rc = 0;
						if (evaluate_write) {
							entry[i]["value"][x] >> string_check;
							strcpy((char*)&buffer[temp_size], string_check.c_str());
							temp_size += string_check.size() + 1;
						}
						else rc = writeEntryTo(entry[i]["value"][x], buffer, &temp_size, value_type);
						if (R_FAILED(rc)) return rc;
					}
				}
				else {
					buffer[temp_size++] = 1;
					Result rc = 0;
					if (evaluate_write) {
						entry[i]["value"] >> string_check;
						strcpy((char*)&buffer[temp_size], string_check.c_str());
						temp_size += string_check.size() + 1;
					}
					else rc = writeEntryTo(entry[i]["value"], buffer, &temp_size, value_type);
					if (R_FAILED(rc)) return rc;
				}
			}
			else if (!string_check.compare("compare") || !string_check.compare("evaluate_compare")) {

				bool evaluate_write = false;
				if (!string_check.compare("evaluate_compare"))
					evaluate_write = true;
				
				buffer[temp_size++] = (evaluate_write ? 0x82 : 2); //+1
				buffer[temp_size++] = entry[i]["compare_address"].num_children();//+1
				entry[i]["compare_address"][0] >> string_check;
				uint8_t address_region = getAddressRegion(string_check);
				buffer[temp_size++] = address_region; //+1
				uint8_t compare_value_type = 0;
				if (address_region != 4) {
					entry[i]["compare_value_type"] >> string_check;
					compare_value_type = getValueType(string_check);
					for (size_t x = 1; x < entry[i]["compare_address"].num_children(); x++) {
						entry[i]["compare_address"][x] >> *(int32_t*)(&buffer[temp_size]);
						temp_size += 4; //+(x*4)
					}
				}
				else {
					entry[i]["compare_address"][1] >> string_check;
					*(uint32_t*)(&buffer[temp_size]) = declared_variables[hash32(string_check.c_str())].cave_offset;
					compare_value_type = declared_variables[hash32(string_check.c_str())].value_type;
					temp_size += 4; //+4
				}
				entry[i]["compare_type"][0] >> string_check;
				buffer[temp_size++] = getCompareType(string_check); //+1
				buffer[temp_size++] = compare_value_type; //+1

				Result rc = writeEntryTo(entry[i]["compare_value"], buffer, &temp_size, compare_value_type); //+x
				if (R_FAILED(rc)) return rc;
				uint8_t value_type = 0;
				if (entry[i].has_child("address") == false) {
					buffer[temp_size++] = 0;
					buffer[temp_size++] = 0;
					entry[i]["value_type"] >> string_check;
					value_type = getValueType(string_check);
				}
				else {
					buffer[temp_size++] = entry[i]["address"].num_children(); // address count
					entry[i]["address"][0] >> string_check;
					address_region = getAddressRegion(string_check);
					buffer[temp_size++] = address_region < 4 ? address_region : 1;
					if (address_region != 4) {
						entry[i]["value_type"] >> string_check;
						value_type = getValueType(string_check);
						for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
							entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
							temp_size += 4;
						}
					}
					else {
						entry[i]["address"][1] >> string_check;
						*(int32_t*)(&buffer[temp_size]) = declared_variables[hash32(string_check.c_str())].cave_offset;
						value_type = declared_variables[hash32(string_check.c_str())].value_type;
						temp_size += 4;
					}
				}
				buffer[temp_size++] = value_type;
				if (entry[i]["value"].is_seq()) {
					buffer[temp_size++] = entry[i]["value"].num_children(); //value_count
					for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
						if (evaluate_write) {
							entry[i]["value"][x] >> string_check;
							strcpy((char*)&buffer[temp_size], string_check.c_str());
							temp_size += string_check.size() + 1;
						}
						else rc = writeEntryTo(entry[i]["value"][x], buffer, &temp_size, value_type);
						if (R_FAILED(rc)) return rc;
					}
				}
				else {
					buffer[temp_size++] = 1;
					if (evaluate_write) {
						entry[i]["value"] >> string_check;
						strcpy((char*)&buffer[temp_size], string_check.c_str());
						temp_size += string_check.size() + 1;
					}
					else rc = writeEntryTo(entry[i]["value"], buffer, &temp_size, value_type);
					if (R_FAILED(rc)) return rc;
				}
			}
			else if (!string_check.compare("block")) {
				buffer[temp_size++] = 3;
				entry[i]["what"] >> string_check;
				if (!string_check.compare("timing")) {
					buffer[temp_size++] = 1;
				}
			}
			else return 0x32;
		}
		if (declared_variables.size() > 0) {
			for (const auto& [key, data] : declared_variables) {
				if (!data.evaluate.compare("")) continue;
				buffer[temp_size++] = 0x81; // type
				buffer[temp_size++] = 2; // address count
				buffer[temp_size++] = 4; //address region VARIABLE
				memcpy(&buffer[temp_size], &data.cave_offset, 4);
				temp_size += 4;
				buffer[temp_size++] = data.value_type;
				buffer[temp_size++] = 1; //value_count
				memcpy(&buffer[temp_size], data.evaluate.c_str(), data.evaluate.length() + 1);
				temp_size += data.evaluate.length() + 1;
			}
		}
		buffer[temp_size++] = 0xFF;
		*out_size = temp_size;
		return 0;
	}

	template <typename T>
	Result NOINLINE processEntry(T entry, bool masterWrite = false) {
		
		size_t temp_size = 0;
		size_t old_temp_size = 0;
		static uint16_t call_count = 0;
		call_count++;
		
		Result rc = calculateSize(entry, &temp_size, masterWrite);

		if (R_FAILED(rc)) return rc;

		uint8_t* buffer = (uint8_t*)calloc(temp_size, sizeof(uint8_t));
		old_temp_size = temp_size;
		temp_size = 0;

		rc = processEntryImpl(entry, buffer, &temp_size, masterWrite);
		if (R_FAILED(rc)) {
			free(buffer);
			return rc;
		}
		if (old_temp_size != temp_size) {
			free(buffer);
			return 10;
		}
		buffer_data* new_struct = (buffer_data*)calloc(sizeof(buffer_data), 1);
		new_struct -> size = temp_size;
		new_struct -> buffer_ptr = &buffer[0];
		buffers.push_back(new_struct);
		return 0;
	}

	template <typename T> 
	Result NOINLINE processVariable(T entry) {
		std::string string_check;
		static ptrdiff_t cave_offset = 0;
		entry["name"] >> string_check;
		uint32_t hash = hash32(string_check.c_str());
		entry["value_type"] >> string_check;
		uint8_t value_type = getValueType(string_check);
		size_t value_size = value_type % 0x10;
		if (cave_offset % value_size != 0) {
			cave_offset += value_size - (cave_offset % value_size);
		}
		if (entry.has_child("evaluate") == true) {
			entry["evaluate"] >> string_check;
		}
		else string_check = "";
		uint64_t value = 0;
		size_t offset = 0;
		writeEntryTo(entry["default_value"], (uint8_t*)&value, &offset, value_type);
		declared_variables[hash] = declare_var{cave_offset, value_type, string_check, value};
		cave_offset += value_type % 0x10;
		return 0;
	}

	template <typename T> 
	Result NOINLINE processConst(T entry) {
		std::string string_check;
		entry["name"] >> string_check;
		uint32_t hash = hash32(string_check.c_str());
		int64_t value = 0;
		entry["value"] >> value;
		uint64_t value2 = 0;
		memcpy(&value2, &value, 8);
		declared_consts[hash] = value2;
		return 0;
	}

	template <typename T> 
	Result NOINLINE processCode(T entry) {
		std::string string_temp;
		entry["name"] >> string_temp;
		std::string string_check = "_" + string_temp + "()";
		uint32_t hash = hash32(string_check.c_str());
		uint32_t start_cave_offset = 0;
		std::unordered_map<std::string, uint32_t> gotos;

		if (declared_codes.size() > 0) {
			auto it = declared_codes.begin()->second;
			start_cave_offset = it.cave_offset + (it.instructions_num * 4);
		}
		uint32_t cave_offset = start_cave_offset;
		size_t instruction_num = 0;
		auto num_children = entry["instructions"].num_children();
		for (size_t i = 0; i < num_children; i++) {
			if (entry["instructions"][i].is_seq()) {
				cave_offset += 4;
				instruction_num++;
			}
			else {
				entry["instructions"][i] >> string_check;
				gotos[string_check] = cave_offset;
			}
		}
		uint32_t* out_buffer = (uint32_t*)calloc(4, instruction_num);
		uint8_t* adjust_types_buffer = (uint8_t*)calloc(1, instruction_num);
		cave_offset = start_cave_offset;
		size_t itr = 0;
		for (size_t i = 0; i < num_children; i++) {
			if (entry["instructions"][i].is_seq()) {
				uint32_t instruction = 0;
				uint8_t adjust_type = 0;
				Result rc = ASM::processArm64(entry["instructions"][i], &instruction, &adjust_type, cave_offset, start_cave_offset, gotos);
				if (R_FAILED(rc)) {
					freeDeclares();
					return rc;
				}
				adjust_types_buffer[itr] = adjust_type;
				out_buffer[itr++] = instruction;
				cave_offset += 4;
			}
		}
		declared_codes[hash] = declare_code(start_cave_offset, instruction_num, out_buffer, adjust_types_buffer);
		return 0;
	}

	template <typename T>
	Result NOINLINE registerDeclarations(T entry) {
		std::string string_check;
		declared_variables.clear();
		declared_consts.clear();
		freeDeclares();
		declared_codes.clear();
		Result rc = 0;
		for (size_t i = 0; i < entry.num_children(); i++) {
			entry[i]["type"] >> string_check;
			switch(hash32(string_check.c_str())) {
				case hash32("variable"): {rc = processVariable(entry[i]); break;}
				case hash32("const"): {rc = processConst(entry[i]); break;}
				case hash32("code"): {rc = processCode(entry[i]); break;}
				default: return 0xE0001;
			}
			if (R_FAILED(rc)) return rc;
		}
		return 0;
	}

	Result createPatch(const char* path) {
		bool unsafeCheck = false;

		char lockMagic[] = "LOCK";
		gen = 3;
		tree["unsafeCheck"] >> unsafeCheck;
		size_t temp_size = 0;

		uint8_t compiledSize = 0;

		if (tree.has_child(tree.root_id(), "DECLARATIONS") == true) {
			gen = 4;
			Result ret = registerDeclarations(tree["DECLARATIONS"]);
			if (R_FAILED(ret)) return ret;
		}

		if (tree.has_child(tree.root_id(), "ALL_FPS") == false) {
			if (master_write == false) return 0x3006;
			uint8_t* buffer = (uint8_t*)calloc(1, sizeof(uint8_t));
			buffer[0] = 0xFF;
			buffer_data* new_struct = (buffer_data*)calloc(sizeof(buffer_data), 1);
			new_struct -> size = 1;
			new_struct -> buffer_ptr = &buffer[0];
			buffers.push_back(new_struct);
			compiledSize = (uint8_t)sqrt(1 + 0x10) + 1;
		}
		else {
			Result ret = calculateSize(tree["ALL_FPS"], &temp_size, false, true);
			if (R_FAILED(ret)) return ret;
		
			ret = processEntry(tree["ALL_FPS"], false);

			if (R_FAILED(ret)) {
				freeBuffers();
				return ret;
			}
			compiledSize = (uint8_t)sqrt(temp_size + 0x10) + 1;
		}

		uint8_t flags[4] = {gen, master_write, compiledSize, unsafeCheck};
		
		if (master_write) {
			Result ret = processEntry(tree["MASTER_WRITE"], true);
			if (R_FAILED(ret)) {
				freeBuffers();
				return ret;
			}
		}

		uint8_t entries_count = (master_write ? 2 : 1);

		uint32_t base_offset = strlen(lockMagic) + sizeof(flags) + (4 * entries_count);
		uint32_t* offsets = (uint32_t*)calloc(entries_count, 4);
		offsets[0] = base_offset;
		base_offset += buffers[0] -> size;
		uint8_t* IDs = (uint8_t*)calloc(entries_count, 1);;
		for (size_t i = 1; i < buffers.size(); i++) {
			for (size_t x = 0; x < i; x++) {
				if (buffers[x] -> size != buffers[i] -> size) {
					if (x + 1 == i) {
						IDs[i] = i;
						offsets[i] = base_offset;
						base_offset += buffers[i] -> size;
					}
					continue;
				}
				if (memcmp(buffers[x] -> buffer_ptr, buffers[i] -> buffer_ptr, buffers[x] -> size)) {
					if (x + 1 == i) {
						IDs[i] = i;
						offsets[i] = base_offset;
						base_offset += buffers[i] -> size;
					}
					continue;
				}
				IDs[i] = IDs[x];
				offsets[i] = offsets[x];
				break;
			}
		}

		if (!buffers.size()) {
			free(offsets);
			free(IDs);
			return 0x18;
		}
		FILE* file = fopen(path, "wb");
		if (!file) {
			freeBuffers();
			free(offsets);
			free(IDs);
			return 0x202;
		}
		fwrite(&lockMagic[0], 4, 1, file);
		fwrite(&flags[0], 4, 1, file);
		for (size_t i = 0; i < entries_count; i++) {
			fwrite(&offsets[i], 4, 1, file);
		}
		for (size_t i = 0; i < buffers.size(); i++) 
			if (IDs[i] == i)
				fwrite(buffers[i] -> buffer_ptr, buffers[i] -> size, 1, file);

		fclose(file);
		freeBuffers();
		free(offsets);
		free(IDs);
		//remove(path);
		return 0;
	}

	Result readConfig(const char* path) {
		FILE* config = fopen(path, "r");
		if (!config)
			return 0x202;
		fread(&configBuffer, 1, 32768, config);
		fclose(config);
		strcat(&configBuffer[0], "\n");

		tree = ryml::parse_in_place(configBuffer);
		size_t root_id = tree.root_id();

		if (!tree.is_map(root_id))
			return 0x1104;
		if (tree.find_child(root_id, "unsafeCheck") == c4::yml::NONE)
			return 0x1101;
		if (!tree["unsafeCheck"].is_keyval())
			return 0x1102;
		if (tree.find_child(root_id, "MASTER_WRITE") != c4::yml::NONE)
			master_write = true;
		if (tree.find_child(root_id, "ALL_FPS") == c4::yml::NONE)
			return 0x1105;
		if (tree.find_child(root_id, "ALL_REFRESH_RATES") != c4::yml::NONE)
			return 0x1106;

		return 0;
	}
}