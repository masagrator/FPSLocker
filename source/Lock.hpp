#pragma once
#include "rapidyaml/ryml.hpp"

namespace LOCK {
	ryml::Tree tree;
	char configBuffer[32769] = "";

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
		if (strncmp(&tree["15FPS"].key()[0], "15FPS", 5))
			return 0x15;
		if (!tree["15FPS"].is_seq())
			return 0x115;
		if (strncmp(&tree["20FPS"].key()[0], "20FPS", 5))
			return 0x20;
		if (!tree["20FPS"].is_seq())
			return 0x120;
		if (strncmp(&tree["25FPS"].key()[0], "25FPS", 5))
			return 0x25;
		if (!tree["25FPS"].is_seq())
			return 0x125;
		if (strncmp(&tree["30FPS"].key()[0], "30FPS", 5))
			return 0x30;
		if (!tree["30FPS"].is_seq())
			return 0x130;
		if (strncmp(&tree["35FPS"].key()[0], "35FPS", 5))
			return 0x35;
		if (!tree["35FPS"].is_seq())
			return 0x135;
		if (strncmp(&tree["40FPS"].key()[0], "40FPS", 5))
			return 0x40;
		if (!tree["40FPS"].is_seq())
			return 0x140;
		if (strncmp(&tree["45FPS"].key()[0], "45FPS", 5))
			return 0x45;
		if (!tree["45FPS"].is_seq())
			return 0x145;
		if (strncmp(&tree["50FPS"].key()[0], "50FPS", 5))
			return 0x50;
		if (!tree["50FPS"].is_seq())
			return 0x150;
		if (strncmp(&tree["55FPS"].key()[0], "55FPS", 5))
			return 0x55;
		if (!tree["55FPS"].is_seq())
			return 0x155;
		if (strncmp(&tree["60FPS"].key()[0], "60FPS", 5))
			return 0x60;
		if (!tree["60FPS"].is_seq())
			return 0x160;
		return 0;
	}
}