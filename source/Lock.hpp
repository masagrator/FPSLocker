#pragma once
#define NOINLINE __attribute__ ((noinline))

#include "rapidyaml/ryml.hpp"
#include "c4/std/string.hpp"
#include "tinyexpr/tinyexpr.h"
#include <cmath>

namespace LOCK {

	const char entries[10][6] = {"15FPS", "20FPS", "25FPS", "30FPS", "35FPS", "40FPS", "45FPS", "50FPS", "55FPS", "60FPS"};
	ryml::Tree tree;
	char configBuffer[32770] = "";
	uint8_t gen = 1;
	bool ALL_FPS = false;

	struct buffer_data {
		size_t size;
		void* buffer_ptr;
	};
	std::vector<buffer_data*> buffers;

	void freeBuffers() {
		for (int i = (buffers.size() - 1); i >= 0; i--) {
			if (buffers[i] -> buffer_ptr) {
				free(buffers[i] -> buffer_ptr);
				free(buffers[i]);
			}
			buffers.pop_back();
		}
	}

	const char compare_types[6][3] = {">", ">=", "<", "<=", "==", "!="};
	uint8_t NOINLINE getCompareType(std::string compare_type) {
		for (size_t i = 0; i < std::size(compare_types); i++)
			if (!compare_type.compare(compare_types[i]))
				return i + 1;
		return 0;
	}

	const char regions[3][6] = {"MAIN", "HEAP", "ALIAS"};
	uint8_t NOINLINE getAddressRegion(std::string region) {
		for (size_t i = 0; i < std::size(regions); i++)
			if (!region.compare(regions[i]))
				return i + 1;
		return 0;		
	}

	uint8_t NOINLINE getValueType(std::string value_type) {
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
		else return 0;
	}

	size_t NOINLINE getTypeSize(std::string value_type) {
		if (!value_type.compare("int8") || !value_type.compare("uint8"))
			return sizeof(uint8_t);
		else if (!value_type.compare("int16") || !value_type.compare("uint16"))
			return sizeof(uint16_t);	
		else if (!value_type.compare("int32") || !value_type.compare("uint32") || !value_type.compare("float"))
			return sizeof(uint32_t);
		else if (!value_type.compare("int64") || !value_type.compare("uint64") || !value_type.compare("double"))
			return sizeof(uint64_t);
		else return 0;
	}

	double TruncDec(double value, double truncator) {
		uint64_t factor = pow(10, truncator);
		return trunc(value*factor) / factor;
	}


	template <typename T>
	size_t NOINLINE calculateSize(T entry, bool masterWrite = false) {
		std::string string_check = "";
		size_t temp_size = 0;

		//calculate bytes size

		if (masterWrite) {
			for (size_t i = 0; i < entry.num_children(); i++) {
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
				else return 2;
			}
			temp_size++;
			return temp_size;
		}
		
		for (size_t i = 0; i < entry.num_children(); i++) {

			entry[i]["type"] >> string_check;
			
			temp_size++;
			if (!string_check.compare("write") || !string_check.compare("evaluate_write")) {
				temp_size++; // address count
				temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				temp_size++; // value_type
				temp_size++; // value count
				entry[i]["value_type"] >> string_check;
				if (entry[i]["value"].is_seq()) {
					temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
				}
				else temp_size += getTypeSize(string_check);
				
			}
			else if (!string_check.compare("compare")) {
				temp_size++; // compare_address count
				temp_size += ((entry[i]["compare_address"].num_children() - 1) * 4) + 1; // address array
				temp_size++; // compare_type
				temp_size++; // compare_value_type
				entry[i]["compare_value_type"] >> string_check;
				temp_size += getTypeSize(string_check);
				temp_size++; // address count
				temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				temp_size++; // value_type
				temp_size++; // value count
				entry[i]["value_type"] >> string_check;
				if (entry[i]["value"].is_seq()) {
					temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
				}
				else temp_size += getTypeSize(string_check);
			}
			else if (!string_check.compare("block")) {
				temp_size++;
			}
			else return 2;
		}
		
		temp_size++;
		return temp_size;
	}

	template <typename T>
	Result NOINLINE processEntryImpl(T entry, uint8_t* buffer, size_t* out_size, bool masterWrite = false, double fps_target = 0) {
		std::string string_check = "";
		size_t temp_size = 0;
		if (masterWrite) {
			for (size_t i = 0; i < entry.num_children(); i++) {
				entry[i]["type"] >> string_check;
				if (!string_check.compare("bytes")) {
					buffer[temp_size] = 1; // type
					temp_size++;
					entry[i]["main_offset"] >> *(uint32_t*)(&buffer[temp_size]);
					temp_size += 4;
					entry[i]["value_type"] >> string_check;
					uint8_t value_type = getValueType(string_check);
					buffer[temp_size] = value_type;
					temp_size++;
					if (entry[i]["value"].is_seq()) {
						buffer[temp_size] = entry[i]["value"].num_children(); //value_count
						temp_size++;
						for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
							switch(value_type) {
								case 1:
									entry[i]["value"][x] >> buffer[temp_size];
									temp_size++;
									break;
								case 2:
									entry[i]["value"][x] >> *(uint16_t*)(&buffer[temp_size]);
									temp_size += 2;
									break;
								case 4:
									entry[i]["value"][x] >> *(uint32_t*)(&buffer[temp_size]);
									temp_size += 4;
									break;
								case 8:
									entry[i]["value"][x] >> *(uint64_t*)(&buffer[temp_size]);
									temp_size += 8;
									break;
								case 0x11:
									entry[i]["value"][x] >> *(int8_t*)(&buffer[temp_size]);
									temp_size++;
									break;
								case 0x12:
									entry[i]["value"][x] >> *(int16_t*)(&buffer[temp_size]);
									temp_size += 2;
									break;
								case 0x14:
									entry[i]["value"][x] >> *(int32_t*)(&buffer[temp_size]);
									temp_size += 4;
									break;
								case 0x18:
									entry[i]["value"][x] >> *(int64_t*)(&buffer[temp_size]);
									temp_size += 8;
									break;
								case 0x24:
									entry[i]["value"][x] >> *(float*)(&buffer[temp_size]);
									temp_size += 4;
									break;
								case 0x28:
									entry[i]["value"][x] >> *(double*)(&buffer[temp_size]);
									temp_size += 8;
									break;
								default:
									return 4;
							}
						}
					}
					else {
						buffer[temp_size] = 1;
						temp_size++;
						switch(value_type) {
							case 1:
								entry[i]["value"] >> buffer[temp_size];
								temp_size++;
								break;
							case 2:
								entry[i]["value"] >> *(uint16_t*)(&buffer[temp_size]);
								temp_size += 2;
								break;
							case 4:
								entry[i]["value"] >> *(uint32_t*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 8:
								entry[i]["value"] >> *(uint64_t*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							case 0x11:
								entry[i]["value"] >> *(int8_t*)(&buffer[temp_size]);
								temp_size++;
								break;
							case 0x12:
								entry[i]["value"] >> *(int16_t*)(&buffer[temp_size]);
								temp_size += 2;
								break;
							case 0x14:
								entry[i]["value"] >> *(int32_t*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 0x18:
								entry[i]["value"] >> *(int64_t*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							case 0x24:
								entry[i]["value"] >> *(float*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 0x28:
								entry[i]["value"] >> *(double*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							default:
								return 4;
						}
					}					
				}
				else return 2;
			}
			buffer[temp_size] = 0xFF;
			temp_size++;
			*out_size = temp_size;
			return 0;
		}
		for (size_t i = 0; i < entry.num_children(); i++) {
		
			entry[i]["type"] >> string_check;
			if (!string_check.compare("write") || !string_check.compare("evaluate_write")) {
				
				bool evaluate_write = false;
				if (!string_check.compare("evaluate_write"))
					evaluate_write = true;

				buffer[temp_size] = 1; // type
				temp_size++;
				buffer[temp_size] = entry[i]["address"].num_children(); // address count
				temp_size++;
				entry[i]["address"][0] >> string_check;
				buffer[temp_size] = getAddressRegion(string_check);
				temp_size++;
				for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
					entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["value_type"] >> string_check;
				uint8_t value_type = getValueType(string_check);
				buffer[temp_size] = value_type;
				temp_size++; // value_type
				if (entry[i]["value"].is_seq()) {
					buffer[temp_size] = entry[i]["value"].num_children(); //value_count
					temp_size++;
					for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
						double evaluated_value = 0;
						if (evaluate_write) {
							double FPS_TARGET = fps_target;
							double FPS_LOCK_TARGET = fps_target;
							if (fps_target >= 60)
								FPS_LOCK_TARGET = 120;
							double FRAMETIME_TARGET = 1000.0 / fps_target;
							double VSYNC_TARGET = 60 / fps_target;
							te_variable vars[] = {
								{"TruncDec", (const void*)TruncDec, TE_FUNCTION2}, /* TE_FUNCTION2 used because my_sum takes two arguments. */
								{"FPS_TARGET", &FPS_TARGET, TE_VARIABLE},
								{"FPS_LOCK_TARGET", &FPS_LOCK_TARGET, TE_VARIABLE},
								{"FRAMETIME_TARGET", &FRAMETIME_TARGET, TE_VARIABLE},
								{"VSYNC_TARGET", &VSYNC_TARGET, TE_VARIABLE}
							};
							std::string equation = "";
							entry[i]["value"][x] >> equation;
							te_expr *n = te_compile(equation.c_str(), vars, 4, 0);
							evaluated_value = te_eval(n);
							te_free(n);
						}
						switch(value_type) {
							case 1:
								if (evaluate_write)
									buffer[temp_size] = (uint8_t)evaluated_value;
								else entry[i]["value"][x] >> buffer[temp_size];
								temp_size++;
								break;
							case 2:
								if (evaluate_write)
									*(uint16_t*)(&buffer[temp_size]) = (uint16_t)evaluated_value;
								else entry[i]["value"][x] >> *(uint16_t*)(&buffer[temp_size]);
								temp_size += 2;
								break;
							case 4:
								if (evaluate_write)
									*(uint32_t*)(&buffer[temp_size]) = (uint32_t)evaluated_value;
								else entry[i]["value"][x] >> *(uint32_t*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 8:
								if (evaluate_write)
									*(uint64_t*)(&buffer[temp_size]) = (uint64_t)evaluated_value;
								else entry[i]["value"][x] >> *(uint64_t*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							case 0x11:
								if (evaluate_write)
									*(int8_t*)(&buffer[temp_size]) = (int8_t)evaluated_value;
								else entry[i]["value"][x] >> *(int8_t*)(&buffer[temp_size]);
								temp_size++;
								break;
							case 0x12:
								if (evaluate_write)
									*(int16_t*)(&buffer[temp_size]) = (int16_t)evaluated_value;
								else entry[i]["value"][x] >> *(int16_t*)(&buffer[temp_size]);
								temp_size += 2;
								break;
							case 0x14:
								if (evaluate_write)
									*(int32_t*)(&buffer[temp_size]) = (int32_t)evaluated_value;
								else entry[i]["value"][x] >> *(int32_t*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 0x18:
								if (evaluate_write)
									*(int64_t*)(&buffer[temp_size]) = (int64_t)evaluated_value;
								else entry[i]["value"][x] >> *(int64_t*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							case 0x24:
								if (evaluate_write)
									*(float*)(&buffer[temp_size]) = (float)evaluated_value;
								else entry[i]["value"][x] >> *(float*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 0x28:
								if (evaluate_write)
									*(double*)(&buffer[temp_size]) = (uint8_t)evaluated_value;
								else entry[i]["value"][x] >> *(double*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							default:
								return 4;
						}
					}
				}
				else {
					buffer[temp_size] = 1;
					temp_size++;
					double evaluated_value = 0;
					if (evaluate_write) {
						double FPS_TARGET = fps_target;
						double FRAMETIME_TARGET = 1000.0 / fps_target;
						double VSYNC_TARGET = 60 / fps_target;
						te_variable vars[] = {
							{"TruncDec", (const void*)TruncDec, TE_FUNCTION2},
							{"FPS_TARGET", &FPS_TARGET, TE_VARIABLE},
							{"FRAMETIME_TARGET", &FRAMETIME_TARGET, TE_VARIABLE},
							{"VSYNC_TARGET", &VSYNC_TARGET, TE_VARIABLE}
						};
						std::string equation = "";
						entry[i]["value"] >> equation;
						te_expr *n = te_compile(equation.c_str(), vars, 4, 0);
						evaluated_value = te_eval(n);
						te_free(n);
					}	
					switch(value_type) {
						case 1:
							if (evaluate_write)
								buffer[temp_size] = (uint8_t)evaluated_value;
							else entry[i]["value"] >> buffer[temp_size];
							temp_size++;
							break;
						case 2:
							if (evaluate_write)
								*(uint16_t*)(&buffer[temp_size]) = (uint16_t)evaluated_value;
							else entry[i]["value"] >> *(uint16_t*)(&buffer[temp_size]);
							temp_size += 2;
							break;
						case 4:
							if (evaluate_write)
								*(uint32_t*)(&buffer[temp_size]) = (uint32_t)evaluated_value;
							else entry[i]["value"] >> *(uint32_t*)(&buffer[temp_size]);
							temp_size += 4;
							break;
						case 8:
							if (evaluate_write)
								*(uint64_t*)(&buffer[temp_size]) = (uint64_t)evaluated_value;
							else entry[i]["value"] >> *(uint64_t*)(&buffer[temp_size]);
							temp_size += 8;
							break;
						case 0x11:
							if (evaluate_write)
								*(int8_t*)(&buffer[temp_size]) = (int8_t)evaluated_value;
							else entry[i]["value"] >> *(int8_t*)(&buffer[temp_size]);
							temp_size++;
							break;
						case 0x12:
							if (evaluate_write)
								*(int16_t*)(&buffer[temp_size]) = (int16_t)evaluated_value;
							else entry[i]["value"] >> *(int16_t*)(&buffer[temp_size]);
							temp_size += 2;
							break;
						case 0x14:
							if (evaluate_write)
								*(int32_t*)(&buffer[temp_size]) = (int32_t)evaluated_value;
							else entry[i]["value"] >> *(int32_t*)(&buffer[temp_size]);
							temp_size += 4;
							break;
						case 0x18:
							if (evaluate_write)
								*(int64_t*)(&buffer[temp_size]) = (int64_t)evaluated_value;
							else entry[i]["value"] >> *(int64_t*)(&buffer[temp_size]);
							temp_size += 8;
							break;
						case 0x24:
							if (evaluate_write)
								*(float*)(&buffer[temp_size]) = (float)evaluated_value;
							else entry[i]["value"] >> *(float*)(&buffer[temp_size]);
							temp_size += 4;
							break;
						case 0x28:
							if (evaluate_write)
								*(double*)(&buffer[temp_size]) = evaluated_value;
							else entry[i]["value"] >> *(double*)(&buffer[temp_size]);
							temp_size += 8;
							break;
						default:
							return 4;
					}
				}
			}
			else if (!string_check.compare("compare")) {
				buffer[temp_size] = 2;
				temp_size++;
				buffer[temp_size] = entry[i]["compare_address"].num_children();
				temp_size++;
				entry[i]["compare_address"][0] >> string_check;
				buffer[temp_size] = getAddressRegion(string_check);
				temp_size++;
				for (size_t x = 1; x < entry[i]["compare_address"].num_children(); x++) {
					entry[i]["compare_address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["compare_type"][0] >> string_check;
				buffer[temp_size] = getCompareType(string_check);
				temp_size++;
				entry[i]["compare_value_type"] >> string_check;
				buffer[temp_size] = getValueType(string_check);
				temp_size++;
				switch(getValueType(string_check)) {
					case 1:
						entry[i]["compare_value"] >> buffer[temp_size];
						temp_size++;
						break;
					case 2:
						entry[i]["compare_value"] >> *(uint16_t*)(&buffer[temp_size]);
						temp_size += 2;
						break;
					case 4:
						entry[i]["compare_value"] >> *(uint32_t*)(&buffer[temp_size]);
						temp_size += 4;
						break;
					case 8:
						entry[i]["compare_value"] >> *(uint64_t*)(&buffer[temp_size]);
						temp_size += 8;
						break;
					case 0x11:
						entry[i]["compare_value"] >> *(int8_t*)(&buffer[temp_size]);
						temp_size++;
						break;
					case 0x12:
						entry[i]["compare_value"] >> *(int16_t*)(&buffer[temp_size]);
						temp_size += 2;
						break;
					case 0x14:
						entry[i]["compare_value"] >> *(int32_t*)(&buffer[temp_size]);
						temp_size += 4;
						break;
					case 0x18:
						entry[i]["compare_value"] >> *(int64_t*)(&buffer[temp_size]);
						temp_size += 8;
						break;
					case 0x24:
						entry[i]["compare_value"] >> *(float*)(&buffer[temp_size]);
						temp_size += 4;
						break;
					case 0x28:
						entry[i]["compare_value"] >> *(double*)(&buffer[temp_size]);
						temp_size += 8;
						break;
					default:
						return 4;
				}
				buffer[temp_size] = entry[i]["address"].num_children(); // address count
				temp_size++;
				entry[i]["address"][0] >> string_check;
				buffer[temp_size] = getAddressRegion(string_check);
				temp_size++;
				for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
					entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["value_type"] >> string_check;
				uint8_t value_type = getValueType(string_check);
				buffer[temp_size] = value_type;
				temp_size++; // value_type
				if (entry[i]["value"].is_seq()) {
					buffer[temp_size] = entry[i]["value"].num_children(); //value_count
					temp_size++;
					for (size_t x = 0; x < entry[i]["value"].num_children(); x++) {
						switch(value_type) {
							case 1:
								entry[i]["value"][x] >> buffer[temp_size];
								temp_size++;
								break;
							case 2:
								entry[i]["value"][x] >> *(uint16_t*)(&buffer[temp_size]);
								temp_size += 2;
								break;
							case 4:
								entry[i]["value"][x] >> *(uint32_t*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 8:
								entry[i]["value"][x] >> *(uint64_t*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							case 0x11:
								entry[i]["value"][x] >> *(int8_t*)(&buffer[temp_size]);
								temp_size++;
								break;
							case 0x12:
								entry[i]["value"][x] >> *(int16_t*)(&buffer[temp_size]);
								temp_size += 2;
								break;
							case 0x14:
								entry[i]["value"][x] >> *(int32_t*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 0x18:
								entry[i]["value"][x] >> *(int64_t*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							case 0x24:
								entry[i]["value"][x] >> *(float*)(&buffer[temp_size]);
								temp_size += 4;
								break;
							case 0x28:
								entry[i]["value"][x] >> *(double*)(&buffer[temp_size]);
								temp_size += 8;
								break;
							default:
								return 4;
						}
					}
				}
				else {
					buffer[temp_size] = 1;
					temp_size++;
					switch(value_type) {
						case 1:
							entry[i]["value"] >> buffer[temp_size];
							temp_size++;
							break;
						case 2:
							entry[i]["value"] >> *(uint16_t*)(&buffer[temp_size]);
							temp_size += 2;
							break;
						case 4:
							entry[i]["value"] >> *(uint32_t*)(&buffer[temp_size]);
							temp_size += 4;
							break;
						case 8:
							entry[i]["value"] >> *(uint64_t*)(&buffer[temp_size]);
							temp_size += 8;
							break;
						case 0x11:
							entry[i]["value"] >> *(int8_t*)(&buffer[temp_size]);
							temp_size++;
							break;
						case 0x12:
							entry[i]["value"] >> *(int16_t*)(&buffer[temp_size]);
							temp_size += 2;
							break;
						case 0x14:
							entry[i]["value"] >> *(int32_t*)(&buffer[temp_size]);
							temp_size += 4;
							break;
						case 0x18:
							entry[i]["value"] >> *(int64_t*)(&buffer[temp_size]);
							temp_size += 8;
							break;
						case 0x24:
							entry[i]["value"] >> *(float*)(&buffer[temp_size]);
							temp_size += 4;
							break;
						case 0x28:
							entry[i]["value"] >> *(double*)(&buffer[temp_size]);
							temp_size += 8;
							break;
						default:
							return 4;
					}
				}
			}
			else if (!string_check.compare("block")) {
				buffer[temp_size] = 3;
				temp_size++;
				entry[i]["what"] >> string_check;
				if (!string_check.compare("timing")) {
					buffer[temp_size] = 1;
					temp_size++;
				}
			}
			else return 2;
		}
		buffer[temp_size] = 0xFF;
		temp_size++;
		*out_size = temp_size;
		return 0;
	}

	template <typename T>
	Result NOINLINE processEntry(T entry, bool masterWrite = false, double fps_target = 0) {
		
		size_t temp_size = 0;
		size_t old_temp_size = 0;
		
		temp_size = calculateSize(entry, masterWrite);

		uint8_t* buffer = (uint8_t*)calloc(temp_size, sizeof(uint8_t));
		old_temp_size = temp_size;
		temp_size = 0;

		Result rc = processEntryImpl(entry, buffer, &temp_size, masterWrite, fps_target);
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

	Result createPatch(const char* path) {
		bool unsafeCheck = false;

		char lockMagic[] = "LOCK";
		tree["unsafeCheck"] >> unsafeCheck;
		uint8_t flags[4] = {1, 0, 0, unsafeCheck};
		
		for (size_t i = 0; i < std::size(entries); i++) {
			Result ret = -1;
			if (ALL_FPS) {
				size_t root_id = tree.root_id();
				if (tree.find_child(root_id, entries[i]) == c4::yml::NONE)
					ret = processEntry(tree["ALL_FPS"], false, atoi(entries[i]));
				else {
					for (size_t x = 0; i < tree["ALL_FPS"].num_children(); x++) {
						size_t temp = tree[entries[i]].num_children();
						tree[entries[i]].append_child();
						tree[entries[i]][temp] = tree["ALL_FPS"][x];
					}
					ret = processEntry(tree[entries[i]], false, atoi(entries[i]));
				}
			}
			else ret = processEntry(tree[entries[i]], false, atoi(entries[i]));
			if (R_FAILED(ret)) {
				freeBuffers();
				return ret;
			}
		}

		if (gen == 2) {
			Result ret = processEntry(tree["MASTER_WRITE"], true, 0);
			if (R_FAILED(ret)) {
				freeBuffers();
				return ret;
			}
			flags[0] = 2;
		}

		uint32_t base_offset = 0x30;
		if (gen == 2) {
			base_offset += 4;
		}
		uint8_t entries_count = 10;
		if (gen == 2) {
			entries_count++;
		}
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

	Result readConfig(char* path) {
		FILE* config = fopen(path, "r");
		if (!config)
			return 0x202;
		fread(&configBuffer, 1, 32768, config);
		fclose(config);
		strcat(&configBuffer[0], "\n");

		tree = ryml::parse_in_place(configBuffer);
		size_t root_id = tree.root_id();

		if (!tree.is_map(root_id))
			return 4;
		if (tree.find_child(root_id, "unsafeCheck") == c4::yml::NONE)
			return 1;
		if (!tree["unsafeCheck"].is_keyval())
			return 2;
		if (tree.find_child(root_id, "MASTER_WRITE") != c4::yml::NONE)
			gen = 2;
		if (tree.find_child(root_id, "ALL_FPS") != c4::yml::NONE)
			ALL_FPS = true;

		if (!ALL_FPS) {
			Result base_err = 0x15;
			for (size_t i = 0; i < std::size(entries); i++) {
				if (tree.find_child(root_id, entries[i]) == c4::yml::NONE)
					return base_err + (5 * i);
				if (!tree[entries[i]].is_seq())
					return base_err + 0x100 + (5 * i);
			}
		}
		return 0;
	}
}