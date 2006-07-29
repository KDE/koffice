/*
   dcraw.c -- Dave Coffin's raw photo decoder
   Copyright 1997-2005 by Dave Coffin, dcoffin a cybercom o net

   This is a command-line ANSI C program to convert raw photos from
   any digital camera on any computer running any operating system.

   Attention!  Some parts of this program are restricted under the
   terms of the GNU General Public License.  Such code is enclosed
   in "BEGIN GPL BLOCK" and "END GPL BLOCK" declarations.
   Any code not declared GPL is free for all uses.

   Starting in Revision 1.237, the code to support Foveon cameras
   is under GPL.

   To lawfully redistribute dcraw.c, you must either (a) include
   full source code for all executable files containing restricted
   functions, (b) remove these functions, re-implement them, or
   copy them from an earlier, non-GPL Revision of dcraw.c, or (c)
   purchase a license from the author.

   $Revision: 1.296 $
   $Date: 2005/11/04 07:11:14 $
 */

#define _GNU_SOURCE
#define _USE_MATH_DEFINES
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/*
   By defining NO_JPEG, you lose only the ability to
   decode compressed .KDC files from the Kodak DC120.
 */
#ifndef NO_JPEG
#include <jpeglib.h>
#endif

#ifdef __CYGWIN__
#include <io.h>
#endif
#ifdef WIN32
#include <sys/utime.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#define strcasecmp stricmp
typedef __int64 INT64;
typedef unsigned __int64 UINT64;
#else
#include <unistd.h>
#include <utime.h>
#include <netinet/in.h>
typedef long long INT64;
typedef unsigned long long UINT64;
#endif

#ifdef LJPEG_DECODE
#error Please compile dcraw.c by itself.
#error Do not link it with ljpeg_decode.
#endif

#ifndef LONG_BIT
#define LONG_BIT (8 * sizeof (long))
#endif

#define ushort UshORt
typedef unsigned char uchar;
typedef unsigned short ushort;

/*
   All global variables are defined here, and all functions that
   access them are prefixed with "CLASS".  Note that a thread-safe
   C++ class cannot have non-const static local variables.
 */
FILE *ifp;
short order;
char *ifname, make[64], model[70], model2[64], *meta_data;
float flash_used, canon_5814;
time_t timestamp;
unsigned shot_order, kodak_cbpp;
int data_offset, meta_offset, meta_length, nikon_curve_offset;
int tiff_bps, tiff_data_compression, kodak_data_compression;
int raw_height, raw_width, top_margin, left_margin;
int height, width, fuji_width, colors, tiff_samples;
int black, maximum, clip_max, clip_color=1;
int iheight, iwidth, shrink;
int dng_version, is_foveon, raw_color, use_gamma;
int flip, xmag, ymag;
int zero_after_ff;
unsigned filters;
ushort (*image)[4], white[8][8], curve[0x1000];
void (*load_raw)();
float bright=1, red_scale=1, blue_scale=1, sigma_d=0, sigma_r=0;
int four_color_rgb=0, document_mode=0;
int verbose=0, use_auto_wb=0, use_camera_wb=0, use_camera_rgb=0;
int fuji_layout, fuji_secondary, use_secondary=0;
float cam_mul[4], pre_mul[4], rgb_cam[3][4];	/* RGB from camera color */
const double xyz_rgb[3][3] = {			/* XYZ from RGB */
  { 0.412453, 0.357580, 0.180423 },
  { 0.212671, 0.715160, 0.072169 },
  { 0.019334, 0.119193, 0.950227 } };
#define camera_red  cam_mul[0]
#define camera_blue cam_mul[2]
int histogram[3][0x2000];
void write_ppm(FILE *);
void (*write_fun)(FILE *) = write_ppm;
jmp_buf failure;

#define USE_LCMS
#ifdef USE_LCMS
#include <lcms.h>
int profile_offset, profile_length;
#endif

struct decode {
  struct decode *branch[2];
  int leaf;
} first_decode[2048], *second_decode, *free_decode;

#define CLASS

#define FORC3 for (c=0; c < 3; c++)
#define FORC4 for (c=0; c < 4; c++)
#define FORCC for (c=0; c < colors; c++)

#define SQR(x) ((x)*(x))
#define ABS(x) (((int)(x) ^ ((int)(x) >> 31)) - ((int)(x) >> 31))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LIM(x,min,max) MAX(min,MIN(x,max))
#define ULIM(x,y,z) ((y) < (z) ? LIM(x,y,z) : LIM(x,z,y))
#define CLIP(x) LIM(x,0,clip_max)

/*
   In order to inline this calculation, I make the risky
   assumption that all filter patterns can be described
   by a repeating pattern of eight rows and two columns

   Return values are either 0/1/2/3 = G/M/C/Y or 0/1/2/3 = R/G1/B/G2
 */
#define FC(row,col) \
	(filters >> ((((row) << 1 & 14) + ((col) & 1)) << 1) & 3)

#define BAYER(row,col) \
	image[((row) >> shrink)*iwidth + ((col) >> shrink)][FC(row,col)]

/*
	PowerShot 600	PowerShot A50	PowerShot Pro70	Pro90 & G1
	0xe1e4e1e4:	0x1b4e4b1e:	0x1e4b4e1b:	0xb4b4b4b4:

	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
	0 G M G M G M	0 C Y C Y C Y	0 Y C Y C Y C	0 G M G M G M
	1 C Y C Y C Y	1 M G M G M G	1 M G M G M G	1 Y C Y C Y C
	2 M G M G M G	2 Y C Y C Y C	2 C Y C Y C Y
	3 C Y C Y C Y	3 G M G M G M	3 G M G M G M
			4 C Y C Y C Y	4 Y C Y C Y C
	PowerShot A5	5 G M G M G M	5 G M G M G M
	0x1e4e1e4e:	6 Y C Y C Y C	6 C Y C Y C Y
			7 M G M G M G	7 M G M G M G
	  0 1 2 3 4 5
	0 C Y C Y C Y
	1 G M G M G M
	2 C Y C Y C Y
	3 M G M G M G

   All RGB cameras use one of these Bayer grids:

	0x16161616:	0x61616161:	0x49494949:	0x94949494:

	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5	  0 1 2 3 4 5
	0 B G B G B G	0 G R G R G R	0 G B G B G B	0 R G R G R G
	1 G R G R G R	1 B G B G B G	1 R G R G R G	1 G B G B G B
	2 B G B G B G	2 G R G R G R	2 G B G B G B	2 R G R G R G
	3 G R G R G R	3 B G B G B G	3 R G R G R G	3 G B G B G B
 */

#ifndef __GLIBC__
char *memmem (char *haystack, size_t haystacklen,
	      char *needle, size_t needlelen)
{
  char *c;
  for (c = haystack; c <= haystack + haystacklen - needlelen; c++)
    if (!memcmp (c, needle, needlelen))
      return c;
  return NULL;
}
#endif

void CLASS merror (void *ptr, char *where)
{
  if (ptr) return;
  fprintf (stderr, "%s: Out of memory in %s\n", ifname, where);
  longjmp (failure, 1);
}

ushort CLASS sget2 (uchar *s)
{
  if (order == 0x4949)		/* "II" means little-endian */
    return s[0] | s[1] << 8;
  else				/* "MM" means big-endian */
    return s[0] << 8 | s[1];
}

ushort CLASS get2()
{
  uchar str[2] = { 0xff,0xff };
  fread (str, 1, 2, ifp);
  return sget2(str);
}

int CLASS sget4 (uchar *s)
{
  if (order == 0x4949)
    return s[0] | s[1] << 8 | s[2] << 16 | s[3] << 24;
  else
    return s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3];
}
#define sget4(s) sget4((uchar *)s)

int CLASS get4()
{
  uchar str[4] = { 0xff,0xff,0xff,0xff };
  fread (str, 1, 4, ifp);
  return sget4(str);
}

double CLASS getrat()
{
  double num = get4();
  return num / get4();
}

float CLASS int_to_float (int i)
{
  union { int i; float f; } u;
  u.i = i;
  return u.f;
}

void CLASS read_shorts (ushort *pixel, int count)
{
  fread (pixel, 2, count, ifp);
  if ((order == 0x4949) == (ntohs(0x1234) == 0x1234))
    swab (pixel, pixel, count*2);
}

void CLASS canon_600_fixed_wb (int temp)
{
  static const short mul[4][5] = {
    {  667, 358,397,565,452 },
    {  731, 390,367,499,517 },
    { 1119, 396,348,448,537 },
    { 1399, 485,431,508,688 } };
  int lo, hi, i;
  float frac=0;

  for (lo=4; --lo; )
    if (*mul[lo] <= temp) break;
  for (hi=0; hi < 3; hi++)
    if (*mul[hi] >= temp) break;
  if (lo != hi)
    frac = (float) (temp - *mul[lo]) / (*mul[hi] - *mul[lo]);
  for (i=1; i < 5; i++)
    pre_mul[i-1] = 1 / (frac * mul[hi][i] + (1-frac) * mul[lo][i]);
}

/* Return values:  0 = white  1 = near white  2 = not white */
int CLASS canon_600_color (int ratio[2], int mar)
{
  int clipped=0, target, miss;

  if (flash_used) {
    if (ratio[1] < -104)
      { ratio[1] = -104; clipped = 1; }
    if (ratio[1] >   12)
      { ratio[1] =   12; clipped = 1; }
  } else {
    if (ratio[1] < -264 || ratio[1] > 461) return 2;
    if (ratio[1] < -50)
      { ratio[1] = -50; clipped = 1; }
    if (ratio[1] > 307)
      { ratio[1] = 307; clipped = 1; }
  }
  target = flash_used || ratio[1] < 197
	? -38 - (398 * ratio[1] >> 10)
	: -123 + (48 * ratio[1] >> 10);
  if (target - mar <= ratio[0] &&
      target + 20  >= ratio[0] && !clipped) return 0;
  miss = target - ratio[0];
  if (abs(miss) >= mar*4) return 2;
  if (miss < -20) miss = -20;
  if (miss > mar) miss = mar;
  ratio[0] = target - miss;
  return 1;
}

void CLASS canon_600_auto_wb ()
{
  int mar, row, col, i, j, st, count[] = { 0,0 };
  int test[8], total[2][8], ratio[2][2], stat[2];

  memset (&total, 0, sizeof total);
  i = canon_5814 + 0.5;
  if      (i < 10) mar = 150;
  else if (i > 12) mar = 20;
  else mar = 280 - 20 * i;
  if (flash_used) mar = 80;
  for (row=14; row < height-14; row+=4)
    for (col=10; col < width; col+=2) {
      for (i=0; i < 8; i++)
	test[(i & 4) + FC(row+(i >> 1),col+(i & 1))] =
		    BAYER(row+(i >> 1),col+(i & 1));
      for (i=0; i < 8; i++)
	if (test[i] < 150 || test[i] > 1500) goto next;
      for (i=0; i < 4; i++)
	if (abs(test[i] - test[i+4]) > 50) goto next;
      for (i=0; i < 2; i++) {
	for (j=0; j < 4; j+=2)
	  ratio[i][j >> 1] = ((test[i*4+j+1]-test[i*4+j]) << 10) / test[i*4+j];
	stat[i] = canon_600_color (ratio[i], mar);
      }
      if ((st = stat[0] | stat[1]) > 1) goto next;
      for (i=0; i < 2; i++)
	if (stat[i])
	  for (j=0; j < 2; j++)
	    test[i*4+j*2+1] = test[i*4+j*2] * (0x400 + ratio[i][j]) >> 10;
      for (i=0; i < 8; i++)
	total[st][i] += test[i];
      count[st]++;
next: continue;
    }
  if (count[0] | count[1]) {
    st = count[0]*200 < count[1];
    for (i=0; i < 4; i++)
      pre_mul[i] = 1.0 / (total[st][i] + total[st][i+4]);
  }
}

void CLASS canon_600_coeff ()
{
  static const short table[6][12] = {
    { -190,702,-1878,2390,   1861,-1349,905,-393, -432,944,2617,-2105  },
    { -1203,1715,-1136,1648, 1388,-876,267,245,  -1641,2153,3921,-3409 },
    { -615,1127,-1563,2075,  1437,-925,509,3,     -756,1268,2519,-2007 },
    { -190,702,-1886,2398,   2153,-1641,763,-251, -452,964,3040,-2528  },
    { -190,702,-1878,2390,   1861,-1349,905,-393, -432,944,2617,-2105  },
    { -807,1319,-1785,2297,  1388,-876,769,-257,  -230,742,2067,-1555  } };
  int t=0, i, c;
  float mc, yc;

  mc = pre_mul[1] / pre_mul[2];
  yc = pre_mul[3] / pre_mul[2];
  if (mc > 1 && mc <= 1.28 && yc < 0.8789) t=1;
  if (mc > 1.28 && mc <= 2) {
    if  (yc < 0.8789) t=3;
    else if (yc <= 2) t=4;
  }
  if (flash_used) t=5;
  for (raw_color = i=0; i < 3; i++)
    FORCC rgb_cam[i][c] = table[t][i*4 + c] / 1024.0;
}

void CLASS canon_600_load_raw()
{
  uchar  data[1120], *dp;
  ushort pixel[896], *pix;
  int irow, row, col, val;
  static const short mul[4][2] =
  { { 1141,1145 }, { 1128,1109 }, { 1178,1149 }, { 1128,1109 } };

  for (irow=row=0; irow < height; irow++)
  {
    fread (data, 1120, 1, ifp);
    for (dp=data, pix=pixel; dp < data+1120; dp+=10, pix+=8)
    {
      pix[0] = (dp[0] << 2) + (dp[1] >> 6    );
      pix[1] = (dp[2] << 2) + (dp[1] >> 4 & 3);
      pix[2] = (dp[3] << 2) + (dp[1] >> 2 & 3);
      pix[3] = (dp[4] << 2) + (dp[1]      & 3);
      pix[4] = (dp[5] << 2) + (dp[9]      & 3);
      pix[5] = (dp[6] << 2) + (dp[9] >> 2 & 3);
      pix[6] = (dp[7] << 2) + (dp[9] >> 4 & 3);
      pix[7] = (dp[8] << 2) + (dp[9] >> 6    );
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = pixel[col];
    for (col=width; col < 896; col++)
      black += pixel[col];
    if ((row+=2) > height) row = 1;
  }
  black = black / ((896 - width) * height) - 4;
  for (row=0; row < height; row++)
    for (col=0; col < width; col++) {
      val = (BAYER(row,col) - black) * mul[row & 3][col & 1] >> 9;
      if (val < 0) val = 0;
      BAYER(row,col) = val;
    }
  canon_600_fixed_wb(1311);
  canon_600_auto_wb();
  canon_600_coeff();
  maximum = (0x3ff - black) * 1109 >> 9;
  black = 0;
}

void CLASS canon_a5_load_raw()
{
  uchar  data[1940], *dp;
  ushort pixel[1552], *pix;
  int row, col;

  for (row=0; row < height; row++) {
    fread (data, raw_width * 10 / 8, 1, ifp);
    for (dp=data, pix=pixel; pix < pixel+raw_width; dp+=10, pix+=8)
    {
      pix[0] = (dp[1] << 2) + (dp[0] >> 6);
      pix[1] = (dp[0] << 4) + (dp[3] >> 4);
      pix[2] = (dp[3] << 6) + (dp[2] >> 2);
      pix[3] = (dp[2] << 8) + (dp[5]     );
      pix[4] = (dp[4] << 2) + (dp[7] >> 6);
      pix[5] = (dp[7] << 4) + (dp[6] >> 4);
      pix[6] = (dp[6] << 6) + (dp[9] >> 2);
      pix[7] = (dp[9] << 8) + (dp[8]     );
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = (pixel[col] & 0x3ff);
    for (col=width; col < raw_width; col++)
      black += pixel[col] & 0x3ff;
  }
  if (raw_width > width)
    black /= (raw_width - width) * height;
  maximum = 0x3ff;
}

/*
   getbits(-1) initializes the buffer
   getbits(n) where 0 <= n <= 25 returns an n-bit integer
 */
unsigned CLASS getbits (int nbits)
{
  static unsigned bitbuf=0;
  static int vbits=0, reset=0;
  unsigned c;

  if (nbits == -1)
    return bitbuf = vbits = reset = 0;
  if (nbits == 0 || reset) return 0;
  while (vbits < nbits) {
    c = fgetc(ifp);
    if ((reset = zero_after_ff && c == 0xff && fgetc(ifp))) return 0;
    bitbuf = (bitbuf << 8) + c;
    vbits += 8;
  }
  vbits -= nbits;
  return bitbuf << (32-nbits-vbits) >> (32-nbits);
}

void CLASS init_decoder ()
{
  memset (first_decode, 0, sizeof first_decode);
  free_decode = first_decode;
}

/*
   Construct a decode tree according the specification in *source.
   The first 16 bytes specify how many codes should be 1-bit, 2-bit
   3-bit, etc.  Bytes after that are the leaf values.

   For example, if the source is

    { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
      0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

   then the code is

	00		0x04
	010		0x03
	011		0x05
	100		0x06
	101		0x02
	1100		0x07
	1101		0x01
	11100		0x08
	11101		0x09
	11110		0x00
	111110		0x0a
	1111110		0x0b
	1111111		0xff
 */
uchar * CLASS make_decoder (const uchar *source, int level)
{
  struct decode *cur;
  static int leaf;
  int i, next;

  if (level==0) leaf=0;
  cur = free_decode++;
  if (free_decode > first_decode+2048) {
    fprintf (stderr, "%s: decoder table overflow\n", ifname);
    longjmp (failure, 2);
  }
  for (i=next=0; i <= leaf && next < 16; )
    i += source[next++];
  if (i > leaf) {
    if (level < next) {
      cur->branch[0] = free_decode;
      make_decoder (source, level+1);
      cur->branch[1] = free_decode;
      make_decoder (source, level+1);
    } else
      cur->leaf = source[16 + leaf++];
  }
  return (uchar *) source + 16 + leaf;
}

void CLASS crw_init_tables (unsigned table)
{
  static const uchar first_tree[3][29] = {
    { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
      0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },
    { 0,2,2,3,1,1,1,1,2,0,0,0,0,0,0,0,
      0x03,0x02,0x04,0x01,0x05,0x00,0x06,0x07,0x09,0x08,0x0a,0x0b,0xff  },
    { 0,0,6,3,1,1,2,0,0,0,0,0,0,0,0,0,
      0x06,0x05,0x07,0x04,0x08,0x03,0x09,0x02,0x00,0x0a,0x01,0x0b,0xff  },
  };
  static const uchar second_tree[3][180] = {
    { 0,2,2,2,1,4,2,1,2,5,1,1,0,0,0,139,
      0x03,0x04,0x02,0x05,0x01,0x06,0x07,0x08,
      0x12,0x13,0x11,0x14,0x09,0x15,0x22,0x00,0x21,0x16,0x0a,0xf0,
      0x23,0x17,0x24,0x31,0x32,0x18,0x19,0x33,0x25,0x41,0x34,0x42,
      0x35,0x51,0x36,0x37,0x38,0x29,0x79,0x26,0x1a,0x39,0x56,0x57,
      0x28,0x27,0x52,0x55,0x58,0x43,0x76,0x59,0x77,0x54,0x61,0xf9,
      0x71,0x78,0x75,0x96,0x97,0x49,0xb7,0x53,0xd7,0x74,0xb6,0x98,
      0x47,0x48,0x95,0x69,0x99,0x91,0xfa,0xb8,0x68,0xb5,0xb9,0xd6,
      0xf7,0xd8,0x67,0x46,0x45,0x94,0x89,0xf8,0x81,0xd5,0xf6,0xb4,
      0x88,0xb1,0x2a,0x44,0x72,0xd9,0x87,0x66,0xd4,0xf5,0x3a,0xa7,
      0x73,0xa9,0xa8,0x86,0x62,0xc7,0x65,0xc8,0xc9,0xa1,0xf4,0xd1,
      0xe9,0x5a,0x92,0x85,0xa6,0xe7,0x93,0xe8,0xc1,0xc6,0x7a,0x64,
      0xe1,0x4a,0x6a,0xe6,0xb3,0xf1,0xd3,0xa5,0x8a,0xb2,0x9a,0xba,
      0x84,0xa4,0x63,0xe5,0xc5,0xf3,0xd2,0xc4,0x82,0xaa,0xda,0xe4,
      0xf2,0xca,0x83,0xa3,0xa2,0xc3,0xea,0xc2,0xe2,0xe3,0xff,0xff  },
    { 0,2,2,1,4,1,4,1,3,3,1,0,0,0,0,140,
      0x02,0x03,0x01,0x04,0x05,0x12,0x11,0x06,
      0x13,0x07,0x08,0x14,0x22,0x09,0x21,0x00,0x23,0x15,0x31,0x32,
      0x0a,0x16,0xf0,0x24,0x33,0x41,0x42,0x19,0x17,0x25,0x18,0x51,
      0x34,0x43,0x52,0x29,0x35,0x61,0x39,0x71,0x62,0x36,0x53,0x26,
      0x38,0x1a,0x37,0x81,0x27,0x91,0x79,0x55,0x45,0x28,0x72,0x59,
      0xa1,0xb1,0x44,0x69,0x54,0x58,0xd1,0xfa,0x57,0xe1,0xf1,0xb9,
      0x49,0x47,0x63,0x6a,0xf9,0x56,0x46,0xa8,0x2a,0x4a,0x78,0x99,
      0x3a,0x75,0x74,0x86,0x65,0xc1,0x76,0xb6,0x96,0xd6,0x89,0x85,
      0xc9,0xf5,0x95,0xb4,0xc7,0xf7,0x8a,0x97,0xb8,0x73,0xb7,0xd8,
      0xd9,0x87,0xa7,0x7a,0x48,0x82,0x84,0xea,0xf4,0xa6,0xc5,0x5a,
      0x94,0xa4,0xc6,0x92,0xc3,0x68,0xb5,0xc8,0xe4,0xe5,0xe6,0xe9,
      0xa2,0xa3,0xe3,0xc2,0x66,0x67,0x93,0xaa,0xd4,0xd5,0xe7,0xf8,
      0x88,0x9a,0xd7,0x77,0xc4,0x64,0xe2,0x98,0xa5,0xca,0xda,0xe8,
      0xf3,0xf6,0xa9,0xb2,0xb3,0xf2,0xd2,0x83,0xba,0xd3,0xff,0xff  },
    { 0,0,6,2,1,3,3,2,5,1,2,2,8,10,0,117,
      0x04,0x05,0x03,0x06,0x02,0x07,0x01,0x08,
      0x09,0x12,0x13,0x14,0x11,0x15,0x0a,0x16,0x17,0xf0,0x00,0x22,
      0x21,0x18,0x23,0x19,0x24,0x32,0x31,0x25,0x33,0x38,0x37,0x34,
      0x35,0x36,0x39,0x79,0x57,0x58,0x59,0x28,0x56,0x78,0x27,0x41,
      0x29,0x77,0x26,0x42,0x76,0x99,0x1a,0x55,0x98,0x97,0xf9,0x48,
      0x54,0x96,0x89,0x47,0xb7,0x49,0xfa,0x75,0x68,0xb6,0x67,0x69,
      0xb9,0xb8,0xd8,0x52,0xd7,0x88,0xb5,0x74,0x51,0x46,0xd9,0xf8,
      0x3a,0xd6,0x87,0x45,0x7a,0x95,0xd5,0xf6,0x86,0xb4,0xa9,0x94,
      0x53,0x2a,0xa8,0x43,0xf5,0xf7,0xd4,0x66,0xa7,0x5a,0x44,0x8a,
      0xc9,0xe8,0xc8,0xe7,0x9a,0x6a,0x73,0x4a,0x61,0xc7,0xf4,0xc6,
      0x65,0xe9,0x72,0xe6,0x71,0x91,0x93,0xa6,0xda,0x92,0x85,0x62,
      0xf3,0xc5,0xb2,0xa4,0x84,0xba,0x64,0xa5,0xb3,0xd2,0x81,0xe5,
      0xd3,0xaa,0xc4,0xca,0xf2,0xb1,0xe4,0xd1,0x83,0x63,0xea,0xc3,
      0xe2,0x82,0xf1,0xa3,0xc2,0xa1,0xc1,0xe3,0xa2,0xe1,0xff,0xff  }
  };
  if (table > 2) table = 2;
  init_decoder();
  make_decoder ( first_tree[table], 0);
  second_decode = free_decode;
  make_decoder (second_tree[table], 0);
}

/*
   Return 0 if the image starts with compressed data,
   1 if it starts with uncompressed low-order bits.

   In Canon compressed data, 0xff is always followed by 0x00.
 */
int CLASS canon_has_lowbits()
{
  uchar test[0x4000];
  int ret=1, i;

  fseek (ifp, 0, SEEK_SET);
  fread (test, 1, sizeof test, ifp);
  for (i=540; i < sizeof test - 1; i++)
    if (test[i] == 0xff) {
      if (test[i+1]) return 1;
      ret=0;
    }
  return ret;
}

void CLASS canon_compressed_load_raw()
{
  ushort *pixel, *prow;
  int lowbits, i, row, r, col, save, val;
  unsigned irow, icol;
  struct decode *decode, *dindex;
  int block, diffbuf[64], leaf, len, diff, carry=0, pnum=0, base[2];
  uchar c;

  pixel = calloc (raw_width*8, sizeof *pixel);
  merror (pixel, "canon_compressed_load_raw()");
  lowbits = canon_has_lowbits();
  if (!lowbits) maximum = 0x3ff;
  fseek (ifp, 540 + lowbits*raw_height*raw_width/4, SEEK_SET);
  zero_after_ff = 1;
  getbits(-1);
  for (row = 0; row < raw_height; row += 8) {
    for (block=0; block < raw_width >> 3; block++) {
      memset (diffbuf, 0, sizeof diffbuf);
      decode = first_decode;
      for (i=0; i < 64; i++ ) {
	for (dindex=decode; dindex->branch[0]; )
	  dindex = dindex->branch[getbits(1)];
	leaf = dindex->leaf;
	decode = second_decode;
	if (leaf == 0 && i) break;
	if (leaf == 0xff) continue;
	i  += leaf >> 4;
	len = leaf & 15;
	if (len == 0) continue;
	diff = getbits(len);
	if ((diff & (1 << (len-1))) == 0)
	  diff -= (1 << len) - 1;
	if (i < 64) diffbuf[i] = diff;
      }
      diffbuf[0] += carry;
      carry = diffbuf[0];
      for (i=0; i < 64; i++ ) {
	if (pnum++ % raw_width == 0)
	  base[0] = base[1] = 512;
	pixel[(block << 6) + i] = ( base[i & 1] += diffbuf[i] );
      }
    }
    if (lowbits) {
      save = ftell(ifp);
      fseek (ifp, 26 + row*raw_width/4, SEEK_SET);
      for (prow=pixel, i=0; i < raw_width*2; i++) {
	c = fgetc(ifp);
	for (r=0; r < 8; r+=2, prow++) {
	  val = (*prow << 2) + ((c >> r) & 3);
	  if (raw_width == 2672 && val < 512) val += 2;
	  *prow = val;
	}
      }
      fseek (ifp, save, SEEK_SET);
    }
    for (r=0; r < 8; r++) {
      irow = row - top_margin + r;
      if (irow >= height) continue;
      for (col = 0; col < raw_width; col++) {
	icol = col - left_margin;
	if (icol < width)
	  BAYER(irow,icol) = pixel[r*raw_width+col];
	else
	  black += pixel[r*raw_width+col];
      }
    }
  }
  free (pixel);
  if (raw_width > width)
    black /= (raw_width - width) * height;
}

/*
   Not a full implementation of Lossless JPEG, just
   enough to decode Canon, Kodak and Adobe DNG images.
 */
struct jhead {
  int bits, high, wide, clrs, restart, vpred[4];
  struct decode *huff[4];
  ushort *row;
};

int CLASS ljpeg_start (struct jhead *jh)
{
  int i, tag, len;
  uchar data[256], *dp;

  init_decoder();
  for (i=0; i < 4; i++)
    jh->huff[i] = free_decode;
  jh->restart = INT_MAX;
  fread (data, 2, 1, ifp);
  if (data[0] != 0xff || data[1] != 0xd8) return 0;
  do {
    fread (data, 2, 2, ifp);
    tag =  data[0] << 8 | data[1];
    len = (data[2] << 8 | data[3]) - 2;
    if (tag <= 0xff00 || len > 255) return 0;
    fread (data, 1, len, ifp);
    switch (tag) {
      case 0xffc3:
	jh->bits = data[0];
	jh->high = data[1] << 8 | data[2];
	jh->wide = data[3] << 8 | data[4];
	jh->clrs = data[5];
	break;
      case 0xffc4:
	for (dp = data; dp < data+len && *dp < 4; ) {
	  jh->huff[*dp] = free_decode;
	  dp = make_decoder (++dp, 0);
	}
	break;
      case 0xffdd:
	jh->restart = data[0] << 8 | data[1];
    }
  } while (tag != 0xffda);
  jh->row = calloc (jh->wide*jh->clrs, 2);
  merror (jh->row, " jpeg_start()");
  zero_after_ff = 1;
  return 1;
}

int CLASS ljpeg_diff (struct decode *dindex)
{
  int len, diff;

  while (dindex->branch[0])
    dindex = dindex->branch[getbits(1)];
  len = dindex->leaf;
  if (len == 16 && (!dng_version || dng_version >= 0x1010000))
    return -32768;
  diff = getbits(len);
  if ((diff & (1 << (len-1))) == 0)
    diff -= (1 << len) - 1;
  return diff;
}

void CLASS ljpeg_row (int jrow, struct jhead *jh)
{
  int col, c, diff;
  ushort *outp=jh->row;

  if (jrow * jh->wide % jh->restart == 0) {
    FORC4 jh->vpred[c] = 1 << (jh->bits-1);
    if (jrow) get2();			/* Eat the FF Dx marker */
    getbits(-1);
  }
  for (col=0; col < jh->wide; col++)
    for (c=0; c < jh->clrs; c++) {
      diff = ljpeg_diff (jh->huff[c]);
      *outp = col ? outp[-jh->clrs]+diff : (jh->vpred[c] += diff);
      outp++;
    }
}

void CLASS lossless_jpeg_load_raw()
{
  int jwide, jrow, jcol, val, jidx, i, row, col;
  struct jhead jh;
  int min=INT_MAX;

  if (!ljpeg_start (&jh)) return;
  jwide = jh.wide * jh.clrs;

  for (jrow=0; jrow < jh.high; jrow++) {
    ljpeg_row (jrow, &jh);
    for (jcol=0; jcol < jwide; jcol++) {
      val = jh.row[jcol];
      if (jh.bits <= 12)
	val = curve[val];
      jidx = jrow*jwide + jcol;
      if (raw_width == 5108) {
	i = jidx / (1680*jh.high);
	if (i < 2) {
	  row = jidx / 1680 % jh.high;
	  col = jidx % 1680 + i*1680;
	} else {
	  jidx -= 2*1680*jh.high;
	  row = jidx / 1748;
	  col = jidx % 1748 + 2*1680;
	}
      } else if (raw_width == 4476 || raw_width == 3516) {
	row = jidx / (raw_width/2);
	col = jidx % (raw_width/2);
	if (row >= raw_height) {
	  row -= raw_height;
	  col += raw_width/2;
	}
      } else {
	row = jidx / raw_width;
	col = jidx % raw_width;
      }
      if ((unsigned) (row-top_margin) >= height) continue;
      if ((unsigned) (col-left_margin) < width) {
	BAYER(row-top_margin,col-left_margin) = val;
	if (min > val) min = val;
      } else
	black += val;
    }
  }
  free (jh.row);
  if (raw_width > width)
    black /= (raw_width - width) * height;
  if (!strcasecmp(make,"KODAK"))
    black = min;
}

void CLASS adobe_copy_pixel (int row, int col, ushort **rp)
{
  unsigned r, c;

  r = row -= top_margin;
  c = col -= left_margin;
  if (fuji_secondary && use_secondary) (*rp)++;
  if (filters) {
    if (fuji_width) {
      r = row + fuji_width - 1 - (col >> 1);
      c = row + ((col+1) >> 1);
    }
    if (r < height && c < width)
      BAYER(r,c) = **rp < 0x1000 ? curve[**rp] : **rp;
    *rp += 1 + fuji_secondary;
  } else {
    if (r < height && c < width)
      for (c=0; c < tiff_samples; c++)
	image[row*width+col][c] = (*rp)[c] < 0x1000 ? curve[(*rp)[c]]:(*rp)[c];
    *rp += tiff_samples;
  }
  if (fuji_secondary && use_secondary) (*rp)--;
}

void CLASS adobe_dng_load_raw_lj()
{
  int save, twide, trow=0, tcol=0, jrow, jcol;
  struct jhead jh;
  ushort *rp;

  while (1) {
    save = ftell(ifp);
    fseek (ifp, get4(), SEEK_SET);
    if (!ljpeg_start (&jh)) break;
    if (trow >= raw_height) break;
    if (jh.high > raw_height-trow)
	jh.high = raw_height-trow;
    twide = jh.wide;
    if (filters) twide *= jh.clrs;
    else         colors = jh.clrs;
    if (fuji_secondary) twide /= 2;
    if (twide > raw_width-tcol)
	twide = raw_width-tcol;

    for (jrow=0; jrow < jh.high; jrow++) {
      ljpeg_row (jrow, &jh);
      for (rp=jh.row, jcol=0; jcol < twide; jcol++)
	adobe_copy_pixel (trow+jrow, tcol+jcol, &rp);
    }
    fseek (ifp, save+4, SEEK_SET);
    if ((tcol += twide) >= raw_width) {
      tcol = 0;
      trow += jh.high;
    }
    free (jh.row);
  }
}

void CLASS adobe_dng_load_raw_nc()
{
  ushort *pixel, *rp;
  int row, col;

  pixel = calloc (raw_width * tiff_samples, sizeof *pixel);
  merror (pixel, "adobe_dng_load_raw_nc()");
  for (row=0; row < raw_height; row++) {
    if (tiff_bps == 16)
      read_shorts (pixel, raw_width * tiff_samples);
    else {
      getbits(-1);
      for (col=0; col < raw_width * tiff_samples; col++)
	pixel[col] = getbits(tiff_bps);
    }
    for (rp=pixel, col=0; col < raw_width; col++)
      adobe_copy_pixel (row, col, &rp);
  }
  free (pixel);
}

void CLASS nikon_compressed_load_raw()
{
  static const uchar nikon_tree[] = {
    0,1,5,1,1,1,1,1,1,2,0,0,0,0,0,0,
    5,4,3,6,2,7,1,0,8,9,11,10,12
  };
  int csize, row, col, i, diff;
  ushort vpred[4], hpred[2], *curve;

  init_decoder();
  make_decoder (nikon_tree, 0);

  fseek (ifp, nikon_curve_offset, SEEK_SET);
  read_shorts (vpred, 4);
  csize = get2();
  curve = calloc (csize, sizeof *curve);
  merror (curve, "nikon_compressed_load_raw()");
  read_shorts (curve, csize);

  fseek (ifp, data_offset, SEEK_SET);
  getbits(-1);

  for (row=0; row < height; row++)
    for (col=0; col < raw_width; col++)
    {
      diff = ljpeg_diff (first_decode);
      if (col < 2) {
	i = 2*(row & 1) + (col & 1);
	vpred[i] += diff;
	hpred[col] = vpred[i];
      } else
	hpred[col & 1] += diff;
      if ((unsigned) (col-left_margin) >= width) continue;
      diff = hpred[col & 1];
      if (diff >= csize) diff = csize-1;
      BAYER(row,col-left_margin) = curve[diff];
    }
  free (curve);
}

void CLASS nikon_load_raw()
{
  int irow, row, col, i;

  getbits(-1);
  for (irow=0; irow < height; irow++) {
    row = irow;
    if (model[0] == 'E') {
      row = irow * 2 % height + irow / (height/2);
      if (row == 1 && data_offset == 0) {
	fseek (ifp, 0, SEEK_END);
	fseek (ifp, ftell(ifp)/2, SEEK_SET);
	getbits(-1);
      }
    }
    for (col=0; col < raw_width; col++) {
      i = getbits(12);
      if ((unsigned) (col-left_margin) < width)
	BAYER(row,col-left_margin) = i;
      if (tiff_data_compression == 34713 && (col % 10) == 9)
	getbits(8);
    }
  }
}

/*
   Figure out if a NEF file is compressed.  These fancy heuristics
   are only needed for the D100, thanks to a bug in some cameras
   that tags all images as "compressed".
 */
int CLASS nikon_is_compressed()
{
  uchar test[256];
  int i;

  if (tiff_data_compression != 34713)
    return 0;
  if (strcmp(model,"D100"))
    return 1;
  fseek (ifp, data_offset, SEEK_SET);
  fread (test, 1, 256, ifp);
  for (i=15; i < 256; i+=16)
    if (test[i]) return 1;
  return 0;
}

/*
   Returns 1 for a Coolpix 990, 0 for a Coolpix 995.
 */
int CLASS nikon_e990()
{
  int i, histo[256];
  const uchar often[] = { 0x00, 0x55, 0xaa, 0xff };

  memset (histo, 0, sizeof histo);
  fseek (ifp, 2064*1540*3/4, SEEK_SET);
  for (i=0; i < 2000; i++)
    histo[fgetc(ifp)]++;
  for (i=0; i < 4; i++)
    if (histo[often[i]] > 400)
      return 1;
  return 0;
}

/*
   Returns 1 for a Coolpix 2100, 0 for anything else.
 */
int CLASS nikon_e2100()
{
  uchar t[12];
  int i;

  fseek (ifp, 0, SEEK_SET);
  for (i=0; i < 1024; i++) {
    fread (t, 1, 12, ifp);
    if (((t[2] & t[4] & t[7] & t[9]) >> 4
	& t[1] & t[6] & t[8] & t[11] & 3) != 3)
      return 0;
  }
  return 1;
}

/*
   Returns 0 for a Pentax Optio 33WR,
	   1 for a Nikon E3700,
	   2 for an Olympus C740UZ.
 */
int CLASS nikon_3700()
{
  int i, sum[] = { 0, 0 };
  uchar tail[952];

  fseek (ifp, -sizeof tail, SEEK_END);
  fread (tail, 1, sizeof tail, ifp);
  for (i=0; i < sizeof tail; i++)
    sum[(i>>2) & 1] += tail[i];
  if (sum[1] > 4*sum[0]) return 2;
  return sum[0] > 4*sum[1];
}

/*
   Separates a Minolta DiMAGE Z2 from a Nikon E4300.
 */
int CLASS minolta_z2()
{
  int i;
  char tail[424];

  fseek (ifp, -sizeof tail, SEEK_END);
  fread (tail, 1, sizeof tail, ifp);
  for (i=0; i < sizeof tail; i++)
    if (tail[i]) return 1;
  return 0;
}

/* Here raw_width is in bytes, not pixels. */
void CLASS nikon_e900_load_raw()
{
  int offset=0, irow, row, col;

  for (irow=0; irow < height; irow++) {
    row = irow * 2 % height;
    if (row == 1)
      offset = - (-offset & -4096);
    fseek (ifp, offset, SEEK_SET);
    offset += raw_width;
    getbits(-1);
    for (col=0; col < width; col++)
      BAYER(row,col) = getbits(10);
  }
}

void CLASS nikon_e2100_load_raw()
{
  uchar   data[3456], *dp;
  ushort pixel[2304], *pix;
  int row, col;

  for (row=0; row <= height; row+=2) {
    if (row == height) {
      fseek (ifp, ((width==1616) << 13) - (-ftell(ifp) & -2048), SEEK_SET);
      row = 1;
    }
    fread (data, 1, width*3/2, ifp);
    for (dp=data, pix=pixel; pix < pixel+width; dp+=12, pix+=8) {
      pix[0] = (dp[2] >> 4) + (dp[ 3] << 4);
      pix[1] = (dp[2] << 8) +  dp[ 1];
      pix[2] = (dp[7] >> 4) + (dp[ 0] << 4);
      pix[3] = (dp[7] << 8) +  dp[ 6];
      pix[4] = (dp[4] >> 4) + (dp[ 5] << 4);
      pix[5] = (dp[4] << 8) +  dp[11];
      pix[6] = (dp[9] >> 4) + (dp[10] << 4);
      pix[7] = (dp[9] << 8) +  dp[ 8];
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = (pixel[col] & 0xfff);
  }
}

/*
   The Fuji Super CCD is just a Bayer grid rotated 45 degrees.
 */
void CLASS fuji_load_raw()
{
  ushort *pixel;
  int row, col, r, c;

  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "fuji_load_raw()");
  for (row=0; row < raw_height; row++) {
    read_shorts (pixel, raw_width);
    for (col=0; col < fuji_width << !fuji_layout; col++) {
      if (fuji_layout) {
	r = fuji_width - 1 - col + (row >> 1);
	c = col + ((row+1) >> 1);
      } else {
	r = fuji_width - 1 + row - (col >> 1);
	c = row + ((col+1) >> 1);
      }
      BAYER(r,c) = pixel[col];
    }
  }
  free (pixel);
}

void CLASS rollei_load_raw()
{
  uchar pixel[10];
  unsigned iten=0, isix, i, buffer=0, row, col, todo[16];

  isix = raw_width * raw_height * 5 / 8;
  while (fread (pixel, 1, 10, ifp) == 10) {
    for (i=0; i < 10; i+=2) {
      todo[i]   = iten++;
      todo[i+1] = pixel[i] << 8 | pixel[i+1];
      buffer    = pixel[i] >> 2 | buffer << 6;
    }
    for (   ; i < 16; i+=2) {
      todo[i]   = isix++;
      todo[i+1] = buffer >> (14-i)*5;
    }
    for (i=0; i < 16; i+=2) {
      row = todo[i] / raw_width - top_margin;
      col = todo[i] % raw_width - left_margin;
      if (row < height && col < width)
	BAYER(row,col) = (todo[i+1] & 0x3ff);
    }
  }
  maximum = 0x3ff;
}

void CLASS phase_one_load_raw()
{
  int row, col, a, b;
  ushort *pixel, akey, bkey, mask;

  fseek (ifp, nikon_curve_offset, SEEK_SET);
  akey = get2();
  bkey = get2();
  mask = tiff_data_compression == 1 ? 0x5555:0x1354;
  fseek (ifp, data_offset + top_margin*raw_width*2, SEEK_SET);
  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "phase_one_load_raw()");
  for (row=0; row < height; row++) {
    read_shorts (pixel, raw_width);
    for (col=0; col < raw_width; col+=2) {
      a = pixel[col+0] ^ akey;
      b = pixel[col+1] ^ bkey;
      pixel[col+0] = (a & mask) | (b & ~mask);
      pixel[col+1] = (b & mask) | (a & ~mask);
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = pixel[col+left_margin];
  }
  free (pixel);
  maximum = 0xffff;
}

unsigned CLASS ph1_bits (int nbits)
{
  static UINT64 bitbuf=0;
  static int vbits=0;

  if (nbits == 0)
    return bitbuf = vbits = 0;
  if (vbits < nbits) {
    bitbuf = bitbuf << 32 | (unsigned) get4();
    vbits += 32;
  }
  vbits -= nbits;
  return bitbuf << (64 - nbits - vbits) >> (64 - nbits);
}

void CLASS phase_one_load_raw_c()
{
  static const int length[] = { 8,7,6,9,11,10,5,12,14,13 };
  int len[2], pred[2], row, col, ncols, i, j;
  ushort *pixel;

  ncols = (raw_width + 7) & -8;
  pixel = calloc (ncols, sizeof *pixel);
  merror (pixel, "phase_one_load_raw_c()");
  for (row=0; row < raw_height; row++) {
    ph1_bits(0);
    pred[0] = pred[1] = 0;
    for (col=0; col < ncols; col++) {
      if (col >= (raw_width & -8))
	len[0] = len[1] = 14;
      else if ((col & 7) == 0)
	for (i=0; i < 2; i++) {
	  for (j=0; j < 5 && !ph1_bits(1); j++);
	  if (j--) len[i] = length[j*2 + ph1_bits(1)];
	}
      if ((i = len[col & 1]) == 14)
	pixel[col] = pred[col & 1] = ph1_bits(16);
      else
	pixel[col] = pred[col & 1] += ph1_bits(i) + 1 - (1 << (i - 1));
    }
    if ((unsigned) (row-top_margin) < height)
      for (col=0; col < width; col++)
	BAYER(row-top_margin,col) = pixel[col+left_margin];
  }
  free (pixel);
  maximum = 0x3fff;
}

void CLASS leaf_load_raw()
{
  ushort *pixel;
  int r, c, row, col;

  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "leaf_load_raw()");
  for (r=0; r < height-32; r+=32)
    FORC3 for (row=r; row < r+32; row++) {
      read_shorts (pixel, raw_width);
      for (col=0; col < width; col++)
	image[row*width+col][c] = pixel[col];
    }
  free (pixel);
}

/* Here raw_width is in bytes, not pixels. */
void CLASS packed_12_load_raw()
{
  int row, col;

  getbits(-1);
  for (row=0; row < height; row++) {
    for (col=0; col < width; col++)
      BAYER(row,col) = getbits(12);
    for (col = width*3/2; col < raw_width; col++)
      getbits(8);
  }
}

void CLASS unpacked_load_raw()
{
  ushort *pixel;
  int row, col;

  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "unpacked_load_raw()");
  for (row=0; row < height; row++) {
    read_shorts (pixel, raw_width);
    for (col=0; col < width; col++)
      BAYER(row,col) = pixel[col];
  }
  free (pixel);
}

void CLASS olympus_e300_load_raw()
{
  uchar  *data,  *dp;
  ushort *pixel, *pix;
  int dwide, row, col;

  dwide = raw_width * 16 / 10;
  data = malloc (dwide + raw_width*2);
  merror (data, "olympus_e300_load_raw()");
  pixel = (ushort *) (data + dwide);
  for (row=0; row < height; row++) {
    fread (data, 1, dwide, ifp);
    for (dp=data, pix=pixel; pix < pixel+raw_width; dp+=3, pix+=2) {
      if (((dp-data) & 15) == 15) dp++;
      pix[0] = dp[1] << 8 | dp[0];
      pix[1] = dp[2] << 4 | dp[1] >> 4;
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = (pixel[col] & 0xfff);
    for (col=width+4; col < raw_width; col++)
      black += pixel[col] & 0xfff;
  }
  black /= (raw_width - width - 4) * height;
  free (data);
}

void CLASS olympus_cseries_load_raw()
{
  int irow, row, col;

  for (irow=0; irow < height; irow++) {
    row = irow * 2 % height + irow / (height/2);
    if (row < 2) {
      fseek (ifp, data_offset - row*(-width*height*3/4 & -2048), SEEK_SET);
      getbits(-1);
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = getbits(12);
  }
}

void CLASS minolta_rd175_load_raw()
{
  uchar pixel[768];
  unsigned irow, box, row, col;

  for (irow=0; irow < 1481; irow++) {
    fread (pixel, 1, 768, ifp);
    box = irow / 82;
    row = irow % 82 * 12 + ((box < 12) ? box | 1 : (box-12)*2);
    switch (irow) {
      case 1477: case 1479: continue;
      case 1476: row = 984; break;
      case 1480: row = 985; break;
      case 1478: row = 985; box = 1;
    }
    if ((box < 12) && (box & 1)) {
      for (col=0; col < 1533; col++, row ^= 1)
	if (col != 1) BAYER(row,col) = (col+1) & 2 ?
		   pixel[col/2-1] + pixel[col/2+1] : pixel[col/2] << 1;
      BAYER(row,1)    = pixel[1]   << 1;
      BAYER(row,1533) = pixel[765] << 1;
    } else
      for (col=row & 1; col < 1534; col+=2)
	BAYER(row,col) = pixel[col/2] << 1;
  }
  maximum = 0xff << 1;
}

void CLASS eight_bit_load_raw()
{
  uchar *pixel;
  int row, col;

  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "eight_bit_load_raw()");
  for (row=0; row < height; row++) {
    fread (pixel, 1, raw_width, ifp);
    for (col=0; col < width; col++)
      BAYER(row,col) = pixel[col];
  }
  free (pixel);
  maximum = 0xff;
}

void CLASS casio_qv5700_load_raw()
{
  uchar  data[3232],  *dp;
  ushort pixel[2576], *pix;
  int row, col;

  for (row=0; row < height; row++) {
    fread (data, 1, 3232, ifp);
    for (dp=data, pix=pixel; dp < data+3220; dp+=5, pix+=4) {
      pix[0] = (dp[0] << 2) + (dp[1] >> 6);
      pix[1] = (dp[1] << 4) + (dp[2] >> 4);
      pix[2] = (dp[2] << 6) + (dp[3] >> 2);
      pix[3] = (dp[3] << 8) + (dp[4]     );
    }
    for (col=0; col < width; col++)
      BAYER(row,col) = (pixel[col] & 0x3ff);
  }
  maximum = 0x3fc;
}

void CLASS nucore_load_raw()
{
  ushort *pixel;
  int irow, row, col;

  pixel = calloc (width, 2);
  merror (pixel, "nucore_load_raw()");
  for (irow=0; irow < height; irow++) {
    read_shorts (pixel, width);
    row = irow/2 + height/2 * (irow & 1);
    for (col=0; col < width; col++)
      BAYER(row,col) = pixel[col];
  }
  free (pixel);
}

const int * CLASS make_decoder_int (const int *source, int level)
{
  struct decode *cur;

  cur = free_decode++;
  if (level < source[0]) {
    cur->branch[0] = free_decode;
    source = make_decoder_int (source, level+1);
    cur->branch[1] = free_decode;
    source = make_decoder_int (source, level+1);
  } else {
    cur->leaf = source[1];
    source += 2;
  }
  return source;
}

int CLASS radc_token (int tree)
{
  int t;
  static struct decode *dstart[18], *dindex;
  static const int *s, source[] = {
    1,1, 2,3, 3,4, 4,2, 5,7, 6,5, 7,6, 7,8,
    1,0, 2,1, 3,3, 4,4, 5,2, 6,7, 7,6, 8,5, 8,8,
    2,1, 2,3, 3,0, 3,2, 3,4, 4,6, 5,5, 6,7, 6,8,
    2,0, 2,1, 2,3, 3,2, 4,4, 5,6, 6,7, 7,5, 7,8,
    2,1, 2,4, 3,0, 3,2, 3,3, 4,7, 5,5, 6,6, 6,8,
    2,3, 3,1, 3,2, 3,4, 3,5, 3,6, 4,7, 5,0, 5,8,
    2,3, 2,6, 3,0, 3,1, 4,4, 4,5, 4,7, 5,2, 5,8,
    2,4, 2,7, 3,3, 3,6, 4,1, 4,2, 4,5, 5,0, 5,8,
    2,6, 3,1, 3,3, 3,5, 3,7, 3,8, 4,0, 5,2, 5,4,
    2,0, 2,1, 3,2, 3,3, 4,4, 4,5, 5,6, 5,7, 4,8,
    1,0, 2,2, 2,-2,
    1,-3, 1,3,
    2,-17, 2,-5, 2,5, 2,17,
    2,-7, 2,2, 2,9, 2,18,
    2,-18, 2,-9, 2,-2, 2,7,
    2,-28, 2,28, 3,-49, 3,-9, 3,9, 4,49, 5,-79, 5,79,
    2,-1, 2,13, 2,26, 3,39, 4,-16, 5,55, 6,-37, 6,76,
    2,-26, 2,-13, 2,1, 3,-39, 4,16, 5,-55, 6,-76, 6,37
  };

  if (free_decode == first_decode)
    for (s=source, t=0; t < 18; t++) {
      dstart[t] = free_decode;
      s = make_decoder_int (s, 0);
    }
  if (tree == 18) {
    if (kodak_cbpp == 243)
      return (getbits(6) << 2) + 2;	/* most DC50 photos */
    else
      return (getbits(5) << 3) + 4;	/* DC40, Fotoman Pixtura */
  }
  for (dindex = dstart[tree]; dindex->branch[0]; )
    dindex = dindex->branch[getbits(1)];
  return dindex->leaf;
}

#define FORYX for (y=1; y < 3; y++) for (x=col+1; x >= col; x--)

#define PREDICTOR (c ? (buf[c][y-1][x] + buf[c][y][x+1]) / 2 \
: (buf[c][y-1][x+1] + 2*buf[c][y-1][x] + buf[c][y][x+1]) / 4)

void CLASS kodak_radc_load_raw()
{
  int row, col, tree, nreps, rep, step, i, c, s, r, x, y, val;
  short last[3] = { 16,16,16 }, mul[3], buf[3][3][386];

  init_decoder();
  getbits(-1);
  for (i=0; i < sizeof(buf)/sizeof(short); i++)
    buf[0][0][i] = 2048;
  for (row=0; row < height; row+=4) {
    FORC3 mul[c] = getbits(6);
    FORC3 {
      val = ((0x1000000/last[c] + 0x7ff) >> 12) * mul[c];
      s = val > 65564 ? 10:12;
      x = ~(-1 << (s-1));
      val <<= 12-s;
      for (i=0; i < sizeof(buf[0])/sizeof(short); i++)
	buf[c][0][i] = (buf[c][0][i] * val + x) >> s;
      last[c] = mul[c];
      for (r=0; r <= !c; r++) {
	buf[c][1][width/2] = buf[c][2][width/2] = mul[c] << 7;
	for (tree=1, col=width/2; col > 0; ) {
	  if ((tree = radc_token(tree))) {
	    col -= 2;
	    if (tree == 8)
	      FORYX buf[c][y][x] = radc_token(tree+10) * mul[c];
	    else
	      FORYX buf[c][y][x] = radc_token(tree+10) * 16 + PREDICTOR;
	  } else
	    do {
	      nreps = (col > 2) ? radc_token(9) + 1 : 1;
	      for (rep=0; rep < 8 && rep < nreps && col > 0; rep++) {
		col -= 2;
		FORYX buf[c][y][x] = PREDICTOR;
		if (rep & 1) {
		  step = radc_token(10) << 4;
		  FORYX buf[c][y][x] += step;
		}
	      }
	    } while (nreps == 9);
	}
	for (y=0; y < 2; y++)
	  for (x=0; x < width/2; x++) {
	    val = (buf[c][y+1][x] << 4) / mul[c];
	    if (val < 0) val = 0;
	    if (c)
	      BAYER(row+y*2+c-1,x*2+2-c) = val;
	    else
	      BAYER(row+r*2+y,x*2+y) = val;
	  }
	memcpy (buf[c][0]+!c, buf[c][2], sizeof buf[c][0]-2*!c);
      }
    }
    for (y=row; y < row+4; y++)
      for (x=0; x < width; x++)
	if ((x+y) & 1) {
	  val = (BAYER(y,x)-2048)*2 + (BAYER(y,x-1)+BAYER(y,x+1))/2;
	  if (val < 0) val = 0;
	  BAYER(y,x) = val;
	}
  }
  maximum = 10000;
}

#undef FORYX
#undef PREDICTOR

#ifdef NO_JPEG
void CLASS kodak_jpeg_load_raw() {}
#else

METHODDEF(boolean)
fill_input_buffer (j_decompress_ptr cinfo)
{
  static uchar jpeg_buffer[4096];
  size_t nbytes;

  nbytes = fread (jpeg_buffer, 1, 4096, ifp);
  swab (jpeg_buffer, jpeg_buffer, nbytes);
  cinfo->src->next_input_byte = jpeg_buffer;
  cinfo->src->bytes_in_buffer = nbytes;
  return TRUE;
}

void CLASS kodak_jpeg_load_raw()
{
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPARRAY buf;
  JSAMPLE (*pixel)[3];
  int row, col;

  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_decompress (&cinfo);
  jpeg_stdio_src (&cinfo, ifp);
  cinfo.src->fill_input_buffer = fill_input_buffer;
  jpeg_read_header (&cinfo, TRUE);
  jpeg_start_decompress (&cinfo);
  if ((cinfo.output_width      != width  ) ||
      (cinfo.output_height*2   != height ) ||
      (cinfo.output_components != 3      )) {
    fprintf (stderr, "%s: incorrect JPEG dimensions\n", ifname);
    jpeg_destroy_decompress (&cinfo);
    longjmp (failure, 3);
  }
  buf = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, width*3, 1);

  while (cinfo.output_scanline < cinfo.output_height) {
    row = cinfo.output_scanline * 2;
    jpeg_read_scanlines (&cinfo, buf, 1);
    pixel = (void *) buf[0];
    for (col=0; col < width; col+=2) {
      BAYER(row+0,col+0) = pixel[col+0][1] << 1;
      BAYER(row+1,col+1) = pixel[col+1][1] << 1;
      BAYER(row+0,col+1) = pixel[col][0] + pixel[col+1][0];
      BAYER(row+1,col+0) = pixel[col][2] + pixel[col+1][2];
    }
  }
  jpeg_finish_decompress (&cinfo);
  jpeg_destroy_decompress (&cinfo);
  maximum = 0xff << 1;
}

#endif

void CLASS kodak_dc120_load_raw()
{
  static const int mul[4] = { 162, 192, 187,  92 };
  static const int add[4] = {   0, 636, 424, 212 };
  uchar pixel[848];
  int row, shift, col;

  for (row=0; row < height; row++) {
    fread (pixel, 848, 1, ifp);
    shift = row * mul[row & 3] + add[row & 3];
    for (col=0; col < width; col++)
      BAYER(row,col) = (ushort) pixel[(col + shift) % 848];
  }
  maximum = 0xff;
}

void CLASS kodak_easy_load_raw()
{
  uchar *pixel;
  unsigned row, col, icol;

  if (raw_width > width)
    black = 0;
  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "kodak_easy_load_raw()");
  for (row=0; row < height; row++) {
    fread (pixel, 1, raw_width, ifp);
    for (col=0; col < raw_width; col++) {
      icol = col - left_margin;
      if (icol < width)
	BAYER(row,icol) = (ushort) curve[pixel[col]];
      else
	black += curve[pixel[col]];
    }
  }
  free (pixel);
  if (raw_width > width)
    black /= (raw_width - width) * height;
  if (!strncmp(model,"DC2",3))
    black = 0;
  maximum = curve[0xff];
}

void CLASS kodak_compressed_load_raw()
{
  uchar c, blen[256];
  ushort raw[6];
  unsigned row, col, len, save, i, israw=0, bits=0, pred[2];
  INT64 bitbuf=0;
  int diff;

  for (row=0; row < height; row++)
    for (col=0; col < width; col++)
    {
      if ((col & 255) == 0) {		/* Get the bit-lengths of the */
	len = width - col;		/* next 256 pixel values      */
	if (len > 256) len = 256;
	save = ftell(ifp);
	for (israw=i=0; i < len; i+=2) {
	  c = fgetc(ifp);
	  if ((blen[i+0] = c & 15) > 12 ||
	      (blen[i+1] = c >> 4) > 12 )
	    israw = 1;
	}
	bitbuf = bits = pred[0] = pred[1] = 0;
	if (len % 8 == 4) {
	  bitbuf  = fgetc(ifp) << 8;
	  bitbuf += fgetc(ifp);
	  bits = 16;
	}
	if (israw)
	  fseek (ifp, save, SEEK_SET);
      }
      if (israw) {			/* If the data is not compressed */
	switch (col & 7) {
	  case 0:
	    read_shorts (raw, 6);
	    diff = raw[0] >> 12 << 8 | raw[2] >> 12 << 4 | raw[4] >> 12;
	    break;
	  case 1:
	    diff = raw[1] >> 12 << 8 | raw[3] >> 12 << 4 | raw[5] >> 12;
	    break;
	  default:
	    diff = raw[(col & 7) - 2] & 0xfff;
	}
      } else {				/* If the data is compressed */
	len = blen[col & 255];		/* Number of bits for this pixel */
	if (bits < len) {		/* Got enough bits in the buffer? */
	  for (i=0; i < 32; i+=8)
	    bitbuf += (INT64) fgetc(ifp) << (bits+(i^8));
	  bits += 32;
	}
	diff = bitbuf & (0xffff >> (16-len));  /* Pull bits from buffer */
	bitbuf >>= len;
	bits -= len;
	if ((diff & (1 << (len-1))) == 0)
	  diff -= (1 << len) - 1;
	pred[col & 1] += diff;
	diff = pred[col & 1];
      }
      BAYER(row,col) = curve[diff];
    }
}

void CLASS kodak_yuv_load_raw()
{
  uchar c, blen[384];
  unsigned row, col, len, bits=0;
  INT64 bitbuf=0;
  int i, li=0, si, diff, six[6], y[4], cb=0, cr=0, rgb[3];
  ushort *ip;

  for (row=0; row < height; row+=2)
    for (col=0; col < width; col+=2) {
      if ((col & 127) == 0) {
	len = (width - col + 1) * 3 & -4;
	if (len > 384) len = 384;
	for (i=0; i < len; ) {
	  c = fgetc(ifp);
	  blen[i++] = c & 15;
	  blen[i++] = c >> 4;
	}
	li = bitbuf = bits = y[1] = y[3] = cb = cr = 0;
	if (len % 8 == 4) {
	  bitbuf  = fgetc(ifp) << 8;
	  bitbuf += fgetc(ifp);
	  bits = 16;
	}
      }
      for (si=0; si < 6; si++) {
	len = blen[li++];
	if (bits < len) {
	  for (i=0; i < 32; i+=8)
	    bitbuf += (INT64) fgetc(ifp) << (bits+(i^8));
	  bits += 32;
	}
	diff = bitbuf & (0xffff >> (16-len));
	bitbuf >>= len;
	bits -= len;
	if ((diff & (1 << (len-1))) == 0)
	  diff -= (1 << len) - 1;
	six[si] = diff;
      }
      y[0] = six[0] + y[1];
      y[1] = six[1] + y[0];
      y[2] = six[2] + y[3];
      y[3] = six[3] + y[2];
      cb  += six[4];
      cr  += six[5];
      for (i=0; i < 4; i++) {
	ip = image[(row+(i >> 1))*width + col+(i & 1)];
	rgb[0] = y[i] + cr;
	rgb[1] = y[i];
	rgb[2] = y[i] + cb;
	FORC3 if (rgb[c] > 0) ip[c] = curve[rgb[c]];
      }
    }
}

void CLASS sony_decrypt (unsigned *data, int len, int start, int key)
{
  static unsigned pad[128], p;

  if (start) {
    for (p=0; p < 4; p++)
      pad[p] = key = key * 48828125 + 1;
    pad[3] = pad[3] << 1 | (pad[0]^pad[2]) >> 31;
    for (p=4; p < 127; p++)
      pad[p] = (pad[p-4]^pad[p-2]) << 1 | (pad[p-3]^pad[p-1]) >> 31;
    for (p=0; p < 127; p++)
      pad[p] = htonl(pad[p]);
  }
  while (len--)
    *data++ ^= pad[p++ & 127] = pad[(p+1) & 127] ^ pad[(p+65) & 127];
}

void CLASS sony_load_raw()
{
  uchar head[40];
  ushort *pixel;
  unsigned i, key, row, col;

  fseek (ifp, 200896, SEEK_SET);
  fseek (ifp, (unsigned) fgetc(ifp)*4 - 1, SEEK_CUR);
  order = 0x4d4d;
  key = get4();
  fseek (ifp, 164600, SEEK_SET);
  fread (head, 1, 40, ifp);
  sony_decrypt ((void *) head, 10, 1, key);
  for (i=26; i-- > 22; )
    key = key << 8 | head[i];
  fseek (ifp, data_offset, SEEK_SET);
  pixel = calloc (raw_width, sizeof *pixel);
  merror (pixel, "sony_load_raw()");
  for (row=0; row < height; row++) {
    fread (pixel, 2, raw_width, ifp);
    sony_decrypt ((void *) pixel, raw_width/2, !row, key);
    for (col=9; col < left_margin; col++)
      black += ntohs(pixel[col]);
    for (col=0; col < width; col++)
      BAYER(row,col) = ntohs(pixel[col+left_margin]);
  }
  free (pixel);
  if (left_margin > 9)
    black /= (left_margin-9) * height;
  maximum = 0x3ff0;
}

#define HOLE(row) ((holes >> (((row) - raw_height) & 7)) & 1)

/* Kudos to Rich Taylor for figuring out SMaL's compression algorithm. */
void CLASS smal_decode_segment (unsigned seg[2][2], int holes)
{
  uchar hist[3][13] = {
    { 7, 7, 0, 0, 63, 55, 47, 39, 31, 23, 15, 7, 0 },
    { 7, 7, 0, 0, 63, 55, 47, 39, 31, 23, 15, 7, 0 },
    { 3, 3, 0, 0, 63,     47,     31,     15,    0 } };
  int low, high=0xff, carry=0, nbits=8;
  int s, count, bin, next, i, sym[3];
  uchar diff, pred[]={0,0};
  ushort data=0, range=0;
  unsigned pix, row, col;

  fseek (ifp, seg[0][1]+1, SEEK_SET);
  getbits(-1);
  for (pix=seg[0][0]; pix < seg[1][0]; pix++) {
    for (s=0; s < 3; s++) {
      data = data << nbits | getbits(nbits);
      if (carry < 0)
	carry = (nbits += carry+1) < 1 ? nbits-1 : 0;
      while (--nbits >= 0)
	if ((data >> nbits & 0xff) == 0xff) break;
      if (nbits > 0)
	  data = ((data & ((1 << (nbits-1)) - 1)) << 1) |
	((data + (((data & (1 << (nbits-1)))) << 1)) & (-1 << nbits));
      if (nbits >= 0) {
	data += getbits(1);
	carry = nbits - 8;
      }
      count = ((((data-range+1) & 0xffff) << 2) - 1) / (high >> 4);
      for (bin=0; hist[s][bin+5] > count; bin++);
		low = hist[s][bin+5] * (high >> 4) >> 2;
      if (bin) high = hist[s][bin+4] * (high >> 4) >> 2;
      high -= low;
      for (nbits=0; high << nbits < 128; nbits++);
      range = (range+low) << nbits;
      high <<= nbits;
      next = hist[s][1];
      if (++hist[s][2] > hist[s][3]) {
	next = (next+1) & hist[s][0];
	hist[s][3] = (hist[s][next+4] - hist[s][next+5]) >> 2;
	hist[s][2] = 1;
      }
      if (hist[s][hist[s][1]+4] - hist[s][hist[s][1]+5] > 1) {
	if (bin < hist[s][1])
	  for (i=bin; i < hist[s][1]; i++) hist[s][i+5]--;
	else if (next <= bin)
	  for (i=hist[s][1]; i < bin; i++) hist[s][i+5]++;
      }
      hist[s][1] = next;
      sym[s] = bin;
    }
    diff = sym[2] << 5 | sym[1] << 2 | (sym[0] & 3);
    if (sym[0] & 4)
      diff = diff ? -diff : 0x80;
    if (ftell(ifp) + 12 >= seg[1][1])
      diff = 0;
    pred[pix & 1] += diff;
    row = pix / raw_width - top_margin;
    col = pix % raw_width - left_margin;
    if (row < height && col < width)
      BAYER(row,col) = pred[pix & 1];
    if (!(pix & 1) && HOLE(row)) pix += 2;
  }
  maximum = 0xff;
}

void CLASS smal_v6_load_raw()
{
  unsigned seg[2][2];

  fseek (ifp, 16, SEEK_SET);  
  seg[0][0] = 0;
  seg[0][1] = get2();
  seg[1][0] = raw_width * raw_height;
  seg[1][1] = INT_MAX;
  smal_decode_segment (seg, 0);
  use_gamma = 0;
}

int CLASS median4 (int *p)
{
  int min, max, sum, i;

  min = max = sum = p[0];
  for (i=1; i < 4; i++) {
    sum += p[i];
    if (min > p[i]) min = p[i];
    if (max < p[i]) max = p[i];
  }
  return (sum - min - max) >> 1;
}

void CLASS fill_holes (int holes)
{
  int row, col, val[4];

  for (row=2; row < height-2; row++) {
    if (!HOLE(row)) continue;
    for (col=1; col < width-1; col+=4) {
      val[0] = BAYER(row-1,col-1);
      val[1] = BAYER(row-1,col+1);
      val[2] = BAYER(row+1,col-1);
      val[3] = BAYER(row+1,col+1);
      BAYER(row,col) = median4(val);
    }
    for (col=2; col < width-2; col+=4)
      if (HOLE(row-2) || HOLE(row+2))
	BAYER(row,col) = (BAYER(row,col-2) + BAYER(row,col+2)) >> 1;
      else {
	val[0] = BAYER(row,col-2);
	val[1] = BAYER(row,col+2);
	val[2] = BAYER(row-2,col);
	val[3] = BAYER(row+2,col);
	BAYER(row,col) = median4(val);
      }
  }
}

void CLASS smal_v9_load_raw()
{
  unsigned seg[256][2], offset, nseg, holes, i;

  fseek (ifp, 67, SEEK_SET);
  offset = get4();
  nseg = fgetc(ifp);
  fseek (ifp, offset, SEEK_SET);
  for (i=0; i < nseg*2; i++)
    seg[0][i] = get4() + data_offset*(i & 1);
  fseek (ifp, 78, SEEK_SET);
  holes = fgetc(ifp);
  fseek (ifp, 88, SEEK_SET);
  seg[nseg][0] = raw_height * raw_width;
  seg[nseg][1] = get4() + data_offset;
  for (i=0; i < nseg; i++)
    smal_decode_segment (seg+i, holes);
  if (holes) fill_holes (holes);
}

/* BEGIN GPL BLOCK */

void CLASS foveon_decoder (unsigned huff[1024], unsigned code)
{
  struct decode *cur;
  int i, len;

  cur = free_decode++;
  if (free_decode > first_decode+2048) {
    fprintf (stderr, "%s: decoder table overflow\n", ifname);
    longjmp (failure, 2);
  }
  if (code)
    for (i=0; i < 1024; i++)
      if (huff[i] == code) {
	cur->leaf = i;
	return;
      }
  if ((len = code >> 27) > 26) return;
  code = (len+1) << 27 | (code & 0x3ffffff) << 1;

  cur->branch[0] = free_decode;
  foveon_decoder (huff, code);
  cur->branch[1] = free_decode;
  foveon_decoder (huff, code+1);
}

void CLASS foveon_load_camf()
{
  unsigned key, i, val;

  fseek (ifp, meta_offset, SEEK_SET);
  key = get4();
  fread (meta_data, 1, meta_length, ifp);
  for (i=0; i < meta_length; i++) {
    key = (key * 1597 + 51749) % 244944;
    val = key * (INT64) 301593171 >> 24;
    meta_data[i] ^= ((((key << 8) - val) >> 1) + val) >> 17;
  }
}

void CLASS foveon_load_raw()
{
  struct decode *dindex;
  short diff[1024], pred[3];
  unsigned huff[1024], bitbuf=0;
  int fixed, row, col, bit=-1, c, i;

  fixed = get4();
  read_shorts ((ushort *) diff, 1024);
  if (!fixed) {
    for (i=0; i < 1024; i++)
      huff[i] = get4();
    init_decoder();
    foveon_decoder (huff, 0);
  }
  for (row=0; row < height; row++) {
    memset (pred, 0, sizeof pred);
    if (!bit && !fixed) get4();
    for (col=bit=0; col < width; col++) {
      if (fixed) {
	bitbuf = get4();
	FORC3 pred[2-c] += diff[bitbuf >> c*10 & 0x3ff];
      }
      else FORC3 {
	for (dindex=first_decode; dindex->branch[0]; ) {
	  if ((bit = (bit-1) & 31) == 31)
	    for (i=0; i < 4; i++)
	      bitbuf = (bitbuf << 8) + fgetc(ifp);
	  dindex = dindex->branch[bitbuf >> bit & 1];
	}
	pred[c] += diff[dindex->leaf];
      }
      FORC3 image[row*width+col][c] = pred[c];
    }
  }
  foveon_load_camf();
  maximum = clip_max = 0xffff;
}

char * CLASS foveon_camf_param (char *block, char *param)
{
  unsigned idx, num;
  char *pos, *cp, *dp;

  for (idx=0; idx < meta_length; idx += sget4(pos+8)) {
    pos = meta_data + idx;
    if (strncmp (pos, "CMb", 3)) break;
    if (pos[3] != 'P') continue;
    if (strcmp (block, pos+sget4(pos+12))) continue;
    cp = pos + sget4(pos+16);
    num = sget4(cp);
    dp = pos + sget4(cp+4);
    while (num--) {
      cp += 8;
      if (!strcmp (param, dp+sget4(cp)))
	return dp+sget4(cp+4);
    }
  }
  return NULL;
}

void * CLASS foveon_camf_matrix (int dim[3], char *name)
{
  unsigned i, idx, type, ndim, size, *mat;
  char *pos, *cp, *dp;

  for (idx=0; idx < meta_length; idx += sget4(pos+8)) {
    pos = meta_data + idx;
    if (strncmp (pos, "CMb", 3)) break;
    if (pos[3] != 'M') continue;
    if (strcmp (name, pos+sget4(pos+12))) continue;
    dim[0] = dim[1] = dim[2] = 1;
    cp = pos + sget4(pos+16);
    type = sget4(cp);
    if ((ndim = sget4(cp+4)) > 3) break;
    dp = pos + sget4(cp+8);
    for (i=ndim; i--; ) {
      cp += 12;
      dim[i] = sget4(cp);
    }
    if ((size = dim[0]*dim[1]*dim[2]) > meta_length/4) break;
    mat = malloc (size * 4);
    merror (mat, "foveon_camf_matrix()");
    for (i=0; i < size; i++)
      if (type && type != 6)
	mat[i] = sget4(dp + i*4);
      else
	mat[i] = sget4(dp + i*2) & 0xffff;
    return mat;
  }
  fprintf (stderr, "%s: \"%s\" matrix not found!\n", ifname, name);
  return NULL;
}

int CLASS foveon_fixed (void *ptr, int size, char *name)
{
  void *dp;
  int dim[3];

  dp = foveon_camf_matrix (dim, name);
  if (!dp) return 0;
  memcpy (ptr, dp, size*4);
  free (dp);
  return 1;
}

float CLASS foveon_avg (short *pix, int range[2], float cfilt)
{
  int i;
  float val, min=FLT_MAX, max=-FLT_MAX, sum=0;

  for (i=range[0]; i <= range[1]; i++) {
    sum += val = pix[i*4] + (pix[i*4]-pix[(i-1)*4]) * cfilt;
    if (min > val) min = val;
    if (max < val) max = val;
  }
  return (sum - min - max) / (range[1] - range[0] - 1);
}

short * CLASS foveon_make_curve (double max, double mul, double filt)
{
  short *curve;
  int i, size;
  double x;

  if (!filt) filt = 0.8;
  size = 4*M_PI*max / filt;
  curve = calloc (size+1, sizeof *curve);
  merror (curve, "foveon_make_curve()");
  curve[0] = size;
  for (i=0; i < size; i++) {
    x = i*filt/max/4;
    curve[i+1] = (cos(x)+1)/2 * tanh(i*filt/mul) * mul + 0.5;
  }
  return curve;
}

void CLASS foveon_make_curves
	(short **curvep, float dq[3], float div[3], float filt)
{
  double mul[3], max=0;
  int c;

  FORC3 mul[c] = dq[c]/div[c];
  FORC3 if (max < mul[c]) max = mul[c];
  FORC3 curvep[c] = foveon_make_curve (max, mul[c], filt);
}

int CLASS foveon_apply_curve (short *curve, int i)
{
  if (abs(i) >= curve[0]) return 0;
  return i < 0 ? -curve[1-i] : curve[1+i];
}

#define image ((short (*)[4]) image)

void CLASS foveon_interpolate()
{
  static const short hood[] = { -1,-1, -1,0, -1,1, 0,-1, 0,1, 1,-1, 1,0, 1,1 };
  short *pix, prev[3], *curve[8], (*shrink)[3];
  float cfilt=0, ddft[3][3][2], ppm[3][3][3];
  float cam_xyz[3][3], correct[3][3], last[3][3], trans[3][3];
  float chroma_dq[3], color_dq[3], diag[3][3], div[3];
  float (*black)[3], (*sgain)[3], (*sgrow)[3];
  float fsum[3], val, frow, num;
  int row, col, c, i, j, diff, sgx, irow, sum, min, max, limit;
  int dim[3], dscr[2][2], dstb[4], (*smrow[7])[3], total[4], ipix[3];
  int work[3][3], smlast, smred, smred_p=0, dev[3];
  int satlev[3], keep[4], active[4];
  unsigned *badpix;
  double dsum=0, trsum[3];
  char str[128], *cp;

  if (verbose)
    fprintf (stderr, "Foveon interpolation...\n");

  foveon_fixed (dscr, 4, "DarkShieldColRange");
  foveon_fixed (ppm[0][0], 27, "PostPolyMatrix");
  foveon_fixed (satlev, 3, "SaturationLevel");
  foveon_fixed (keep, 4, "KeepImageArea");
  foveon_fixed (active, 4, "ActiveImageArea");
  foveon_fixed (chroma_dq, 3, "ChromaDQ");
  foveon_fixed (color_dq, 3,
	foveon_camf_param ("IncludeBlocks", "ColorDQ") ?
		"ColorDQ" : "ColorDQCamRGB");
  if (foveon_camf_param ("IncludeBlocks", "ColumnFilter"))
  		 foveon_fixed (&cfilt, 1, "ColumnFilter");

  memset (ddft, 0, sizeof ddft);
  if (!foveon_camf_param ("IncludeBlocks", "DarkDrift")
	 || !foveon_fixed (ddft[1][0], 12, "DarkDrift"))
    for (i=0; i < 2; i++) {
      foveon_fixed (dstb, 4, i ? "DarkShieldBottom":"DarkShieldTop");
      for (row = dstb[1]; row <= dstb[3]; row++)
	for (col = dstb[0]; col <= dstb[2]; col++)
	  FORC3 ddft[i+1][c][1] += (short) image[row*width+col][c];
      FORC3 ddft[i+1][c][1] /= (dstb[3]-dstb[1]+1) * (dstb[2]-dstb[0]+1);
    }

  if (!(cp = foveon_camf_param ("WhiteBalanceIlluminants", model2)))
  { fprintf (stderr, "%s: Invalid white balance \"%s\"\n", ifname, model2);
    return; }
  foveon_fixed (cam_xyz, 9, cp);
  foveon_fixed (correct, 9,
	foveon_camf_param ("WhiteBalanceCorrections", model2));
  memset (last, 0, sizeof last);
  for (i=0; i < 3; i++)
    for (j=0; j < 3; j++)
      FORC3 last[i][j] += correct[i][c] * cam_xyz[c][j];

  sprintf (str, "%sRGBNeutral", model2);
  if (foveon_camf_param ("IncludeBlocks", str))
    foveon_fixed (div, 3, str);
  else {
    #define LAST(x,y) last[(i+x)%3][(c+y)%3]
    for (i=0; i < 3; i++)
      FORC3 diag[c][i] = LAST(1,1)*LAST(2,2) - LAST(1,2)*LAST(2,1);
    #undef LAST
    FORC3 div[c] = diag[c][0]*0.3127 + diag[c][1]*0.329 + diag[c][2]*0.3583;
  }
  num = 0;
  FORC3 if (num < div[c]) num = div[c];
  FORC3 div[c] /= num;

  memset (trans, 0, sizeof trans);
  for (i=0; i < 3; i++)
    for (j=0; j < 3; j++)
      FORC3 trans[i][j] += rgb_cam[i][c] * last[c][j] * div[j];
  FORC3 trsum[c] = trans[c][0] + trans[c][1] + trans[c][2];
  dsum = (6*trsum[0] + 11*trsum[1] + 3*trsum[2]) / 20;
  for (i=0; i < 3; i++)
    FORC3 last[i][c] = trans[i][c] * dsum / trsum[i];
  memset (trans, 0, sizeof trans);
  for (i=0; i < 3; i++)
    for (j=0; j < 3; j++)
      FORC3 trans[i][j] += (i==c ? 32 : -1) * last[c][j] / 30;

  foveon_make_curves (curve, color_dq, div, cfilt);
  FORC3 chroma_dq[c] /= 3;
  foveon_make_curves (curve+3, chroma_dq, div, cfilt);
  FORC3 dsum += chroma_dq[c] / div[c];
  curve[6] = foveon_make_curve (dsum, dsum, cfilt);
  curve[7] = foveon_make_curve (dsum*2, dsum*2, cfilt);

  sgain = foveon_camf_matrix (dim, "SpatialGain");
  if (!sgain) return;
  sgrow = calloc (dim[1], sizeof *sgrow);
  sgx = (width + dim[1]-2) / (dim[1]-1);

  black = calloc (height, sizeof *black);
  for (row=0; row < height; row++) {
    for (i=0; i < 6; i++)
      ddft[0][0][i] = ddft[1][0][i] +
	row / (height-1.0) * (ddft[2][0][i] - ddft[1][0][i]);
    FORC3 black[row][c] =
 	( foveon_avg (image[row*width]+c, dscr[0], cfilt) +
	  foveon_avg (image[row*width]+c, dscr[1], cfilt) * 3
	  - ddft[0][c][0] ) / 4 - ddft[0][c][1];
  }
  memcpy (black, black+8, sizeof *black*8);
  memcpy (black+height-11, black+height-22, 11*sizeof *black);
  memcpy (last, black, sizeof last);

  for (row=1; row < height-1; row++) {
    FORC3 if (last[1][c] > last[0][c]) {
	if (last[1][c] > last[2][c])
	  black[row][c] = (last[0][c] > last[2][c]) ? last[0][c]:last[2][c];
      } else
	if (last[1][c] < last[2][c])
	  black[row][c] = (last[0][c] < last[2][c]) ? last[0][c]:last[2][c];
    memmove (last, last+1, 2*sizeof last[0]);
    memcpy (last[2], black[row+1], sizeof last[2]);
  }
  FORC3 black[row][c] = (last[0][c] + last[1][c])/2;
  FORC3 black[0][c] = (black[1][c] + black[3][c])/2;

  val = 1 - exp(-1/24.0);
  memcpy (fsum, black, sizeof fsum);
  for (row=1; row < height; row++)
    FORC3 fsum[c] += black[row][c] =
	(black[row][c] - black[row-1][c])*val + black[row-1][c];
  memcpy (last[0], black[height-1], sizeof last[0]);
  FORC3 fsum[c] /= height;
  for (row = height; row--; )
    FORC3 last[0][c] = black[row][c] =
	(black[row][c] - fsum[c] - last[0][c])*val + last[0][c];

  memset (total, 0, sizeof total);
  for (row=2; row < height; row+=4)
    for (col=2; col < width; col+=4) {
      FORC3 total[c] += (short) image[row*width+col][c];
      total[3]++;
    }
  for (row=0; row < height; row++)
    FORC3 black[row][c] += fsum[c]/2 + total[c]/(total[3]*100.0);

  for (row=0; row < height; row++) {
    for (i=0; i < 6; i++)
      ddft[0][0][i] = ddft[1][0][i] +
	row / (height-1.0) * (ddft[2][0][i] - ddft[1][0][i]);
    pix = image[row*width];
    memcpy (prev, pix, sizeof prev);
    frow = row / (height-1.0) * (dim[2]-1);
    if ((irow = frow) == dim[2]-1) irow--;
    frow -= irow;
    for (i=0; i < dim[1]; i++)
      FORC3 sgrow[i][c] = sgain[ irow   *dim[1]+i][c] * (1-frow) +
			  sgain[(irow+1)*dim[1]+i][c] *    frow;
    for (col=0; col < width; col++) {
      FORC3 {
	diff = pix[c] - prev[c];
	prev[c] = pix[c];
	ipix[c] = pix[c] + floor ((diff + (diff*diff >> 14)) * cfilt
		- ddft[0][c][1] - ddft[0][c][0] * ((float) col/width - 0.5)
		- black[row][c] );
      }
      FORC3 {
	work[0][c] = ipix[c] * ipix[c] >> 14;
	work[2][c] = ipix[c] * work[0][c] >> 14;
	work[1][2-c] = ipix[(c+1) % 3] * ipix[(c+2) % 3] >> 14;
      }
      FORC3 {
	for (val=i=0; i < 3; i++)
	  for (  j=0; j < 3; j++)
	    val += ppm[c][i][j] * work[i][j];
	ipix[c] = floor ((ipix[c] + floor(val)) *
		( sgrow[col/sgx  ][c] * (sgx - col%sgx) +
		  sgrow[col/sgx+1][c] * (col%sgx) ) / sgx / div[c]);
	if (ipix[c] > 32000) ipix[c] = 32000;
	pix[c] = ipix[c];
      }
      pix += 4;
    }
  }
  free (black);
  free (sgrow);
  free (sgain);

  if ((badpix = foveon_camf_matrix (dim, "BadPixels"))) {
    for (i=0; i < dim[0]; i++) {
      col = (badpix[i] >> 8 & 0xfff) - keep[0];
      row = (badpix[i] >> 20       ) - keep[1];
      if ((unsigned)(row-1) > height-3 || (unsigned)(col-1) > width-3)
	continue;
      memset (fsum, 0, sizeof fsum);
      for (sum=j=0; j < 8; j++)
	if (badpix[i] & (1 << j)) {
	  FORC3 fsum[c] += (short)
		image[(row+hood[j*2])*width+col+hood[j*2+1]][c];
	  sum++;
	}
      if (sum) FORC3 image[row*width+col][c] = fsum[c]/sum;
    }
    free (badpix);
  }

  /* Array for 5x5 Gaussian averaging of red values */
  smrow[6] = calloc (width*5, sizeof **smrow);
  merror (smrow[6], "foveon_interpolate()");
  for (i=0; i < 5; i++)
    smrow[i] = smrow[6] + i*width;

  /* Sharpen the reds against these Gaussian averages */
  for (smlast=-1, row=2; row < height-2; row++) {
    while (smlast < row+2) {
      for (i=0; i < 6; i++)
	smrow[(i+5) % 6] = smrow[i];
      pix = image[++smlast*width+2];
      for (col=2; col < width-2; col++) {
	smrow[4][col][0] =
	  (pix[0]*6 + (pix[-4]+pix[4])*4 + pix[-8]+pix[8] + 8) >> 4;
	pix += 4;
      }
    }
    pix = image[row*width+2];
    for (col=2; col < width-2; col++) {
      smred = ( 6 *  smrow[2][col][0]
	      + 4 * (smrow[1][col][0] + smrow[3][col][0])
	      +      smrow[0][col][0] + smrow[4][col][0] + 8 ) >> 4;
      if (col == 2)
	smred_p = smred;
      i = pix[0] + ((pix[0] - ((smred*7 + smred_p) >> 3)) >> 3);
      if (i > 32000) i = 32000;
      pix[0] = i;
      smred_p = smred;
      pix += 4;
    }
  }

  /* Adjust the brighter pixels for better linearity */
  min = 0xffff;
  FORC3 {
    i = satlev[c] / div[c];
    if (min > i) min = i;
  }
  limit = min * 9 >> 4;
  for (pix=image[0]; pix < image[height*width]; pix+=4) {
    if (pix[0] <= limit || pix[1] <= limit || pix[2] <= limit)
      continue;
    min = max = pix[0];
    for (c=1; c < 3; c++) {
      if (min > pix[c]) min = pix[c];
      if (max < pix[c]) max = pix[c];
    }
    if (min >= limit*2) {
      pix[0] = pix[1] = pix[2] = max;
    } else {
      i = 0x4000 - ((min - limit) << 14) / limit;
      i = 0x4000 - (i*i >> 14);
      i = i*i >> 14;
      FORC3 pix[c] += (max - pix[c]) * i >> 14;
    }
  }
/*
   Because photons that miss one detector often hit another,
   the sum R+G+B is much less noisy than the individual colors.
   So smooth the hues without smoothing the total.
 */
  for (smlast=-1, row=2; row < height-2; row++) {
    while (smlast < row+2) {
      for (i=0; i < 6; i++)
	smrow[(i+5) % 6] = smrow[i];
      pix = image[++smlast*width+2];
      for (col=2; col < width-2; col++) {
	FORC3 smrow[4][col][c] = (pix[c-4]+2*pix[c]+pix[c+4]+2) >> 2;
	pix += 4;
      }
    }
    pix = image[row*width+2];
    for (col=2; col < width-2; col++) {
      FORC3 dev[c] = -foveon_apply_curve (curve[7], pix[c] -
	((smrow[1][col][c] + 2*smrow[2][col][c] + smrow[3][col][c]) >> 2));
      sum = (dev[0] + dev[1] + dev[2]) >> 3;
      FORC3 pix[c] += dev[c] - sum;
      pix += 4;
    }
  }
  for (smlast=-1, row=2; row < height-2; row++) {
    while (smlast < row+2) {
      for (i=0; i < 6; i++)
	smrow[(i+5) % 6] = smrow[i];
      pix = image[++smlast*width+2];
      for (col=2; col < width-2; col++) {
	FORC3 smrow[4][col][c] =
		(pix[c-8]+pix[c-4]+pix[c]+pix[c+4]+pix[c+8]+2) >> 2;
	pix += 4;
      }
    }
    pix = image[row*width+2];
    for (col=2; col < width-2; col++) {
      for (total[3]=375, sum=60, c=0; c < 3; c++) {
	for (total[c]=i=0; i < 5; i++)
	  total[c] += smrow[i][col][c];
	total[3] += total[c];
	sum += pix[c];
      }
      if (sum < 0) sum = 0;
      j = total[3] > 375 ? (sum << 16) / total[3] : sum * 174;
      FORC3 pix[c] += foveon_apply_curve (curve[6],
		((j*total[c] + 0x8000) >> 16) - pix[c]);
      pix += 4;
    }
  }

  /* Transform the image to a different colorspace */
  for (pix=image[0]; pix < image[height*width]; pix+=4) {
    FORC3 pix[c] -= foveon_apply_curve (curve[c], pix[c]);
    sum = (pix[0]+pix[1]+pix[1]+pix[2]) >> 2;
    FORC3 pix[c] -= foveon_apply_curve (curve[c], pix[c]-sum);
    FORC3 {
      for (dsum=i=0; i < 3; i++)
	dsum += trans[c][i] * pix[i];
      if (dsum < 0)  dsum = 0;
      if (dsum > 24000) dsum = 24000;
      ipix[c] = dsum + 0.5;
    }
    FORC3 pix[c] = ipix[c];
  }

  /* Smooth the image bottom-to-top and save at 1/4 scale */
  shrink = calloc ((width/4) * (height/4), sizeof *shrink);
  merror (shrink, "foveon_interpolate()");
  for (row = height/4; row--; )
    for (col=0; col < width/4; col++) {
      ipix[0] = ipix[1] = ipix[2] = 0;
      for (i=0; i < 4; i++)
	for (j=0; j < 4; j++)
	  FORC3 ipix[c] += image[(row*4+i)*width+col*4+j][c];
      FORC3
	if (row+2 > height/4)
	  shrink[row*(width/4)+col][c] = ipix[c] >> 4;
	else
	  shrink[row*(width/4)+col][c] =
	    (shrink[(row+1)*(width/4)+col][c]*1840 + ipix[c]*141 + 2048) >> 12;
    }
  /* From the 1/4-scale image, smooth right-to-left */
  for (row=0; row < (height & ~3); row++) {
    ipix[0] = ipix[1] = ipix[2] = 0;
    if ((row & 3) == 0)
      for (col = width & ~3 ; col--; )
	FORC3 smrow[0][col][c] = ipix[c] =
	  (shrink[(row/4)*(width/4)+col/4][c]*1485 + ipix[c]*6707 + 4096) >> 13;

  /* Then smooth left-to-right */
    ipix[0] = ipix[1] = ipix[2] = 0;
    for (col=0; col < (width & ~3); col++)
      FORC3 smrow[1][col][c] = ipix[c] =
	(smrow[0][col][c]*1485 + ipix[c]*6707 + 4096) >> 13;

  /* Smooth top-to-bottom */
    if (row == 0)
      memcpy (smrow[2], smrow[1], sizeof **smrow * width);
    else
      for (col=0; col < (width & ~3); col++)
	FORC3 smrow[2][col][c] =
	  (smrow[2][col][c]*6707 + smrow[1][col][c]*1485 + 4096) >> 13;

  /* Adjust the chroma toward the smooth values */
    for (col=0; col < (width & ~3); col++) {
      for (i=j=30, c=0; c < 3; c++) {
	i += smrow[2][col][c];
	j += image[row*width+col][c];
      }
      j = (j << 16) / i;
      for (sum=c=0; c < 3; c++) {
	ipix[c] = foveon_apply_curve (curve[c+3],
	  ((smrow[2][col][c] * j + 0x8000) >> 16) - image[row*width+col][c]);
	sum += ipix[c];
      }
      sum >>= 3;
      FORC3 {
	i = image[row*width+col][c] + ipix[c] - sum;
	if (i < 0) i = 0;
	image[row*width+col][c] = i;
      }
    }
  }
  free (shrink);
  free (smrow[6]);
  for (i=0; i < 8; i++)
    free (curve[i]);

  /* Trim off the black border */
  active[1] -= keep[1];
  active[3] -= 2;
  i = active[2] - active[0];
  for (row = 0; row < active[3]-active[1]; row++)
    memcpy (image[row*i], image[(row+active[1])*width+active[0]],
	 i * sizeof *image);
  width = i;
  height = row;
}
#undef image

/* END GPL BLOCK */

/*
   Seach from the current directory up to the root looking for
   a ".badpixels" file, and fix those pixels now.
 */
void CLASS bad_pixels()
{
  FILE *fp=NULL;
  char *fname, *cp, line[128];
  int len, time, row, col, r, c, rad, tot, n, fixed=0;

  if (!filters) return;
  for (len=16 ; ; len *= 2) {
    fname = malloc (len);
    if (!fname) return;
    if (getcwd (fname, len-12)) break;
    free (fname);
    if (errno != ERANGE) return;
  }
#ifdef WIN32
  if (fname[1] == ':')
    memmove (fname, fname+2, len-2);
  for (cp=fname; *cp; cp++)
    if (*cp == '\\') *cp = '/';
#endif
  cp = fname + strlen(fname);
  if (cp[-1] == '/') cp--;
  while (*fname == '/') {
    strcpy (cp, "/.badpixels");
    if ((fp = fopen (fname, "r"))) break;
    if (cp == fname) break;
    while (*--cp != '/');
  }
  free (fname);
  if (!fp) return;
  while (fgets (line, 128, fp)) {
    cp = strchr (line, '#');
    if (cp) *cp = 0;
    if (sscanf (line, "%d %d %d", &col, &row, &time) != 3) continue;
    if ((unsigned) col >= width || (unsigned) row >= height) continue;
    if (time > timestamp) continue;
    for (tot=n=0, rad=1; rad < 3 && n==0; rad++)
      for (r = row-rad; r <= row+rad; r++)
	for (c = col-rad; c <= col+rad; c++)
	  if ((unsigned) r < height && (unsigned) c < width &&
		(r != row || c != col) && FC(r,c) == FC(row,col)) {
	    tot += BAYER(r,c);
	    n++;
	  }
    BAYER(row,col) = tot/n;
    if (verbose) {
      if (!fixed++)
	fprintf (stderr, "Fixed bad pixels at:");
      fprintf (stderr, " %d,%d", col, row);
    }
  }
  if (fixed) fputc ('\n', stderr);
  fclose (fp);
}

void CLASS pseudoinverse (const double (*in)[3], double (*out)[3], int size)
{
  double work[3][6], num;
  int i, j, k;

  for (i=0; i < 3; i++) {
    for (j=0; j < 6; j++)
      work[i][j] = j == i+3;
    for (j=0; j < 3; j++)
      for (k=0; k < size; k++)
	work[i][j] += in[k][i] * in[k][j];
  }
  for (i=0; i < 3; i++) {
    num = work[i][i];
    for (j=0; j < 6; j++)
      work[i][j] /= num;
    for (k=0; k < 3; k++) {
      if (k==i) continue;
      num = work[k][i];
      for (j=0; j < 6; j++)
	work[k][j] -= work[i][j] * num;
    }
  }
  for (i=0; i < size; i++)
    for (j=0; j < 3; j++)
      for (out[i][j]=k=0; k < 3; k++)
	out[i][j] += work[j][k+3] * in[i][k];
}

void CLASS cam_xyz_coeff (double cam_xyz[4][3])
{
  double cam_rgb[4][3], inverse[4][3], num;
  int i, j, k;

  for (i=0; i < colors; i++)		/* Multiply out XYZ colorspace */
    for (j=0; j < 3; j++)
      for (cam_rgb[i][j] = k=0; k < 3; k++)
	cam_rgb[i][j] += cam_xyz[i][k] * xyz_rgb[k][j];

  for (i=0; i < colors; i++) {		/* Normalize cam_rgb so that */
    for (num=j=0; j < 3; j++)		/* cam_rgb * (1,1,1) is (1,1,1,1) */
      num += cam_rgb[i][j];
    for (j=0; j < 3; j++)
      cam_rgb[i][j] /= num;
    pre_mul[i] = 1 / num;
  }
  pseudoinverse ((const double (*)[3]) cam_rgb, inverse, colors);
  for (raw_color = i=0; i < 3; i++)
    for (j=0; j < colors; j++)
      rgb_cam[i][j] = inverse[j][i];
}

#ifdef COLORCHECK
void CLASS colorcheck()
{
#define NSQ 24
// Coordinates of the GretagMacbeth ColorChecker squares
// width, height, 1st_column, 1st_row
  static const int cut[NSQ][4] = {
    { 241, 231, 234, 274 },
    { 251, 235, 534, 274 },
    { 255, 239, 838, 272 },
    { 255, 240, 1146, 274 },
    { 251, 237, 1452, 278 },
    { 243, 238, 1758, 288 },
    { 253, 253, 218, 558 },
    { 255, 249, 524, 562 },
    { 261, 253, 830, 562 },
    { 260, 255, 1144, 564 },
    { 261, 255, 1450, 566 },
    { 247, 247, 1764, 576 },
    { 255, 251, 212, 862 },
    { 259, 259, 518, 862 },
    { 263, 261, 826, 864 },
    { 265, 263, 1138, 866 },
    { 265, 257, 1450, 872 },
    { 257, 255, 1762, 874 },
    { 257, 253, 212, 1164 },
    { 262, 251, 516, 1172 },
    { 263, 257, 826, 1172 },
    { 263, 255, 1136, 1176 },
    { 255, 252, 1452, 1182 },
    { 257, 253, 1760, 1180 } };
// ColorChecker Chart under 6500-kelvin illumination
  static const double gmb_xyz[NSQ][3] = {
    { 11.078,  9.870,  6.738 },		// Dark Skin
    { 37.471, 35.004, 26.057 },		// Light Skin
    { 18.187, 19.306, 35.425 },		// Blue Sky
    { 10.825, 13.827,  7.600 },		// Foliage
    { 24.769, 23.304, 43.943 },		// Blue Flower
    { 31.174, 42.684, 45.277 },		// Bluish Green
    { 36.238, 29.188,  6.222 },		// Orange
    { 13.661, 11.845, 38.929 },		// Purplish Blue
    { 27.999, 19.272, 14.265 },		// Moderate Red
    {  8.398,  6.309, 14.211 },		// Purple
    { 33.692, 44.346, 11.288 },		// Yellow Green
    { 45.000, 42.144,  8.429 },		// Orange Yellow
    {  8.721,  6.130, 31.181 },		// Blue
    { 14.743, 24.049,  9.778 },		// Green
    { 19.777, 11.530,  5.101 },		// Red
    { 55.978, 59.599, 10.047 },		// Yellow
    { 29.421, 19.271, 31.167 },		// Magenta
    { 13.972, 18.952, 37.646 },		// Cyan
    { 82.819, 87.727, 94.479 },		// White
    { 55.950, 58.959, 64.375 },		// Neutral 8
    { 32.877, 34.536, 38.097 },		// Neutral 6.5
    { 18.556, 19.701, 21.487 },		// Neutral 5
    {  8.353,  8.849,  9.812 },		// Neutral 3.5
    {  2.841,  2.980,  3.332 } };	// Black
  double inverse[NSQ][3], gmb_cam[NSQ][4], cam_xyz[4][3];
  double num, error, minerr=DBL_MAX, best[4][3];
  int b, c, i, j, k, sq, row, col, count[4];

  memset (gmb_cam, 0, sizeof gmb_cam);
  for (sq=0; sq < NSQ; sq++) {
    FORCC count[c] = 0;
    for   (row=cut[sq][3]; row < cut[sq][3]+cut[sq][1]; row++)
      for (col=cut[sq][2]; col < cut[sq][2]+cut[sq][0]; col++) {
	c = FC(row,col);
	if (c >= colors) c -= 2;
	gmb_cam[sq][c] += BAYER(row,col);
	count[c]++;
      }
    FORCC gmb_cam[sq][c] /= count[c];
  }
  for (b=0; b < 2000; b++) {
    pseudoinverse (gmb_xyz, inverse, NSQ);
    for (i=0; i < colors; i++)
      for (j=0; j < 3; j++)
	for (cam_xyz[i][j] = k=0; k < NSQ; k++)
	  cam_xyz[i][j] += gmb_cam[k][i] * inverse[k][j];

    for (error=sq=0; sq < NSQ; sq++)
      FORCC {
	for (num=j=0; j < 3; j++)
	  num += cam_xyz[c][j] * gmb_xyz[sq][j];
	if (num < 0) num=0;
	error += pow (num - gmb_cam[sq][c], 2);
	gmb_cam[sq][c]--;		// for the next black value
      }
    if (error < minerr) {
      black = b;
      minerr = error;
      memcpy (best, cam_xyz, sizeof best);
    }
  }
  cam_xyz_coeff (best);
  if (verbose) {
    fprintf (stderr, "    { \"%s %s\",\n\t{ ", make, model);
    num = 10000 / (best[1][0] + best[1][1] + best[1][2]);
    FORCC for (j=0; j < 3; j++)
      fprintf (stderr, "%d,", (int) (best[c][j] * num + 0.5));
    fprintf (stderr, "\b } },\n");
  }
#undef NSQ
}
#endif

void CLASS scale_colors()
{
  int row, col, c, val, shift=0;
  int min[4], max[4], count[4];
  double sum[4], dmin;

  maximum -= black;
  if (use_auto_wb || (use_camera_wb && camera_red == -1)) {
    FORC4 min[c] = INT_MAX;
    FORC4 max[c] = count[c] = sum[c] = 0;
    for (row=0; row < height; row++)
      for (col=0; col < width; col++)
	FORC4 {
	  val = image[row*width+col][c];
	  if (!val) continue;
	  if (min[c] > val) min[c] = val;
	  if (max[c] < val) max[c] = val;
	  val -= black;
	  if (val > maximum-25) continue;
	  if (val < 0) val = 0;
	  sum[c] += val;
	  count[c]++;
	}
    FORC4 if (sum[c]) pre_mul[c] = count[c] / sum[c];
  }
  if (use_camera_wb && camera_red != -1) {
    FORC4 count[c] = sum[c] = 0;
    for (row=0; row < 8; row++)
      for (col=0; col < 8; col++) {
	c = FC(row,col);
	if ((val = white[row][col] - black) > 0)
	  sum[c] += val;
	count[c]++;
      }
    if (sum[0] && sum[1] && sum[2] && sum[3])
      FORC4 pre_mul[c] = count[c] / sum[c];
    else if (camera_red && camera_blue)
      memcpy (pre_mul, cam_mul, sizeof pre_mul);
    else
      fprintf (stderr, "%s: Cannot use camera white balance.\n", ifname);
  }
  if (raw_color) {
    pre_mul[0] *= red_scale;
    pre_mul[2] *= blue_scale;
  }
  if (pre_mul[3] == 0) pre_mul[3] = colors < 4 ? pre_mul[1] : 1;
  dmin = DBL_MAX;
  FORC4 if (dmin > pre_mul[c])
	    dmin = pre_mul[c];
  FORC4 pre_mul[c] /= dmin;

  while (maximum << shift < 0x8000) shift++;
  FORC4 pre_mul[c] *= 1 << shift;
  maximum <<= shift;

  if (write_fun != write_ppm || bright < 1) {
    maximum *= bright;
    if (maximum > 0xffff)
	maximum = 0xffff;
    FORC4 pre_mul[c] *= bright;
  }
  if (verbose) {
    fprintf (stderr, "Scaling with black=%d, pre_mul[] =", black);
    FORC4 fprintf (stderr, " %f", pre_mul[c]);
    fputc ('\n', stderr);
  }
  clip_max = clip_color ? maximum : 0xffff;
  for (row=0; row < height; row++)
    for (col=0; col < width; col++)
      FORC4 {
	val = image[row*width+col][c];
	if (!val) continue;
	val -= black;
	val *= pre_mul[c];
	image[row*width+col][c] = CLIP(val);
      }
  if (filters && colors == 3) {
    if (four_color_rgb) {
      colors++;
      FORC3 rgb_cam[c][3] = rgb_cam[c][1] /= 2;
    } else {
      for (row = FC(1,0) >> 1; row < height; row+=2)
	for (col = FC(row,1) & 1; col < width; col+=2)
	  image[row*width+col][1] = image[row*width+col][3];
      filters &= ~((filters & 0x55555555) << 1);
    }
  }
}

void CLASS border_interpolate (int border)
{
  unsigned row, col, y, x, c, sum[8];

  for (row=0; row < height; row++)
    for (col=0; col < width; col++) {
      if (col==border && row >= border && row < height-border)
	col = width-border;
      memset (sum, 0, sizeof sum);
      for (y=row-1; y != row+2; y++)
	for (x=col-1; x != col+2; x++)
	  if (y < height && x < width) {
	    sum[FC(y,x)] += BAYER(y,x);
	    sum[FC(y,x)+4]++;
	  }
      FORCC if (c != FC(row,col))
	image[row*width+col][c] = sum[c] / sum[c+4];
    }
}

void CLASS lin_interpolate()
{
  int code[8][2][32], *ip, sum[4];
  int c, i, x, y, row, col, shift, color;
  ushort *pix;

  if (verbose) fprintf (stderr, "Bilinear interpolation...\n");

  border_interpolate(1);
  for (row=0; row < 8; row++)
    for (col=0; col < 2; col++) {
      ip = code[row][col];
      memset (sum, 0, sizeof sum);
      for (y=-1; y <= 1; y++)
	for (x=-1; x <= 1; x++) {
	  shift = (y==0) + (x==0);
	  if (shift == 2) continue;
	  color = FC(row+y,col+x);
	  *ip++ = (width*y + x)*4 + color;
	  *ip++ = shift;
	  *ip++ = color;
	  sum[color] += 1 << shift;
	}
      FORCC
	if (c != FC(row,col)) {
	  *ip++ = c;
	  *ip++ = sum[c];
	}
    }
  for (row=1; row < height-1; row++)
    for (col=1; col < width-1; col++) {
      pix = image[row*width+col];
      ip = code[row & 7][col & 1];
      memset (sum, 0, sizeof sum);
      for (i=8; i--; ip+=3)
	sum[ip[2]] += pix[ip[0]] << ip[1];
      for (i=colors; --i; ip+=2)
	pix[ip[0]] = sum[ip[0]] / ip[1];
    }
}

/*
   This algorithm is officially called:

   "Interpolation using a Threshold-based variable number of gradients"

   described in http://www-ise.stanford.edu/~tingchen/algodep/vargra.html

   I've extended the basic idea to work with non-Bayer filter arrays.
   Gradients are numbered clockwise from NW=0 to W=7.
 */
void CLASS vng_interpolate()
{
  static const signed char *cp, terms[] = {
    -2,-2,+0,-1,0,0x01, -2,-2,+0,+0,1,0x01, -2,-1,-1,+0,0,0x01,
    -2,-1,+0,-1,0,0x02, -2,-1,+0,+0,0,0x03, -2,-1,+0,+1,1,0x01,
    -2,+0,+0,-1,0,0x06, -2,+0,+0,+0,1,0x02, -2,+0,+0,+1,0,0x03,
    -2,+1,-1,+0,0,0x04, -2,+1,+0,-1,1,0x04, -2,+1,+0,+0,0,0x06,
    -2,+1,+0,+1,0,0x02, -2,+2,+0,+0,1,0x04, -2,+2,+0,+1,0,0x04,
    -1,-2,-1,+0,0,0x80, -1,-2,+0,-1,0,0x01, -1,-2,+1,-1,0,0x01,
    -1,-2,+1,+0,1,0x01, -1,-1,-1,+1,0,0x88, -1,-1,+1,-2,0,0x40,
    -1,-1,+1,-1,0,0x22, -1,-1,+1,+0,0,0x33, -1,-1,+1,+1,1,0x11,
    -1,+0,-1,+2,0,0x08, -1,+0,+0,-1,0,0x44, -1,+0,+0,+1,0,0x11,
    -1,+0,+1,-2,1,0x40, -1,+0,+1,-1,0,0x66, -1,+0,+1,+0,1,0x22,
    -1,+0,+1,+1,0,0x33, -1,+0,+1,+2,1,0x10, -1,+1,+1,-1,1,0x44,
    -1,+1,+1,+0,0,0x66, -1,+1,+1,+1,0,0x22, -1,+1,+1,+2,0,0x10,
    -1,+2,+0,+1,0,0x04, -1,+2,+1,+0,1,0x04, -1,+2,+1,+1,0,0x04,
    +0,-2,+0,+0,1,0x80, +0,-1,+0,+1,1,0x88, +0,-1,+1,-2,0,0x40,
    +0,-1,+1,+0,0,0x11, +0,-1,+2,-2,0,0x40, +0,-1,+2,-1,0,0x20,
    +0,-1,+2,+0,0,0x30, +0,-1,+2,+1,1,0x10, +0,+0,+0,+2,1,0x08,
    +0,+0,+2,-2,1,0x40, +0,+0,+2,-1,0,0x60, +0,+0,+2,+0,1,0x20,
    +0,+0,+2,+1,0,0x30, +0,+0,+2,+2,1,0x10, +0,+1,+1,+0,0,0x44,
    +0,+1,+1,+2,0,0x10, +0,+1,+2,-1,1,0x40, +0,+1,+2,+0,0,0x60,
    +0,+1,+2,+1,0,0x20, +0,+1,+2,+2,0,0x10, +1,-2,+1,+0,0,0x80,
    +1,-1,+1,+1,0,0x88, +1,+0,+1,+2,0,0x08, +1,+0,+2,-1,0,0x40,
    +1,+0,+2,+1,0,0x10
  }, chood[] = { -1,-1, -1,0, -1,+1, 0,+1, +1,+1, +1,0, +1,-1, 0,-1 };
  ushort (*brow[5])[4], *pix;
  int code[8][2][320], *ip, gval[8], gmin, gmax, sum[4];
  int row, col, x, y, x1, x2, y1, y2, t, weight, grads, color, diag;
  int g, diff, thold, num, c;

  lin_interpolate();
  if (verbose) fprintf (stderr, "VNG interpolation...\n");

  for (row=0; row < 8; row++) {		/* Precalculate for VNG */
    for (col=0; col < 2; col++) {
      ip = code[row][col];
      for (cp=terms, t=0; t < 64; t++) {
	y1 = *cp++;  x1 = *cp++;
	y2 = *cp++;  x2 = *cp++;
	weight = *cp++;
	grads = *cp++;
	color = FC(row+y1,col+x1);
	if (FC(row+y2,col+x2) != color) continue;
	diag = (FC(row,col+1) == color && FC(row+1,col) == color) ? 2:1;
	if (abs(y1-y2) == diag && abs(x1-x2) == diag) continue;
	*ip++ = (y1*width + x1)*4 + color;
	*ip++ = (y2*width + x2)*4 + color;
	*ip++ = weight;
	for (g=0; g < 8; g++)
	  if (grads & 1<<g) *ip++ = g;
	*ip++ = -1;
      }
      *ip++ = INT_MAX;
      for (cp=chood, g=0; g < 8; g++) {
	y = *cp++;  x = *cp++;
	*ip++ = (y*width + x) * 4;
	color = FC(row,col);
	if (FC(row+y,col+x) != color && FC(row+y*2,col+x*2) == color)
	  *ip++ = (y*width + x) * 8 + color;
	else
	  *ip++ = 0;
      }
    }
  }
  brow[4] = calloc (width*3, sizeof **brow);
  merror (brow[4], "vng_interpolate()");
  for (row=0; row < 3; row++)
    brow[row] = brow[4] + row*width;
  for (row=2; row < height-2; row++) {		/* Do VNG interpolation */
    for (col=2; col < width-2; col++) {
      pix = image[row*width+col];
      ip = code[row & 7][col & 1];
      memset (gval, 0, sizeof gval);
      while ((g = ip[0]) != INT_MAX) {		/* Calculate gradients */
	diff = ABS(pix[g] - pix[ip[1]]) << ip[2];
	gval[ip[3]] += diff;
	ip += 5;
	if ((g = ip[-1]) == -1) continue;
	gval[g] += diff;
	while ((g = *ip++) != -1)
	  gval[g] += diff;
      }
      ip++;
      gmin = gmax = gval[0];			/* Choose a threshold */
      for (g=1; g < 8; g++) {
	if (gmin > gval[g]) gmin = gval[g];
	if (gmax < gval[g]) gmax = gval[g];
      }
      if (gmax == 0) {
	memcpy (brow[2][col], pix, sizeof *image);
	continue;
      }
      thold = gmin + (gmax >> 1);
      memset (sum, 0, sizeof sum);
      color = FC(row,col);
      for (num=g=0; g < 8; g++,ip+=2) {		/* Average the neighbors */
	if (gval[g] <= thold) {
	  FORCC
	    if (c == color && ip[1])
	      sum[c] += (pix[c] + pix[ip[1]]) >> 1;
	    else
	      sum[c] += pix[ip[0] + c];
	  num++;
	}
      }
      FORCC {					/* Save to buffer */
	t = pix[color];
	if (c != color)
	  t += (sum[c] - sum[color]) / num;
	brow[2][col][c] = CLIP(t);
      }
    }
    if (row > 3)				/* Write buffer to image */
      memcpy (image[(row-2)*width+2], brow[0]+2, (width-4)*sizeof *image);
    for (g=0; g < 4; g++)
      brow[(g-1) & 3] = brow[g];
  }
  memcpy (image[(row-2)*width+2], brow[0]+2, (width-4)*sizeof *image);
  memcpy (image[(row-1)*width+2], brow[1]+2, (width-4)*sizeof *image);
  free (brow[4]);
}

void CLASS cam_to_cielab (ushort cam[4], float lab[3])
{
  int c, i, j, k;
  float r, xyz[3];
  static const float d65[3] = { 0.950456, 1, 1.088754 };
  static float cbrt[0x10000], xyz_cam[3][4];

  if (cam == NULL) {
    for (i=0; i < 0x10000; i++) {
      r = (float) i / maximum;
      cbrt[i] = r > 0.008856 ? pow(r,1/3.0) : 7.787*r + 16/116.0;
    }
    for (i=0; i < 3; i++)
      for (j=0; j < colors; j++)
        for (xyz_cam[i][j] = k=0; k < 3; k++)
	  xyz_cam[i][j] += xyz_rgb[i][k] * rgb_cam[k][j] / d65[i];
  } else {
    for (i=0; i < 3; i++) {
      for (xyz[i]=0.5, c=0; c < colors; c++)
	xyz[i] += xyz_cam[i][c] * cam[c];
      xyz[i] = cbrt[CLIP((int) xyz[i])];
    }
    lab[0] = 116 * xyz[1] - 16;
    lab[1] = 500 * (xyz[0] - xyz[1]);
    lab[2] = 200 * (xyz[1] - xyz[2]);
  }
}

/*
   Adaptive Homogeneity-Directed interpolation is based on
   the work of Keigo Hirakawa, Thomas Parks, and Paul Lee.
 */
#define TS 256		/* Tile Size */

void CLASS ahd_interpolate()
{
  int i, j, top, left, row, col, tr, tc, fc, c, d, val, hm[2];
  ushort (*pix)[4], (*rix)[3];
  static const int dir[4] = { -1, 1, -TS, TS };
  unsigned ldiff[2][4], abdiff[2][4], leps, abeps;
  float flab[3];
  ushort (*rgb)[TS][TS][3];
   short (*lab)[TS][TS][3];
   char (*homo)[TS][TS], *buffer;

  if (verbose) fprintf (stderr, "AHD interpolation...\n");

  border_interpolate(3);
  buffer = malloc (26*TS*TS);			/* 1664 kB */
  merror (buffer, "ahd_interpolate()");
  rgb  = (void *) buffer;
  lab  = (void *) (buffer + 12*TS*TS);
  homo = (void *) (buffer + 24*TS*TS);

  for (top=0; top < height; top += TS-6)
    for (left=0; left < width; left += TS-6) {
      memset (rgb, 0, 12*TS*TS);

/*  Interpolate green horizontally and vertically:		*/
      for (row = top < 2 ? 2:top; row < top+TS && row < height-2; row++) {
	col = left + (FC(row,left) == 1);
	if (col < 2) col += 2;
	for (fc = FC(row,col); col < left+TS && col < width-2; col+=2) {
	  pix = image + row*width+col;
	  val = ((pix[-1][1] + pix[0][fc] + pix[1][1]) * 2
		- pix[-2][fc] - pix[2][fc]) >> 2;
	  rgb[0][row-top][col-left][1] = ULIM(val,pix[-1][1],pix[1][1]);
	  val = ((pix[-width][1] + pix[0][fc] + pix[width][1]) * 2
		- pix[-2*width][fc] - pix[2*width][fc]) >> 2;
	  rgb[1][row-top][col-left][1] = ULIM(val,pix[-width][1],pix[width][1]);
	}
      }
/*  Interpolate red and blue, and convert to CIELab:		*/
      for (d=0; d < 2; d++)
	for (row=top+1; row < top+TS-1 && row < height-1; row++)
	  for (col=left+1; col < left+TS-1 && col < width-1; col++) {
	    pix = image + row*width+col;
	    rix = &rgb[d][row-top][col-left];
	    if ((c = 2 - FC(row,col)) == 1) {
	      c = FC(row+1,col);
	      val = pix[0][1] + (( pix[-1][2-c] + pix[1][2-c]
				 - rix[-1][1] - rix[1][1] ) >> 1);
	      rix[0][2-c] = CLIP(val);
	      val = pix[0][1] + (( pix[-width][c] + pix[width][c]
				 - rix[-TS][1] - rix[TS][1] ) >> 1);
	    } else
	      val = rix[0][1] + (( pix[-width-1][c] + pix[-width+1][c]
				 + pix[+width-1][c] + pix[+width+1][c]
				 - rix[-TS-1][1] - rix[-TS+1][1]
				 - rix[+TS-1][1] - rix[+TS+1][1] + 1) >> 2);
	    rix[0][c] = CLIP(val);
	    c = FC(row,col);
	    rix[0][c] = pix[0][c];
	    cam_to_cielab (rix[0], flab);
	    FORC3 lab[d][row-top][col-left][c] = 64*flab[c];
	  }
/*  Build homogeneity maps from the CIELab images:		*/
      memset (homo, 0, 2*TS*TS);
      for (row=top+2; row < top+TS-2 && row < height; row++) {
	tr = row-top;
	for (col=left+2; col < left+TS-2 && col < width; col++) {
	  tc = col-left;
	  for (d=0; d < 2; d++)
	    for (i=0; i < 4; i++)
	      ldiff[d][i] = ABS(lab[d][tr][tc][0]-lab[d][tr][tc+dir[i]][0]);
	  leps = MIN(MAX(ldiff[0][0],ldiff[0][1]),
		     MAX(ldiff[1][2],ldiff[1][3]));
	  for (d=0; d < 2; d++)
	    for (i=0; i < 4; i++)
	      if (i >> 1 == d || ldiff[d][i] <= leps)
		abdiff[d][i] = SQR(lab[d][tr][tc][1]-lab[d][tr][tc+dir[i]][1])
			     + SQR(lab[d][tr][tc][2]-lab[d][tr][tc+dir[i]][2]);
	  abeps = MIN(MAX(abdiff[0][0],abdiff[0][1]),
		      MAX(abdiff[1][2],abdiff[1][3]));
	  for (d=0; d < 2; d++)
	    for (i=0; i < 4; i++)
	      if (ldiff[d][i] <= leps && abdiff[d][i] <= abeps)
		homo[d][tr][tc]++;
	}
      }
/*  Combine the most homogenous pixels for the final result:	*/
      for (row=top+3; row < top+TS-3 && row < height-3; row++) {
	tr = row-top;
	for (col=left+3; col < left+TS-3 && col < width-3; col++) {
	  tc = col-left;
	  for (d=0; d < 2; d++)
	    for (hm[d]=0, i=tr-1; i <= tr+1; i++)
	      for (j=tc-1; j <= tc+1; j++)
		hm[d] += homo[d][i][j];
	  if (hm[0] != hm[1])
	    FORC3 image[row*width+col][c] = rgb[hm[1] > hm[0]][tr][tc][c];
	  else
	    FORC3 image[row*width+col][c] =
		(rgb[0][tr][tc][c] + rgb[1][tr][tc][c]) >> 1;
	}
      }
    }
  free (buffer);
}
#undef TS

/*
   Bilateral Filtering was developed by C. Tomasi and R. Manduchi.
 */
void CLASS bilateral_filter()
{
  float (**window)[7], *kernel, scale_r, elut[1024], sum[5];
  int c, i, wr, ws, wlast, row, col, y, x;
  unsigned sep;

  if (verbose) fprintf (stderr, "Bilateral filtering...\n");

  wr = ceil(sigma_d*2);		/* window radius */
  ws = 2*wr + 1;		/* window size */
  window = calloc ((ws+1)*sizeof  *window +
		 ws*width*sizeof **window + ws*sizeof *kernel, 1);
  merror (window, "bilateral_filter()");
  for (i=0; i <= ws; i++)
    window[i] = (float(*)[7]) (window+ws+1) + i*width;
  kernel = (float *) window[ws] + wr;
  for (i=-wr; i <= wr; i++)
    kernel[i] = 256 / (2*SQR(sigma_d)) * i*i + 0.25;
  scale_r     = 256 / (2*SQR(sigma_r));
  for (i=0; i < 1024; i++)
    elut[i] = exp (-i/256.0);

  for (wlast=-1, row=0; row < height; row++) {
    while (wlast < row+wr) {
      wlast++;
      for (i=0; i <= ws; i++)	/* rotate window rows */
	window[(ws+i) % (ws+1)] = window[i];
      if (wlast < height)
	for (col=0; col < width; col++) {
	  FORCC window[ws-1][col][c] = image[wlast*width+col][c];
	  cam_to_cielab (image[wlast*width+col], window[ws-1][col]+4);
	}
    }
    for (col=0; col < width; col++) {
      memset (sum, 0, sizeof sum);
      for (y=-wr; y <= wr; y++)
	if ((unsigned)(row+y) < height)
	  for (x=-wr; x <= wr; x++)
	    if ((unsigned)(col+x) < width) {
	      sep = ( SQR(window[wr+y][col+x][4] - window[wr][col][4])
		    + SQR(window[wr+y][col+x][5] - window[wr][col][5])
		    + SQR(window[wr+y][col+x][6] - window[wr][col][6]) )
			* scale_r + kernel[y] + kernel[x];
	      if (sep < 1024) {
		FORCC sum[c] += elut[sep] * window[wr+y][col+x][c];
		sum[4] += elut[sep];
	      }
	    }
      FORCC image[row*width+col][c] = sum[c]/sum[4];
    }
  }
  free (window);
}

void CLASS parse_makernote()
{
  static const uchar xlat[2][256] = {
  { 0xc1,0xbf,0x6d,0x0d,0x59,0xc5,0x13,0x9d,0x83,0x61,0x6b,0x4f,0xc7,0x7f,0x3d,0x3d,
    0x53,0x59,0xe3,0xc7,0xe9,0x2f,0x95,0xa7,0x95,0x1f,0xdf,0x7f,0x2b,0x29,0xc7,0x0d,
    0xdf,0x07,0xef,0x71,0x89,0x3d,0x13,0x3d,0x3b,0x13,0xfb,0x0d,0x89,0xc1,0x65,0x1f,
    0xb3,0x0d,0x6b,0x29,0xe3,0xfb,0xef,0xa3,0x6b,0x47,0x7f,0x95,0x35,0xa7,0x47,0x4f,
    0xc7,0xf1,0x59,0x95,0x35,0x11,0x29,0x61,0xf1,0x3d,0xb3,0x2b,0x0d,0x43,0x89,0xc1,
    0x9d,0x9d,0x89,0x65,0xf1,0xe9,0xdf,0xbf,0x3d,0x7f,0x53,0x97,0xe5,0xe9,0x95,0x17,
    0x1d,0x3d,0x8b,0xfb,0xc7,0xe3,0x67,0xa7,0x07,0xf1,0x71,0xa7,0x53,0xb5,0x29,0x89,
    0xe5,0x2b,0xa7,0x17,0x29,0xe9,0x4f,0xc5,0x65,0x6d,0x6b,0xef,0x0d,0x89,0x49,0x2f,
    0xb3,0x43,0x53,0x65,0x1d,0x49,0xa3,0x13,0x89,0x59,0xef,0x6b,0xef,0x65,0x1d,0x0b,
    0x59,0x13,0xe3,0x4f,0x9d,0xb3,0x29,0x43,0x2b,0x07,0x1d,0x95,0x59,0x59,0x47,0xfb,
    0xe5,0xe9,0x61,0x47,0x2f,0x35,0x7f,0x17,0x7f,0xef,0x7f,0x95,0x95,0x71,0xd3,0xa3,
    0x0b,0x71,0xa3,0xad,0x0b,0x3b,0xb5,0xfb,0xa3,0xbf,0x4f,0x83,0x1d,0xad,0xe9,0x2f,
    0x71,0x65,0xa3,0xe5,0x07,0x35,0x3d,0x0d,0xb5,0xe9,0xe5,0x47,0x3b,0x9d,0xef,0x35,
    0xa3,0xbf,0xb3,0xdf,0x53,0xd3,0x97,0x53,0x49,0x71,0x07,0x35,0x61,0x71,0x2f,0x43,
    0x2f,0x11,0xdf,0x17,0x97,0xfb,0x95,0x3b,0x7f,0x6b,0xd3,0x25,0xbf,0xad,0xc7,0xc5,
    0xc5,0xb5,0x8b,0xef,0x2f,0xd3,0x07,0x6b,0x25,0x49,0x95,0x25,0x49,0x6d,0x71,0xc7 },
  { 0xa7,0xbc,0xc9,0xad,0x91,0xdf,0x85,0xe5,0xd4,0x78,0xd5,0x17,0x46,0x7c,0x29,0x4c,
    0x4d,0x03,0xe9,0x25,0x68,0x11,0x86,0xb3,0xbd,0xf7,0x6f,0x61,0x22,0xa2,0x26,0x34,
    0x2a,0xbe,0x1e,0x46,0x14,0x68,0x9d,0x44,0x18,0xc2,0x40,0xf4,0x7e,0x5f,0x1b,0xad,
    0x0b,0x94,0xb6,0x67,0xb4,0x0b,0xe1,0xea,0x95,0x9c,0x66,0xdc,0xe7,0x5d,0x6c,0x05,
    0xda,0xd5,0xdf,0x7a,0xef,0xf6,0xdb,0x1f,0x82,0x4c,0xc0,0x68,0x47,0xa1,0xbd,0xee,
    0x39,0x50,0x56,0x4a,0xdd,0xdf,0xa5,0xf8,0xc6,0xda,0xca,0x90,0xca,0x01,0x42,0x9d,
    0x8b,0x0c,0x73,0x43,0x75,0x05,0x94,0xde,0x24,0xb3,0x80,0x34,0xe5,0x2c,0xdc,0x9b,
    0x3f,0xca,0x33,0x45,0xd0,0xdb,0x5f,0xf5,0x52,0xc3,0x21,0xda,0xe2,0x22,0x72,0x6b,
    0x3e,0xd0,0x5b,0xa8,0x87,0x8c,0x06,0x5d,0x0f,0xdd,0x09,0x19,0x93,0xd0,0xb9,0xfc,
    0x8b,0x0f,0x84,0x60,0x33,0x1c,0x9b,0x45,0xf1,0xf0,0xa3,0x94,0x3a,0x12,0x77,0x33,
    0x4d,0x44,0x78,0x28,0x3c,0x9e,0xfd,0x65,0x57,0x16,0x94,0x6b,0xfb,0x59,0xd0,0xc8,
    0x22,0x36,0xdb,0xd2,0x63,0x98,0x43,0xa1,0x04,0x87,0x86,0xf7,0xa6,0x26,0xbb,0xd6,
    0x59,0x4d,0xbf,0x6a,0x2e,0xaa,0x2b,0xef,0xe6,0x78,0xb6,0x4e,0xe0,0x2f,0xdc,0x7c,
    0xbe,0x57,0x19,0x32,0x7e,0x2a,0xd0,0xb8,0xba,0x29,0x00,0x3c,0x52,0x7d,0xa8,0x49,
    0x3b,0x2d,0xeb,0x25,0x49,0xfa,0xa3,0xaa,0x39,0xa7,0xc5,0xa7,0x50,0x11,0x36,0xfb,
    0xc6,0x67,0x4a,0xf5,0xa5,0x12,0x65,0x7e,0xb0,0xdf,0xaf,0x4e,0xb3,0x61,0x7f,0x2f } };
  unsigned base=0, offset=0, entries, tag, type, len, save, c;
  unsigned ver97=0, serial=0, i;
  uchar buf97[324], ci, cj, ck;
  static const int size[] = { 1,1,1,2,4,8,1,1,2,4,8,4,8 };
  short sorder;
  char buf[10];
/*
   The MakerNote might have its own TIFF header (possibly with
   its own byte-order!), or it might just be a table.
 */
  sorder = order;
  fread (buf, 1, 10, ifp);
  if (!strncmp (buf,"KC" ,2) ||		/* these aren't TIFF format */
      !strncmp (buf,"MLY",3)) return;
  if (!strcmp (buf,"Nikon")) {
    base = ftell(ifp);
    order = get2();
    if (get2() != 42) goto quit;
    offset = get4();
    fseek (ifp, offset-8, SEEK_CUR);
  } else if (!strncmp (buf,"FUJIFILM",8) ||
	     !strcmp  (buf,"Panasonic")) {
    order = 0x4949;
    fseek (ifp,  2, SEEK_CUR);
  } else if (!strcmp (buf,"OLYMP") ||
	     !strcmp (buf,"LEICA") ||
	     !strcmp (buf,"EPSON"))
    fseek (ifp, -2, SEEK_CUR);
  else if (!strcmp (buf,"AOC") ||
	   !strcmp (buf,"QVC"))
    fseek (ifp, -4, SEEK_CUR);
  else fseek (ifp, -10, SEEK_CUR);

  entries = get2();
  while (entries--) {
    tag  = get2();
    type = get2();
    len  = get4();
    save = ftell(ifp);
    if (len * size[type < 13 ? type:0] > 4)
      fseek (ifp, get4()+base, SEEK_SET);

    if (tag == 8 && type == 4)
      shot_order = get4();
    if (tag == 0xc && len == 4) {
      camera_red  = getrat();
      camera_blue = getrat();
    }
    if (tag == 0x14 && len == 2560 && type == 7) {
      fseek (ifp, 1248, SEEK_CUR);
      goto get2_256;
    }
    if (strstr(make,"PENTAX")) {
      if (tag == 0x1b) tag = 0x1018;
      if (tag == 0x1c) tag = 0x1017;
    }
    if (tag == 0x1d)
      while ((c = fgetc(ifp)))
	serial = serial*10 + (isdigit(c) ? c - '0' : c % 10);
    if (tag == 0x8c)
      nikon_curve_offset = ftell(ifp) + 2112;
    if (tag == 0x96)
      nikon_curve_offset = ftell(ifp) + 2;
    if (tag == 0x97) {
      for (i=0; i < 4; i++)
	ver97 = (ver97 << 4) + fgetc(ifp)-'0';
      switch (ver97) {
	case 0x100:
	  fseek (ifp, 68, SEEK_CUR);
	  FORC4 cam_mul[(c >> 1) | ((c & 1) << 1)] = get2();
	  break;
	case 0x102:
	  fseek (ifp, 6, SEEK_CUR);
	  goto get2_rggb;
	case 0x103:
	  fseek (ifp, 16, SEEK_CUR);
	  FORC4 cam_mul[c] = get2();
      }
      if (ver97 >> 8 == 2) {
	if (ver97 != 0x205) fseek (ifp, 280, SEEK_CUR);
	fread (buf97, 324, 1, ifp);
      }
    }
    if (tag == 0xa7 && ver97 >> 8 == 2) {
      ci = xlat[0][serial & 0xff];
      cj = xlat[1][fgetc(ifp)^fgetc(ifp)^fgetc(ifp)^fgetc(ifp)];
      ck = 0x60;
      for (i=0; i < 324; i++)
	buf97[i] ^= (cj += ci * ck++);
      FORC4 cam_mul[c ^ (c >> 1)] =
	sget2 (buf97 + (ver97 == 0x205 ? 14:6) + c*2);
    }
    if (tag == 0xe0 && len == 17) {
      get2();
      raw_width  = get2();
      raw_height = get2();
    }
    if (tag == 0x200 && len == 4)
      black = (get2()+get2()+get2()+get2())/4;
    if (tag == 0x201 && len == 4)
      goto get2_rggb;
    if (tag == 0x401 && len == 4) {
      black = (get4()+get4()+get4()+get4())/4;
    }
    if (tag == 0xe01) {		/* Nikon Capture Note */
      type = order;
      order = 0x4949;
      fseek (ifp, 22, SEEK_CUR);
      for (offset=22; offset+22 < len; offset += 22+i) {
	tag = get4();
	fseek (ifp, 14, SEEK_CUR);
	i = get4()-4;
	if (tag == 0x76a43207) flip = get2();
	else fseek (ifp, i, SEEK_CUR);
      }
      order = type;
    }
    if (tag == 0xe80 && len == 256 && type == 7) {
      fseek (ifp, 48, SEEK_CUR);
      camera_red  = get2() * 508 * 1.078 / 0x10000;
      camera_blue = get2() * 382 * 1.173 / 0x10000;
    }
    if (tag == 0xf00 && len == 614 && type == 7) {
      fseek (ifp, 188, SEEK_CUR);
      goto get2_256;
    }
    if (tag == 0x1017)
      camera_red  = get2() / 256.0;
    if (tag == 0x1018)
      camera_blue = get2() / 256.0;
    if (tag == 0x2011 && len == 2) {
get2_256:
      order = 0x4d4d;
      camera_red  = get2() / 256.0;
      camera_blue = get2() / 256.0;
    }
    if (tag == 0x4001) {
      fseek (ifp, strstr(model,"EOS-1D") ?  68 :
		  strstr(model,"EOS 5D") ? 126 : 50, SEEK_CUR);
get2_rggb:
      FORC4 cam_mul[c ^ (c >> 1)] = get2();
    }
    fseek (ifp, save+4, SEEK_SET);
  }
quit:
  order = sorder;
}

/*
   Since the TIFF DateTime string has no timezone information,
   assume that the camera's clock was set to Universal Time.
 */
void CLASS get_timestamp (int reversed)
{
  struct tm t;
  char str[20];
  int i;

  if (timestamp) return;
  str[19] = 0;
  if (reversed)
    for (i=19; i--; ) str[i] = fgetc(ifp);
  else
    fread (str, 19, 1, ifp);
  if (sscanf (str, "%d:%d:%d %d:%d:%d", &t.tm_year, &t.tm_mon,
	&t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec) != 6)
    return;
  t.tm_year -= 1900;
  t.tm_mon -= 1;
  if (mktime(&t) > 0)
    timestamp = mktime(&t);
}

void CLASS parse_exif (int base)
{
  int entries, tag, type, len, val, save;

  entries = get2();
  while (entries--) {
    tag  = get2();
    type = get2();
    len  = get4();
    val  = get4();
    save = ftell(ifp);
    fseek (ifp, base+val, SEEK_SET);
    if (tag == 0x9003 || tag == 0x9004)
      get_timestamp(0);
    if (tag == 0x927c)
      parse_makernote();
    fseek (ifp, save, SEEK_SET);
  }
}

void CLASS parse_mos (int offset);
void CLASS sony_decrypt (unsigned *data, int len, int start, int key);

int CLASS parse_tiff_ifd (int base, int level)
{
  unsigned entries, tag, type, len, plen=16, save;
  int done=0, use_cm=0, cfa, i, j, c;
  static const int size[] = { 1,1,1,2,4,8,1,1,2,4,8,4,8 };
  char software[64], *cbuf, *cp;
  static const int flip_map[] = { 0,1,3,2,4,6,7,5 };
  uchar cfa_pat[16], cfa_pc[] = { 0,1,2,3 }, tab[256];
  double dblack, cc[4][4], cm[4][3], cam_xyz[4][3];
  double ab[]={ 1,1,1,1 }, asn[] = { 0,0,0,0 }, xyz[] = { 1,1,1 };
  unsigned *buf, sony_offset=0, sony_length=0, sony_key=0;
  FILE *sfp;

  for (j=0; j < 4; j++)
    for (i=0; i < 4; i++)
      cc[j][i] = i == j;
  entries = get2();
  if (entries > 512) return 1;
  while (entries--) {
    tag  = get2();
    type = get2();
    len  = get4();
    save = ftell(ifp);
    if (tag > 50700 && tag < 50800) done = 1;
    if (len * size[type < 13 ? type:0] > 4)
      fseek (ifp, get4()+base, SEEK_SET);
    switch (tag) {
      case 0x11:
      case 0x12:
	if (type == 3 && len == 1)
	  cam_mul[(tag-0x11)*2] = get2() / 256.0;
	break;
      case 0x27:
	if (len < 50) break;
	fseek (ifp, 12, SEEK_CUR);
	FORC3 cam_mul[c] = get2();
	break;
      case 0x2:
      case 0x100:			/* ImageWidth */
	if ((strcmp(make,"Canon") || level) && len == 1)
	  raw_width = type==3 ? get2() : get4();
	break;
      case 0x3:
      case 0x101:			/* ImageHeight */
	if ((strcmp(make,"Canon") || level) && len == 1)
	  raw_height = type==3 ? get2() : get4();
	break;
      case 0x102:			/* Bits per sample */
	fuji_secondary = len == 2;
	maximum = (1 << (tiff_bps = get2())) - 1;
	break;
      case 0x103:			/* Compression */
	tiff_data_compression = get2();
	break;
      case 0x106:			/* Kodak color format */
	kodak_data_compression = get2();
	break;
      case 0x10f:			/* Make */
	fgets (make, 64, ifp);
	break;
      case 0x110:			/* Model */
	fgets (model, 64, ifp);
	break;
      case 0x111:			/* StripOffset */
	data_offset = get4();
	break;
      case 0x112:			/* Orientation */
	flip = flip_map[(get2()-1) & 7];
	break;
      case 0x115:			/* SamplesPerPixel */
	tiff_samples = get2();
	break;
      case 0x131:			/* Software tag */
	fgets (software, 64, ifp);
	if (!strncmp(software,"Adobe",5) ||
	    !strncmp(software,"Bibble",6) ||
	    !strcmp (software,"Digital Photo Professional"))
	  make[0] = 0;
	break;
      case 0x132:			/* DateTime tag */
	get_timestamp(0);
	break;
      case 0x144:			/* TileOffsets */
	if (level) {
	  data_offset = ftell(ifp);
	} else {
	  data_offset = get4();
	  done = 1;
	}
	break;
      case 0x14a:			/* SubIFD tag */
	if (len > 2 && !dng_version && !strcmp(make,"Kodak"))
	    len = 2;
	while (len--) {
	  i = ftell(ifp);
	  fseek (ifp, get4()+base, SEEK_SET);
	  if (parse_tiff_ifd (base, level+1)) break;
	  fseek (ifp, i+4, SEEK_SET);
	}
	break;
      case 29184: sony_offset = get4();  break;
      case 29185: sony_length = get4();  break;
      case 29217: sony_key    = get4();  break;
      case 29443:
	FORC4 cam_mul[c ^ (c < 2)] = get2();
	break;
      case 33405:			/* Model2 */
	fgets (model2, 64, ifp);
	break;
      case 33422:			/* CFAPattern */
	if ((plen=len) > 16) plen = 16;
	fread (cfa_pat, 1, plen, ifp);
	for (colors=cfa=i=0; i < plen; i++) {
	  colors += !(cfa & (1 << cfa_pat[i]));
	  cfa |= 1 << cfa_pat[i];
	}
	if (cfa == 070) memcpy (cfa_pc,"\003\004\005",3);	/* CMY */
	if (cfa == 072) memcpy (cfa_pc,"\005\003\004\001",4);	/* GMCY */
	goto guess_cfa_pc;
      case 34310:
	parse_mos (ftell(ifp));
	break;
      case 34665:			/* EXIF tag */
	fseek (ifp, get4()+base, SEEK_SET);
	parse_exif (base);
	break;
      case 37122:			/* CompressedBitsPerPixel */
	kodak_cbpp = get4();
	break;
      case 37400:
	for (raw_color = i=0; i < 3; i++) {
	  getrat();
	  FORC3 rgb_cam[i][c] = getrat();
	}
	break;
      case 46275:
	strcpy (make, "Imacon");
	data_offset = ftell(ifp);
	raw_width = 4090;
	raw_height = len / raw_width / 2;
	done = 1;
	break;
      case 50454:			/* Sinar tag */
      case 50455:
	if (!(cbuf = malloc(len))) break;
	fread (cbuf, 1, len, ifp);
	for (cp = cbuf-1; cp && cp < cbuf+len; cp = strchr(cp,'\n'))
	  if (!strncmp (++cp,"Neutral ",8))
	    sscanf (cp+8, "%f %f %f", cam_mul, cam_mul+1, cam_mul+2);
	free (cbuf);
	break;
      case 50706:			/* DNGVersion */
	FORC4 dng_version = (dng_version << 8) + fgetc(ifp);
	break;
      case 50710:			/* CFAPlaneColor */
	if (len > 4) len = 4;
	colors = len;
	fread (cfa_pc, 1, colors, ifp);
guess_cfa_pc:
	FORCC tab[cfa_pc[c]] = c;
	for (i=16; i--; )
	  filters = filters << 2 | tab[cfa_pat[i % plen]];
	break;
      case 50711:			/* CFALayout */
	if (get2() == 2) {
	  fuji_width = (raw_width+1)/2;
	  filters = 0x49494949;
	}
	break;
      case 0x123:
      case 0x90d:
      case 50712:			/* LinearizationTable */
	if (len > 0x1000)
	    len = 0x1000;
	read_shorts (curve, len);
	for (i=len; i < 0x1000; i++)
	  maximum = curve[i] = curve[i-1];
	break;
      case 50714:			/* BlackLevel */
      case 50715:			/* BlackLevelDeltaH */
      case 50716:			/* BlackLevelDeltaV */
	for (dblack=i=0; i < len; i++)
	  dblack += getrat();
	black += dblack/len + 0.5;
	break;
      case 50717:			/* WhiteLevel */
	maximum = get2();
	break;
      case 50718:			/* DefaultScale */
	i  = get4();
	j  = get4() * get4();
	i *= get4();
	if (i > j) xmag = i / j;
	else	   ymag = j / i;
	break;
      case 50721:			/* ColorMatrix1 */
      case 50722:			/* ColorMatrix2 */
	FORCC for (j=0; j < 3; j++)
	  cm[c][j] = getrat();
	use_cm = 1;
	break;
      case 50723:			/* CameraCalibration1 */
      case 50724:			/* CameraCalibration2 */
	for (i=0; i < colors; i++)
	  FORCC cc[i][c] = getrat();
      case 50727:			/* AnalogBalance */
	FORCC ab[c] = getrat();
	break;
      case 50728:			/* AsShotNeutral */
	FORCC asn[c] = getrat();
	break;
      case 50729:			/* AsShotWhiteXY */
	xyz[0] = getrat();
	xyz[1] = getrat();
	xyz[2] = 1 - xyz[0] - xyz[1];
	break;
      case 50740:			/* DNGPrivateData */
	if (dng_version) break;
	fseek (ifp, get4()+base, SEEK_SET);
	parse_tiff_ifd (base, level+1);
	break;
      case 50829:			/* ActiveArea */
	top_margin = get4();
	left_margin = get4();
	height = get4() - top_margin;
	width = get4() - left_margin;
    }
    fseek (ifp, save+4, SEEK_SET);
  }
  if (sony_length && (buf = malloc(sony_length))) {
    fseek (ifp, sony_offset, SEEK_SET);
    fread (buf, sony_length, 1, ifp);
    sony_decrypt (buf, sony_length/4, 1, sony_key);
    sfp = ifp;
    if ((ifp = tmpfile())) {
      fwrite (buf, sony_length, 1, ifp);
      fseek (ifp, 0, SEEK_SET);
      parse_tiff_ifd (-sony_offset, level);
      fclose (ifp);
    }
    ifp = sfp;
    free (buf);
  }
  if (!(base | level | dng_version) &&
    (strstr(make,"Minolta") || strstr(make,"MINOLTA"))) make[0] = 0;
  for (i=0; i < colors; i++)
    FORCC cc[i][c] *= ab[i];
  if (use_cm) {
    FORCC for (i=0; i < 3; i++)
      for (cam_xyz[c][i]=j=0; j < colors; j++)
	cam_xyz[c][i] += cc[c][j] * cm[j][i] * xyz[i];
    cam_xyz_coeff (cam_xyz);
  }
  if (asn[0])
    FORCC pre_mul[c] = 1 / asn[c];
  if (!use_cm)
    FORCC pre_mul[c] /= cc[c][c];
  return done;
}

void CLASS parse_tiff (int base)
{
  int doff, maxifd=1000;

  fseek (ifp, base, SEEK_SET);
  order = get2();
  if (order != 0x4949 && order != 0x4d4d) return;
  get2();
  while ((doff = get4()) && maxifd--) {
    fseek (ifp, doff+base, SEEK_SET);
    if (parse_tiff_ifd (base, 0)) break;
    if (!dng_version && data_offset == 8) make[0] = 0;
  }
  if (!dng_version && !strncmp(make,"Kodak",5)) {
    fseek (ifp, 12+base, SEEK_SET);
    parse_tiff_ifd (base, 2);
  }
}

void CLASS parse_minolta()
{
  int save, tag, len, offset, high=0, wide=0, i, c;

  fseek (ifp, 4, SEEK_SET);
  offset = get4() + 8;
  while ((save=ftell(ifp)) < offset) {
    tag = get4();
    len = get4();
    switch (tag) {
      case 0x505244:				/* PRD */
	fseek (ifp, 8, SEEK_CUR);
	high = get2();
	wide = get2();
	break;
      case 0x574247:				/* WBG */
	get4();
	i = strstr(model,"A200") ? 3:0;
	FORC4 cam_mul[c ^ (c >> 1) ^ i] = get2();
	break;
      case 0x545457:				/* TTW */
	parse_tiff (ftell(ifp));
    }
    fseek (ifp, save+len+8, SEEK_SET);
  }
  raw_height = high;
  raw_width  = wide;
  data_offset = offset;
}

/*
   Many cameras have a "debug mode" that writes JPEG and raw
   at the same time.  The raw file has no header, so try to
   to open the matching JPEG file and read its metadata.
 */
void CLASS parse_external_jpeg()
{
  char *file, *ext, *jname, *jfile, *jext;
  FILE *save=ifp;

  ext  = strrchr (ifname, '.');
  file = strrchr (ifname, '/');
  if (!file) file = strrchr (ifname, '\\');
  if (!file) file = ifname-1;
  file++;
  if (strlen(ext) != 4 || ext-file != 8) return;
  jname = malloc (strlen(ifname) + 1);
  merror (jname, "parse_external()");
  strcpy (jname, ifname);
  jfile = file - ifname + jname;
  jext  = ext  - ifname + jname;
  if (strcasecmp (ext, ".jpg")) {
    strcpy (jext, isupper(ext[1]) ? ".JPG":".jpg");
    memcpy (jfile, file+4, 4);
    memcpy (jfile+4, file, 4);
  } else
    while (isdigit(*--jext)) {
      if (*jext != '9') {
        (*jext)++;
	break;
      }
      *jext = '0';
    }
  if (strcmp (jname, ifname)) {
    if ((ifp = fopen (jname, "rb"))) {
      if (verbose)
	fprintf (stderr, "Reading metadata from %s...\n", jname);
      parse_tiff (12);
      fclose (ifp);
    }
  }
  if (!timestamp)
    fprintf (stderr, "Failed to read metadata from %s\n", jname);
  free (jname);
  ifp = save;
}

/*
   CIFF block 0x1030 contains an 8x8 white sample.
   Load this into white[][] for use in scale_colors().
 */
void CLASS ciff_block_1030()
{
  static const ushort key[] = { 0x410, 0x45f3 };
  int i, bpp, row, col, vbits=0;
  unsigned long bitbuf=0;

  get2();
  if (get4() != 0x80008) return;
  if (get4() == 0) return;
  bpp = get2();
  if (bpp != 10 && bpp != 12) return;
  for (i=row=0; row < 8; row++)
    for (col=0; col < 8; col++) {
      if (vbits < bpp) {
	bitbuf = bitbuf << 16 | (get2() ^ key[i++ & 1]);
	vbits += 16;
      }
      white[row][col] =
	bitbuf << (LONG_BIT - vbits) >> (LONG_BIT - bpp);
      vbits -= bpp;
    }
}

/*
   Parse a CIFF file, better known as Canon CRW format.
 */
void CLASS parse_ciff (int offset, int length)
{
  int tboff, nrecs, i, c, type, len, roff, aoff, save, wbi=-1;
  static const int remap[] = { 1,2,3,4,5,1 };
  static const int remap_10d[] = { 0,1,3,4,5,6,0,0,2,8 };
  static const int remap_s70[] = { 0,1,2,9,4,3,6,7,8,9,10,0,0,0,7,0,0,8 };
  ushort key[] = { 0x410, 0x45f3 };

  if (strcmp(model,"Canon PowerShot G6") &&
      strcmp(model,"Canon PowerShot S60") &&
      strcmp(model,"Canon PowerShot S70") &&
      strcmp(model,"Canon PowerShot Pro1"))
    key[0] = key[1] = 0;
  fseek (ifp, offset+length-4, SEEK_SET);
  tboff = get4() + offset;
  fseek (ifp, tboff, SEEK_SET);
  nrecs = get2();
  if (nrecs > 100) return;
  for (i = 0; i < nrecs; i++) {
    type = get2();
    len  = get4();
    roff = get4();
    aoff = offset + roff;
    save = ftell(ifp);
    if (type == 0x080a) {		/* Get the camera make and model */
      fseek (ifp, aoff, SEEK_SET);
      fread (make, 64, 1, ifp);
      fseek (ifp, aoff+strlen(make)+1, SEEK_SET);
      fread (model, 64, 1, ifp);
    }
    if (type == 0x102a) {		/* Find the White Balance index */
      fseek (ifp, aoff+14, SEEK_SET);	/* 0=auto, 1=daylight, 2=cloudy ... */
      wbi = get2();
      if (((!strcmp(model,"Canon EOS DIGITAL REBEL") ||
	    !strcmp(model,"Canon EOS 300D DIGITAL"))) && wbi == 6)
	wbi++;
    }
    if (type == 0x102c) {		/* Get white balance (G2) */
      if (!strcmp(model,"Canon PowerShot G1") ||
	  !strcmp(model,"Canon PowerShot Pro90 IS")) {
	fseek (ifp, aoff+120, SEEK_SET);
	FORC4 cam_mul[c ^ 2] = get2();
      } else {
	fseek (ifp, aoff+100, SEEK_SET);
	goto common;
      }
    }
    if (type == 0x0032) {		/* Get white balance (D30 & G3) */
      if (!strcmp(model,"Canon EOS D30")) {
	fseek (ifp, aoff+72, SEEK_SET);
common:
	camera_red   = get2() ^ key[0];
	camera_red   =(get2() ^ key[1]) / camera_red;
	camera_blue  = get2() ^ key[0];
	camera_blue /= get2() ^ key[1];
	if (!wbi) camera_red = -1;	/* Use my auto WB for this photo */
      } else if (!strcmp(model,"Canon PowerShot G6") ||
		 !strcmp(model,"Canon PowerShot S60") ||
		 !strcmp(model,"Canon PowerShot S70")) {
	fseek (ifp, aoff+96 + remap_s70[wbi]*8, SEEK_SET);
	goto common;
      } else if (!strcmp(model,"Canon PowerShot Pro1")) {
	fseek (ifp, aoff+96 + wbi*8, SEEK_SET);
	goto common;
      } else {
	fseek (ifp, aoff+80 + (wbi < 6 ? remap[wbi]*8 : 0), SEEK_SET);
	if (!camera_red)
	  goto common;
      }
    }
    if (type == 0x10a9) {		/* Get white balance (D60) */
      if (!strcmp(model,"Canon EOS 10D"))
	wbi = remap_10d[wbi];
      fseek (ifp, aoff+2 + wbi*8, SEEK_SET);
      camera_red  = get2();
      camera_red /= get2();
      camera_blue = get2();
      camera_blue = get2() / camera_blue;
    }
    if (type == 0x1030 && (wbi == 6 || wbi == 15)) {
      fseek (ifp, aoff, SEEK_SET);	/* Get white sample */
      ciff_block_1030();
    }
    if (type == 0x1031) {		/* Get the raw width and height */
      fseek (ifp, aoff+2, SEEK_SET);
      raw_width  = get2();
      raw_height = get2();
    }
    if (type == 0x180e) {		/* Get the timestamp */
      fseek (ifp, aoff, SEEK_SET);
      timestamp = get4();
    }
    if (type == 0x5817)
      shot_order = len;
    if (type == 0x580e)
      timestamp = len;
#ifdef LOCALTIME
    if ((type | 0x4000) == 0x580e)
      timestamp = mktime (gmtime (&timestamp));
#endif
    if (type == 0x5813)
      flash_used = int_to_float(len);
    if (type == 0x5814)
      canon_5814 = int_to_float(len);
    if (type == 0x1810) {		/* Get the rotation */
      fseek (ifp, aoff+12, SEEK_SET);
      flip = get4();
    }
    if (type == 0x1835) {		/* Get the decoder table */
      fseek (ifp, aoff, SEEK_SET);
      crw_init_tables (get4());
    }
    if (type >> 8 == 0x28 || type >> 8 == 0x30)	/* Get sub-tables */
      parse_ciff(aoff, len);
    fseek (ifp, save, SEEK_SET);
  }
}

void CLASS parse_rollei()
{
  char line[128], *val;
  int tx=0, ty=0;
  struct tm t;

  fseek (ifp, 0, SEEK_SET);
  do {
    fgets (line, 128, ifp);
    if ((val = strchr(line,'=')))
      *val++ = 0;
    else
      val = line + strlen(line);
    if (!strcmp(line,"DAT"))
      sscanf (val, "%d.%d.%d", &t.tm_mday, &t.tm_mon, &t.tm_year);
    if (!strcmp(line,"TIM"))
      sscanf (val, "%d:%d:%d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    if (!strcmp(line,"HDR"))
      data_offset = atoi(val);
    if (!strcmp(line,"X  "))
      raw_width = atoi(val);
    if (!strcmp(line,"Y  "))
      raw_height = atoi(val);
    if (!strcmp(line,"TX "))
      tx = atoi(val);
    if (!strcmp(line,"TY "))
      ty = atoi(val);
  } while (strncmp(line,"EOHD",4));
  t.tm_year -= 1900;
  t.tm_mon -= 1;
  if (mktime(&t) > 0)
    timestamp = mktime(&t);
  data_offset += tx * ty * 2;
  strcpy (make, "Rollei");
  strcpy (model,"d530flex");
}

void CLASS parse_mos (int offset)
{
  char data[40];
  int skip, from, i, c, neut[4];
  static const unsigned bayer[] =
	{ 0x94949494, 0x61616161, 0x16161616, 0x49494949 };

  fseek (ifp, offset, SEEK_SET);
  while (1) {
    fread (data, 1, 8, ifp);
    if (strcmp(data,"PKTS")) break;
    if (!make[0]) strcpy(make,"Leaf");
    fread (data, 1, 40, ifp);
    skip = get4();
    from = ftell(ifp);
#ifdef USE_LCMS
    if (!strcmp(data,"icc_camera_profile")) {
      profile_length = skip;
      profile_offset = from;
    }
#endif
    if (!strcmp(data,"CaptProf_number_of_planes")) {
      fscanf (ifp, "%d", &i);
      if (i > 1) filters = 0;
    }
    if (!strcmp(data,"CaptProf_raw_data_rotation") && filters) {
      fscanf (ifp, "%d", &i);
      filters = bayer[i/90];
    }
    if (!strcmp(data,"NeutObj_neutrals")) {
      for (i=0; i < 4; i++)
	fscanf (ifp, "%d", neut+i);
      FORC3 cam_mul[c] = 1.0 / neut[c+1];
    }
    parse_mos (from);
    fseek (ifp, skip+from, SEEK_SET);
  }
}

void CLASS parse_phase_one (int base)
{
  unsigned entries, tag, type, len, data, save, i, c;
  char *cp;

  fseek (ifp, base, SEEK_SET);
  order = get4() & 0xffff;
  if (get4() >> 8 != 0x526177) return;		/* "Raw" */
  fseek (ifp, base+get4(), SEEK_SET);
  entries = get4();
  get4();
  while (entries--) {
    tag  = get4();
    type = get4();
    len  = get4();
    data = get4();
    save = ftell(ifp);
    fseek (ifp, base+data, SEEK_SET);
    switch (tag) {
      case 0x106:
	for (raw_color = i=0; i < 3; i++)
	  FORC3 rgb_cam[i][c] = int_to_float(get4());
	break;
      case 0x107:
	FORC3 cam_mul[c] = pre_mul[c] = int_to_float(get4());
	break;
      case 0x108:  raw_width   = data;  break;
      case 0x109:  raw_height  = data;  break;
      case 0x10a:  left_margin = data;  break;
      case 0x10b:  top_margin  = data;  break;
      case 0x10c:  width       = data;  break;
      case 0x10d:  height      = data;  break;
      case 0x10e:  tiff_data_compression = data;  break;
      case 0x10f:  data_offset = data+base;  break;
      case 0x112:
	nikon_curve_offset = save - 4;  break;
      case 0x301:
	fread (model, 64, 1, ifp);
	cp = strstr(model," camera");
	if (cp && cp < model+64) *cp = 0;
    }
    fseek (ifp, save, SEEK_SET);
  }
  load_raw = tiff_data_compression < 3 ?
	phase_one_load_raw:phase_one_load_raw_c;
  strcpy (make, "Phase One");
  if (model[0]) return;
  sprintf (model, "%dx%d", width, height);
  switch (raw_height) {
    case 2060: strcpy (model,"LightPhase");	break;
    case 2682: strcpy (model,"H 10");		break;
    case 4128: strcpy (model,"H 20");		break;
    case 5488: strcpy (model,"H 25");		break;
  }
}

void CLASS parse_fuji (int offset)
{
  int entries, tag, len, save, c;

  fseek (ifp, offset, SEEK_SET);
  entries = get4();
  if (entries > 255) return;
  while (entries--) {
    tag = get2();
    len = get2();
    save = ftell(ifp);
    if (tag == 0x100) {
      raw_height = get2();
      raw_width  = get2();
    } else if (tag == 0x121) {
      height = get2();
      if ((width = get2()) == 4284) width += 3;
    } else if (tag == 0x130)
      fuji_layout = fgetc(ifp) >> 7;
    if (tag == 0x2ff0)
      FORC4 cam_mul[c ^ 1] = get2();
    fseek (ifp, save+len, SEEK_SET);
  }
  if (fuji_layout) {
    height *= 2;
    width  /= 2;
  }
}

int CLASS parse_jpeg (int offset)
{
  int len, save, hlen;

  fseek (ifp, offset, SEEK_SET);
  if (fgetc(ifp) != 0xff || fgetc(ifp) != 0xd8) return 0;

  while (fgetc(ifp) == 0xff && fgetc(ifp) >> 4 != 0xd) {
    order = 0x4d4d;
    len   = get2() - 2;
    save  = ftell(ifp);
    order = get2();
    hlen  = get4();
    if (get4() == 0x48454150)		/* "HEAP" */
      parse_ciff (save+hlen, len-hlen);
    parse_tiff (save+6);
    fseek (ifp, save+len, SEEK_SET);
  }
  return 1;
}

void CLASS parse_riff()
{
  unsigned i, size, end;
  char tag[4], date[64], month[64];
  static const char mon[12][4] =
  { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
  struct tm t;

  order = 0x4949;
  fread (tag, 4, 1, ifp);
  size = get4();
  if (!memcmp(tag,"RIFF",4) || !memcmp(tag,"LIST",4)) {
    end = ftell(ifp) + size;
    get4();
    while (ftell(ifp) < end)
      parse_riff();
  } else if (!memcmp(tag,"IDIT",4) && size < 64) {
    fread (date, 64, 1, ifp);
    date[size] = 0;
    if (sscanf (date, "%*s %s %d %d:%d:%d %d", month, &t.tm_mday,
	&t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year) == 6) {
      for (i=0; i < 12 && strcmp(mon[i],month); i++);
      t.tm_mon = i;
      t.tm_year -= 1900;
      if (mktime(&t) > 0)
	timestamp = mktime(&t);
    }
  } else
    fseek (ifp, size, SEEK_CUR);
}

void CLASS parse_smal (int offset, int fsize)
{
  int ver;

  fseek (ifp, offset+2, SEEK_SET);
  order = 0x4949;
  ver = fgetc(ifp);
  if (ver == 6)
    fseek (ifp, 5, SEEK_CUR);
  if (get4() != fsize) return;
  if (ver > 6) data_offset = get4();
  raw_height = height = get2();
  raw_width  = width  = get2();
  strcpy (make, "SMaL");
  sprintf (model, "v%d %dx%d", ver, width, height);
  if (ver == 6) load_raw = smal_v6_load_raw;
  if (ver == 9) load_raw = smal_v9_load_raw;
}

char * CLASS foveon_gets (int offset, char *str, int len)
{
  int i;
  fseek (ifp, offset, SEEK_SET);
  for (i=0; i < len-1; i++)
    if ((str[i] = get2()) == 0) break;
  str[i] = 0;
  return str;
}

void CLASS parse_foveon()
{
  int entries, off, len, tag, save, i, wide, high, pent, poff[256][2];
  char name[64];

  order = 0x4949;			/* Little-endian */
  fseek (ifp, 36, SEEK_SET);
  flip = get4();
  fseek (ifp, -4, SEEK_END);
  fseek (ifp, get4(), SEEK_SET);
  if (get4() != 0x64434553) return;	/* SECd */
  get4();
  entries = get4();
  while (entries--) {
    off = get4();
    len = get4();
    tag = get4();
    save = ftell(ifp);
    fseek (ifp, off, SEEK_SET);
    if (get4() != (0x20434553 | (tag << 24))) return;
    switch (tag) {
      case 0x47414d49:			/* IMAG */
      case 0x32414d49:			/* IMA2 */
	fseek (ifp, 12, SEEK_CUR);
	wide = get4();
	high = get4();
	if (wide > raw_width && high > raw_height) {
	  raw_width  = wide;
	  raw_height = high;
	  data_offset = off + 24;
	}
	break;
      case 0x464d4143:			/* CAMF */
	meta_offset = off + 24;
	meta_length = len - 28;
	if (meta_length > 0x20000)
	    meta_length = 0x20000;
	break;
      case 0x504f5250:			/* PROP */
	get4();
	pent = get4();
	fseek (ifp, 12, SEEK_CUR);
	off += pent*8 + 24;
	if (pent > 256) pent=256;
	for (i=0; i < pent*2; i++)
	  poff[0][i] = off + get4()*2;
	for (i=0; i < pent; i++) {
	  foveon_gets (poff[i][0], name, 64);
	  if (!strcmp (name, "CAMMANUF"))
	    foveon_gets (poff[i][1], make, 64);
	  if (!strcmp (name, "CAMMODEL"))
	    foveon_gets (poff[i][1], model, 64);
	  if (!strcmp (name, "WB_DESC"))
	    foveon_gets (poff[i][1], model2, 64);
	  if (!strcmp (name, "TIME"))
	    timestamp = atoi (foveon_gets (poff[i][1], name, 64));
	}
#ifdef LOCALTIME
	timestamp = mktime (gmtime (&timestamp));
#endif
    }
    fseek (ifp, save, SEEK_SET);
  }
  is_foveon = 1;
}

/*
   Thanks to Adobe for providing these excellent CAM -> XYZ matrices!
 */
void CLASS adobe_coeff()
{
  static const struct {
    const char *prefix;
    short trans[12];
  } table[] = {
    { "Canon EOS D2000",
	{ 24542,-10860,-3401,-1490,11370,-297,2858,-605,3225 } },
    { "Canon EOS D6000",
	{ 20482,-7172,-3125,-1033,10410,-285,2542,226,3136 } },
    { "Canon EOS D30",
	{ 9805,-2689,-1312,-5803,13064,3068,-2438,3075,8775 } },
    { "Canon EOS D60",
	{ 6188,-1341,-890,-7168,14489,2937,-2640,3228,8483 } },
    { "Canon EOS 5D",
	{ 6228,-404,-967,-8314,16108,2312,-1923,2179,7499 } },
    { "Canon EOS 20D",
	{ 6599,-537,-891,-8071,15783,2424,-1983,2234,7462 } },
    { "Canon EOS 350D",
	{ 6018,-617,-965,-8645,15881,2975,-1530,1719,7642 } },
    { "Canon EOS DIGITAL REBEL XT",
	{ 6018,-617,-965,-8645,15881,2975,-1530,1719,7642 } },
    { "Canon EOS-1Ds Mark II",
	{ 6517,-602,-867,-8180,15926,2378,-1618,1771,7633 } },
    { "Canon EOS-1D Mark II",
	{ 6264,-582,-724,-8312,15948,2504,-1744,1919,8664 } },
    { "Canon EOS-1DS",
	{ 4374,3631,-1743,-7520,15212,2472,-2892,3632,8161 } },
    { "Canon EOS-1D",
	{ 6806,-179,-1020,-8097,16415,1687,-3267,4236,7690 } },
    { "Canon EOS",
	{ 8197,-2000,-1118,-6714,14335,2592,-2536,3178,8266 } },
    { "Canon PowerShot A50",
	{ -5300,9846,1776,3436,684,3939,-5540,9879,6200,-1404,11175,217 } },
    { "Canon PowerShot A5",
	{ -4801,9475,1952,2926,1611,4094,-5259,10164,5947,-1554,10883,547 } },
    { "Canon PowerShot G1",
	{ -4778,9467,2172,4743,-1141,4344,-5146,9908,6077,-1566,11051,557 } },
    { "Canon PowerShot G2",
	{ 9087,-2693,-1049,-6715,14382,2537,-2291,2819,7790 } },
    { "Canon PowerShot G3",
	{ 9212,-2781,-1073,-6573,14189,2605,-2300,2844,7664 } },
    { "Canon PowerShot G5",
	{ 9757,-2872,-933,-5972,13861,2301,-1622,2328,7212 } },
    { "Canon PowerShot G6",
	{ 9877,-3775,-871,-7613,14807,3072,-1448,1305,7485 } },
    { "Canon PowerShot Pro1",
	{ 10062,-3522,-999,-7643,15117,2730,-765,817,7323 } },
    { "Canon PowerShot Pro70",
	{ -4155,9818,1529,3939,-25,4522,-5521,9870,6610,-2238,10873,1342 } },
    { "Canon PowerShot Pro90",
	{ -4963,9896,2235,4642,-987,4294,-5162,10011,5859,-1770,11230,577 } },
    { "Canon PowerShot S30",
	{ 10566,-3652,-1129,-6552,14662,2006,-2197,2581,7670 } },
    { "Canon PowerShot S40",
	{ 8510,-2487,-940,-6869,14231,2900,-2318,2829,9013 } },
    { "Canon PowerShot S45",
	{ 8163,-2333,-955,-6682,14174,2751,-2077,2597,8041 } },
    { "Canon PowerShot S50",
	{ 8882,-2571,-863,-6348,14234,2288,-1516,2172,6569 } },
    { "Canon PowerShot S60",
	{ 8795,-2482,-797,-7804,15403,2573,-1422,1996,7082 } },
    { "Canon PowerShot S70",
	{ 9976,-3810,-832,-7115,14463,2906,-901,989,7889 } },
    { "Contax N Digital",
	{ 7777,1285,-1053,-9280,16543,2916,-3677,5679,7060 } },
    { "EPSON R-D1",
	{ 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { "FUJIFILM FinePix E550",
	{ 11044,-3888,-1120,-7248,15168,2208,-1531,2277,8069 } },
    { "FUJIFILM FinePix F8",
	{ 11044,-3888,-1120,-7248,15168,2208,-1531,2277,8069 } },
    { "FUJIFILM FinePix F7",
	{ 10004,-3219,-1201,-7036,15047,2107,-1863,2565,7736 } },
    { "FUJIFILM FinePix S20Pro",
	{ 10004,-3219,-1201,-7036,15047,2107,-1863,2565,7736 } },
    { "FUJIFILM FinePix S2Pro",
	{ 12492,-4690,-1402,-7033,15423,1647,-1507,2111,7697 } },
    { "FUJIFILM FinePix S3Pro",
	{ 11807,-4612,-1294,-8927,16968,1988,-2120,2741,8006 } },
    { "FUJIFILM FinePix S5000",
	{ 8754,-2732,-1019,-7204,15069,2276,-1702,2334,6982 } },
    { "FUJIFILM FinePix S5100",
	{ 11940,-4431,-1255,-6766,14428,2542,-993,1165,7421 } },
    { "FUJIFILM FinePix S7000",
	{ 10190,-3506,-1312,-7153,15051,2238,-2003,2399,7505 } },
    { "FUJIFILM FinePix S9",	/* copied from S7000 */
	{ 10190,-3506,-1312,-7153,15051,2238,-2003,2399,7505 } },
    { "KODAK NC2000F",		/* DJC */
	{ 16475,-6903,-1218,-851,10375,477,2505,-7,1020 } },
    { "Kodak DCS315C",
	{ 17523,-4827,-2510,756,8546,-137,6113,1649,2250 } },
    { "Kodak DCS330C",
	{ 20620,-7572,-2801,-103,10073,-396,3551,-233,2220 } },
    { "KODAK DCS420",
	{ 10868,-1852,-644,-1537,11083,484,2343,628,2216 } },
    { "KODAK DCS460",
	{ 10592,-2206,-967,-1944,11685,230,2206,670,1273 } },
    { "KODAK EOSDCS1",
	{ 10592,-2206,-967,-1944,11685,230,2206,670,1273 } },
    { "KODAK EOSDCS3B",
	{ 9898,-2700,-940,-2478,12219,206,1985,634,1031 } },
    { "Kodak DCS520C",
	{ 24542,-10860,-3401,-1490,11370,-297,2858,-605,3225 } },
    { "Kodak DCS560C",
	{ 20482,-7172,-3125,-1033,10410,-285,2542,226,3136 } },
    { "Kodak DCS620C",
	{ 23617,-10175,-3149,-2054,11749,-272,2586,-489,3453 } },
    { "Kodak DCS620X",
	{ 13095,-6231,154,12221,-21,-2137,895,4602,2258 } },
    { "Kodak DCS660C",
	{ 18244,-6351,-2739,-791,11193,-521,3711,-129,2802 } },
    { "Kodak DCS720X",
	{ 11775,-5884,950,9556,1846,-1286,-1019,6221,2728 } },
    { "Kodak DCS760C",
	{ 16623,-6309,-1411,-4344,13923,323,2285,274,2926 } },
    { "Kodak DCS Pro SLR",
	{ 5494,2393,-232,-6427,13850,2846,-1876,3997,5445 } },
    { "Kodak DCS Pro 14nx",
	{ 5494,2393,-232,-6427,13850,2846,-1876,3997,5445 } },
    { "Kodak DCS Pro 14",
	{ 7791,3128,-776,-8588,16458,2039,-2455,4006,6198 } },
    { "Kodak ProBack645",
	{ 16414,-6060,-1470,-3555,13037,473,2545,122,4948 } },
    { "Kodak ProBack",
	{ 21179,-8316,-2918,-915,11019,-165,3477,-180,4210 } },
    { "LEICA DIGILUX 2",
	{ 11340,-4069,-1275,-7555,15266,2448,-2960,3426,7685 } },
    { "Leaf Valeo",
	{ 8236,1746,-1314,-8251,15953,2428,-3673,5786,5771 } },
    { "Minolta DiMAGE 5",
	{ 8983,-2942,-963,-6556,14476,2237,-2426,2887,8014 } },
    { "Minolta DiMAGE 7Hi",
	{ 11368,-3894,-1242,-6521,14358,2339,-2475,3056,7285 } },
    { "Minolta DiMAGE 7",
	{ 9144,-2777,-998,-6676,14556,2281,-2470,3019,7744 } },
    { "Minolta DiMAGE A1",
	{ 9274,-2547,-1167,-8220,16323,1943,-2273,2720,8340 } },
    { "MINOLTA DiMAGE A200",
	{ 8560,-2487,-986,-8112,15535,2771,-1209,1324,7743 } },
    { "Minolta DiMAGE A2",
	{ 9097,-2726,-1053,-8073,15506,2762,-966,981,7763 } },
    { "Minolta DiMAGE Z2",
	{ 11428,-3512,-1706,-5895,13815,2266,-2382,2719,5651 } },
    { "MINOLTA DYNAX 5",
	{ 10284,-3283,-1086,-7957,15762,2316,-829,882,6644 } },
    { "MINOLTA DYNAX 7",
	{ 10239,-3104,-1099,-8037,15727,2451,-927,925,6871 } },
    { "NIKON D100",
	{ 5902,-933,-782,-8983,16719,2354,-1402,1455,6464 } },
    { "NIKON D1H",
	{ 7577,-2166,-926,-7454,15592,1934,-2377,2808,8606 } },
    { "NIKON D1X",
	{ 7702,-2245,-975,-9114,17242,1875,-2679,3055,8521 } },
    { "NIKON D1",
	{ 7559,-2130,-965,-7611,15713,1972,-2478,3042,8290 } },
    { "NIKON D2H",
	{ 5710,-901,-615,-8594,16617,2024,-2975,4120,6830 } },
    { "NIKON D2X",
	{ 10231,-2769,-1255,-8301,15900,2552,-797,680,7148 } },
    { "NIKON D50",
	{ 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
    { "NIKON D70",
	{ 7732,-2422,-789,-8238,15884,2498,-859,783,7330 } },
    { "NIKON E995",	/* copied from E5000 */
	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
    { "NIKON E2500",
	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
    { "NIKON E4500",
	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
    { "NIKON E5000",
	{ -5547,11762,2189,5814,-558,3342,-4924,9840,5949,688,9083,96 } },
    { "NIKON E5400",
	{ 9349,-2987,-1001,-7919,15766,2266,-2098,2680,6839 } },
    { "NIKON E5700",
	{ -5368,11478,2368,5537,-113,3148,-4969,10021,5782,778,9028,211 } },
    { "NIKON E8400",
	{ 7842,-2320,-992,-8154,15718,2599,-1098,1342,7560 } },
    { "NIKON E8700",
	{ 8489,-2583,-1036,-8051,15583,2643,-1307,1407,7354 } },
    { "NIKON E8800",
	{ 7971,-2314,-913,-8451,15762,2894,-1442,1520,7610 } },
    { "OLYMPUS C5050",
	{ 10508,-3124,-1273,-6079,14294,1901,-1653,2306,6237 } },
    { "OLYMPUS C5060",
	{ 10445,-3362,-1307,-7662,15690,2058,-1135,1176,7602 } },
    { "OLYMPUS C70",
	{ 10793,-3791,-1146,-7498,15177,2488,-1390,1577,7321 } },
    { "OLYMPUS C80",
	{ 8606,-2509,-1014,-8238,15714,2703,-942,979,7760 } },
    { "OLYMPUS E-10",
	{ 12745,-4500,-1416,-6062,14542,1580,-1934,2256,6603 } },
    { "OLYMPUS E-1",
	{ 11846,-4767,-945,-7027,15878,1089,-2699,4122,8311 } },
    { "OLYMPUS E-20",
	{ 13173,-4732,-1499,-5807,14036,1895,-2045,2452,7142 } },
    { "OLYMPUS E-300",
	{ 7828,-1761,-348,-5788,14071,1830,-2853,4518,6557 } },
    { "OLYMPUS E-500",	/* copied from E-300 */
	{ 7828,-1761,-348,-5788,14071,1830,-2853,4518,6557 } },
    { "PENTAX *ist DS",
	{ 10371,-2333,-1206,-8688,16231,2602,-1230,1116,11282 } },
    { "PENTAX *ist D",
	{ 9651,-2059,-1189,-8881,16512,2487,-1460,1345,10687 } },
    { "Panasonic DMC-FZ30",
	{ 10976,-4029,-1141,-7918,15491,2600,-1670,2071,8246 } },
    { "Panasonic DMC-LC1",
	{ 11340,-4069,-1275,-7555,15266,2448,-2960,3426,7685 } },
    { "Panasonic DMC-LX1",
	{ 10704,-4187,-1230,-8314,15952,2501,-920,945,8927 } },
    { "SONY DSC-F828",
	{ 7924,-1910,-777,-8226,15459,2998,-1517,2199,6818,-7242,11401,3481 } },
    { "SONY DSC-R1",		/* DJC */
	{ 10528,-3695,-517,-2822,10699,2124,406,1240,5342 } },
    { "SONY DSC-V3",
	{ 7511,-2571,-692,-7894,15088,3060,-948,1111,8128 } }
  };
  double cam_xyz[4][3];
  char name[130];
  int i, j;

  sprintf (name, "%s %s", make, model);
  for (i=0; i < sizeof table / sizeof *table; i++)
    if (!strncmp (name, table[i].prefix, strlen(table[i].prefix))) {
      for (j=0; j < 12; j++)
	cam_xyz[0][j] = table[i].trans[j];
      cam_xyz_coeff (cam_xyz);
      break;
    }
}

void CLASS simple_coeff (int index)
{
  static const float table[][12] = {
  /* index 0 -- all Foveon cameras */
  { 1.4032,-0.2231,-0.1016,-0.5263,1.4816,0.017,-0.0112,0.0183,0.9113 },
  /* index 1 -- Kodak DC20 and DC25 */
  { 2.25,0.75,-1.75,-0.25,-0.25,0.75,0.75,-0.25,-0.25,-1.75,0.75,2.25 },
  /* index 2 -- Logitech Fotoman Pixtura */
  { 1.893,-0.418,-0.476,-0.495,1.773,-0.278,-1.017,-0.655,2.672 },
  /* index 3 -- Nikon E700, E800, and E950 */
  { -1.936280,  1.800443, -1.448486,  2.584324,
     1.405365, -0.524955, -0.289090,  0.408680,
    -1.204965,  1.082304,  2.941367, -1.818705 }
  };
  int i, c;

  for (raw_color = i=0; i < 3; i++)
    FORCC rgb_cam[i][c] = table[index][i*colors+c];
}

short CLASS guess_byte_order (int words)
{
  uchar test[4][2];
  int t=2, msb;
  double diff, sum[2] = {0,0};

  fread (test[0], 2, 2, ifp);
  for (words-=2; words--; ) {
    fread (test[t], 2, 1, ifp);
    for (msb=0; msb < 2; msb++) {
      diff = (test[t^2][msb] << 8 | test[t^2][!msb])
	   - (test[t  ][msb] << 8 | test[t  ][!msb]);
      sum[msb] += diff*diff;
    }
    t = (t+1) & 3;
  }
  return sum[0] < sum[1] ? 0x4d4d : 0x4949;
}

/*
   Identify which camera created this file, and set global variables
   accordingly.  Return nonzero if the file cannot be decoded.
 */
int CLASS identify (int no_decode)
{
  char head[32], *cp;
  unsigned hlen, fsize, i, c, is_jpeg=0, is_canon;
  static const struct {
    int fsize;
    char make[12], model[15], withjpeg;
  } table[] = {
    {    62464, "Kodak",    "DC20"       ,0 },
    {   124928, "Kodak",    "DC20"       ,0 },
    {   311696, "ST Micro", "STV680 VGA" ,0 },  /* SPYz */
    {   787456, "Creative", "PC-CAM 600" ,0 },
    {  1138688, "Minolta",  "RD175"      ,0 },
    {  3840000, "Foculus",  "531C"       ,0 },
    {  1920000, "AVT",      "F-201C"     ,0 },
    {  5067304, "AVT",      "F-510C"     ,0 },
    { 10134608, "AVT",      "F-510C"     ,0 },
    { 16157136, "AVT",      "F-810C"     ,0 },
    {  6624000, "Pixelink", "A782"       ,0 },
    { 13248000, "Pixelink", "A782"       ,0 },
    {  6291456, "RoverShot","3320AF"     ,0 },
    {  5939200, "OLYMPUS",  "C770UZ"     ,0 },
    {  1581060, "NIKON",    "E900"       ,1 },  /* or E900s,E910 */
    {  2465792, "NIKON",    "E950"       ,1 },  /* or E800,E700 */
    {  2940928, "NIKON",    "E2100"      ,1 },  /* or E2500 */
    {  4771840, "NIKON",    "E990"       ,1 },  /* or E995 */
    {  4775936, "NIKON",    "E3700"      ,1 },  /* or Optio 33WR */
    {  5869568, "NIKON",    "E4300"      ,1 },  /* or DiMAGE Z2 */
    {  5865472, "NIKON",    "E4500"      ,1 },
    {  7438336, "NIKON",    "E5000"      ,1 },  /* or E5700 */
    {  1976352, "CASIO",    "QV-2000UX"  ,1 },
    {  3217760, "CASIO",    "QV-3*00EX"  ,1 },
    {  6218368, "CASIO",    "QV-5700"    ,1 },
    {  7530816, "CASIO",    "QV-R51"     ,1 },
    {  7684000, "CASIO",    "QV-4000"    ,1 },
    {  4948608, "CASIO",    "EX-S100"    ,1 },
    {  7542528, "CASIO",    "EX-Z50"     ,1 },
    {  7753344, "CASIO",    "EX-Z55"     ,1 },
    {  7426656, "CASIO",    "EX-P505"    ,1 },
    {  9313536, "CASIO",    "EX-P600"    ,1 },
    { 10979200, "CASIO",    "EX-P700"    ,1 },
    {  3178560, "PENTAX",   "Optio S"    ,1 },  /*  8-bit */
    {  4841984, "PENTAX",   "Optio S"    ,1 },  /* 12-bit */
    {  6114240, "PENTAX",   "Optio S4"   ,1 },  /* or S4i */
    { 12582980, "Sinar",    ""           ,0 } };
  static const char *corp[] =
    { "Canon", "NIKON", "EPSON", "KODAK", "Kodak", "OLYMPUS", "PENTAX",
      "MINOLTA", "Minolta", "Konica", "CASIO", "Sinar" };

/*  What format is this file?  Set make[] if we recognize it. */

  load_raw = NULL;
  raw_height = raw_width = fuji_width = flip = 0;
  height = width = top_margin = left_margin = 0;
  make[0] = model[0] = model2[0] = 0;
  memset (white, 0, sizeof white);
  data_offset = meta_length = tiff_bps = tiff_data_compression = 0;
  kodak_cbpp = zero_after_ff = dng_version = fuji_secondary = 0;
  timestamp = shot_order = tiff_samples = black = is_foveon = 0;
  raw_color = use_gamma = xmag = ymag = 1;
  filters = UINT_MAX;	/* 0 = no filters, UINT_MAX = unknown */
  for (i=0; i < 4; i++) {
    cam_mul[i] = i == 1;
    pre_mul[i] = i < 3;
    FORC3 rgb_cam[c][i] = c == i;
  }
  colors = 3;
  for (i=0; i < 0x1000; i++) curve[i] = i;
  maximum = 0xfff;
#ifdef USE_LCMS
  profile_length = 0;
#endif

  order = get2();
  hlen = get4();
  fseek (ifp, 0, SEEK_SET);
  fread (head, 1, 32, ifp);
  fseek (ifp, 0, SEEK_END);
  fsize = ftell(ifp);
  if ((cp = memmem (head, 32, "MMMM", 4)) ||
      (cp = memmem (head, 32, "IIII", 4)))
    parse_phase_one (cp-head);
  else if (order == 0x4949 || order == 0x4d4d) {
    if (!memcmp (head+6,"HEAPCCDR",8)) {
      data_offset = hlen;
      parse_ciff (hlen, fsize - hlen);
    } else {
      parse_tiff(0);
      if (!dng_version && !strncmp(make,"NIKON",5) && filters == UINT_MAX)
	make[0] = 0;
    }
  } else if (!memcmp (head,"\xff\xd8\xff\xe1",4) &&
	     !memcmp (head+6,"Exif",4)) {
    fseek (ifp, 4, SEEK_SET);
    fseek (ifp, 4 + get2(), SEEK_SET);
    if (fgetc(ifp) != 0xff)
      parse_tiff(12);
  } else if (!memcmp (head,"BM",2) &&
	head[26] == 1 && head[28] == 16 && head[30] == 0) {
    data_offset = 0x1000;
    order = 0x4949;
    fseek (ifp, 38, SEEK_SET);
    if (get4() == 2834 && get4() == 2834 && get4() == 0 && get4() == 4096) {
      strcpy (model, "BMQ");
      flip = 3;
      goto nucore;
    }
  } else if (!memcmp (head,"BR",2)) {
    strcpy (model, "RAW");
nucore:
    strcpy (make, "Nucore");
    order = 0x4949;
    fseek (ifp, 10, SEEK_SET);
    data_offset += get4();
    get4();
    raw_width  = get4();
    raw_height = get4();
    if (model[0] == 'B' && raw_width == 2597) {
      raw_width++;
      data_offset -= 0x1000;
    }
  } else if (!memcmp (head+25,"ARECOYK",7)) {
    strcpy (make, "Contax");
    strcpy (model,"N Digital");
    fseek (ifp, 33, SEEK_SET);
    get_timestamp(1);
    fseek (ifp, 60, SEEK_SET);
    FORC4 cam_mul[c ^ (c >> 1)] = get4();
  } else if (!strcmp (head, "PXN")) {
    strcpy (make, "Logitech");
    strcpy (model,"Fotoman Pixtura");
  } else if (!memcmp (head,"FUJIFILM",8)) {
    fseek (ifp, 92, SEEK_SET);
    parse_fuji (get4());
    fseek (ifp, 84, SEEK_SET);
    if ((hlen = get4()) > 120) {
      fseek (ifp, 120, SEEK_SET);
      fuji_secondary = (i = get4()) && 1;
      if (fuji_secondary && use_secondary)
	parse_fuji (i);
    }
    fseek (ifp, 100, SEEK_SET);
    i = get4();
    parse_tiff (hlen+12);
    data_offset = i;
  } else if (!memcmp (head,"RIFF",4)) {
    fseek (ifp, 0, SEEK_SET);
    parse_riff();
  } else if (!memcmp (head,"DSC-Image",9))
    parse_rollei();
  else if (!memcmp (head,"\0MRM",4))
    parse_minolta();
  else if (!memcmp (head,"FOVb",4))
    parse_foveon();
  else
    for (i=0; i < sizeof table / sizeof *table; i++)
      if (fsize == table[i].fsize) {
	strcpy (make,  table[i].make );
	strcpy (model, table[i].model);
	if (table[i].withjpeg)
	  parse_external_jpeg();
      }
  parse_mos(8);
  parse_mos(3472);
  if (make[0] == 0) parse_smal (0, fsize);
  if (make[0] == 0) is_jpeg = parse_jpeg(0);
  if (no_decode) return !timestamp;

  for (i=0; i < sizeof corp / sizeof *corp; i++)
    if (strstr (make, corp[i]))		/* Simplify company names */
	strcpy (make, corp[i]);
  if (!strncmp (make,"KODAK",5))
    make[16] = model[16] = 0;
  cp = make + strlen(make);		/* Remove trailing spaces */
  while (*--cp == ' ') *cp = 0;
  cp = model + strlen(model);
  while (*--cp == ' ') *cp = 0;
  i = strlen(make);			/* Remove make from model */
  if (!strncmp (model, make, i) && model[i++] == ' ')
    memmove (model, model+i, 64-i);
  make[63] = model[63] = model2[63] = 0;

  if (make[0] == 0) {
    fprintf (stderr, "%s: unsupported file format.\n", ifname);
    return 1;
  }
  if ((raw_height | raw_width) < 0)
       raw_height = raw_width  = 0;
  if (!height) height = raw_height;
  if (!width)  width  = raw_width;
  if (fuji_width) {
    width = height + fuji_width;
    height = width - 1;
    xmag = ymag = 1;
  }
  if (dng_version) {
    strcat (model," DNG");
    if (filters == UINT_MAX) filters = 0;
    if (!filters)
      colors = tiff_samples;
    if (tiff_data_compression == 1)
      load_raw = adobe_dng_load_raw_nc;
    if (tiff_data_compression == 7)
      load_raw = adobe_dng_load_raw_lj;
    FORC4 cam_mul[c] = pre_mul[c];
    goto dng_skip;
  }

/*  We'll try to decode anything from Canon or Nikon. */

  if (filters == UINT_MAX) filters = 0x94949494;
  if ((is_canon = !strcmp(make,"Canon"))) {
    load_raw = memcmp (head+6,"HEAPCCDR",8) ?
	lossless_jpeg_load_raw : canon_compressed_load_raw;
    maximum = 0xfff;
  }
  if (!strcmp(make,"NIKON"))
    load_raw = nikon_is_compressed() ?
	nikon_compressed_load_raw : nikon_load_raw;
  if (!strncmp (make,"OLYMPUS",7))
    height += height & 1;

/* Set parameters based on camera name (for non-DNG files). */

  if (is_foveon) {
    if (height*2 < width) ymag = 2;
    if (height   > width) xmag = 2;
    filters = 0;
    load_raw = foveon_load_raw;
    simple_coeff(0);
  } else if (!strcmp(model,"PowerShot 600")) {
    height = 613;
    width  = 854;
    colors = 4;
    filters = 0xe1e4e1e4;
    load_raw = canon_600_load_raw;
  } else if (!strcmp(model,"PowerShot A5") ||
	     !strcmp(model,"PowerShot A5 Zoom")) {
    height = 773;
    width  = 960;
    raw_width = 992;
    colors = 4;
    filters = 0x1e4e1e4e;
    load_raw = canon_a5_load_raw;
  } else if (!strcmp(model,"PowerShot A50")) {
    height =  968;
    width  = 1290;
    raw_width = 1320;
    colors = 4;
    filters = 0x1b4e4b1e;
    load_raw = canon_a5_load_raw;
  } else if (!strcmp(model,"PowerShot Pro70")) {
    height = 1024;
    width  = 1552;
    colors = 4;
    filters = 0x1e4b4e1b;
    load_raw = canon_a5_load_raw;
    black = 34;
  } else if (!strcmp(model,"PowerShot Pro90 IS")) {
    width  = 1896;
    colors = 4;
    filters = 0xb4b4b4b4;
  } else if (is_canon && raw_width == 2144) {
    height = 1550;
    width  = 2088;
    top_margin  = 8;
    left_margin = 4;
    if (!strcmp(model,"PowerShot G1")) {
      colors = 4;
      filters = 0xb4b4b4b4;
    }
  } else if (is_canon && raw_width == 2224) {
    height = 1448;
    width  = 2176;
    top_margin  = 6;
    left_margin = 48;
  } else if (is_canon && raw_width == 2376) {
    height = 1720;
    width  = 2312;
    top_margin  = 6;
    left_margin = 12;
  } else if (is_canon && raw_width == 2672) {
    height = 1960;
    width  = 2616;
    top_margin  = 6;
    left_margin = 12;
  } else if (is_canon && raw_width == 3152) {
    height = 2056;
    width  = 3088;
    top_margin  = 12;
    left_margin = 64;
    maximum = 0xfa0;
  } else if (is_canon && raw_width == 3160) {
    height = 2328;
    width  = 3112;
    top_margin  = 12;
    left_margin = 44;
  } else if (is_canon && raw_width == 3344) {
    height = 2472;
    width  = 3288;
    top_margin  = 6;
    left_margin = 4;
  } else if (!strcmp(model,"EOS D2000C")) {
    filters = 0x61616161;
    black = curve[200];
  } else if (!strcmp(model,"EOS-1D")) {
    raw_height = height = 1662;
    raw_width  = width  = 2496;
    data_offset = 288912;
    filters = 0x61616161;
  } else if (!strcmp(model,"EOS-1DS")) {
    raw_height = height = 2718;
    raw_width  = width  = 4082;
    data_offset = 289168;
    filters = 0x61616161;
  } else if (is_canon && raw_width == 3516) {
    top_margin  = 14;
    left_margin = 42;
    goto canon_cr2;
  } else if (is_canon && raw_width == 3596) {
    top_margin  = 12;
    left_margin = 74;
    goto canon_cr2;
  } else if (is_canon && raw_width == 4476) {
    top_margin  = 34;
    left_margin = 90;
    goto canon_cr2;
  } else if (is_canon && raw_width == 5108) {
    top_margin  = 13;
    left_margin = 98;
    maximum = 0xe80;
canon_cr2:
    height = raw_height - top_margin;
    width  = raw_width - left_margin;
  } else if (!strcmp(model,"D1")) {
    camera_red  *= 256/527.0;
    camera_blue *= 256/317.0;
  } else if (!strcmp(model,"D1X")) {
    width  = 4024;
    ymag = 2;
  } else if (!strcmp(model,"D70")) {
    maximum = 0xf53;
  } else if (!strcmp(model,"D100")) {
    if (tiff_data_compression == 34713 && load_raw == nikon_load_raw)
      raw_width = (width += 3) + 3;
    maximum = 0xf44;
  } else if (!strcmp(model,"D2H")) {
    width  = 2482;
    left_margin = 6;
  } else if (!strcmp(model,"D2X")) {
    width  = 4312;
  } else if (fsize == 1581060) {
    height = 963;
    width = 1287;
    raw_width = 1632;
    load_raw = nikon_e900_load_raw;
    maximum = 0x3f4;
    colors = 4;
    filters = 0x1e1e1e1e;
    simple_coeff(3);
    pre_mul[0] = 1.2085;
    pre_mul[1] = 1.0943;
    pre_mul[3] = 1.1103;
  } else if (fsize == 2465792) {
    height = 1203;
    width  = 1616;
    raw_width = 2048;
    load_raw = nikon_e900_load_raw;
    maximum = 0x3dd;
    colors = 4;
    filters = 0x4b4b4b4b;
    simple_coeff(3);
    pre_mul[0] = 1.18193;
    pre_mul[2] = 1.16452;
    pre_mul[3] = 1.17250;
  } else if (!strcmp(model,"E880") ||
	     !strcmp(model,"E990")) {
    if (!timestamp && !nikon_e990()) goto cp_e995;
    height = 1540;
    width  = 2064;
    colors = 4;
    filters = 0xb4b4b4b4;
    simple_coeff(3);
    pre_mul[0] = 1.196;
    pre_mul[1] = 1.246;
    pre_mul[2] = 1.018;
  } else if (!strcmp(model,"E995")) {
cp_e995:
    strcpy (model, "E995");
    height = 1540;
    width  = 2064;
    colors = 4;
    filters = 0xe1e1e1e1;
  } else if (!strcmp(model,"E2100")) {
    if (!timestamp && !nikon_e2100()) goto cp_e2500;
    height = 1206;
    width  = 1616;
    load_raw = nikon_e2100_load_raw;
    pre_mul[0] = 1.945;
    pre_mul[2] = 1.040;
  } else if (!strcmp(model,"E2500")) {
cp_e2500:
    strcpy (model, "E2500");
    height = 1204;
    width  = 1616;
    colors = 4;
    filters = 0x4b4b4b4b;
  } else if (fsize == 4775936) {
    height = 1542;
    width  = 2064;
    load_raw = nikon_e2100_load_raw;
    pre_mul[0] = 1.818;
    pre_mul[2] = 1.618;
    if ((i = nikon_3700()) == 2) {
      strcpy (make, "OLYMPUS");
      strcpy (model, "C740UZ");
    } else if (i == 0) {
      strcpy (make, "PENTAX");
      strcpy (model,"Optio 33WR");
      flip = 1;
      filters = 0x16161616;
      pre_mul[0] = 1.331;
      pre_mul[2] = 1.820;
    }
  } else if (!strcmp(model,"E4300")) {
    if (!timestamp && minolta_z2()) goto dimage_z2;
    height = 1710;
    width  = 2288;
    filters = 0x16161616;
    pre_mul[0] = 508;
    pre_mul[1] = 256;
    pre_mul[2] = 322;
  } else if (!strcmp(model,"DiMAGE Z2")) {
dimage_z2:
    strcpy (make, "MINOLTA");
    strcpy (model,"DiMAGE Z2");
    height = 1710;
    width  = 2288;
    filters = 0x16161616;
    load_raw = nikon_e2100_load_raw;
    black = 68;
  } else if (!strcmp(model,"E4500")) {
    height = 1708;
    width  = 2288;
    colors = 4;
    filters = 0xb4b4b4b4;
  } else if (fsize == 7438336) {
    height = 1924;
    width  = 2576;
    colors = 4;
    filters = 0xb4b4b4b4;
  } else if (!strcmp(model,"R-D1")) {
    tiff_data_compression = 34713;
    load_raw = nikon_load_raw;
  } else if (!strcmp(model,"FinePix S5100") ||
	     !strcmp(model,"FinePix S5500")) {
    load_raw = unpacked_load_raw;
    maximum = 0xffff;
  } else if (!strncmp(model,"FinePix",7)) {
    if (!strcmp(model+7,"S2Pro")) {
      strcpy (model+7," S2Pro");
      height = 2144;
      width  = 2880;
      black = 128;
      flip = 6;
    } else
      maximum = 0x3e00;
    top_margin = (raw_height - height)/2;
    left_margin = (raw_width - width )/2;
    data_offset += (top_margin*raw_width + left_margin) * 2;
    if (fuji_secondary)
      data_offset += use_secondary * ( strcmp(model+7," S3Pro")
		? (raw_width *= 2) : raw_height*raw_width*2 );
    fuji_width = width >> !fuji_layout;
    width = (height >> fuji_layout) + fuji_width;
    raw_height = height;
    height = width - 1;
    load_raw = fuji_load_raw;
    if (!(fuji_width & 1)) filters = 0x49494949;
  } else if (!strcmp(model,"RD175")) {
    height = 986;
    width = 1534;
    data_offset = 513;
    filters = 0x61616161;
    load_raw = minolta_rd175_load_raw;
  } else if (!strcmp(model,"Digital Camera KD-400Z")) {
    height = 1712;
    width  = 2312;
    raw_width = 2336;
    data_offset = 4034;
    fseek (ifp, 2032, SEEK_SET);
    goto konica_400z;
  } else if (!strcmp(model,"Digital Camera KD-510Z")) {
    data_offset = 4032;
    pre_mul[0] = 1.297;
    pre_mul[2] = 1.438;
    fseek (ifp, 2032, SEEK_SET);
    goto konica_510z;
  } else if (!strcasecmp(make,"MINOLTA")) {
    load_raw = unpacked_load_raw;
    maximum = 0xf7d;
    if (!strncmp(model,"DiMAGE A",8)) {
      if (!strcmp(model,"DiMAGE A200"))
	filters = 0x49494949;
      load_raw = packed_12_load_raw;
      maximum = model[8] == '1' ? 0xf8b : 0xfff;
    } else if (!strncmp(model,"ALPHA",5) ||
	       !strncmp(model,"DYNAX",5) ||
	       !strncmp(model,"MAXXUM",6)) {
      sprintf (model, "DYNAX %s", model+6 + (model[0]=='M'));
      load_raw = packed_12_load_raw;
      maximum = 0xffb;
    } else if (!strncmp(model,"DiMAGE G",8)) {
      if (model[8] == '4') {
	data_offset = 5056;
	pre_mul[0] = 1.602;
	pre_mul[2] = 1.441;
	fseek (ifp, 2078, SEEK_SET);
	height = 1716;
	width  = 2304;
      } else if (model[8] == '5') {
	data_offset = 4016;
	fseek (ifp, 1936, SEEK_SET);
konica_510z:
	height = 1956;
	width  = 2607;
	raw_width = 2624;
      } else if (model[8] == '6') {
	data_offset = 4032;
	fseek (ifp, 2030, SEEK_SET);
	height = 2136;
	width  = 2848;
      }
      filters = 0x61616161;
konica_400z:
      load_raw = unpacked_load_raw;
      maximum = 0x3df;
      order = 0x4d4d;
      FORC4 cam_mul[(c >> 1) | ((c & 1) << 1)] = get2();
    }
    if (pre_mul[0] == 1 && pre_mul[2] == 1) {
      pre_mul[0] = 1.42;
      pre_mul[2] = 1.25;
    }
  } else if (!strncmp(model,"*ist D",6)) {
    load_raw = model[6] ? packed_12_load_raw : unpacked_load_raw;
    if (model[6] == 'S') height -= 2;
  } else if (!strcmp(model,"Optio S")) {
    if (fsize == 3178560) {
      height = 1540;
      width  = 2064;
      load_raw = eight_bit_load_raw;
      camera_red  *= 4;
      camera_blue *= 4;
      pre_mul[0] = 1.391;
      pre_mul[2] = 1.188;
    } else {
      height = 1544;
      width  = 2068;
      raw_width = 3136;
      load_raw = packed_12_load_raw;
      maximum = 0xf7c;
      pre_mul[0] = 1.137;
      pre_mul[2] = 1.453;
    }
  } else if (!strncmp(model,"Optio S4",8)) {
    height = 1737;
    width  = 2324;
    raw_width = 3520;
    load_raw = packed_12_load_raw;
    maximum = 0xf7a;
    pre_mul[0] = 1.980;
    pre_mul[2] = 1.570;
  } else if (!strcmp(model,"STV680 VGA")) {
    height = 484;
    width  = 644;
    load_raw = eight_bit_load_raw;
    flip = 2;
    filters = 0x16161616;
    black = 16;
    pre_mul[0] = 1.097;
    pre_mul[2] = 1.128;
  } else if (!strcmp(model,"531C")) {
    height = 1200;
    width  = 1600;
    load_raw = unpacked_load_raw;
    filters = 0x49494949;
    pre_mul[1] = 1.218;
  } else if (!strcmp(model,"F-201C")) {
    height = 1200;
    width  = 1600;
    load_raw = eight_bit_load_raw;
  } else if (!strcmp(model,"F-510C")) {
    height = 1958;
    width  = 2588;
    load_raw = (fsize < 7500000) ? eight_bit_load_raw : unpacked_load_raw;
    maximum = 0xfff0;
  } else if (!strcmp(model,"F-810C")) {
    height = 2469;
    width  = 3272;
    load_raw = unpacked_load_raw;
    maximum = 0xfff0;
  } else if (!strcmp(model,"A782")) {
    height = 3000;
    width  = 2208;
    filters = 0x61616161;
    load_raw = (fsize < 10000000) ? eight_bit_load_raw : unpacked_load_raw;
    maximum = 0xffc0;
  } else if (!strcmp(model,"3320AF")) {
    height = 1536;
    raw_width = width = 2048;
    filters = 0x61616161;
    load_raw = unpacked_load_raw;
    maximum = 0x3ff;
    pre_mul[0] = 1.717;
    pre_mul[2] = 1.138;
    fseek (ifp, 0x300000, SEEK_SET);
    if ((order = guess_byte_order(0x10000)) == 0x4d4d) {
      data_offset = (2048 * 16 + 28) * 2;
      height -= 16;
      width  -= 28;
      maximum = 0xf5c0;
      strcpy (make, "ISG");
      sprintf (model, "%dx%d", width, height);
    }
  } else if (!strcmp(make,"Imacon")) {
    height = raw_height - 6;
    width  = raw_width - 10;
    data_offset += 6 + raw_width*12;
    flip = height > width+10 ? 5:3;
    sprintf (model, "Ixpress %d-Mp", height*width/1000000);
    filters = 0x61616161;
    load_raw = unpacked_load_raw;
    maximum = 0xffff;
    pre_mul[0] = 1.963;
    pre_mul[2] = 1.430;
  } else if (!strcmp(make,"Sinar")) {
    if (!memcmp(head,"8BPS",4)) {
      fseek (ifp, 14, SEEK_SET);
      height = get4();
      width  = get4();
      filters = 0x61616161;
      data_offset = 68;
    }
    load_raw = unpacked_load_raw;
    maximum = 0x3fff;
  } else if (!strcmp(make,"Leaf")) {
    load_raw = unpacked_load_raw;
    if (tiff_data_compression == 99)
      load_raw = lossless_jpeg_load_raw;
    maximum = 0x3fff;
    strcpy (model, "Valeo");
    if (filters == 0) {
      load_raw = leaf_load_raw;
      maximum = 0xffff;
      strcpy (model, "Volare");
    }
  } else if (!strcmp(make,"LEICA") || !strcmp(make,"Panasonic")) {
    if (width == 3880) {
      data_offset += 12;
      maximum = 0xf7f0;
      width -= 22;
    } else if (width == 3304) {
      maximum = 0xf94c;
      width -= 16;
    } else maximum = 0xfff0;
    load_raw = unpacked_load_raw;
  } else if (!strcmp(model,"E-1")) {
    filters = 0x61616161;
    load_raw = unpacked_load_raw;
    maximum = 0xfff0;
    black = 1024;
  } else if (!strcmp(model,"E-10")) {
    load_raw = unpacked_load_raw;
    maximum = 0xfff0;
    black = 2048;
  } else if (!strncmp(model,"E-20",4)) {
    load_raw = unpacked_load_raw;
    maximum = 0xffc0;
    black = 2560;
  } else if (!strcmp(model,"E-300") ||
	     !strcmp(model,"E-500")) {
    width -= 20;
    load_raw = olympus_e300_load_raw;
    maximum = 0xfff;
    if (fsize > 15728640) {
      load_raw = unpacked_load_raw;
      maximum = 0xfc30;
    }
  } else if (!strcmp(model,"C770UZ")) {
    height = 1718;
    width  = 2304;
    filters = 0x16161616;
    load_raw = nikon_e2100_load_raw;
  } else if (!strcmp(make,"OLYMPUS")) {
    load_raw = olympus_cseries_load_raw;
    if (!strcmp(model,"C5050Z") ||
	!strcmp(model,"C8080WZ"))
      filters = 0x16161616;
  } else if (!strcmp(model,"N Digital")) {
    height = 2047;
    width  = 3072;
    filters = 0x61616161;
    data_offset = 0x1a00;
    load_raw = packed_12_load_raw;
    maximum = 0xf1e;
  } else if (!strcmp(model,"DSC-F828")) {
    width = 3288;
    left_margin = 5;
    data_offset = 862144;
    load_raw = sony_load_raw;
    filters = 0x9c9c9c9c;
    colors = 4;
    black = 491;
  } else if (!strcmp(model,"DSC-V3")) {
    width = 3109;
    left_margin = 59;
    data_offset = 787392;
    load_raw = sony_load_raw;
  } else if (!strcmp(model,"DSC-R1")) {
    width = 3925;
    order = 0x4d4d;
    load_raw = unpacked_load_raw;
    black = 512;
  } else if (!strncmp(model,"P850",4)) {
    height = 1950;
    width  = 2608;
    data_offset = 76456;
    filters = 0x16161616;
    load_raw = packed_12_load_raw;
  } else if (!strcasecmp(make,"KODAK")) {
    filters = 0x61616161;
    if (!strcmp(model,"NC2000F")) {
      width -= 4;
      left_margin = 2;
    } else if (!strcmp(model,"EOSDCS3B")) {
      width -= 4;
      left_margin = 2;
    } else if (!strcmp(model,"EOSDCS1")) {
      width -= 4;
      left_margin = 2;
    } else if (!strcmp(model,"DCS315C")) {
      black = 8;
    } else if (!strcmp(model,"DCS330C")) {
      black = 8;
    } else if (!strcmp(model,"DCS420")) {
      width -= 4;
      left_margin = 2;
    } else if (!strcmp(model,"DCS460")) {
      width -= 4;
      left_margin = 2;
    } else if (!strcmp(model,"DCS460A")) {
      width -= 4;
      left_margin = 2;
      colors = 1;
      filters = 0;
    } else if (!strcmp(model,"DCS520C")) {
      black = 180;
    } else if (!strcmp(model,"DCS560C")) {
      black = 188;
    } else if (!strcmp(model,"DCS620C")) {
      black = 180;
    } else if (!strcmp(model,"DCS620X")) {
      black = 185;
    } else if (!strcmp(model,"DCS660C")) {
      black = 214;
    } else if (!strcmp(model,"DCS660M")) {
      black = 214;
      colors = 1;
      filters = 0;
    } else if (!strcmp(model,"DCS760M")) {
      colors = 1;
      filters = 0;
    }
    switch (tiff_data_compression) {
      case 0:				/* No compression */
      case 1:
	load_raw = kodak_easy_load_raw;
	break;
      case 7:				/* Lossless JPEG */
	load_raw = lossless_jpeg_load_raw;
      case 32867:
	break;
      case 65000:			/* Kodak DCR compression */
	if (kodak_data_compression == 32803)
	  load_raw = kodak_compressed_load_raw;
	else {
	  load_raw = kodak_yuv_load_raw;
	  height = (height+1) & -2;
	  width  = (width +1) & -2;
	  filters = 0;
	}
	break;
      default:
	fprintf (stderr, "%s: %s %s uses unsupported compression method %d.\n",
		ifname, make, model, tiff_data_compression);
	return 1;
    }
    if (strstr(model,"DC25")) {
      strcpy (model, "DC25");
      data_offset = 15424;
    }
    if (!strncmp(model,"DC2",3)) {
      height = 242;
      if (fsize < 100000) {
	raw_width = 256; width = 249;
      } else {
	raw_width = 512; width = 501;
      }
      data_offset += raw_width + 1;
      colors = 4;
      filters = 0x8d8d8d8d;
      simple_coeff(1);
      pre_mul[1] = 1.179;
      pre_mul[2] = 1.209;
      pre_mul[3] = 1.036;
      load_raw = kodak_easy_load_raw;
    } else if (!strcmp(model,"Digital Camera 40")) {
      strcpy (model, "DC40");
      height = 512;
      width  = 768;
      data_offset = 1152;
      load_raw = kodak_radc_load_raw;
    } else if (strstr(model,"DC50")) {
      strcpy (model, "DC50");
      height = 512;
      width  = 768;
      data_offset = 19712;
      load_raw = kodak_radc_load_raw;
    } else if (strstr(model,"DC120")) {
      strcpy (model, "DC120");
      height = 976;
      width  = 848;
      if (tiff_data_compression == 7)
	load_raw = kodak_jpeg_load_raw;
      else
	load_raw = kodak_dc120_load_raw;
    }
  } else if (!strcmp(model,"Fotoman Pixtura")) {
    height = 512;
    width  = 768;
    data_offset = 3632;
    load_raw = kodak_radc_load_raw;
    filters = 0x61616161;
    simple_coeff(2);
  } else if (!strcmp(make,"Rollei")) {
    switch (raw_width) {
      case 1316:
	height = 1030;
	width  = 1300;
	top_margin  = 1;
	left_margin = 6;
	break;
      case 2568:
	height = 1960;
	width  = 2560;
	top_margin  = 2;
	left_margin = 8;
    }
    filters = 0x16161616;
    load_raw = rollei_load_raw;
    pre_mul[0] = 1.8;
    pre_mul[2] = 1.3;
  } else if (!strcmp(model,"PC-CAM 600")) {
    height = 768;
    data_offset = width = 1024;
    filters = 0x49494949;
    load_raw = eight_bit_load_raw;
    pre_mul[0] = 1.14;
    pre_mul[2] = 2.73;
  } else if (!strcmp(model,"QV-2000UX")) {
    height = 1208;
    width  = 1632;
    data_offset = width * 2;
    load_raw = eight_bit_load_raw;
  } else if (fsize == 3217760) {
    height = 1546;
    width  = 2070;
    raw_width = 2080;
    load_raw = eight_bit_load_raw;
  } else if (!strcmp(model,"QV-4000")) {
    height = 1700;
    width  = 2260;
    load_raw = unpacked_load_raw;
    maximum = 0xffff;
  } else if (!strcmp(model,"QV-5700")) {
    height = 1924;
    width  = 2576;
    load_raw = casio_qv5700_load_raw;
  } else if (!strcmp(model,"QV-R51")) {
    height = 1926;
    width  = 2576;
    raw_width = 3904;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 1.340;
    pre_mul[2] = 1.672;
  } else if (!strcmp(model,"EX-S100")) {
    height = 1544;
    width  = 2058;
    raw_width = 3136;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 1.631;
    pre_mul[2] = 1.106;
  } else if (!strcmp(model,"EX-Z50")) {
    height = 1931;
    width  = 2570;
    raw_width = 3904;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 2.529;
    pre_mul[2] = 1.185;
  } else if (!strcmp(model,"EX-Z55")) {
    height = 1960;
    width  = 2570;
    raw_width = 3904;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 1.520;
    pre_mul[2] = 1.316;
  } else if (!strcmp(model,"EX-P505")) {
    height = 1928;
    width  = 2568;
    raw_width = 3852;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 2.07;
    pre_mul[2] = 1.88;
  } else if (!strcmp(model,"EX-P600")) {
    height = 2142;
    width  = 2844;
    raw_width = 4288;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 1.797;
    pre_mul[2] = 1.219;
  } else if (!strcmp(model,"EX-P700")) {
    height = 2318;
    width  = 3082;
    raw_width = 4672;
    load_raw = packed_12_load_raw;
    pre_mul[0] = 1.758;
    pre_mul[2] = 1.504;
  } else if (!strcmp(make,"Nucore")) {
    filters = 0x61616161;
    load_raw = unpacked_load_raw;
    if (width == 2598) {
      filters = 0x16161616;
      load_raw = nucore_load_raw;
      flip = 2;
    }
  }
  if (raw_color) adobe_coeff();
dng_skip:
  if (!load_raw || !height || is_jpeg) {
    fprintf (stderr, "%s: Cannot decode %s %s%s images.\n",
      ifname, make, model, is_jpeg ? " JPEG":"");
    return 1;
  }
#ifdef NO_JPEG
  if (load_raw == kodak_jpeg_load_raw) {
    fprintf (stderr, "%s: decoder was not linked with libjpeg.\n", ifname);
    return 1;
  }
#endif
  if (!raw_height) raw_height = height;
  if (!raw_width ) raw_width  = width;
  raw_color |= use_camera_rgb && colors == 3;
  FORCC {			/* Apply user-selected color balance */
    rgb_cam[0][c] *= red_scale;
    rgb_cam[2][c] *= blue_scale;
  }
  if (filters && colors == 3)
    for (i=0; i < 32; i+=4) {
      if ((filters >> i & 15) == 9)
	filters |= 2 << i;
      if ((filters >> i & 15) == 6)
	filters |= 8 << i;
    }
  fseek (ifp, data_offset, SEEK_SET);
  return 0;
}

#ifdef USE_LCMS
void CLASS apply_profile (char *pfname)
{
  char *prof;
  cmsHPROFILE hInProfile=NULL, hOutProfile;
  cmsHTRANSFORM hTransform;

  if (pfname)
    hInProfile = cmsOpenProfileFromFile (pfname, "r");
  else if (profile_length) {
    prof = malloc (profile_length);
    merror (prof, "apply_profile()");
    fseek (ifp, profile_offset, SEEK_SET);
    fread (prof, 1, profile_length, ifp);
    hInProfile = cmsOpenProfileFromMem (prof, profile_length);
    free (prof);
  }
  if (!hInProfile) return;
  if (verbose)
    fprintf (stderr, "Applying color profile...\n");
  maximum = 0xffff;
  use_gamma = 0;
  raw_color = 1;		/* Don't use rgb_cam with a profile */

  hOutProfile = cmsCreate_sRGBProfile();
  hTransform = cmsCreateTransform (hInProfile, TYPE_RGBA_16,
	hOutProfile, TYPE_RGBA_16, INTENT_PERCEPTUAL, 0);
  cmsDoTransform (hTransform, image, image, width*height);

  cmsDeleteTransform (hTransform);
  cmsCloseProfile (hInProfile);
  cmsCloseProfile (hOutProfile);
}
#endif

/*
   Convert the entire image to RGB colorspace and build a histogram.
 */
void CLASS convert_to_rgb()
{
  int row, col, c, i, fc=0;
  ushort *img;
  float rgb[3];

  if (verbose)
    fprintf (stderr, raw_color ?
	"Building histograms...\n" : "Converting to sRGB colorspace...\n");

  if (document_mode)
    colors = 1;
  memset (histogram, 0, sizeof histogram);
  for (row = 0; row < height; row++)
    for (col = 0; col < width; col++) {
      img = image[row*width+col];
      if (document_mode)
	fc = FC(row,col);
      if (colors == 4 && raw_color)	/* Recombine the greens */
	img[1] = (img[1] + img[3]) >> 1;
      if (colors == 1)			/* RGB from grayscale */
	FORC3 rgb[c] = img[fc];
      else if (raw_color)		/* RGB from RGB (easy) */
	goto norgb;
      else FORC3			/* RGB via rgb_cam */
	for (rgb[c]=i=0; i < colors; i++)
	  rgb[c] += img[i] * rgb_cam[c][i];
      FORC3 img[c] = CLIP((int) rgb[c]);
norgb:
      FORC3 histogram[c][img[c] >> 3]++;
    }
}

void CLASS fuji_rotate()
{
  int i, wide, high, row, col;
  double step;
  float r, c, fr, fc;
  unsigned ur, uc;
  ushort (*img)[4], (*pix)[4];

  if (!fuji_width) return;
  if (verbose)
    fprintf (stderr, "Rotating image 45 degrees...\n");
  fuji_width = (fuji_width - 1 + shrink) >> shrink;
  step = sqrt(0.5);
  wide = fuji_width / step;
  high = (height - fuji_width) / step;
  img = calloc (wide*high, sizeof *img);
  merror (img, "fuji_rotate()");

  for (row=0; row < high; row++)
    for (col=0; col < wide; col++) {
      ur = r = fuji_width + (row-col)*step;
      uc = c = (row+col)*step;
      if (ur > height-2 || uc > width-2) continue;
      fr = r - ur;
      fc = c - uc;
      pix = image + ur*width + uc;
      for (i=0; i < colors; i++)
	img[row*wide+col][i] =
	  (pix[    0][i]*(1-fc) + pix[      1][i]*fc) * (1-fr) +
	  (pix[width][i]*(1-fc) + pix[width+1][i]*fc) * fr;
    }
  free (image);
  width  = wide;
  height = high;
  image  = img;
  fuji_width = 0;
}

void CLASS flip_image()
{
  unsigned *flag;
  int size, base, dest, next, row, col, temp;
  INT64 *img, hold;

  if (verbose)
    fprintf (stderr, "Flipping image %c:%c:%c...\n",
	flip & 1 ? 'H':'0', flip & 2 ? 'V':'0', flip & 4 ? 'T':'0');

  img = (INT64 *) image;
  size = height * width;
  flag = calloc ((size+31) >> 5, sizeof *flag);
  merror (flag, "flip_image()");
  for (base = 0; base < size; base++) {
    if (flag[base >> 5] & (1 << (base & 31)))
      continue;
    dest = base;
    hold = img[base];
    while (1) {
      if (flip & 4) {
	row = dest % height;
	col = dest / height;
      } else {
	row = dest / width;
	col = dest % width;
      }
      if (flip & 2)
	row = height - 1 - row;
      if (flip & 1)
	col = width - 1 - col;
      next = row * width + col;
      if (next == base) break;
      flag[next >> 5] |= 1 << (next & 31);
      img[dest] = img[next];
      dest = next;
    }
    img[dest] = hold;
  }
  free (flag);
  if (flip & 4) {
    temp = height;
    height = width;
    width = temp;
    temp = ymag;
    ymag = xmag;
    xmag = temp;
  }
}

/*
   Write the image to an 8-bit PPM file.
 */
void CLASS write_ppm (FILE *ofp)
{
  uchar (*ppm)[3], lut[0x10000];
  int perc, c, val, total, i, row, col;
  float white=0, r;

  fprintf (ofp, "P6\n%d %d\n255\n", xmag*width, ymag*height);
  ppm = calloc (width, 3*xmag);
  merror (ppm, "write_ppm()");

  perc = width * height * 0.01;		/* 99th percentile white point */
  if (fuji_width) perc /= 2;
  FORC3 {
    for (val=0x2000, total=0; --val > 32; )
      if ((total += histogram[c][val]) > perc) break;
    if (white < val) white = val;
  }
  white *= 8 / bright;
  for (i=0; i < 0x10000; i++) {
    r = i / white;
    val = 256 * ( !use_gamma ? r :
#ifdef SRGB_GAMMA
	r <= 0.00304 ? r*12.92 : pow(r,2.5/6)*1.055-0.055 );
#else
	r <= 0.018 ? r*4.5 : pow(r,0.45)*1.099-0.099 );
#endif
    if (val > 255) val = 255;
    lut[i] = val;
  }
  for (row=0; row < height; row++) {
    for (col=0; col < width; col++)
      FORC3 for (i=0; i < xmag; i++)
	ppm[xmag*col+i][c] = lut[image[row*width+col][c]];
    for (i=0; i < ymag; i++)
      fwrite (ppm, width, 3*xmag, ofp);
  }
  free (ppm);
}

/*
   Write the image to a 16-bit Photoshop file.
 */
void CLASS write_psd (FILE *ofp)
{
  char head[] = {
    '8','B','P','S',		/* signature */
    0,1,0,0,0,0,0,0,		/* version and reserved */
    0,3,			/* number of channels */
    0,0,0,0,			/* height, big-endian */
    0,0,0,0,			/* width, big-endian */
    0,16,			/* 16-bit color */
    0,3,			/* mode (1=grey, 3=rgb) */
    0,0,0,0,			/* color mode data */
    0,0,0,0,			/* image resources */
    0,0,0,0,			/* layer/mask info */
    0,0				/* no compression */
  };
  int hw[2], psize, row, col, c;
  ushort *buffer, *pred;

  hw[0] = htonl(height);	/* write the header */
  hw[1] = htonl(width);
  memcpy (head+14, hw, sizeof hw);
  fwrite (head, 40, 1, ofp);

  psize = height*width;
  buffer = calloc (6, psize);
  merror (buffer, "write_psd()");
  pred = buffer;

  for (row = 0; row < height; row++)
    for (col = 0; col < width; col++) {
      FORC3 pred[c*psize] = htons(image[row*width+col][c]);
      pred++;
    }
  fwrite(buffer, psize, 6, ofp);
  free (buffer);
}

/*
   Write the image to a 16-bit PPM file.
 */
void CLASS write_ppm16 (FILE *ofp)
{
  int row, col, c;
  ushort (*ppm)[3];

  if (maximum < 256) maximum = 256;
  fprintf (ofp, "P6\n%d %d\n%d\n", width, height, maximum);

  ppm = calloc (width, 6);
  merror (ppm, "write_ppm16()");

  for (row = 0; row < height; row++) {
    for (col = 0; col < width; col++)
      FORC3 ppm[col][c] = htons(image[row*width+col][c]);
    fwrite (ppm, width, 6, ofp);
  }
  free (ppm);
}

int CLASS main (int argc, char **argv)
{
  int arg, status=0, user_flip=-1, user_black=-1, user_qual=-1;
  int timestamp_only=0, identify_only=0, write_to_stdout=0;
  int half_size=0, use_fuji_rotate=1, quality;
  char opt, *ofname, *cp;
  struct utimbuf ut;
  const char *write_ext = ".ppm";
  FILE *ofp = stdout;
#ifdef USE_LCMS
  char *profile = NULL;
#endif

#ifndef LOCALTIME
  putenv ("TZ=UTC");
#endif
  if (argc == 1)
  {
    fprintf (stderr,
    "\nRaw Photo Decoder \"dcraw\" v7.82"
    "\nby Dave Coffin, dcoffin a cybercom o net"
    "\n\nUsage:  %s [options] file1 file2 ...\n"
    "\nValid options:"
    "\n-v        Print verbose messages"
    "\n-z        Change file dates to camera timestamp"
    "\n-i        Identify files without decoding them"
    "\n-c        Write to standard output"
    "\n-a        Use automatic white balance"
    "\n-w        Use camera white balance, if possible"
    "\n-r <num>  Set red  multiplier (default = 1.0)"
    "\n-l <num>  Set blue multiplier (default = 1.0)"
    "\n-b <num>  Set brightness      (default = 1.0)"
    "\n-k <num>  Set black point"
    "\n-n        Don't clip colors"
    "\n-m        Don't convert camera RGB to sRGB"
#ifdef USE_LCMS
    "\n-p <file> Apply color profile from file"
#endif
    "\n-d        Document Mode (no color, no interpolation)"
    "\n-q [0-3]  Set the interpolation quality"
    "\n-h        Half-size color image (twice as fast as \"-q 0\")"
    "\n-f        Interpolate RGGB as four colors"
    "\n-B <domain> <range>  Apply bilateral filter to reduce noise"
    "\n-j        Show Fuji Super CCD images tilted 45 degrees"
    "\n-s        Use secondary pixels (Fuji Super CCD SR only)"
    "\n-t [0-7]  Flip image (0 = none, 3 = 180, 5 = 90CCW, 6 = 90CW)"
    "\n-2        Write  8-bit PPM with 0.45 gamma (default)"
    "\n-3        Write 16-bit linear PSD (Adobe Photoshop)"
    "\n-4        Write 16-bit linear PPM"
    "\n\n", argv[0]);
    return 1;
  }

  argv[argc] = "";
  for (arg=1; argv[arg][0] == '-'; ) {
    opt = argv[arg++][1];
    if ((strchr("Bbrlktq", opt) && !isdigit(argv[arg][0])) ||
		   (opt == 'B' && !isdigit(argv[arg+1][0]))) {
      fprintf (stderr, "Non-numeric argument to \"-%c\"\n", opt);
      return 1;
    }
    switch (opt)
    {
      case 'B':  sigma_d     = atof(argv[arg++]);
		 sigma_r     = atof(argv[arg++]);  break;
      case 'b':  bright      = atof(argv[arg++]);  break;
      case 'r':  red_scale   = atof(argv[arg++]);  break;
      case 'l':  blue_scale  = atof(argv[arg++]);  break;
      case 'k':  user_black  = atoi(argv[arg++]);  break;
      case 't':  user_flip   = atoi(argv[arg++]);  break;
      case 'q':  user_qual   = atoi(argv[arg++]);  break;
#ifdef USE_LCMS
      case 'p':  profile     =      argv[arg++] ;  break;
#endif
      case 'z':  timestamp_only    = 1;  break;
      case 'i':  identify_only     = 1;  break;
      case 'c':  write_to_stdout   = 1;  break;
      case 'v':  verbose           = 1;  break;
      case 'h':  half_size         = 1;		/* "-h" implies "-f" */
      case 'f':  four_color_rgb    = 1;  break;
      case 'd':  document_mode     = 1;  break;
      case 'a':  use_auto_wb       = 1;  break;
      case 'w':  use_camera_wb     = 1;  break;
      case 'j':  use_fuji_rotate   = 0;  break;
      case 's':  use_secondary     = 1;  break;
      case 'n':  clip_color        = 0;  break;
      case 'm':  use_camera_rgb    = 1;  break;

      case '2':  write_fun = write_ppm;   write_ext = ".ppm";  break;
      case '3':  write_fun = write_psd;   write_ext = ".psd";  break;
      case '4':  write_fun = write_ppm16; write_ext = ".ppm";  break;

      default:
	fprintf (stderr, "Unknown option \"-%c\".\n", opt);
	return 1;
    }
  }
  if (arg == argc) {
    fprintf (stderr, "No files to process.\n");
    return 1;
  }
  if (write_to_stdout) {
    if (isatty(1)) {
      fprintf (stderr, "Will not write an image to the terminal!\n");
      return 1;
    }
#if defined(WIN32) || defined(DJGPP) || defined(__CYGWIN__)
    if (setmode(1,O_BINARY) < 0) {
      perror("setmode()");
      return 1;
    }
#endif
  }
  for ( ; arg < argc; arg++) {
    status = 1;
    image = NULL;
    if (setjmp (failure)) {
      if (fileno(ifp) > 2) fclose(ifp);
      if (fileno(ofp) > 2) fclose(ofp);
      if (image) free (image);
      status = 1;
      continue;
    }
    ifname = argv[arg];
    if (!(ifp = fopen (ifname, "rb"))) {
      perror (ifname);
      continue;
    }
    if (timestamp_only) {
      if ((status = identify(1)))
	fprintf (stderr, "%s has no timestamp.\n", ifname);
      else if (identify_only)
	printf ("%10ld%10d %s\n", timestamp, shot_order, ifname);
      else {
	if (verbose)
	  fprintf (stderr, "%s time set to %d.\n", ifname, (int) timestamp);
	ut.actime = ut.modtime = timestamp;
	utime (ifname, &ut);
      }
      goto next;
    }
    if ((status = identify(0))) goto next;
    if (user_flip >= 0)
      flip = user_flip;
    switch ((flip+3600) % 360) {
      case 270:  flip = 5;  break;
      case 180:  flip = 3;  break;
      case  90:  flip = 6;
    }
    if (identify_only) {
      fprintf (stderr, "%s is a %s %s image.\n", ifname, make, model);
next:
      fclose(ifp);
      continue;
    }
    shrink = half_size && filters;
    iheight = (height + shrink) >> shrink;
    iwidth  = (width  + shrink) >> shrink;
    image = calloc (iheight*iwidth*sizeof *image + meta_length, 1);
    merror (image, "main()");
    meta_data = (char *) (image + iheight*iwidth);
    if (verbose)
      fprintf (stderr,
	"Loading %s %s image from %s...\n", make, model, ifname);
    (*load_raw)();
    bad_pixels();
    height = iheight;
    width  = iwidth;
#ifdef COLORCHECK
    colorcheck();
#endif
    quality = 2 + !fuji_width;
    if (user_qual >= 0) quality = user_qual;
    if (user_black >= 0) black = user_black;
    if (is_foveon) foveon_interpolate();
    else scale_colors();
    if (shrink) filters = 0;
    cam_to_cielab (NULL,NULL);
    if (filters && !document_mode) {
      if (quality == 0)
	lin_interpolate();
      else if (quality < 3 || colors > 3)
	   vng_interpolate();
      else ahd_interpolate();
    }
    if (sigma_d > 0 && sigma_r > 0) bilateral_filter();
    if (use_fuji_rotate) fuji_rotate();
#ifdef USE_LCMS
    apply_profile (profile);
#endif
    convert_to_rgb();
    if (flip) flip_image();
    fclose(ifp);
    ofname = malloc (strlen(ifname) + 16);
    merror (ofname, "main()");
    if (write_to_stdout)
      strcpy (ofname, "standard output");
    else {
      strcpy (ofname, ifname);
      if ((cp = strrchr (ofname, '.'))) *cp = 0;
      strcat (ofname, write_ext);
      ofp = fopen (ofname, "wb");
      if (!ofp) {
	status = 1;
	perror(ofname);
	goto cleanup;
      }
    }
    if (verbose)
      fprintf (stderr, "Writing data to %s...\n", ofname);
    (*write_fun)(ofp);
    if (ofp != stdout)
      fclose(ofp);
cleanup:
    free (ofname);
    free (image);
  }
  return status;
}
