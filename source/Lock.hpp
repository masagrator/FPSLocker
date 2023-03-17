#pragma once
#include "rapidyaml/ryml.hpp"
#include "c4/std/string.hpp"

namespace LOCK {

	const char entries[10][6] = {"15FPS", "20FPS", "25FPS", "30FPS", "35FPS", "40FPS", "45FPS", "50FPS", "55FPS", "60FPS"};
	ryml::Tree tree;
	char configBuffer[32769] = "";

	struct buffer_data {
		size_t size;
		void* buffer_ptr;
	};

	void freeBuffers(std::vector<buffer_data*> buffers) {
		for (size_t i = (buffers.size() - 1); i >= 0; i--) {
			free(buffers[i] -> buffer_ptr);
			buffers.pop_back();
		}
	}

	uint8_t getCompareType(std::string compare_type) {
		if (!compare_type.compare(">")) {
			return 1;
		}
		else if (!compare_type.compare(">=")) {
			return 2;
		}
		else if (!compare_type.compare("<")) {
			return 3;
		}
		else if (!compare_type.compare("<=")) {
			return 4;
		}
		else if (!compare_type.compare("==")) {
			return 5;
		}
		else if (!compare_type.compare("!=")) {
			return 6;
		}
		else return 0;
	}

	uint8_t getAddressRegion(std::string region) {
		if (!region.compare("MAIN")) {
			return 0x1;
		}
		else if (!region.compare("HEAP")) {
			return 0x2;
		}
		else if (!region.compare("ALIAS")) {
			return 0x3;	
		}
		else return 0;
	}

	uint8_t getValueType(std::string value_type) {
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

	size_t getTypeSize(std::string value_type) {
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
	template <typename T>
	Result processEntry(T entry, std::vector<buffer_data*> buffers) {
		
		size_t temp_size = 0;
		size_t old_temp_size = 0;
		std::string string_check = "";

		//calculate bytes size
		
		for (size_t i = 0; i < entry.num_children(); i++) {

			entry[i]["type"] >> string_check;
			
			temp_size += 1;
			if (!string_check.compare("write")) {
				temp_size += 1; // address count
				temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				temp_size += 1; // value_type
				temp_size += 1; // value count
				entry[i]["value_type"] >> string_check;
				if (entry[i]["value"].is_seq()) {
					temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
				}
				else temp_size += getTypeSize(string_check);
				
			}
			else if (!string_check.compare("compare")) {
				temp_size += ((entry[i]["compare_address"].num_children() - 1) * 4) + 1; // address array
				temp_size += 1; // compare_type
				temp_size += 1; // compare_value_type
				entry[i]["compare_value_type"] >> string_check;
				temp_size += getTypeSize(string_check);
				temp_size += 1; // address count
				temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				temp_size += 1; // value_type
				temp_size += 1; // value count
				entry[i]["value_type"] >> string_check;
				if (entry[i]["value"].is_seq()) {
					temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
				}
				else temp_size += getTypeSize(string_check);
			}
			else if (!string_check.compare("block")) {
				temp_size += 1;
			}
			else return 2;
		}
		
		temp_size += 1;
		
		uint8_t* buffer = (uint8_t*)calloc(temp_size, sizeof(uint8_t));
		old_temp_size = temp_size;
		temp_size = 0;

		//calculate bytes size
		for (size_t i = 0; i < entry.num_children(); i++) {
		
			entry[i]["type"] >> string_check;
			if (!string_check.compare("write")) {
				
				buffer[temp_size] = 1; // type
				temp_size += 1;
				buffer[temp_size] = entry[i]["address"].num_children(); // address count
				temp_size += 1;
				entry[i]["address"][0] >> string_check;
				buffer[temp_size] = getAddressRegion(string_check);
				temp_size += 1;
				for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
					entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["value_type"] >> string_check;
				uint8_t value_type = getValueType(string_check);
				buffer[temp_size] = value_type;
				temp_size += 1; // value_type
				if (entry[i]["value"].is_seq()) {
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
								temp_size += 1;
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
							temp_size += 1;
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
			else if (!string_check.compare("compare")) {
				buffer[temp_size] = 2;
				temp_size += 1;
				buffer[temp_size] = entry[i]["compare_address"].num_children();
				temp_size += 1;
				entry[i]["compare_address"][0] >> string_check;
				buffer[temp_size] = getAddressRegion(string_check);
				temp_size += 1;
				for (size_t x = 1; x < entry[i]["compare_address"].num_children(); x++) {
					entry[i]["compare_address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["compare_type"][0] >> string_check;
				buffer[temp_size] = getCompareType(string_check);
				temp_size += 1;
				entry[i]["compare_value_type"] >> string_check;
				buffer[temp_size] = getValueType(string_check);
				temp_size += 1;
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
						temp_size += 1;
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
				temp_size += 1;
				entry[i]["address"][0] >> string_check;
				buffer[temp_size] = getAddressRegion(string_check);
				temp_size += 1;
				for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
					entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["value_type"] >> string_check;
				uint8_t value_type = getValueType(string_check);
				buffer[temp_size] = value_type;
				temp_size += 1; // value_type
				if (entry[i]["value"].is_seq()) {
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
								temp_size += 1;
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
							temp_size += 1;
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
				temp_size += 1;
				entry[i]["what"] >> string_check;
				if (!string_check.compare("timing")) {
					buffer[temp_size] = 1;
					temp_size += 1;
				}
			}
			else return 2;
		}
		buffer[temp_size] = 0xFF;
		temp_size += 1;
		if (old_temp_size != temp_size)
			return 10;
		buffer_data* new_struct = (buffer_data*)calloc(sizeof(buffer_data), 1);
		new_struct -> size = temp_size;
		new_struct -> buffer_ptr = &buffer[0];
		buffers.push_back(new_struct);
		return 0;
	}

	Result readConfig(char* path) {
		FILE* config = fopen(path, "r");
		if (!config)
			return 0x202;
		fread(&configBuffer, 1, 32768, config);
		fclose(config);

		tree = ryml::parse_in_place(configBuffer);
		size_t root_id = tree.root_id();
		if (!tree.is_map(root_id))
			return 4;
		if (strncmp(&tree["unsafeCheck"].key()[0], "unsafeCheck", 11))
			return 1;
		if (!tree["unsafeCheck"].is_keyval())
			return 2;

		Result base_err = 0x15;
		for (size_t i = 0; i < std::size(entries); i++) {
			if (strncmp(&tree[entries[i]].key()[0], entries[i], 5))
				return base_err + (5 * i);
			if (!tree[entries[i]].is_seq())
				return base_err + 0x100 + (5 * i);
		}
		return 0;
	}

	Result createPatch(const char* path) {
		bool unsafeCheck = false;

		char lockMagic[] = "LOCK";
		uint8_t flags[3] = {1, 0, 0};
		tree["unsafeCheck"] >> unsafeCheck;

		std::vector<buffer_data*> buffers;
		
		for (size_t i = 0; i < std::size(entries); i++) {
			Result ret = processEntry(tree[entries[i]], buffers);
			if (R_FAILED(ret)) {
				freeBuffers(buffers);
				return ret;
			}
		}

		FILE* file = fopen(path, "wb");
		if (!file)
			return 0x202;
		fwrite(&lockMagic[0], 4, 1, file);
		fwrite(&flags[0], 3, 1, file);
		fwrite(&unsafeCheck, 1, 1, file);
		fclose(file);
		//remove(path);
		return 1;
	}
}