
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <stdlib.h>
#include <cstddef>
#include <vector>

#include "parse-cta-block.hpp"

#define PACKED     __attribute__((packed))
//#define DEBUG

/// BlockType
typedef enum PACKED {
	SetSysBlockType_Audio           = 1,
	SetSysBlockType_Video           = 2,
	SetSysBlockType_VendorSpecific  = 3,
	SetSysBlockType_Speaker         = 4,
} SetSysBlockType;

typedef struct {
	uint16_t pixel_clock;                                ///< In 10 kHz units.
	uint8_t horizontal_active_pixels_lsb;
	uint8_t horizontal_blanking_pixels_lsb;
	uint8_t horizontal_blanking_pixels_msb : 4;
	uint8_t horizontal_active_pixels_msb : 4;
	uint8_t vertical_active_lines_lsb;
	uint8_t vertical_blanking_lines_lsb;
	uint8_t vertical_blanking_lines_msb : 4;
	uint8_t vertical_active_lines_msb : 4;
	uint8_t horizontal_sync_offset_pixels_lsb;
	uint8_t horizontal_sync_pulse_width_pixels_lsb;
	uint8_t vertical_sync_pulse_width_lines_lsb : 4;
	uint8_t vertical_sync_offset_lines_lsb : 4;
	uint8_t vertical_sync_pulse_width_lines_msb : 2;
	uint8_t vertical_sync_offset_lines_msb : 2;
	uint8_t horizontal_sync_pulse_width_pixels_msb : 2;
	uint8_t horizontal_sync_offset_pixels_msb : 2;
	uint8_t horizontal_image_size_mm_lsb;
	uint8_t vertical_image_size_mm_lsb;
	uint8_t vertical_image_size_mm_msb : 4;
	uint8_t horizontal_image_size_mm_msb : 4;
	uint8_t horizontal_border_pixels;
	uint8_t vertical_border_lines;
	uint8_t features_bitmap_0 : 1;
	uint8_t features_bitmap_1 : 1;
	uint8_t features_bitmap_2 : 1;
	uint8_t features_bitmap_34 : 2;
	uint8_t features_bitmap_56 : 2;
	uint8_t interlaced : 1;
} SetSysModeLine;

typedef struct {
	struct {
		uint8_t size : 5;
		SetSysBlockType block_type : 3;
		struct {
			uint8_t svd_index : 7;
			uint8_t native_flag : 1;
		} svd[0xC];
	} PACKED video;
	struct {
		uint8_t size : 5;
		SetSysBlockType block_type : 3;
		uint8_t channel_count : 3;
		uint8_t format_code : 4;
		uint8_t padding1 : 1;
		uint8_t sampling_rates_bitmap;
		uint8_t bitrate;
	} PACKED audio;
	struct {
		uint8_t size : 5;
		SetSysBlockType block_type : 3;
		uint8_t ieee_registration_id[3];
		uint16_t source_physical_address;
		uint8_t mode_bitmap;
		uint8_t max_tmds_frequency;
		uint8_t latency_bitmap;
	} PACKED vendor_specific;
	uint8_t padding[2];
} SetSysDataBlock;

static_assert(sizeof(SetSysDataBlock) == 28);

/// Edid
typedef struct {
	uint8_t pattern[8];                          ///< Fixed pattern 00 FF FF FF FF FF FF 00.
	uint16_t pnp_id;                             ///< Big-endian set of 3 5-bit values representing letters, 1 = A .. 26 = Z.
	uint16_t product_code;
	uint32_t serial_number;
	uint8_t manufacture_week;
	uint8_t manufacture_year;                    ///< Real value is val - 10.
	uint8_t edid_version;
	uint8_t edid_revision;
	uint8_t video_input_parameters_bitmap;
	uint8_t display_width;
	uint8_t display_height;
	uint8_t display_gamma;
	uint8_t supported_features_bitmap;
	struct {
		uint8_t green_y_lsb : 2;
		uint8_t green_x_lsb : 2;
		uint8_t red_y_lsb : 2;
		uint8_t red_x_lsb : 2;
		uint8_t blue_lsb : 4;
		uint8_t white_lsb : 4;
		uint8_t red_x_msb;
		uint8_t red_y_msb;
		uint8_t green_x_msb;
		uint8_t green_y_msb;
		uint8_t blue_x_msb;
		uint8_t blue_y_msb;
		uint8_t white_x_msb;
		uint8_t white_y_msb;
	} chromaticity;
	uint8_t timing_bitmap[3];
	struct {
		uint8_t x_resolution;                    ///< Real value is (val + 31) * 8 pixels.
		uint8_t vertical_frequency : 6;          ///< Real value is val + 60 Hz.
		uint8_t aspect_ratio : 2;                ///< 0 = 16:10, 1 = 4:3, 2 = 5:4, 3 = 16:9.
	} timing_info[8];
	SetSysModeLine timing_descriptor[2];
	struct {
		uint16_t display_descriptor_zero;
		uint8_t padding1;
		uint8_t descriptor_type;
		uint8_t padding2;
		char name[0xD];
	} display_descriptor_name;
	struct {
		uint16_t display_descriptor_zero;
		uint8_t padding1;
		uint8_t descriptor_type;
		uint8_t range_limit_offsets;
		uint8_t vertical_field_rate_min;
		uint8_t vertical_field_rate_max;
		uint8_t horizontal_line_rate_min;
		uint8_t horizontal_line_rate_max;
		uint8_t pixel_clock_rate_max;            ///< Rounded up to multiples of 10 MHz.
		uint8_t extended_timing_info;
		uint8_t padding[7];
	} display_descriptor_range_limits;
	uint8_t extension_count;                     ///< Always 1.
	uint8_t checksum;                            ///< Sum of all 128 bytes should equal 0 mod 256.
	///< Extended data.
	uint8_t extension_tag;                       ///< Always 2 = CEA EDID timing extension.
	uint8_t revision;
	uint8_t dtd_start;
	uint8_t native_dtd_count : 4;
	uint8_t native_dtd_feature_bitmap : 4;
	SetSysDataBlock data_block;
	SetSysModeLine extended_timing_descriptor[5];
	uint8_t padding[5];
	uint8_t extended_checksum;                   ///< Sum of 128 extended bytes should equal 0 mod 256.
	uint8_t data2[0x80];                         ///< [13.0.0+]
	uint8_t data3[0x80];                         ///< [13.0.0+]
} SetSysEdid;

static_assert(sizeof(SetSysEdid) == 0x200);

typedef struct {
	uint8_t x_resolution;                    ///< Real value is (val + 31) * 8 pixels.
	uint8_t vertical_frequency : 6;          ///< Real value is val + 60 Hz.
	uint8_t aspect_ratio : 2;                ///< 0 = 16:10, 1 = 4:3, 2 = 5:4, 3 = 16:9.
} timing_info;

struct DisplayData {
	uint32_t pixelClockkHz;
	uint16_t width;
	uint16_t height;
	float refreshRate;
	uint16_t widthFrontPorch;
	uint16_t heightFrontPorch;
	uint16_t widthSync;
	uint16_t heightSync;
	uint16_t widthBackPorch;
	uint16_t heightBackPorch;
};

std::vector<DisplayData> EdidData;

float parseEdid(unsigned char* edid) {
	uint8_t magic[8] = {0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0};
	if (memcmp(magic, edid, 8)) {
		#ifdef DEBUG
		printf("WRONG MAGIC! %d\n", memcmp(magic, edid, 8));
		#endif
		return 0;
	}
	SetSysModeLine* timingd = (SetSysModeLine*)calloc(sizeof(SetSysModeLine), 2);
	memcpy(timingd, &edid[offsetof(SetSysEdid, timing_descriptor)], sizeof(SetSysModeLine)*2);
	float highestRefreshRate = 0;
	for (size_t i = 0; i < 2; i++) {
		SetSysModeLine td = timingd[i];
		uint32_t width = (uint32_t)td.horizontal_active_pixels_msb << 8 | td.horizontal_active_pixels_lsb;
		uint32_t height = (uint32_t)td.vertical_active_lines_msb << 8 | td.vertical_active_lines_lsb;
		uint32_t h_total = width + ((uint32_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb);
		uint32_t v_total = height + ((uint32_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb);
		float refreshRate = ((float)(td.pixel_clock * 10000) / (float)(h_total * v_total));
		#ifdef DEBUG
		if (td.pixel_clock) printf("Res: %dx%d, pixel clock: %d, refresh rate: %0.4f\n", width, height, td.pixel_clock * 10, refreshRate);
		#endif
		if (!td.interlaced && refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
		if (!td.interlaced && ((width == 1280 && height == 720) || (width == 1920 && height == 1080)) && (refreshRate > 60)) {
			uint16_t widthSync = (uint16_t)td.horizontal_sync_pulse_width_pixels_msb << 8 | td.horizontal_sync_pulse_width_pixels_lsb;
			uint16_t heightSync = (uint16_t)td.vertical_sync_pulse_width_lines_msb << 8 | td.vertical_sync_pulse_width_lines_lsb;
			uint16_t widthFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
			uint16_t heightFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
			EdidData.push_back({
				.pixelClockkHz = (uint32_t)td.pixel_clock * 10,
				.width = (uint16_t)width,
				.height = (uint16_t)height,
				.refreshRate = refreshRate,
				.widthFrontPorch = widthFrontPorch,
				.heightFrontPorch = heightFrontPorch,
				.widthSync = widthSync,
				.heightSync = heightSync,
				.widthBackPorch = (uint16_t)(((uint16_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb) - (widthSync + widthFrontPorch)),
				.heightBackPorch = (uint16_t)(((uint16_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb) - (heightSync + heightFrontPorch))
			});
		}
	}
	#ifdef DEBUG
	printf("\n");
	#endif
	uint8_t extension_count = 0;
	memcpy(&extension_count, &edid[offsetof(SetSysEdid, extension_count)], 1);
	if (extension_count != 1) {
		#ifdef DEBUG
		printf("More than one extension count!\n");
		#endif
	}
	uint8_t extension_type = 0;
	memcpy(&extension_type, &edid[offsetof(SetSysEdid, extension_tag)], 1);
	if (extension_type != 2) {
		#ifdef DEBUG
		printf("Wrong extension type!\n");
		#endif
		return 0;
	}
	uint8_t dtd_start = 0;
	memcpy(&dtd_start, &edid[offsetof(SetSysEdid, dtd_start)], 1);
	if (dtd_start) {
		uint8_t native_dtd_count = 0;
		memcpy(&native_dtd_count, &edid[offsetof(SetSysEdid, dtd_start) + 1], 1);
		native_dtd_count &= 0b1111;
		SetSysDataBlock* data = (SetSysDataBlock*)calloc(sizeof(SetSysDataBlock), 1);
		size_t offset = offsetof(SetSysEdid, data_block);
		memcpy(data, &edid[offset], sizeof(SetSysDataBlock));
		if (data -> video.block_type != 2) {
			while (offset < (size_t)offsetof(SetSysEdid, extension_tag)+dtd_start) {
				offset += 1;
				offset += data -> video.size;
				memcpy(data, &edid[offset], sizeof(SetSysDataBlock));
				if (data -> video.block_type == 2) break;
			}
		}
		if (data -> video.block_type == 2) {
			for (size_t i = 0; i < data -> video.size; i++) {
				const timings* timing = find_vic_id(data -> video.svd[i].svd_index);
				double refreshRate = (double)(timing->pixclk_khz * 1000) / ((timing->hact + timing->hfp + timing->hsync + timing->hbp) * ((timing->vact / (timing->interlaced ? 2 : 1)) + timing->vfp + timing->vsync + timing->vbp));
				#ifdef DEBUG
				printf("VIC: %u%s, Res: %ux%u%s, refresh rate: %.4f\n", data -> video.svd[i].svd_index, (data -> video.svd[i].native_flag ? " (native)" : ""), timing->hact, timing->vact, (timing->interlaced ? "i" : ""), refreshRate);
				#endif
				if (!timing->interlaced && refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
				if (!timing->interlaced && ((timing->hact == 1280 && timing->vact == 720) || (timing->hact == 1920 && timing->vact == 1080)) && (refreshRate > 60)) {
					EdidData.push_back({
						.pixelClockkHz = timing->pixclk_khz,
						.width = (uint16_t)timing->hact,
						.height = (uint16_t)timing->vact,
						.refreshRate = (float)refreshRate,
						.widthFrontPorch = (uint16_t)timing->vfp,
						.heightFrontPorch = (uint16_t)timing->hfp,
						.widthSync = (uint16_t)timing->hsync,
						.heightSync = (uint16_t)timing->vsync,
						.widthBackPorch = (uint16_t)timing->hbp,
						.heightBackPorch = (uint16_t)timing->vbp
					});
				}
			}
			#ifdef DEBUG
			printf("\n");
			#endif
		}
		free(data);
		SetSysModeLine* modeline = (SetSysModeLine*)calloc(sizeof(SetSysModeLine), 5);
		memcpy(modeline, &edid[offsetof(SetSysEdid, extension_tag)+dtd_start], sizeof(SetSysModeLine) * 5);
		for (size_t i = 0; i < 5; i++) {
			SetSysModeLine td = modeline[i];
			uint32_t width = (uint32_t)td.horizontal_active_pixels_msb << 8 | td.horizontal_active_pixels_lsb;
			uint32_t height = (uint32_t)td.vertical_active_lines_msb << 8 | td.vertical_active_lines_lsb;
			uint32_t h_total = width + ((uint32_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb);
			uint32_t v_total = height + ((uint32_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb);
			float refreshRate = ((float)(td.pixel_clock * 10000) / (float)(h_total * v_total));
			#ifdef DEBUG
			if (td.pixel_clock) printf("Res: %dx%d, pixel clock: %d, refresh rate: %0.4f\n", width, height, td.pixel_clock * 10, refreshRate);
			#endif
			if (!td.interlaced && refreshRate > highestRefreshRate) highestRefreshRate = refreshRate;
			if (!td.interlaced && ((width == 1280 && height == 720) || (width == 1920 && height == 1080)) && (refreshRate > 60)) {
				uint16_t widthSync = (uint16_t)td.horizontal_sync_pulse_width_pixels_msb << 8 | td.horizontal_sync_pulse_width_pixels_lsb;
				uint16_t heightSync = (uint16_t)td.vertical_sync_pulse_width_lines_msb << 8 | td.vertical_sync_pulse_width_lines_lsb;
				uint16_t widthFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
				uint16_t heightFrontPorch = (uint16_t)td.horizontal_sync_offset_pixels_msb << 8 | td.horizontal_sync_offset_pixels_lsb;
				EdidData.push_back({
				.pixelClockkHz = (uint32_t)td.pixel_clock * 10,
				.width = (uint16_t)width,
				.height = (uint16_t)height,
				.refreshRate = refreshRate,
				.widthFrontPorch = widthFrontPorch,
				.heightFrontPorch = heightFrontPorch,
				.widthSync = widthSync,
				.heightSync = heightSync,
				.widthBackPorch = (uint16_t)(((uint16_t)td.horizontal_blanking_pixels_msb << 8 | td.horizontal_blanking_pixels_lsb) - (widthSync + widthFrontPorch)),
				.heightBackPorch = (uint16_t)(((uint16_t)td.vertical_blanking_lines_msb << 8 | td.vertical_blanking_lines_lsb) - (heightSync + heightFrontPorch))
				});
			}
		}
		free(modeline);
	}
	return highestRefreshRate;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("No filepath provided!\n");
		return 1;
	}

	FILE* file = fopen(argv[1], "rb");
	if (!file) {
		printf("File doesn't exist!\n");
		return 1;
	}
	unsigned char* edid = (unsigned char*)calloc(sizeof(SetSysEdid), 1);
	fread(edid, 1, sizeof(SetSysEdid), file);
	fclose(file);
	float highestRefreshRate = parseEdid(edid);
	free(edid);
	printf("\nDetected highest progressive refresh rate: %0.4f Hz\n\n", highestRefreshRate);
	printf("Resolutions parsed:\n");
	for (size_t i = 0; i < EdidData.size(); i++) {
		printf("Res: %ux%u, refresh rate: %0.2f Hz, timings: H: %u, %u, %u; V: %u, %u, %u, pixel clock: %u kHz\n",	EdidData[i].width, EdidData[i].height, EdidData[i].refreshRate, 
																								EdidData[i].widthFrontPorch, EdidData[i].widthSync, EdidData[i].widthBackPorch,
																								EdidData[i].heightFrontPorch, EdidData[i].heightSync, EdidData[i].heightBackPorch, EdidData[i].pixelClockkHz);
	}
	EdidData.clear();
	return 0;
}