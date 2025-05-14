// SPDX-License-Identifier: MIT
/*
 * Copyright 2006-2012 Red Hat, Inc.
 * Copyright 2018-2020 Cisco Systems, Inc. and/or its affiliates. All rights reserved.
 *
 * Author: Adam Jackson <ajax@nwnk.net>
 * Maintainer: Hans Verkuil <hverkuil-cisco@xs4all.nl>
 */

// Video Timings
// If interlaced is true, then the vertical blanking
// for each field is (vfp + vsync + vbp + 0.5), except for
// the VIC 39 timings that doesn't have the 0.5 constant.
//
// The sequence of the various video parameters is as follows:
//
// border - front porch - sync - back porch - border - active video
//
// Note: this is slightly different from EDID 1.4 which calls
// 'active video' as 'addressable video' and the EDID 1.4 term
// 'active video' includes the borders.
//
// But since borders are rarely used, the term 'active video' will
// typically be the same as 'addressable video', and that's how I
// use it.

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

struct timings {
	// Active horizontal and vertical frame height, excluding any
	// borders, if present.
	// Note: for interlaced formats the active field height is vact / 2
	unsigned hact, vact;
	unsigned hratio, vratio;
	unsigned pixclk_khz;
	// 0: no reduced blanking
	// 1: CVT reduced blanking version 1
	// 2: CVT reduced blanking version 2
	// 2 | RB_ALT: CVT reduced blanking version 2 video-optimized (1000/1001 fps)
	// 3: CVT reduced blanking version 3
	// 3 | RB_ALT: v3 with a horizontal blank of 160
	// 4: GTF Secondary Curve
	unsigned rb;
	bool interlaced;
	// The horizontal frontporch may be negative in GTF calculations,
	// so use int instead of unsigned for hfp. Example: 292x176@76.
	int hfp;
	unsigned hsync;
	// The backporch may be negative in buggy detailed timings.
	// So use int instead of unsigned for hbp and vbp.
	int hbp;
	bool pos_pol_hsync;
	// For interlaced formats the vertical front porch of the Even Field
	// is actually a half-line longer.
	unsigned vfp, vsync;
	// For interlaced formats the vertical back porch of the Odd Field
	// is actually a half-line longer.
	int vbp;
	bool pos_pol_vsync;
	unsigned hborder, vborder;
	bool even_vtotal; // special for VIC 39
	bool no_pol_vsync; // digital composite signals have no vsync polarity
	unsigned hsize_mm, vsize_mm;
	bool ycbcr420; // YCbCr 4:2:0 encoding
};

static const struct timings edid_cta_modes1[] = {
	/* VIC 1 */
	{  640,  480,   4,   3,   25175, 0, false,   16,  96,  48, false, 10,  2,  33, false },
	{  720,  480,   4,   3,   27000, 0, false,   16,  62,  60, false,  9,  6,  30, false },
	{  720,  480,  16,   9,   27000, 0, false,   16,  62,  60, false,  9,  6,  30, false },
	{ 1280,  720,  16,   9,   74250, 0, false,  110,  40, 220, true,   5,  5,  20, true  },
	{ 1920, 1080,  16,   9,   74250, 0, true,    88,  44, 148, true,   2,  5,  15, true  },
	{ 1440,  480,   4,   3,   27000, 0, true,    38, 124, 114, false,  4,  3,  15, false },
	{ 1440,  480,  16,   9,   27000, 0, true,    38, 124, 114, false,  4,  3,  15, false },
	{ 1440,  240,   4,   3,   27000, 0, false,   38, 124, 114, false,  4,  3,  15, false },
	{ 1440,  240,  16,   9,   27000, 0, false,   38, 124, 114, false,  4,  3,  15, false },
	{ 2880,  480,   4,   3,   54000, 0, true,    76, 248, 228, false,  4,  3,  15, false },
	/* VIC 11 */
	{ 2880,  480,  16,   9,   54000, 0, true,    76, 248, 228, false,  4,  3,  15, false },
	{ 2880,  240,   4,   3,   54000, 0, false,   76, 248, 228, false,  4,  3,  15, false },
	{ 2880,  240,  16,   9,   54000, 0, false,   76, 248, 228, false,  4,  3,  15, false },
	{ 1440,  480,   4,   3,   54000, 0, false,   32, 124, 120, false,  9,  6,  30, false },
	{ 1440,  480,  16,   9,   54000, 0, false,   32, 124, 120, false,  9,  6,  30, false },
	{ 1920, 1080,  16,   9,  148500, 0, false,   88,  44, 148, true,   4,  5,  36, true  },
	{  720,  576,   4,   3,   27000, 0, false,   12,  64,  68, false,  5,  5,  39, false },
	{  720,  576,  16,   9,   27000, 0, false,   12,  64,  68, false,  5,  5,  39, false },
	{ 1280,  720,  16,   9,   74250, 0, false,  440,  40, 220, true,   5,  5,  20, true  },
	{ 1920, 1080,  16,   9,   74250, 0, true,   528,  44, 148, true,   2,  5,  15, true  },
	/* VIC 21 */
	{ 1440,  576,   4,   3,   27000, 0, true,    24, 126, 138, false,  2,  3,  19, false },
	{ 1440,  576,  16,   9,   27000, 0, true,    24, 126, 138, false,  2,  3,  19, false },
	{ 1440,  288,   4,   3,   27000, 0, false,   24, 126, 138, false,  2,  3,  19, false },
	{ 1440,  288,  16,   9,   27000, 0, false,   24, 126, 138, false,  2,  3,  19, false },
	{ 2880,  576,   4,   3,   54000, 0, true,    48, 252, 276, false,  2,  3,  19, false },
	{ 2880,  576,  16,   9,   54000, 0, true,    48, 252, 276, false,  2,  3,  19, false },
	{ 2880,  288,   4,   3,   54000, 0, false,   48, 252, 276, false,  2,  3,  19, false },
	{ 2880,  288,  16,   9,   54000, 0, false,   48, 252, 276, false,  2,  3,  19, false },
	{ 1440,  576,   4,   3,   54000, 0, false,   24, 128, 136, false,  5,  5,  39, false },
	{ 1440,  576,  16,   9,   54000, 0, false,   24, 128, 136, false,  5,  5,  39, false },
	/* VIC 31 */
	{ 1920, 1080,  16,   9,  148500, 0, false,  528,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  16,   9,   74250, 0, false,  638,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  16,   9,   74250, 0, false,  528,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  16,   9,   74250, 0, false,   88,  44, 148, true,   4,  5,  36, true  },
	{ 2880,  480,   4,   3,  108000, 0, false,   64, 248, 240, false,  9,  6,  30, false },
	{ 2880,  480,  16,   9,  108000, 0, false,   64, 248, 240, false,  9,  6,  30, false },
	{ 2880,  576,   4,   3,  108000, 0, false,   48, 256, 272, false,  5,  5,  39, false },
	{ 2880,  576,  16,   9,  108000, 0, false,   48, 256, 272, false,  5,  5,  39, false },
	{ 1920, 1080,  16,   9,   72000, 0, true,    32, 168, 184, true,  23,  5,  57, false, 0, 0, true },
	{ 1920, 1080,  16,   9,  148500, 0, true,   528,  44, 148, true,   2,  5,  15, true  },
	/* VIC 41 */
	{ 1280,  720,  16,   9,  148500, 0, false,  440,  40, 220, true,   5,  5,  20, true  },
	{  720,  576,   4,   3,   54000, 0, false,   12,  64,  68, false,  5,  5,  39, false },
	{  720,  576,  16,   9,   54000, 0, false,   12,  64,  68, false,  5,  5,  39, false },
	{ 1440,  576,   4,   3,   54000, 0, true,    24, 126, 138, false,  2,  3,  19, false },
	{ 1440,  576,  16,   9,   54000, 0, true,    24, 126, 138, false,  2,  3,  19, false },
	{ 1920, 1080,  16,   9,  148500, 0, true,    88,  44, 148, true,   2,  5,  15, true  },
	{ 1280,  720,  16,   9,  148500, 0, false,  110,  40, 220, true,   5,  5,  20, true  },
	{  720,  480,   4,   3,   54000, 0, false,   16,  62,  60, false,  9,  6,  30, false },
	{  720,  480,  16,   9,   54000, 0, false,   16,  62,  60, false,  9,  6,  30, false },
	{ 1440,  480,   4,   3,   54000, 0, true,    38, 124, 114, false,  4,  3,  15, false },
	/* VIC 51 */
	{ 1440,  480,  16,   9,   54000, 0, true,    38, 124, 114, false,  4,  3,  15, false },
	{  720,  576,   4,   3,  108000, 0, false,   12,  64,  68, false,  5,  5,  39, false },
	{  720,  576,  16,   9,  108000, 0, false,   12,  64,  68, false,  5,  5,  39, false },
	{ 1440,  576,   4,   3,  108000, 0, true,    24, 126, 138, false,  2,  3,  19, false },
	{ 1440,  576,  16,   9,  108000, 0, true,    24, 126, 138, false,  2,  3,  19, false },
	{  720,  480,   4,   3,  108000, 0, false,   16,  62,  60, false,  9,  6,  30, false },
	{  720,  480,  16,   9,  108000, 0, false,   16,  62,  60, false,  9,  6,  30, false },
	{ 1440,  480,   4,   3,  108000, 0, true,    38, 124, 114, false,  4,  3,  15, false },
	{ 1440,  480,  16,   9,  108000, 0, true,    38, 124, 114, false,  4,  3,  15, false },
	{ 1280,  720,  16,   9,   59400, 0, false, 1760,  40, 220, true,   5,  5,  20, true  },
	/* VIC 61 */
	{ 1280,  720,  16,   9,   74250, 0, false, 2420,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  16,   9,   74250, 0, false, 1760,  40, 220, true,   5,  5,  20, true  },
	{ 1920, 1080,  16,   9,  297000, 0, false,   88,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  16,   9,  297000, 0, false,  528,  44, 148, true,   4,  5,  36, true  },
	{ 1280,  720,  64,  27,   59400, 0, false, 1760,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  64,  27,   74250, 0, false, 2420,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  64,  27,   74250, 0, false, 1760,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  64,  27,   74250, 0, false,  440,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  64,  27,   74250, 0, false,  110,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  64,  27,  148500, 0, false,  440,  40, 220, true,   5,  5,  20, true  },
	/* VIC 71 */
	{ 1280,  720,  64,  27,  148500, 0, false,  110,  40, 220, true,   5,  5,  20, true  },
	{ 1920, 1080,  64,  27,   74250, 0, false,  638,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,   74250, 0, false,  528,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,   74250, 0, false,   88,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,  148500, 0, false,  528,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,  148500, 0, false,   88,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,  297000, 0, false,  528,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,  297000, 0, false,   88,  44, 148, true,   4,  5,  36, true  },
	{ 1680,  720,  64,  27,   59400, 0, false, 1360,  40, 220, true,   5,  5,  20, true  },
	{ 1680,  720,  64,  27,   59400, 0, false, 1228,  40, 220, true,   5,  5,  20, true  },
	/* VIC 81 */
	{ 1680,  720,  64,  27,   59400, 0, false,  700,  40, 220, true,   5,  5,  20, true  },
	{ 1680,  720,  64,  27,   82500, 0, false,  260,  40, 220, true,   5,  5,  20, true  },
	{ 1680,  720,  64,  27,   99000, 0, false,  260,  40, 220, true,   5,  5,  20, true  },
	{ 1680,  720,  64,  27,  165000, 0, false,   60,  40, 220, true,   5,  5,  95, true  },
	{ 1680,  720,  64,  27,  198000, 0, false,   60,  40, 220, true,   5,  5,  95, true  },
	{ 2560, 1080,  64,  27,   99000, 0, false,  998,  44, 148, true,   4,  5,  11, true  },
	{ 2560, 1080,  64,  27,   90000, 0, false,  448,  44, 148, true,   4,  5,  36, true  },
	{ 2560, 1080,  64,  27,  118800, 0, false,  768,  44, 148, true,   4,  5,  36, true  },
	{ 2560, 1080,  64,  27,  185625, 0, false,  548,  44, 148, true,   4,  5,  36, true  },
	{ 2560, 1080,  64,  27,  198000, 0, false,  248,  44, 148, true,   4,  5,  11, true  },
	/* VIC 91 */
	{ 2560, 1080,  64,  27,  371250, 0, false,  218,  44, 148, true,   4,  5, 161, true  },
	{ 2560, 1080,  64,  27,  495000, 0, false,  548,  44, 148, true,   4,  5, 161, true  },
	{ 3840, 2160,  16,   9,  297000, 0, false, 1276,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  16,   9,  297000, 0, false, 1056,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  16,   9,  297000, 0, false,  176,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  16,   9,  594000, 0, false, 1056,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  16,   9,  594000, 0, false,  176,  88, 296, true,   8, 10,  72, true  },
	{ 4096, 2160, 256, 135,  297000, 0, false, 1020,  88, 296, true,   8, 10,  72, true  },
	{ 4096, 2160, 256, 135,  297000, 0, false,  968,  88, 128, true,   8, 10,  72, true  },
	{ 4096, 2160, 256, 135,  297000, 0, false,   88,  88, 128, true,   8, 10,  72, true  },
	/* VIC 101 */
	{ 4096, 2160, 256, 135,  594000, 0, false,  968,  88, 128, true,   8, 10,  72, true  },
	{ 4096, 2160, 256, 135,  594000, 0, false,   88,  88, 128, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27,  297000, 0, false, 1276,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27,  297000, 0, false, 1056,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27,  297000, 0, false,  176,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27,  594000, 0, false, 1056,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27,  594000, 0, false,  176,  88, 296, true,   8, 10,  72, true  },
	{ 1280,  720,  16,   9,   90000, 0, false,  960,  40, 220, true,   5,  5,  20, true  },
	{ 1280,  720,  64,  27,   90000, 0, false,  960,  40, 220, true,   5,  5,  20, true  },
	{ 1680,  720,  64,  27,   99000, 0, false,  810,  40, 220, true,   5,  5,  20, true  },
	/* VIC 111 */
	{ 1920, 1080,  16,   9,  148500, 0, false,  638,  44, 148, true,   4,  5,  36, true  },
	{ 1920, 1080,  64,  27,  148500, 0, false,  638,  44, 148, true,   4,  5,  36, true  },
	{ 2560, 1080,  64,  27,  198000, 0, false,  998,  44, 148, true,   4,  5,  11, true  },
	{ 3840, 2160,  16,   9,  594000, 0, false, 1276,  88, 296, true,   8, 10,  72, true  },
	{ 4096, 2160, 256, 135,  594000, 0, false, 1020,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27,  594000, 0, false, 1276,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  16,   9, 1188000, 0, false, 1056,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  16,   9, 1188000, 0, false,  176,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27, 1188000, 0, false, 1056,  88, 296, true,   8, 10,  72, true  },
	{ 3840, 2160,  64,  27, 1188000, 0, false,  176,  88, 296, true,   8, 10,  72, true  },
	/* VIC 121 */
	{ 5120, 2160,  64,  27,  396000, 0, false, 1996,  88, 296, true,   8, 10,  22, true  },
	{ 5120, 2160,  64,  27,  396000, 0, false, 1696,  88, 296, true,   8, 10,  22, true  },
	{ 5120, 2160,  64,  27,  396000, 0, false,  664,  88, 128, true,   8, 10,  22, true  },
	{ 5120, 2160,  64,  27,  742500, 0, false,  746,  88, 296, true,   8, 10, 297, true  },
	{ 5120, 2160,  64,  27,  742500, 0, false, 1096,  88, 296, true,   8, 10,  72, true  },
	{ 5120, 2160,  64,  27,  742500, 0, false,  164,  88, 128, true,   8, 10,  72, true  },
	{ 5120, 2160,  64,  27, 1485000, 0, false, 1096,  88, 296, true,   8, 10,  72, true  },
};

static const struct timings edid_cta_modes2[] = {
	/* VIC 193 */
	{  5120, 2160,  64,  27, 1485000, 0, false,  164,  88, 128, true,   8, 10,  72, true  },
	{  7680, 4320,  16,   9, 1188000, 0, false, 2552, 176, 592, true,  16, 20, 144, true  },
	{  7680, 4320,  16,   9, 1188000, 0, false, 2352, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  16,   9, 1188000, 0, false,  552, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  16,   9, 2376000, 0, false, 2552, 176, 592, true,  16, 20, 144, true  },
	{  7680, 4320,  16,   9, 2376000, 0, false, 2352, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  16,   9, 2376000, 0, false,  552, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  16,   9, 4752000, 0, false, 2112, 176, 592, true,  16, 20, 144, true  },
	/* VIC 201 */
	{  7680, 4320,  16,   9, 4752000, 0, false,  352, 176, 592, true,  16, 20, 144, true  },
	{  7680, 4320,  64,  27, 1188000, 0, false, 2552, 176, 592, true,  16, 20, 144, true  },
	{  7680, 4320,  64,  27, 1188000, 0, false, 2352, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  64,  27, 1188000, 0, false,  552, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  64,  27, 2376000, 0, false, 2552, 176, 592, true,  16, 20, 144, true  },
	{  7680, 4320,  64,  27, 2376000, 0, false, 2352, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  64,  27, 2376000, 0, false,  552, 176, 592, true,  16, 20,  44, true  },
	{  7680, 4320,  64,  27, 4752000, 0, false, 2112, 176, 592, true,  16, 20, 144, true  },
	{  7680, 4320,  64,  27, 4752000, 0, false,  352, 176, 592, true,  16, 20, 144, true  },
	{ 10240, 4320,  64,  27, 1485000, 0, false, 1492, 176, 592, true,  16, 20, 594, true  },
	/* VIC 211 */
	{ 10240, 4320,  64,  27, 1485000, 0, false, 2492, 176, 592, true,  16, 20,  44, true  },
	{ 10240, 4320,  64,  27, 1485000, 0, false,  288, 176, 296, true,  16, 20, 144, true  },
	{ 10240, 4320,  64,  27, 2970000, 0, false, 1492, 176, 592, true,  16, 20, 594, true  },
	{ 10240, 4320,  64,  27, 2970000, 0, false, 2492, 176, 592, true,  16, 20,  44, true  },
	{ 10240, 4320,  64,  27, 2970000, 0, false,  288, 176, 296, true,  16, 20, 144, true  },
	{ 10240, 4320,  64,  27, 5940000, 0, false, 2192, 176, 592, true,  16, 20, 144, true  },
	{ 10240, 4320,  64,  27, 5940000, 0, false,  288, 176, 296, true,  16, 20, 144, true  },
	{  4096, 2160, 256, 135, 1188000, 0, false,  800,  88, 296, true,   8, 10,  72, true  },
	{  4096, 2160, 256, 135, 1188000, 0, false,   88,  88, 128, true,   8, 10,  72, true  },
};

const struct timings *find_vic_id(unsigned char vic)
{
	if (vic > 0 && vic <= ARRAY_SIZE(edid_cta_modes1))
		return edid_cta_modes1 + vic - 1;
	if (vic >= 193 && vic < ARRAY_SIZE(edid_cta_modes2) + 193)
		return edid_cta_modes2 + vic - 193;
	return NULL;
}