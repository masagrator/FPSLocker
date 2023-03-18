#pragma once
#define NOINLINE __attribute__ ((noinline))

#include "rapidyaml/ryml.hpp"
#include "c4/std/string.hpp"

namespace LOCK {

	const char entries[10][6] = {"15FPS", "20FPS", "25FPS", "30FPS", "35FPS", "40FPS", "45FPS", "50FPS", "55FPS", "60FPS"};
	ryml::Tree tree;
	char configBuffer[32770] = "";

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

	template <typename T>
	size_t NOINLINE calculateSize(T entry) {
		std::string string_check = "";
		size_t temp_size = 0;

		//calculate bytes size
		
		for (size_t i = 0; i < entry.num_children(); i++) {

			entry[i]["type"] >> string_check;
			
			temp_size++;
			if (!string_check.compare("write")) {
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
	Result NOINLINE processEntryImpl(T entry, uint8_t* buffer, size_t* out_size) {
		std::string string_check = "";
		size_t temp_size = 0;

		for (size_t i = 0; i < entry.num_children(); i++) {
		
			entry[i]["type"] >> string_check;
			if (!string_check.compare("write")) {
				
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
	Result NOINLINE processEntry(T entry) {
		
		size_t temp_size = 0;
		size_t old_temp_size = 0;
		
		temp_size = calculateSize(entry);

		uint8_t* buffer = (uint8_t*)calloc(temp_size, sizeof(uint8_t));
		old_temp_size = temp_size;
		temp_size = 0;

		Result rc = processEntryImpl(entry, buffer, &temp_size);
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
		uint8_t flags[3] = {1, 0, 0};
		tree["unsafeCheck"] >> unsafeCheck;
		
		for (size_t i = 0; i < std::size(entries); i++) {
			Result ret = processEntry(tree[entries[i]]);
			if (R_FAILED(ret)) {
				freeBuffers();
				return ret;
			}
		}

		uint32_t base_offset = 0x30;
		uint32_t offsets[10] = {0};
		offsets[0] = base_offset;
		base_offset += buffers[0] -> size;
		uint8_t IDs[10] = {0};
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

		if (!buffers.size())
			return 0x18;
		FILE* file = fopen(path, "wb");
		if (!file)
			return 0x202;
		fwrite(&lockMagic[0], 4, 1, file);
		fwrite(&flags[0], 3, 1, file);
		fwrite(&unsafeCheck, 1, 1, file);
		for (size_t i = 0; i < std::size(offsets); i++) {
			fwrite(&offsets[i], 4, 1, file);
		}
		for (size_t i = 0; i < buffers.size(); i++) 
			if (IDs[i] == i)
				fwrite(buffers[i] -> buffer_ptr, buffers[i] -> size, 1, file);

		fclose(file);
		freeBuffers();
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
}