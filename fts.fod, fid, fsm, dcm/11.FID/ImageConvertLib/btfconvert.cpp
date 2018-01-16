#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tfconv.h"
#include "btfconvert.h"
#include "t4.h"


int	ScDecodeTiff(sc_convert_tiff * s);
int	ScDecodeIfd(sc_convert_tiff * s);
int	ScConvert(sc_convert_tiff * s);
int	ScGetStripOffsetArray(sc_convert_tiff * s, sc_ifd_entry * d);
int	ScGetStripLenArray(sc_convert_tiff * s, sc_ifd_entry * d);
void	ScCheckStripContiguity(sc_convert_tiff * s);
char	* ScGetTagName(int tag);


int	HmpTiffConvert2High(char * infile, char * outfile)
{
	sc_convert_tiff s;
	SYSTEMTIME	st;
	int	ret;

	memset(&s, 0, sizeof(s));

	s.in_fp=fopen(infile, "rb");
	if(!s.in_fp)
		return	Sc_Err_File_Open;

	if(ret=ScDecodeTiff(&s)){
		fclose(s.in_fp);
		return	ret;
	}

	while(s.in_next_ifd){
		if(ret=ScDecodeIfd(&s)){
			fclose(s.in_fp);
			return	ret;
		}
	}

	if(!s.need_convert){
		fclose(s.in_fp);
		return	Sc_Nothing_To_Convert;
	}
	if(s.unsupported_encoding){
		fclose(s.in_fp);
		return	Sc_Err_Unsupported_Encoding;
	}

	if(!outfile || !outfile[0] || !strcmp(outfile, infile)){
		s.out_rename=1;
		GetLocalTime(&st);
		sprintf(s.out_file_name, "%s.tmp_%4d%02d%02d_%02d%02d%02d_%05d.tiff", infile, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, sc_pid);
	}
	else
		strcpy(s.out_file_name, outfile);

	s.out_fp=fopen(s.out_file_name, "wb");
	if(!s.out_fp){
		fclose(s.in_fp);
		return	Sc_Err_File_Open;
	}

	fseek(s.in_fp, 0, SEEK_SET);
	ScDecodeTiff(&s);
	while(s.in_next_ifd){
		ScDecodeIfd(&s);
		ScConvert(&s);
	}

	fclose(s.in_fp);
	fclose(s.out_fp);

	if(s.out_rename)
		rename(s.out_file_name, infile);

	return	0;
}

int	ScDecodeTiff(sc_convert_tiff * s)
{
	sc_tiff_header h;
	int	i;

	i=fread(&h, 1, sizeof(h), s->in_fp);
	if(i<sizeof(h))
		return	Sc_Err_Decode_Tiff;

	if(memcmp(h.order, "II", 2) && memcmp(h.order, "MM", 2))
		return	Sc_Err_Decode_Tiff;
	if(memcmp(h.order, "MM", 2))
		s->in_byte_order=0;
	else
		s->in_byte_order=1;

	if(s->in_byte_order){
		h.signature=htons(h.signature);
		h.offset=htonl(h.offset);
	}
	if(h.signature!=42)
		return	Sc_Err_Decode_Tiff;

	s->in_next_ifd=h.offset;

	if(!s->out_fp)
		return	0;

	memset(&h, 0, sizeof(h));
	memcpy(h.order,"II", 2);
	h.signature=42;
	h.offset=8;
	fwrite(&h, 1, sizeof(h), s->out_fp);

	return	0;
}

int	ScDecodeIfd(sc_convert_tiff * s)
{
	sc_ifd_entry d;
	int	i, len, next, r[4];
	short	entry;
	int	BadFaxLines=0;
	int	CleanFaxData=0;
	int	ConsBadFaxLines=0;
	int	compression=3, t4option=0, xresol=0, yresol=0;
	float	ratio;

	i=fseek(s->in_fp, s->in_next_ifd, SEEK_SET);
	if(i)
		return	Sc_Err_Decode_Ifd;

	i=fread(&entry, 1, sizeof(entry), s->in_fp);
	if(i<sizeof(entry))
		return	Sc_Err_Decode_Ifd;
	if(s->in_byte_order)
		entry=htons(entry);

	for(i=0;i<entry;i++){
		len=fread(&d, 1, sizeof(d), s->in_fp);
		if(len<sizeof(d))
			return	Sc_Err_Decode_Ifd;
		if(s->in_byte_order){
			d.tag=htons(d.tag);
			d.type=htons(d.type);
			d.cnt=htonl(d.cnt);
			if(d.type==3)
				d.value=htons(d.value);
			else
				d.value=htonl(d.value);
		}
		if(d.type==5){
			next=ftell(s->in_fp);
			memset(r, 0, sizeof(r));
			len=fseek(s->in_fp, d.value, SEEK_SET);
			if(!len){
				fread(r, 1, sizeof(r), s->in_fp);
				if(s->in_byte_order){
					r[0]=htonl(r[0]);
					r[1]=htonl(r[1]);
				}
			}
			if(!r[1])
				r[1]=1;
			d.value=r[0]/r[1];
			fseek(s->in_fp, next, SEEK_SET);
		}
		if(d.tag==257)
			s->in_strip_length=d.value;
		else if(d.tag==259)
			compression=d.value;
		else if(d.tag==266)
			s->in_fill_order=d.value;
		else if(d.tag==273){
			if(d.cnt>1){
				len=ScGetStripOffsetArray(s, &d);
				if(len)
					return	Sc_Err_Decode_Ifd;
			}
			else
				s->in_strip_offset=d.value;
		}
		else if(d.tag==279){
			if(d.cnt>1){
				len=ScGetStripLenArray(s, &d);
				if(len)
					return	Sc_Err_Decode_Ifd;
			}
			else
				s->in_strip_byte=d.value;
		}
		else if(d.tag==282)
			xresol=d.value;
		else if(d.tag==283)
			yresol=d.value;
		else if(d.tag==292)
			t4option=d.value;
		else if(d.tag==326)
			BadFaxLines=d.value;
		else if(d.tag==327)
			CleanFaxData=d.value;
		else if(d.tag==328)
			ConsBadFaxLines=d.value;
	}
	len=fread(&s->in_next_ifd, 1, 4, s->in_fp);
	if(len<4)
		return	Sc_Err_Decode_Ifd;

	if(s->in_byte_order)
		s->in_next_ifd=htonl(s->in_next_ifd);

	if(compression!=3 || t4option&1)
		s->unsupported_encoding=1;

	s->ratio=0;
	if(xresol && yresol){
		ratio=(float)yresol/(float)xresol;
		if(ratio<0.7){
			s->need_convert=1;
			s->ratio=2;
			BadFaxLines *= 2;
			ConsBadFaxLines *= 2;
		}
		else if(ratio>1.4){
			s->need_convert=1;
			s->ratio=1;
			BadFaxLines=(BadFaxLines+1)/2;
			ConsBadFaxLines=(ConsBadFaxLines+1)/2;
		}
	}

	if(s->in_strcnt)
		ScCheckStripContiguity(s);

	if(!s->out_fp)
		return	0;

	entry=20;
	fwrite(&entry, 1, 2, s->out_fp);

	d.tag=254;
	d.type=4;
	d.cnt=1;
	d.value=2;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=256;
	d.type=4;
	d.cnt=1;
	d.value=1728;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=257;
	d.type=4;
	d.cnt=1;
	d.value=0;
	fwrite(&d, 1, sizeof(d), s->out_fp);
	s->offset_strip_length=ftell(s->out_fp)-4;

	d.tag=258;
	d.type=3;
	d.cnt=1;
	d.value=1;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=259;
	d.type=3;
	d.cnt=1;
	d.value=3;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=262;
	d.type=3;
	d.cnt=1;
	d.value=0;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=266;
	d.type=3;
	d.cnt=1;
	d.value=1;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=273;
	d.type=4;
	d.cnt=1;
	d.value=ftell(s->out_fp)+12*13+20;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=277;
	d.type=3;
	d.cnt=1;
	d.value=1;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=278;
	d.type=4;
	d.cnt=1;
	d.value=0;
	fwrite(&d, 1, sizeof(d), s->out_fp);
	s->offset_rows_per_strip=ftell(s->out_fp)-4;

	d.tag=279;
	d.type=4;
	d.cnt=1;
	d.value=0;
	fwrite(&d, 1, sizeof(d), s->out_fp);
	s->offset_strip_byte=ftell(s->out_fp)-4;

	d.tag=282;
	d.type=5;
	d.cnt=1;
	d.value=sizeof(sc_tiff_header)+2+20*sizeof(sc_ifd_entry)+4;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=283;
	d.type=5;
	d.cnt=1;
	d.value=sizeof(sc_tiff_header)+2+20*sizeof(sc_ifd_entry)+4+8;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=292;
	d.type=4;
	d.cnt=1;
	d.value=0;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=293;
	d.type=4;
	d.cnt=1;
	d.value=0;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=296;
	d.type=3;
	d.cnt=1;
	d.value=2;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=297;
	d.type=3;
	d.cnt=2;
	d.value=s->page_cnt;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=326;
	d.type=4;
	d.cnt=1;
	d.value=BadFaxLines;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=327;
	d.type=3;
	d.cnt=1;
	d.value=CleanFaxData;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	d.tag=328;
	d.type=4;
	d.cnt=1;
	d.value=ConsBadFaxLines;
	fwrite(&d, 1, sizeof(d), s->out_fp);

	s->offset_next_ifd=ftell(s->out_fp);
	i=0;
	fwrite(&i, 1, 4, s->out_fp);

	r[0]=204;
	r[1]=1;
	r[2]=196;
	r[3]=1;
	fwrite(r, 1, sizeof(r), s->out_fp);

	return	0;
}

int	ScConvert(sc_convert_tiff * s)
{
	scuchar	buf[4096];
	int	i, len, remain;

//	printf("ScConvert, page=%2d, StripOffsets=%d, StripByteCounts=%d.\n", s->page_cnt, s->in_strip_offset, s->in_strip_byte);

	fseek(s->in_fp, s->in_strip_offset, SEEK_SET);

	s->out_strip_length=0;
	s->out_strip_byte=0;
	s->eol=s->black=s->bcnt=s->zcnt=s->bseq=s->rbcnt=s->clinebitlen=0;
	s->scnt=0xff;
	memset(s->rbit, 0, sizeof(s->rbit));
	memset(s->currline, 0, sizeof(s->currline));

	remain=s->in_strip_byte;
	while(remain){
		if(remain>=sizeof(buf))
			i=sizeof(buf);
		else
			i=remain;
		len=fread(buf, 1, i, s->in_fp);
		if(len<=0)
			break;
		remain -= len;

		if(s->in_fill_order==2)
			for(i=0;i<len;i++)
				buf[i]=sc_bswap[buf[i]];

		ScaleMhBlock(s, len, buf);
	}
	if(s->ratio==2 && s->out_strip_length && s->clinebitlen){
		ScAddBitString(s, 12, 1);
		ScAddByteString(s, s->clinebitlen, s->currline);
		s->out_strip_length++;
	}
	i=(s->rbcnt+7)>>3;
	fwrite(s->rbit, 1, i, s->out_fp);
	s->out_strip_byte += s->rbcnt;

	fseek(s->out_fp, s->offset_strip_length, SEEK_SET);
	fwrite(&s->out_strip_length, 1, 4, s->out_fp);

	fseek(s->out_fp, s->offset_rows_per_strip, SEEK_SET);
	fwrite(&s->out_strip_length, 1, 4, s->out_fp);

	fseek(s->out_fp, s->offset_strip_byte, SEEK_SET);
	i=(s->out_strip_byte+7)>>3;
	fwrite(&i, 1, 4, s->out_fp);

	fseek(s->out_fp, 0, SEEK_END);
	if(s->in_next_ifd){
		i=ftell(s->out_fp);
		fseek(s->out_fp, s->offset_next_ifd, SEEK_SET);
		fwrite(&i, 1, 4, s->out_fp);
		fseek(s->out_fp, 0, SEEK_END);
	}

	s->page_cnt++;

	return	0;
}

int	ScGetStripOffsetArray(sc_convert_tiff * s, sc_ifd_entry * d)
{
	int	i, len, size, offset, back;

	if(s->in_strcnt){
		if(s->in_strcnt!=d->cnt)
			;//printf("WARNING: strip_offset cnt=%d, strip_len cnt=%d\n", d->cnt, s->in_strcnt);
	}
	else
		s->in_strcnt=d->cnt;

	s->inptr_strip_offset=(int *)calloc(d->cnt, sizeof(int));

	back=ftell(s->in_fp);
	fseek(s->in_fp, d->value, SEEK_SET);

	if(d->type==3)
		size=2;
	else
		size=4;
	for(i=0;i<d->cnt;i++){
		offset=0;
		len=fread(&offset, 1, size, s->in_fp);
		if(len<4)
			return	2;
		if(s->in_byte_order){
			if(d->type==3)
				offset=htons(offset);
			else
				offset=htonl(offset);
		}
		s->inptr_strip_offset[i]=offset;
	}

	s->in_strip_offset=s->inptr_strip_offset[0];

	fseek(s->in_fp, back, SEEK_SET);

	return	0;
}

int	ScGetStripLenArray(sc_convert_tiff * s, sc_ifd_entry * d)
{
	int	i, len, size, byte, total, back;

	if(s->in_strcnt){
		if(s->in_strcnt!=d->cnt)
			;//printf("WARNING: strip_len cnt=%d, strip_offset cnt=%d\n", d->cnt, s->in_strcnt);
	}
	else
		s->in_strcnt=d->cnt;

	s->inptr_strip_len=(int *)calloc(d->cnt, sizeof(int));

	back=ftell(s->in_fp);
	fseek(s->in_fp, d->value, SEEK_SET);

	if(d->type==3)
		size=2;
	else
		size=4;
	total=0;
	for(i=0;i<d->cnt;i++){
		byte=0;
		len=fread(&byte, 1, size, s->in_fp);
		if(len<4)
			return	2;
		if(s->in_byte_order){
			if(d->type==3)
				byte=htons(byte);
			else
				byte=htonl(byte);
		}
		s->inptr_strip_len[i]=byte;
		total += byte;
	}

	s->in_strip_byte=total;

	fseek(s->in_fp, back, SEEK_SET);

	return	0;
}

void	ScCheckStripContiguity(sc_convert_tiff * s)
{
	int	i;

	for(i=0;i<(s->in_strcnt-1);i++){
		if(s->inptr_strip_offset[i+1]!=(s->inptr_strip_offset[i]+s->inptr_strip_len[i])){
			;//printf("\nWARNING: strip not contiguous, strip_%d_offset=%d, len=%d, strip_%d_offset=%d.\n", i, s->inptr_strip_offset[i], s->inptr_strip_len[i], i+1, s->inptr_strip_offset[i+1]);
			return;
		}
	}

	free(s->inptr_strip_offset);
	free(s->inptr_strip_len);
	s->inptr_strip_offset=0;
	s->inptr_strip_len=0;
	s->in_strcnt=0;
}

char	* ScGetTagName(int tag)
{
	switch(tag){
	case 254:	return	"NewSubFileType";
	case 256:	return	"ImageWidth";
	case 257:	return	"ImageLength";
	case 258:	return	"BitsPerSample";
	case 259:	return	"Compression";
	case 262:	return	"PhotoInterpret";
	case 266:	return	"FillOrder";
	case 273:	return	"StripOffsets";
	case 277:	return	"SamplesPerPixel";
	case 278:	return	"RowsPerStrip";
	case 279:	return	"StripByteCounts";
	case 282:	return	"XResolution";
	case 283:	return	"YResolution";
	case 292:	return	"T4Options";
	case 293:	return	"T6Options";
	case 296:	return	"ResolutionUnit";
	case 297:	return	"PageNumber";
	case 326:	return	"BadFaxLines";
	case 327:	return	"CleanFaxData";
	case 328:	return	"ConsBadFaxLines";
	default:	return	"???";
	}
}
