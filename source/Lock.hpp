#pragma once
#define NOINLINE __attribute__ ((noinline))

#include "rapidyaml/ryml.hpp"
#include "c4/std/string.hpp"
#include <cmath>

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
		else if (!value_type.compare("refresh_rate"))
			return 0x38;
		else return 0;
	}

	size_t NOINLINE getTypeSize(std::string value_type) {
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
	size_t NOINLINE calculateSize(T entry, bool masterWrite = false, bool compiled = false) {
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
				if (!string_check.compare("write") || compiled) {
					entry[i]["value_type"] >> string_check;
					if (entry[i]["value"].is_seq()) {
						temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
					}
					else temp_size += getTypeSize(string_check);
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
				entry[i]["compare_value_type"] >> string_check;
				temp_size += getTypeSize(string_check);
				temp_size++; // address count
				temp_size += ((entry[i]["address"].num_children() - 1) * 4) + 1; // address array
				temp_size++; // value_type
				temp_size++; // value count
				if (!evaluate_compare || compiled) {
					entry[i]["value_type"] >> string_check;
					if (entry[i]["value"].is_seq()) {
						temp_size += (getTypeSize(string_check) * entry[i]["value"].num_children());
					}
					else temp_size += getTypeSize(string_check);
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
			else return 2;
		}
		
		temp_size++;
		return temp_size;
	}

	template <typename T>
	Result NOINLINE processEntryImpl(T entry, uint8_t* buffer, size_t* out_size, bool masterWrite = false) {
		std::string string_check = "";
		size_t temp_size = 0;
		if (masterWrite) {
			for (size_t i = 0; i < entry.num_children(); i++) {
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
				else return 2;
			}
			buffer[temp_size++] = 0xFF;
			*out_size = temp_size;
			return 0;
		}
		for (size_t i = 0; i < entry.num_children(); i++) {
		
			entry[i]["type"] >> string_check;
			if (!string_check.compare("write") || !string_check.compare("evaluate_write")) {
				
				bool evaluate_write = false;
				if (!string_check.compare("evaluate_write"))
					evaluate_write = true;

				buffer[temp_size++] = (evaluate_write ? 0x81 : 1); // type
				buffer[temp_size++] = entry[i]["address"].num_children(); // address count
				entry[i]["address"][0] >> string_check;
				buffer[temp_size++] = getAddressRegion(string_check);
				for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
					entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["value_type"] >> string_check;
				uint8_t value_type = getValueType(string_check);
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
				
				buffer[temp_size++] = (evaluate_write ? 0x82 : 2);
				buffer[temp_size++] = entry[i]["compare_address"].num_children();
				entry[i]["compare_address"][0] >> string_check;
				buffer[temp_size++] = getAddressRegion(string_check);
				for (size_t x = 1; x < entry[i]["compare_address"].num_children(); x++) {
					entry[i]["compare_address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["compare_type"][0] >> string_check;
				buffer[temp_size++] = getCompareType(string_check);
				entry[i]["compare_value_type"] >> string_check;
				buffer[temp_size++] = getValueType(string_check);

				Result rc = writeEntryTo(entry[i]["compare_value"], buffer, &temp_size, getValueType(string_check));
				if (R_FAILED(rc)) return rc;

				buffer[temp_size++] = entry[i]["address"].num_children(); // address count
				entry[i]["address"][0] >> string_check;
				buffer[temp_size++] = getAddressRegion(string_check);
				for (size_t x = 1; x < entry[i]["address"].num_children(); x++) {
					entry[i]["address"][x] >> *(int32_t*)(&buffer[temp_size]);
					temp_size += 4;
				}
				entry[i]["value_type"] >> string_check;
				uint8_t value_type = getValueType(string_check);
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
			else return 2;
		}
		buffer[temp_size++] = 0xFF;
		*out_size = temp_size;
		return 0;
	}

	template <typename T>
	Result NOINLINE processEntry(T entry, bool masterWrite = false) {
		
		size_t temp_size = 0;
		size_t old_temp_size = 0;
		
		temp_size = calculateSize(entry, masterWrite);

		uint8_t* buffer = (uint8_t*)calloc(temp_size, sizeof(uint8_t));
		old_temp_size = temp_size;
		temp_size = 0;

		Result rc = processEntryImpl(entry, buffer, &temp_size, masterWrite);
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
		uint8_t compiledSize = (uint8_t)sqrt(calculateSize(tree["ALL_FPS"], false, true) + 0x10) + 1;
		uint8_t flags[4] = {gen, master_write, compiledSize, unsafeCheck};

		Result ret = -1;
		
		ret = processEntry(tree["ALL_FPS"], false);

		if (R_FAILED(ret)) {
			freeBuffers();
			return ret;
		}
		
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