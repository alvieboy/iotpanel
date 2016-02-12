#include "font.h"

// Created from bdf2c Version 3, (c) 2009, 2010 by Lutz Sammer
//	License AGPLv3: GNU Affero General Public License version 3

// original public domain font source:
// http://www.opensource.apple.com/source/X11fonts/X11fonts-14/font-misc-misc/font-misc-misc-1.1.2/4x6.bdf

static unsigned char __apple3x5_bitmap__[] = {

 0x00,
 0x00,
 0x00,
 0x00,
 0x00,
 0x00,


 0x40,
 0x40,
 0x40,
 0x00,
 0x40,
 0x00,


 0xA0,
 0xA0,
 0x00,
 0x00,
 0x00,
 0x00,


 0xA0,
 0xF0,
 0xA0,
 0xF0,
 0xA0,
 0x00,


 0x40,
 0xE0,
 0xC0,
 0x20,
 0xE0,
 0x40,


 0x80,
 0x20,
 0x40,
 0x80,
 0x20,
 0x00,


 0x40,
 0xA0,
 0x40,
 0xA0,
 0x50,
 0x00,


 0x40,
 0x40,
 0x00,
 0x00,
 0x00,
 0x00,


 0x20,
 0x40,
 0x40,
 0x40,
 0x40,
 0x20,


 0x80,
 0x40,
 0x40,
 0x40,
 0x40,
 0x80,


 0xA0,
 0x40,
 0xE0,
 0x40,
 0xA0,
 0x00,


 0x40,
 0x40,
 0xE0,
 0x40,
 0x40,
 0x00,


 0x00,
 0x00,
 0x00,
 0x00,
 0x40,
 0x80,


 0x00,
 0x00,
 0xE0,
 0x00,
 0x00,
 0x00,


 0x00,
 0x00,
 0x00,
 0x00,
 0x40,
 0x00,


 0x20,
 0x20,
 0x40,
 0x80,
 0x80,
 0x00,


 0x40,
 0xA0,
 0xE0,
 0xA0,
 0x40,
 0x00,


 0x40,
 0xC0,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x40,
 0xA0,
 0x20,
 0x40,
 0xE0,
 0x00,


 0xE0,
 0x20,
 0x40,
 0x20,
 0xC0,
 0x00,


 0xA0,
 0xA0,
 0xE0,
 0x20,
 0x20,
 0x00,


 0xE0,
 0x80,
 0xC0,
 0x20,
 0xC0,
 0x00,


 0x60,
 0x80,
 0xC0,
 0xA0,
 0x40,
 0x00,


 0xE0,
 0x20,
 0x40,
 0x80,
 0x80,
 0x00,


 0x60,
 0xA0,
 0x40,
 0xA0,
 0xC0,
 0x00,


 0x40,
 0xA0,
 0x60,
 0x20,
 0xC0,
 0x00,


 0x00,
 0x40,
 0x00,
 0x00,
 0x40,
 0x00,


 0x00,
 0x40,
 0x00,
 0x00,
 0x40,
 0x80,


 0x20,
 0x40,
 0x80,
 0x40,
 0x20,
 0x00,


 0x00,
 0xE0,
 0x00,
 0xE0,
 0x00,
 0x00,


 0x80,
 0x40,
 0x20,
 0x40,
 0x80,
 0x00,


 0xC0,
 0x20,
 0x40,
 0x00,
 0x40,
 0x00,


 0x60,
 0xA0,
 0xA0,
 0x80,
 0x60,
 0x00,


 0x40,
 0xA0,
 0xE0,
 0xA0,
 0xA0,
 0x00,


 0xC0,
 0xA0,
 0xC0,
 0xA0,
 0xC0,
 0x00,


 0x40,
 0xA0,
 0x80,
 0xA0,
 0x40,
 0x00,


 0xC0,
 0xA0,
 0xA0,
 0xA0,
 0xC0,
 0x00,


 0xE0,
 0x80,
 0xC0,
 0x80,
 0xE0,
 0x00,


 0xE0,
 0x80,
 0xC0,
 0x80,
 0x80,
 0x00,


 0x60,
 0x80,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0xA0,
 0xA0,
 0xE0,
 0xA0,
 0xA0,
 0x00,


 0xE0,
 0x40,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x20,
 0x20,
 0x20,
 0xA0,
 0x40,
 0x00,


 0xA0,
 0xA0,
 0xC0,
 0xA0,
 0xA0,
 0x00,


 0x80,
 0x80,
 0x80,
 0x80,
 0xE0,
 0x00,


 0xA0,
 0xE0,
 0xE0,
 0xA0,
 0xA0,
 0x00,


 0x20,
 0xA0,
 0xE0,
 0xA0,
 0x80,
 0x00,


 0x40,
 0xA0,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0xC0,
 0xA0,
 0xC0,
 0x80,
 0x80,
 0x00,


 0x40,
 0xA0,
 0xA0,
 0xA0,
 0x40,
 0x20,


 0xC0,
 0xA0,
 0xC0,
 0xA0,
 0xA0,
 0x00,


 0x60,
 0x80,
 0x40,
 0x20,
 0xC0,
 0x00,


 0xE0,
 0x40,
 0x40,
 0x40,
 0x40,
 0x00,


 0xA0,
 0xA0,
 0xA0,
 0xA0,
 0xE0,
 0x00,


 0xA0,
 0xA0,
 0xA0,
 0xE0,
 0x40,
 0x00,


 0xA0,
 0xA0,
 0xE0,
 0xE0,
 0xA0,
 0x00,


 0xA0,
 0xA0,
 0x40,
 0xA0,
 0xA0,
 0x00,


 0xA0,
 0xA0,
 0x40,
 0x40,
 0x40,
 0x00,


 0xE0,
 0x20,
 0x40,
 0x80,
 0xE0,
 0x00,


 0x60,
 0x40,
 0x40,
 0x40,
 0x60,
 0x00,


 0x80,
 0x80,
 0x40,
 0x20,
 0x20,
 0x00,


 0xC0,
 0x40,
 0x40,
 0x40,
 0xC0,
 0x00,


 0x40,
 0xA0,
 0x00,
 0x00,
 0x00,
 0x00,


 0x00,
 0x00,
 0x00,
 0x00,
 0x00,
 0xE0,


 0x40,
 0x20,
 0x00,
 0x00,
 0x00,
 0x00,


 0x00,
 0x60,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0x80,
 0xC0,
 0xA0,
 0xA0,
 0xC0,
 0x00,


 0x00,
 0x60,
 0x80,
 0x80,
 0x60,
 0x00,


 0x20,
 0x60,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0x00,
 0x40,
 0xA0,
 0xC0,
 0x60,
 0x00,


 0x20,
 0x40,
 0xE0,
 0x40,
 0x40,
 0x00,


 0x00,
 0x60,
 0xA0,
 0x60,
 0x20,
 0xC0,


 0x80,
 0xC0,
 0xA0,
 0xA0,
 0xA0,
 0x00,


 0x40,
 0x00,
 0xC0,
 0x40,
 0xE0,
 0x00,


 0x20,
 0x00,
 0x20,
 0x20,
 0x20,
 0xC0,


 0x80,
 0xA0,
 0xC0,
 0xA0,
 0xA0,
 0x00,


 0xC0,
 0x40,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x00,
 0xA0,
 0xE0,
 0xA0,
 0xA0,
 0x00,


 0x00,
 0xC0,
 0xA0,
 0xA0,
 0xA0,
 0x00,


 0x00,
 0x40,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0x00,
 0xC0,
 0xA0,
 0xC0,
 0x80,
 0x80,


 0x00,
 0x60,
 0xA0,
 0xA0,
 0x60,
 0x20,


 0x00,
 0xA0,
 0xC0,
 0x80,
 0x80,
 0x00,


 0x00,
 0x60,
 0xC0,
 0x20,
 0xC0,
 0x00,


 0x40,
 0xE0,
 0x40,
 0x40,
 0x20,
 0x00,


 0x00,
 0xA0,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0x00,
 0xA0,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0x00,
 0xA0,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0x00,
 0xA0,
 0x40,
 0x40,
 0xA0,
 0x00,


 0x00,
 0xA0,
 0xA0,
 0x60,
 0x20,
 0xC0,


 0x00,
 0xE0,
 0x20,
 0x40,
 0xE0,
 0x00,


 0x20,
 0x40,
 0xC0,
 0x40,
 0x40,
 0x20,


 0x40,
 0x40,
 0x40,
 0x40,
 0x40,
 0x00,


 0x80,
 0x40,
 0x60,
 0x40,
 0x40,
 0x80,


 0x50,
 0xA0,
 0x00,
 0x00,
 0x00,
 0x00,


 0x00,
 0x00,
 0x00,
 0x00,
 0x00,
 0x00,


 0x40,
 0x00,
 0x40,
 0x40,
 0x40,
 0x00,


 0x40,
 0xE0,
 0x80,
 0xE0,
 0x40,
 0x00,


 0x20,
 0x40,
 0x60,
 0x40,
 0xA0,
 0x00,


 0x00,
 0x90,
 0x60,
 0x60,
 0x90,
 0x00,


 0xA0,
 0x40,
 0xE0,
 0x40,
 0x40,
 0x00,


 0x40,
 0x40,
 0x00,
 0x40,
 0x40,
 0x00,


 0x60,
 0xC0,
 0xA0,
 0x60,
 0x20,
 0xC0,


 0xA0,
 0x00,
 0x00,
 0x00,
 0x00,
 0x00,


 0x60,
 0x90,
 0xD0,
 0xD0,
 0x90,
 0x60,


 0x60,
 0xA0,
 0x60,
 0x00,
 0xE0,
 0x00,


 0x00,
 0x50,
 0xA0,
 0x50,
 0x00,
 0x00,


 0x00,
 0x00,
 0xE0,
 0x20,
 0x00,
 0x00,


 0x00,
 0x00,
 0xE0,
 0x00,
 0x00,
 0x00,


 0x60,
 0xF0,
 0xD0,
 0x60,
 0x00,
 0x00,


 0xE0,
 0x00,
 0x00,
 0x00,
 0x00,
 0x00,


 0x40,
 0xA0,
 0x40,
 0x00,
 0x00,
 0x00,


 0x40,
 0xE0,
 0x40,
 0x00,
 0xE0,
 0x00,


 0xC0,
 0x40,
 0x80,
 0xC0,
 0x00,
 0x00,


 0xC0,
 0x40,
 0x80,
 0x40,
 0x80,
 0x00,


 0x20,
 0x40,
 0x00,
 0x00,
 0x00,
 0x00,


 0x00,
 0xA0,
 0xA0,
 0xA0,
 0xC0,
 0x80,


 0x70,
 0xD0,
 0xD0,
 0x50,
 0x50,
 0x00,


 0x00,
 0x00,
 0x40,
 0x00,
 0x00,
 0x00,


 0x00,
 0x00,
 0x00,
 0x20,
 0x40,
 0x00,


 0x40,
 0xC0,
 0x40,
 0x40,
 0x00,
 0x00,


 0x40,
 0xA0,
 0x40,
 0x00,
 0xE0,
 0x00,


 0x00,
 0xA0,
 0x50,
 0xA0,
 0x00,
 0x00,


 0x80,
 0x80,
 0x80,
 0x50,
 0x70,
 0x10,


 0x80,
 0x80,
 0xB0,
 0x10,
 0x20,
 0x30,


 0xC0,
 0x40,
 0x80,
 0x50,
 0xB0,
 0x10,


 0x40,
 0x00,
 0x40,
 0x80,
 0x60,
 0x00,


 0x80,
 0x40,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0x20,
 0x40,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0xC0,
 0x40,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0x60,
 0xC0,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0xA0,
 0x40,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0x40,
 0x40,
 0xA0,
 0xE0,
 0xA0,
 0x00,


 0x70,
 0xA0,
 0xF0,
 0xA0,
 0xB0,
 0x00,


 0x40,
 0xA0,
 0x80,
 0xA0,
 0x40,
 0x80,


 0x80,
 0xE0,
 0xC0,
 0x80,
 0xE0,
 0x00,


 0x20,
 0xE0,
 0xC0,
 0x80,
 0xE0,
 0x00,


 0x60,
 0xE0,
 0xC0,
 0x80,
 0xE0,
 0x00,


 0xA0,
 0xE0,
 0xC0,
 0x80,
 0xE0,
 0x00,


 0x80,
 0xE0,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x20,
 0xE0,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x40,
 0xE0,
 0x40,
 0x40,
 0xE0,
 0x00,


 0xA0,
 0x40,
 0x40,
 0x40,
 0xE0,
 0x00,


 0xE0,
 0x50,
 0xD0,
 0x50,
 0xE0,
 0x00,


 0x50,
 0xA0,
 0xE0,
 0xE0,
 0xA0,
 0x00,


 0x80,
 0x40,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0x20,
 0x40,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0x40,
 0x40,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0x70,
 0xE0,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0xA0,
 0x40,
 0xA0,
 0xA0,
 0x40,
 0x00,


 0x00,
 0xA0,
 0x40,
 0xA0,
 0x00,
 0x00,


 0x60,
 0xA0,
 0xE0,
 0xA0,
 0xC0,
 0x00,


 0x80,
 0x40,
 0xA0,
 0xA0,
 0xE0,
 0x00,


 0x20,
 0x40,
 0xA0,
 0xA0,
 0xE0,
 0x00,


 0x40,
 0x00,
 0xA0,
 0xA0,
 0xE0,
 0x00,


 0xA0,
 0x00,
 0xA0,
 0xA0,
 0xE0,
 0x00,


 0x20,
 0x00,
 0xA0,
 0x40,
 0x40,
 0x00,


 0x80,
 0xC0,
 0xA0,
 0xC0,
 0x80,
 0x00,


 0x40,
 0xA0,
 0xC0,
 0xA0,
 0xE0,
 0x80,


 0x80,
 0x40,
 0x60,
 0xA0,
 0x60,
 0x00,


 0x20,
 0x40,
 0x60,
 0xA0,
 0x60,
 0x00,


 0x60,
 0x00,
 0x60,
 0xA0,
 0x60,
 0x00,


 0x50,
 0xA0,
 0x60,
 0xA0,
 0x60,
 0x00,


 0xA0,
 0x00,
 0x60,
 0xA0,
 0x60,
 0x00,


 0x40,
 0x00,
 0x60,
 0xA0,
 0x60,
 0x00,


 0x00,
 0x70,
 0xB0,
 0xA0,
 0x70,
 0x00,


 0x00,
 0x40,
 0xA0,
 0x80,
 0x60,
 0x40,


 0x80,
 0x40,
 0xA0,
 0xC0,
 0x60,
 0x00,


 0x20,
 0x40,
 0xA0,
 0xC0,
 0x60,
 0x00,


 0xC0,
 0x40,
 0xA0,
 0xC0,
 0x60,
 0x00,


 0xA0,
 0x40,
 0xA0,
 0xC0,
 0x60,
 0x00,


 0x80,
 0x40,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x20,
 0xC0,
 0x40,
 0x40,
 0xE0,
 0x00,


 0x40,
 0xA0,
 0x40,
 0x40,
 0xE0,
 0x00,


 0xA0,
 0x00,
 0xC0,
 0x40,
 0xE0,
 0x00,


 0xA0,
 0x40,
 0x60,
 0xA0,
 0x40,
 0x00,


 0x50,
 0xA0,
 0xC0,
 0xA0,
 0xA0,
 0x00,


 0x80,
 0x40,
 0x40,
 0xA0,
 0x40,
 0x00,


 0x20,
 0x40,
 0x40,
 0xA0,
 0x40,
 0x00,


 0x40,
 0x00,
 0x40,
 0xA0,
 0x40,
 0x00,


 0xE0,
 0x00,
 0x40,
 0xA0,
 0x40,
 0x00,


 0xA0,
 0x00,
 0x40,
 0xA0,
 0x40,
 0x00,


 0x40,
 0x00,
 0xE0,
 0x00,
 0x40,
 0x00,


 0x00,
 0x60,
 0xA0,
 0xA0,
 0xC0,
 0x00,


 0x80,
 0x40,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0x20,
 0x40,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0x40,
 0x00,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0xA0,
 0x00,
 0xA0,
 0xA0,
 0x60,
 0x00,


 0x20,
 0x40,
 0xA0,
 0xE0,
 0x20,
 0xC0,


 0x80,
 0x80,
 0xC0,
 0xA0,
 0xC0,
 0x80,


 0xA0,
 0x00,
 0xA0,
 0xE0,
 0x20,
 0xC0,
};



static const unsigned short __apple3x5_index__[] = {
 0,
 32,
 33,
 34,
 35,
 36,
 37,
 38,
 39,
 40,
 41,
 42,
 43,
 44,
 45,
 46,
 47,
 48,
 49,
 50,
 51,
 52,
 53,
 54,
 55,
 56,
 57,
 58,
 59,
 60,
 61,
 62,
 63,
 64,
 65,
 66,
 67,
 68,
 69,
 70,
 71,
 72,
 73,
 74,
 75,
 76,
 77,
 78,
 79,
 80,
 81,
 82,
 83,
 84,
 85,
 86,
 87,
 88,
 89,
 90,
 91,
 92,
 93,
 94,
 95,
 96,
 97,
 98,
 99,
 100,
 101,
 102,
 103,
 104,
 105,
 106,
 107,
 108,
 109,
 110,
 111,
 112,
 113,
 114,
 115,
 116,
 117,
 118,
 119,
 120,
 121,
 122,
 123,
 124,
 125,
 126,
 160,
 161,
 162,
 163,
 164,
 165,
 166,
 167,
 168,
 169,
 170,
 171,
 172,
 173,
 174,
 175,
 176,
 177,
 178,
 179,
 180,
 181,
 182,
 183,
 184,
 185,
 186,
 187,
 188,
 189,
 190,
 191,
 192,
 193,
 194,
 195,
 196,
 197,
 198,
 199,
 200,
 201,
 202,
 203,
 204,
 205,
 206,
 207,
 208,
 209,
 210,
 211,
 212,
 213,
 214,
 215,
 216,
 217,
 218,
 219,
 220,
 221,
 222,
 223,
 224,
 225,
 226,
 227,
 228,
 229,
 230,
 231,
 232,
 233,
 234,
 235,
 236,
 237,
 238,
 239,
 240,
 241,
 242,
 243,
 244,
 245,
 246,
 247,
 248,
 249,
 250,
 251,
 252,
 253,
 254,
 255,
};

const font_t apple4x6_font = {
    .hdr = {
        .w = 4,
        .h = 6,
        .start = 32,
        .end = 255,
    },
    .bitmap = __apple3x5_bitmap__,
    .name = "4x6"
};


#ifdef FONTGEN
#include <stdio.h>
#include <inttypes.h>

#define ALIGN(x, alignment) (((x)+(alignment-1)) & ~(alignment-1))

int main(int argc, char **argv)
{
    const font_t *font = &apple4x6_font;
    if (argc<2)
        return -1;
    FILE *out;
    out = fopen(argv[1], "w");
    if (out==NULL) {
        perror("open");
        return -1;
    }
    fwrite( &font->hdr, sizeof(font->hdr),1, out );
    unsigned bpp = ALIGN((font->hdr.w-1),8) / 8;
    bpp *= ((font->hdr.end - font->hdr.start)+1);
    bpp *= font->hdr.h;
    fwrite( font->bitmap, bpp, 1, out);
    fclose( out );
}

#endif