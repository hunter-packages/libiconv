/*
 * Copyright (C) 1999-2001 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 59 Temple Place -
 * Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * TCVN-5712
 */

#include "flushwc.h"
#include "vietcomb.h"

static const unsigned char tcvn_comb_table[] = {
  0xb0, 0xb3, 0xb2, 0xb1, 0xb4,
};

static const unsigned short tcvn_2uni_1[32] = {
  /* 0x00 */
  0x0000, 0x00da, 0x1ee4, 0x0003, 0x1eea, 0x1eec, 0x1eee, 0x0007,
  0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
  /* 0x10 */
  0x0010, 0x1ee8, 0x1ef0, 0x1ef2, 0x1ef6, 0x1ef8, 0x00dd, 0x1ef4,
  0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
};
static const unsigned short tcvn_2uni_2[128] = {
  /* 0x80 */
  0x00c0, 0x1ea2, 0x00c3, 0x00c1, 0x1ea0, 0x1eb6, 0x1eac, 0x00c8,
  0x1eba, 0x1ebc, 0x00c9, 0x1eb8, 0x1ec6, 0x00cc, 0x1ec8, 0x0128,
  /* 0x90 */
  0x00cd, 0x1eca, 0x00d2, 0x1ece, 0x00d5, 0x00d3, 0x1ecc, 0x1ed8,
  0x1edc, 0x1ede, 0x1ee0, 0x1eda, 0x1ee2, 0x00d9, 0x1ee6, 0x0168,
  /* 0xa0 */
  0x00a0, 0x0102, 0x00c2, 0x00ca, 0x00d4, 0x01a0, 0x01af, 0x0110,
  0x0103, 0x00e2, 0x00ea, 0x00f4, 0x01a1, 0x01b0, 0x0111, 0x1eb0,
  /* 0xb0 */
  0x0300, 0x0309, 0x0303, 0x0301, 0x0323, 0x00e0, 0x1ea3, 0x00e3,
  0x00e1, 0x1ea1, 0x1eb2, 0x1eb1, 0x1eb3, 0x1eb5, 0x1eaf, 0x1eb4,
  /* 0xc0 */
  0x1eae, 0x1ea6, 0x1ea8, 0x1eaa, 0x1ea4, 0x1ec0, 0x1eb7, 0x1ea7,
  0x1ea9, 0x1eab, 0x1ea5, 0x1ead, 0x00e8, 0x1ec2, 0x1ebb, 0x1ebd,
  /* 0xd0 */
  0x00e9, 0x1eb9, 0x1ec1, 0x1ec3, 0x1ec5, 0x1ebf, 0x1ec7, 0x00ec,
  0x1ec9, 0x1ec4, 0x1ebe, 0x1ed2, 0x0129, 0x00ed, 0x1ecb, 0x00f2,
  /* 0xe0 */
  0x1ed4, 0x1ecf, 0x00f5, 0x00f3, 0x1ecd, 0x1ed3, 0x1ed5, 0x1ed7,
  0x1ed1, 0x1ed9, 0x1edd, 0x1edf, 0x1ee1, 0x1edb, 0x1ee3, 0x00f9,
  /* 0xf0 */
  0x1ed6, 0x1ee7, 0x0169, 0x00fa, 0x1ee5, 0x1eeb, 0x1eed, 0x1eef,
  0x1ee9, 0x1ef1, 0x1ef3, 0x1ef7, 0x1ef9, 0x00fd, 0x1ef5, 0x1ed0,
};

/* In the TCVN to Unicode direction, the state contains a buffered
   character, or 0 if none. */

static int
tcvn_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *s, int n)
{
  unsigned char c = *s;
  unsigned short wc;
  unsigned short last_wc;
  if (c < 0x20)
    wc = tcvn_2uni_1[c];
  else if (c < 0x80)
    wc = c;
  else
    wc = tcvn_2uni_2[c-0x80];
  last_wc = conv->istate;
  if (last_wc) {
    if (wc >= 0x0300 && wc < 0x0340) {
      /* See whether last_wc and wc can be combined. */
      unsigned int k;
      unsigned int i1, i2;
      switch (wc) {
        case 0x0300: k = 0; break;
        case 0x0301: k = 1; break;
        case 0x0303: k = 2; break;
        case 0x0309: k = 3; break;
        case 0x0323: k = 4; break;
        default: abort();
      }
      i1 = viet_comp_table[k].idx;
      i2 = i1 + viet_comp_table[k].len-1;
      if (last_wc >= viet_comp_table_data[i1].base
          && last_wc <= viet_comp_table_data[i2].base) {
        unsigned int i;
        for (;;) {
          i = (i1+i2)>>1;
          if (last_wc == viet_comp_table_data[i].base)
            break;
          if (last_wc < viet_comp_table_data[i].base) {
            if (i1 == i)
              goto not_combining;
            i2 = i;
          } else {
            if (i1 != i)
              i1 = i;
            else {
              i = i2;
              if (last_wc == viet_comp_table_data[i].base)
                break;
              goto not_combining;
            }
          }
        }
        last_wc = viet_comp_table_data[i].composed;
        /* Output the combined character. */
        conv->istate = 0;
        *pwc = (ucs4_t) last_wc;
        return 1;
      }
    }
  not_combining:
    /* Output the buffered character. */
    conv->istate = 0;
    *pwc = (ucs4_t) last_wc;
    return 0; /* Don't advance the input pointer. */
  }
  if (wc >= 0x0041 && wc <= 0x01b0) {
    /* wc is a possible match in viet_comp_table_data. Buffer it. */
    conv->istate = wc;
    return RET_TOOFEW(1);
  } else {
    /* Output wc immediately. */
    *pwc = (ucs4_t) wc;
    return 1;
  }
}

#define tcvn_flushwc normal_flushwc

static const unsigned char tcvn_page00[96+184] = {
  0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa0-0xa7 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa8-0xaf */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */
  0x80, 0x83, 0xa2, 0x82, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */
  0x87, 0x8a, 0xa3, 0x00, 0x8d, 0x90, 0x00, 0x00, /* 0xc8-0xcf */
  0x00, 0x00, 0x92, 0x95, 0xa4, 0x94, 0x00, 0x00, /* 0xd0-0xd7 */
  0x00, 0x9d, 0x01, 0x00, 0x00, 0x16, 0x00, 0x00, /* 0xd8-0xdf */
  0xb5, 0xb8, 0xa9, 0xb7, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */
  0xcc, 0xd0, 0xaa, 0x00, 0xd7, 0xdd, 0x00, 0x00, /* 0xe8-0xef */
  0x00, 0x00, 0xdf, 0xe3, 0xab, 0xe2, 0x00, 0x00, /* 0xf0-0xf7 */
  0x00, 0xef, 0xf3, 0x00, 0x00, 0xfd, 0x00, 0x00, /* 0xf8-0xff */
  /* 0x0100 */
  0x00, 0x00, 0xa1, 0xa8, 0x00, 0x00, 0x00, 0x00, /* 0x00-0x07 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x08-0x0f */
  0xa7, 0xae, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x10-0x17 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x18-0x1f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
  0x8f, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40-0x47 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x48-0x4f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x57 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x58-0x5f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
  0x9f, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x68-0x6f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x78-0x7f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */
  0xa5, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa0-0xa7 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa6, /* 0xa8-0xaf */
  0xad, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb0-0xb7 */
};
static const unsigned char tcvn_page03[40] = {
  0xb0, 0xb3, 0x00, 0xb2, 0x00, 0x00, 0x00, 0x00, /* 0x00-0x07 */
  0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x08-0x0f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x10-0x17 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x18-0x1f */
  0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */
};
static const unsigned char tcvn_page1e[96] = {
  0x84, 0xb9, 0x81, 0xb6, 0xc4, 0xca, 0xc1, 0xc7, /* 0xa0-0xa7 */
  0xc2, 0xc8, 0xc3, 0xc9, 0x86, 0xcb, 0xc0, 0xbe, /* 0xa8-0xaf */
  0xaf, 0xbb, 0xba, 0xbc, 0xbf, 0xbd, 0x85, 0xc6, /* 0xb0-0xb7 */
  0x8b, 0xd1, 0x88, 0xce, 0x89, 0xcf, 0xda, 0xd5, /* 0xb8-0xbf */
  0xc5, 0xd2, 0xcd, 0xd3, 0xd9, 0xd4, 0x8c, 0xd6, /* 0xc0-0xc7 */
  0x8e, 0xd8, 0x91, 0xde, 0x96, 0xe4, 0x93, 0xe1, /* 0xc8-0xcf */
  0xff, 0xe8, 0xdb, 0xe5, 0xe0, 0xe6, 0xf0, 0xe7, /* 0xd0-0xd7 */
  0x97, 0xe9, 0x9b, 0xed, 0x98, 0xea, 0x99, 0xeb, /* 0xd8-0xdf */
  0x9a, 0xec, 0x9c, 0xee, 0x02, 0xf4, 0x9e, 0xf1, /* 0xe0-0xe7 */
  0x11, 0xf8, 0x04, 0xf5, 0x05, 0xf6, 0x06, 0xf7, /* 0xe8-0xef */
  0x12, 0xf9, 0x13, 0xfa, 0x17, 0xfe, 0x14, 0xfb, /* 0xf0-0xf7 */
  0x15, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xf8-0xff */
};

static int
tcvn_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n)
{
  unsigned char c = 0;
  if (wc < 0x0080 && (wc >= 0x0020 || (0x00fe0076 & (1 << wc)) == 0)) {
    *r = wc;
    return 1;
  }
  else if (wc >= 0x00a0 && wc < 0x01b8)
    c = tcvn_page00[wc-0x00a0];
  else if (wc >= 0x0300 && wc < 0x0328)
    c = tcvn_page03[wc-0x0300];
  else if (wc >= 0x0340 && wc < 0x0342) /* deprecated Vietnamese tone marks */
    c = tcvn_page03[wc-0x0340];
  else if (wc >= 0x1ea0 && wc < 0x1f00)
    c = tcvn_page1e[wc-0x1ea0];
  if (c != 0) {
    *r = c;
    return 1;
  }
  /* Try compatibility or canonical decomposition. */
  {
    /* Binary search through viet_decomp_table. */
    unsigned int i1 = 0;
    unsigned int i2 = sizeof(viet_decomp_table)/sizeof(viet_decomp_table[0])-1;
    if (wc >= viet_decomp_table[i1].composed
        && wc <= viet_decomp_table[i2].composed) {
      unsigned int i;
      for (;;) {
        /* Here i2 - i1 > 0. */
        i = (i1+i2)>>1;
        if (wc == viet_decomp_table[i].composed)
          break;
        if (wc < viet_decomp_table[i].composed) {
          if (i1 == i)
            return RET_ILUNI;
          /* Here i1 < i < i2. */
          i2 = i;
        } else {
          /* Here i1 <= i < i2. */
          if (i1 != i)
            i1 = i;
          else {
            /* Here i2 - i1 = 1. */
            i = i2;
            if (wc == viet_decomp_table[i].composed)
              break;
            else
              return RET_ILUNI;
          }
        }
      }
      /* Found a compatibility or canonical decomposition. */
      wc = viet_decomp_table[i].base;
      /* wc is one of 0x0020, 0x0041..0x005a, 0x0061..0x007a, 0x00a5, 0x00a8,
         0x00c2, 0x00c5..0x00c7, 0x00ca, 0x00cf, 0x00d3, 0x00d4, 0x00d6,
         0x00d8, 0x00da, 0x00dc, 0x00e2, 0x00e5..0x00e7, 0x00ea, 0x00ef,
         0x00f3, 0x00f4, 0x00f6, 0x00f8, 0x00fc, 0x0102, 0x0103, 0x01a0,
         0x01a1, 0x01af, 0x01b0. */
      if (wc < 0x0080)
        c = wc;
      else {
        c = tcvn_page00[wc-0x00a0];
        if (c == 0)
          return RET_ILUNI;
      }
      if (n < 2)
        return RET_TOOSMALL;
      r[0] = c;
      r[1] = tcvn_comb_table[viet_decomp_table[i].comb1];
      return 2;
    }
  }
  return RET_ILUNI;
}
