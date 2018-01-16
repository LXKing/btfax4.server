#ifndef TIFF_CONV_H
#define TIFF_CONV_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef unsigned char	scuchar;
typedef unsigned short	sushort;
typedef unsigned int	scuint;


typedef struct{
	char	order[2];
	sushort	signature;
	scuint	offset;
}sc_tiff_header;

typedef struct{
	sushort	tag;
	sushort	type;
	scuint	cnt;
	scuint	value;
}sc_ifd_entry;

typedef struct{
	int	need_convert;
	int	unsupported_encoding;
	FILE	* in_fp;
	int	in_byte_order;
	int	in_next_ifd;
	int	in_strip_length;
	int	in_strip_offset;
	int	in_strip_byte;
	int	in_fill_order;
	int	in_strcnt;
	int	* inptr_strip_offset;
	int	* inptr_strip_len;
	int	ratio;
	int	page_cnt;
	FILE	* out_fp;
	int	out_rename;
	char	out_file_name[384];
	int	out_strip_length;
	int	out_strip_byte;
	int	offset_strip_length;
	int	offset_rows_per_strip;
	int	offset_strip_byte;
	int	offset_next_ifd;
	scuchar	scnt;
	char	eol;
	char	black;
	char	bcnt;
	scuint	zcnt;
	sushort	bseq;
	int	rbcnt;
	scuchar	rbit[4100];
	int	clinebitlen;
	scuchar	currline[2048];
}sc_convert_tiff;


void	ScaleFaxBlock(sc_convert_tiff * s, int len, scuchar * buf);
void	ScAddBitString(sc_convert_tiff * s, int len, sushort w);
void	ScAddByteString(sc_convert_tiff * s, int len, scuchar * w);
void	ScFlushRbit(sc_convert_tiff * s);


extern	int	sc_pid;
extern	scuchar	sc_bswap[256];


#ifdef __cplusplus
}
#endif

#endif
