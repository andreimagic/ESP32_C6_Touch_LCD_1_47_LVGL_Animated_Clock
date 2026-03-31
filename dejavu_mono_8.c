/*******************************************************************************
 * Size: 8 px
 * Bpp: 2
 * Opts: --bpp 2 --size 8 --no-compress --stride 1 --align 1 --font DejaVuSansMono.ttf --range 32-126,9675,9679 --format lvgl -o dejavu_mono_8.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef DEJAVU_MONO_8
#define DEJAVU_MONO_8 1
#endif

#if DEJAVU_MONO_8

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0xfe, 0x4c,

    /* U+0022 "\"" */
    0x8a, 0x24, 0x0,

    /* U+0023 "#" */
    0x9, 0x1, 0x87, 0xb8, 0x88, 0xbf, 0xa, 0x5,
    0x80,

    /* U+0024 "$" */
    0x4, 0xb, 0x82, 0x40, 0xe0, 0xb, 0x1, 0x52,
    0xe0, 0x10, 0x0, 0x0,

    /* U+0025 "%" */
    0x60, 0x22, 0x6, 0x51, 0x58, 0x8, 0x81, 0x90,

    /* U+0026 "&" */
    0x29, 0x8, 0x3, 0x1, 0xe0, 0x88, 0xa0, 0xd2,
    0xa4,

    /* U+0027 "'" */
    0xa4,

    /* U+0028 "(" */
    0x14, 0x82, 0x14, 0x50, 0x82, 0x5,

    /* U+0029 ")" */
    0x40, 0x82, 0x8, 0x20, 0x82, 0x10,

    /* U+002A "*" */
    0x4, 0x6, 0x41, 0x90, 0x10, 0x0, 0x0,

    /* U+002B "+" */
    0x4, 0x2, 0x7, 0xf4, 0x20, 0x8, 0x0,

    /* U+002C "," */
    0x23, 0x50,

    /* U+002D "-" */
    0x64,

    /* U+002E "." */
    0x13,

    /* U+002F "/" */
    0x2, 0x0, 0x80, 0x80, 0x20, 0x14, 0x8, 0x2,
    0x0, 0x0,

    /* U+0030 "0" */
    0x29, 0x8, 0x85, 0x21, 0x79, 0x52, 0x8, 0x82,
    0x90,

    /* U+0031 "1" */
    0x3c, 0x2, 0x0, 0x80, 0x20, 0x8, 0x2, 0x2,
    0xf0,

    /* U+0032 "2" */
    0x3d, 0x0, 0xc0, 0x30, 0x18, 0x8, 0x8, 0x7,
    0xa0,

    /* U+0033 "3" */
    0x3d, 0x0, 0xc0, 0x20, 0x78, 0x2, 0x0, 0x87,
    0xe0,

    /* U+0034 "4" */
    0x6, 0x2, 0x81, 0x60, 0x88, 0x52, 0x2b, 0xd0,
    0x20,

    /* U+0035 "5" */
    0x3a, 0x8, 0x3, 0x90, 0xc, 0x2, 0x0, 0xc7,
    0xd0,

    /* U+0036 "6" */
    0x1e, 0x8, 0x6, 0x91, 0x88, 0x52, 0x48, 0x82,
    0xa0,

    /* U+0037 "7" */
    0x6b, 0x0, 0xc0, 0x20, 0x20, 0xc, 0x2, 0x2,
    0x40,

    /* U+0038 "8" */
    0x2a, 0x18, 0x82, 0x20, 0xb8, 0x52, 0x14, 0x92,
    0xa0,

    /* U+0039 "9" */
    0x29, 0x14, 0x85, 0x21, 0x4d, 0x2a, 0x0, 0x82,
    0xd0,

    /* U+003A ":" */
    0x3, 0x0, 0x13,

    /* U+003B ";" */
    0x3, 0x0, 0x23, 0x50,

    /* U+003C "<" */
    0x2, 0x4a, 0x6, 0x0, 0x28, 0x0, 0x0,

    /* U+003D "=" */
    0x6a, 0x40, 0x7, 0xf4,

    /* U+003E ">" */
    0x90, 0x6, 0x80, 0x75, 0x90, 0x0, 0x0,

    /* U+003F "?" */
    0x2a, 0x0, 0xc0, 0x20, 0x20, 0x8, 0x0, 0x0,
    0x80,

    /* U+0040 "@" */
    0x1a, 0x14, 0x18, 0xaa, 0x52, 0x94, 0xa2, 0x92,
    0x0, 0x68,

    /* U+0041 "A" */
    0xc, 0x6, 0x42, 0x20, 0x88, 0x3b, 0x14, 0x58,
    0x8,

    /* U+0042 "B" */
    0x7a, 0x14, 0x85, 0x21, 0xf8, 0x52, 0x54, 0x57,
    0xa0,

    /* U+0043 "C" */
    0x1b, 0x8, 0x5, 0x1, 0x40, 0x50, 0x8, 0x1,
    0xb0,

    /* U+0044 "D" */
    0x79, 0x14, 0xc5, 0x21, 0x49, 0x52, 0x14, 0xc7,
    0x90,

    /* U+0045 "E" */
    0x3a, 0x8, 0x2, 0x0, 0xfc, 0x20, 0x8, 0x3,
    0xa0,

    /* U+0046 "F" */
    0x3a, 0x48, 0x2, 0x0, 0xe8, 0x20, 0x8, 0x2,
    0x0,

    /* U+0047 "G" */
    0x1a, 0x8, 0x5, 0x2, 0xd, 0x51, 0x48, 0x52,
    0xa0,

    /* U+0048 "H" */
    0x52, 0x54, 0x95, 0x25, 0xfd, 0x52, 0x54, 0x95,
    0x24,

    /* U+0049 "I" */
    0x2e, 0x3, 0x0, 0xc0, 0x30, 0xc, 0x3, 0x2,
    0xe0,

    /* U+004A "J" */
    0x1a, 0x2, 0x2, 0x2, 0x2, 0x2, 0x7c,

    /* U+004B "K" */
    0x52, 0x55, 0x86, 0x81, 0xf0, 0x59, 0x14, 0xc5,
    0x18,

    /* U+004C "L" */
    0x20, 0x8, 0x2, 0x0, 0x80, 0x20, 0x8, 0x3,
    0xa4,

    /* U+004D "M" */
    0xa2, 0x68, 0xda, 0x66, 0x35, 0x85, 0x60, 0x58,
    0x14,

    /* U+004E "N" */
    0x61, 0x5c, 0x56, 0x55, 0x65, 0x5a, 0x54, 0xd5,
    0x34,

    /* U+004F "O" */
    0x29, 0x18, 0x85, 0x26, 0x45, 0x52, 0x58, 0x82,
    0xa0,

    /* U+0050 "P" */
    0x3a, 0x8, 0x92, 0x14, 0x89, 0x3a, 0x8, 0x2,
    0x0,

    /* U+0051 "Q" */
    0x29, 0x18, 0x85, 0x26, 0x45, 0x52, 0x58, 0x82,
    0xe0, 0x8, 0x0, 0x0,

    /* U+0052 "R" */
    0x7a, 0x14, 0xc5, 0x31, 0xf4, 0x52, 0x14, 0x85,
    0x8,

    /* U+0053 "S" */
    0x2a, 0x14, 0x6, 0x0, 0xb8, 0x2, 0x0, 0x87,
    0xa0,

    /* U+0054 "T" */
    0xae, 0x43, 0x0, 0xc0, 0x30, 0xc, 0x3, 0x0,
    0xc0,

    /* U+0055 "U" */
    0x52, 0x14, 0x85, 0x21, 0x48, 0x52, 0x14, 0x82,
    0xa0,

    /* U+0056 "V" */
    0x81, 0x54, 0x82, 0x30, 0xc8, 0x25, 0x7, 0x40,
    0xc0,

    /* U+0057 "W" */
    0xc0, 0xa0, 0x28, 0xca, 0x75, 0x66, 0x5c, 0xc2,
    0x30,

    /* U+0058 "X" */
    0x51, 0x4c, 0x81, 0xd0, 0x30, 0x25, 0x8, 0x88,
    0x14,

    /* U+0059 "Y" */
    0x81, 0x48, 0xc2, 0x50, 0x30, 0xc, 0x3, 0x0,
    0xc0,

    /* U+005A "Z" */
    0x2b, 0x40, 0xc0, 0x50, 0x30, 0x14, 0x8, 0x7,
    0xa4,

    /* U+005B "[" */
    0x34, 0x82, 0x8, 0x20, 0x82, 0x9,

    /* U+005C "\\" */
    0x50, 0x8, 0x2, 0x0, 0x20, 0x8, 0x1, 0x40,
    0x20, 0x4,

    /* U+005D "]" */
    0x70, 0x82, 0x8, 0x20, 0x82, 0x1c,

    /* U+005E "^" */
    0x4, 0xa, 0x45, 0x20,

    /* U+005F "_" */
    0x55, 0x40,

    /* U+0060 "`" */
    0x42,

    /* U+0061 "a" */
    0x3a, 0x0, 0x82, 0xb1, 0x48, 0x2b, 0x0,

    /* U+0062 "b" */
    0x50, 0x14, 0x7, 0xa1, 0x88, 0x51, 0x58, 0x87,
    0xa0,

    /* U+0063 "c" */
    0x1a, 0x8, 0x2, 0x0, 0x80, 0x1a, 0x0,

    /* U+0064 "d" */
    0x2, 0x0, 0x82, 0xb1, 0x4c, 0x52, 0x14, 0xc2,
    0xb0,

    /* U+0065 "e" */
    0x2a, 0x14, 0x86, 0xa5, 0x40, 0x2a, 0x0,

    /* U+0066 "f" */
    0xa, 0x2, 0x2, 0xe0, 0x20, 0x8, 0x2, 0x0,
    0x80,

    /* U+0067 "g" */
    0x2b, 0x14, 0xc5, 0x21, 0x4c, 0x2b, 0x0, 0x82,
    0x90,

    /* U+0068 "h" */
    0x10, 0x4, 0x2, 0xa0, 0x88, 0x12, 0x4, 0x81,
    0x20,

    /* U+0069 "i" */
    0x8, 0x0, 0x2, 0xc0, 0x20, 0x8, 0x2, 0x2,
    0xe0,

    /* U+006A "j" */
    0x8, 0x0, 0x2c, 0x8, 0x8, 0x8, 0x8, 0x8,
    0x28,

    /* U+006B "k" */
    0x20, 0x8, 0x2, 0x20, 0xa0, 0x3c, 0x8, 0x82,
    0x24,

    /* U+006C "l" */
    0x28, 0x2, 0x0, 0x80, 0x20, 0x8, 0x2, 0x0,
    0xa0,

    /* U+006D "m" */
    0xab, 0x22, 0x58, 0x86, 0x21, 0x88, 0x40,

    /* U+006E "n" */
    0x2a, 0x8, 0x81, 0x20, 0x48, 0x12, 0x0,

    /* U+006F "o" */
    0x2a, 0x14, 0x85, 0x25, 0x48, 0x2a, 0x0,

    /* U+0070 "p" */
    0x7a, 0x18, 0x85, 0x15, 0x88, 0x7a, 0x14, 0x5,
    0x0,

    /* U+0071 "q" */
    0x2b, 0x14, 0xc5, 0x21, 0x4c, 0x2b, 0x0, 0x80,
    0x20,

    /* U+0072 "r" */
    0xa9, 0x90, 0x80, 0x80, 0x80,

    /* U+0073 "s" */
    0x2a, 0x8, 0x1, 0xd0, 0xc, 0x2a, 0x0,

    /* U+0074 "t" */
    0x0, 0x5, 0x6, 0xe0, 0x50, 0x14, 0x5, 0x0,
    0xa0,

    /* U+0075 "u" */
    0x12, 0x4, 0x81, 0x20, 0x88, 0x2b, 0x0,

    /* U+0076 "v" */
    0x51, 0x48, 0x82, 0x20, 0xa4, 0xc, 0x0,

    /* U+0077 "w" */
    0x80, 0xa1, 0x25, 0xd5, 0xdc, 0x33, 0x0,

    /* U+0078 "x" */
    0x22, 0xa, 0x40, 0xc0, 0x98, 0x52, 0x0,

    /* U+0079 "y" */
    0x51, 0x48, 0x82, 0x20, 0x64, 0xc, 0x2, 0x3,
    0x0,

    /* U+007A "z" */
    0x2b, 0x1, 0x80, 0x80, 0x80, 0x3a, 0x0,

    /* U+007B "{" */
    0xa, 0x8, 0x8, 0x8, 0x34, 0x8, 0x8, 0x6,

    /* U+007C "|" */
    0xaa, 0xaa, 0x80,

    /* U+007D "}" */
    0x24, 0x8, 0x8, 0x8, 0xa, 0x8, 0x8, 0x24,

    /* U+007E "~" */
    0x68, 0x51, 0xc0,

    /* U+25CB "○" */
    0x4, 0x14, 0x58, 0xa, 0x2, 0x51, 0x41, 0x0,

    /* U+25CF "●" */
    0x19, 0x2f, 0xdf, 0xff, 0xfe, 0x3f, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 77, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 77, .box_w = 1, .box_h = 7, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2, .adv_w = 77, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 5, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 77, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 26, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 77, .box_w = 1, .box_h = 3, .ofs_x = 2, .ofs_y = 4},
    {.bitmap_index = 44, .adv_w = 77, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 50, .adv_w = 77, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 56, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 63, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 77, .box_w = 2, .box_h = 3, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 72, .adv_w = 77, .box_w = 3, .box_h = 1, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 73, .adv_w = 77, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 77, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 84, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 120, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 147, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 156, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 77, .box_w = 2, .box_h = 6, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 77, .box_w = 2, .box_h = 7, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 181, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 77, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 192, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 199, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 77, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 218, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 236, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 254, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 263, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 281, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 290, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 299, .adv_w = 77, .box_w = 4, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 306, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 333, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 77, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 372, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 381, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 399, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 408, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 417, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 435, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 444, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 77, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 459, .adv_w = 77, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 469, .adv_w = 77, .box_w = 3, .box_h = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 475, .adv_w = 77, .box_w = 5, .box_h = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 479, .adv_w = 77, .box_w = 5, .box_h = 1, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 481, .adv_w = 77, .box_w = 2, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 482, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 489, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 498, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 514, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 530, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 539, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 548, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 77, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 566, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 584, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 591, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 598, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 605, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 614, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 623, .adv_w = 77, .box_w = 4, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 628, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 635, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 644, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 651, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 658, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 665, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 672, .adv_w = 77, .box_w = 5, .box_h = 7, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 681, .adv_w = 77, .box_w = 5, .box_h = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 688, .adv_w = 77, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 696, .adv_w = 77, .box_w = 1, .box_h = 9, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 699, .adv_w = 77, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 707, .adv_w = 77, .box_w = 5, .box_h = 2, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 710, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 718, .adv_w = 77, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_1[] = {
    0x0, 0x4
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 9675, .range_length = 5, .glyph_id_start = 96,
        .unicode_list = unicode_list_1, .glyph_id_ofs_list = NULL, .list_length = 2, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 2,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t dejavu_mono_8 = {
#else
lv_font_t dejavu_mono_8 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 0,
#endif
    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if DEJAVU_MONO_8*/
