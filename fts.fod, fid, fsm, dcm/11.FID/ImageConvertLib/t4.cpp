#include "stdafx.h"
#include "t4.h"
#include <string.h>
#include "tfconv.h"
#include "btfconvert.h"


void	ScCheckLine(sc_convert_tiff * s, scuchar b);
void	ScAddTxBit(sc_convert_tiff * s, int len, sushort w);


int	sc_pid;
scuchar	sc_bswap[256];

void	(*LineWord4[16])(sc_convert_tiff * s);
void	(*LineWord5[32])(sc_convert_tiff * s);
void	(*LineWord6[64])(sc_convert_tiff * s);
void	(*LineWord7[64])(sc_convert_tiff * s);	// leading 0
void	(*LineWord8[128])(sc_convert_tiff * s);	// leading 0
void	(*LineWord9[128])(sc_convert_tiff * s);	// leading 01
void	(*LineWord10[8])(sc_convert_tiff * s);	// leading 0000000
void	(*LineWord11[16])(sc_convert_tiff * s);	// leading 0000000
void	(*LineWord12[32])(sc_convert_tiff * s);	// leading 0000000

void	(*LineBord2[4])(sc_convert_tiff * s);
void	(*LineBord3[4])(sc_convert_tiff * s);	// leading 0
void	(*LineBord4[4])(sc_convert_tiff * s);	// leading 00
void	(*LineBord5[4])(sc_convert_tiff * s);	// leading 000
void	(*LineBord6[8])(sc_convert_tiff * s);	// leading 000
void	(*LineBord7[8])(sc_convert_tiff * s);	// leading 0000
void	(*LineBord8[16])(sc_convert_tiff * s);	// leading 0000
void	(*LineBord9[32])(sc_convert_tiff * s);	// leading 0000
void	(*LineBord10[64])(sc_convert_tiff * s);	// leading 0000
void	(*LineBord11[128])(sc_convert_tiff * s);// leading 0000
void	(*LineBord12[256])(sc_convert_tiff * s);// leading 0000
void	(*LineBord13[64])(sc_convert_tiff * s);	// leading 0000001


void	ScaleMhBlock(sc_convert_tiff * s, int len, scuchar * buf)
{
	scuchar	b0;

	while(len){
		len--;
		b0=*buf;
		buf++;
		ScCheckLine(s, b0 & 0x80);
		ScCheckLine(s, b0 & 0x40);
		ScCheckLine(s, b0 & 0x20);
		ScCheckLine(s, b0 & 0x10);
		ScCheckLine(s, b0 & 0x08);
		ScCheckLine(s, b0 & 0x04);
		ScCheckLine(s, b0 & 0x02);
		ScCheckLine(s, b0 & 0x01);
	}
}

void	ScCheckLine(sc_convert_tiff * s, scuchar b)
{
	sushort	bseq;

	if(s->eol){
		s->bcnt++;
		if(b)
			s->bseq=(s->bseq<<1)+1;
		else
			s->bseq=(s->bseq<<1);

		if(!s->black){
			if(s->bcnt<4)
				return;
			bseq=s->bseq;
			switch(s->bcnt){
			case 4: if(*LineWord4[bseq])  (*LineWord4[bseq])(s);  break;
			case 5: if(*LineWord5[bseq])  (*LineWord5[bseq])(s);  break;
			case 6: if(*LineWord6[bseq])  (*LineWord6[bseq])(s);  break;
			case 7: if(*LineWord7[bseq])  (*LineWord7[bseq])(s);  break;
			case 8: if(*LineWord8[bseq])  (*LineWord8[bseq])(s);  break;
			case 9: if(*LineWord9[bseq&0x7f]) (*LineWord9[bseq&0x7f])(s); break;
			case 10:if(*LineWord10[bseq]) (*LineWord10[bseq])(s); break;
			case 11:if(*LineWord11[bseq]) (*LineWord11[bseq])(s); break;
			case 12:if(*LineWord12[bseq]) (*LineWord12[bseq])(s); break;
			}
		}
		else{
			if(s->bcnt<2)
				return;
			bseq=s->bseq;
			switch(s->bcnt){
			case 2: if(*LineBord2[bseq])  (*LineBord2[bseq])(s);  break;
			case 3: if(*LineBord3[bseq])  (*LineBord3[bseq])(s);  break;
			case 4: if(*LineBord4[bseq])  (*LineBord4[bseq])(s);  break;
			case 5: if(*LineBord5[bseq])  (*LineBord5[bseq])(s);  break;
			case 6: if(*LineBord6[bseq])  (*LineBord6[bseq])(s);  break;
			case 7: if(*LineBord7[bseq])  (*LineBord7[bseq])(s);  break;
			case 8: if(*LineBord8[bseq])  (*LineBord8[bseq])(s);  break;
			case 9: if(*LineBord9[bseq])  (*LineBord9[bseq])(s);  break;
			case 10:if(*LineBord10[bseq]) (*LineBord10[bseq])(s); break;
			case 11:if(*LineBord11[bseq]) (*LineBord11[bseq])(s); break;
			case 12:if(*LineBord12[bseq]) (*LineBord12[bseq])(s); break;
			case 13:if(*LineBord13[bseq&0x3f]) (*LineBord13[bseq&0x3f])(s); break;
			}
		}
	}
	else{
		if(!b)
			s->zcnt++;
		else{
			if(s->zcnt>=11){
				s->eol=1;
				s->black=0;
				s->bcnt=0;
				s->bseq=0;
				s->scnt++;
				ScAddTxBit(s, 12, 1);
				if(s->ratio==2 && s->out_strip_length){
					ScAddByteString(s, s->clinebitlen, s->currline);
					s->out_strip_length++;
				}
				memset(s->currline, 0, (s->clinebitlen+7)>>3);
				s->clinebitlen=0;
				if(s->ratio!=1 || !(s->scnt%2))
					s->out_strip_length++;
			}
			s->zcnt=0;
		}
	}
}

void	ScAddBitString(sc_convert_tiff * s, int len, sushort w)
{
	int	need;
	scuchar	* p;

	need=8-(s->rbcnt&7);
	p=s->rbit+(s->rbcnt>>3);

	s->rbcnt += len;

	if(len>need){
		len -= need;
		p[0] |= w>>len;
		p++;
	}
	else{
		p[0] |= w<<(need-len);
		if(s->rbcnt>=8*(sizeof(s->rbit)-4))
			ScFlushRbit(s);
		return;
	}

	while(1){
		if(len>8){
			len -= 8;
			p[0] |= w>>len;
			p++;
		}
		else{
			p[0] |= w<<(8-len);
			if(s->rbcnt>=8*(sizeof(s->rbit)-4))
				ScFlushRbit(s);
			return;
		}
	}
}

void	ScAddByteString(sc_convert_tiff * s, int len, scuchar * w)
{
	int	i, len0, remain;

	len0=len>>3;
	remain=len&7;
	for(i=0;i<len0;i++)
		ScAddBitString(s, 8, w[i]);
	if(remain)
		ScAddBitString(s, remain, w[i]>>(8-remain));
}

void	ScAdd2CurrLine(sc_convert_tiff * s, int len, sushort w)
{
	int	need;
	scuchar	* p;

	need=8-(s->clinebitlen&7);
	p=s->currline+(s->clinebitlen>>3);

	s->clinebitlen += len;

	if(len>need){
		len -= need;
		p[0] |= w>>len;
		p++;
	}
	else{
		p[0] |= w<<(need-len);
		return;
	}

	while(1){
		if(len>8){
			len -= 8;
			p[0] |= w>>len;
			p++;
		}
		else{
			p[0] |= w<<(8-len);
			return;
		}
	}
}

void	ScAddTxBit(sc_convert_tiff * s, int len, sushort w)
{
	int	need;
	scuchar	* p;

	if(s->ratio==1 && s->scnt%2)
		return;

	if(s->ratio==2)
		ScAdd2CurrLine(s, len, w);

	need=8-(s->rbcnt&7);
	p=s->rbit+(s->rbcnt>>3);

	s->rbcnt += len;

	if(len>need){
		len -= need;
		p[0] |= w>>len;
		p++;
	}
	else{
		p[0] |= w<<(need-len);
		if(s->rbcnt>=8*(sizeof(s->rbit)-4))
			ScFlushRbit(s);
		return;
	}

	while(1){
		if(len>8){
			len -= 8;
			p[0] |= w>>len;
			p++;
		}
		else{
			p[0] |= w<<(8-len);
			if(s->rbcnt>=8*(sizeof(s->rbit)-4))
				ScFlushRbit(s);
			return;
		}
	}
}

void	ScFlushRbit(sc_convert_tiff * s)
{
	s->out_strip_byte += 8*(sizeof(s->rbit)-4);

	fwrite(s->rbit, 1, sizeof(s->rbit)-4, s->out_fp);

	memcpy(s->rbit, s->rbit+sizeof(s->rbit)-4, 4);
	memset(s->rbit+4, 0, sizeof(s->rbit)-4);
	s->rbcnt -= 8*(sizeof(s->rbit)-4);
}


void	Line4Bit0111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 4, 7);
}

void	Line4Bit1000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 4, 8);
}

void	Line4Bit1011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 4, 11);
}

void	Line4Bit1100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 4, 12);
}

void	Line4Bit1110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 4, 14);
}

void	Line4Bit1111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 4, 15);
}

void	Line5Bit00111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 5, 7);
}

void	Line5Bit01000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 5, 8);
}

void	Line5Bit10010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 5, 18);
}

void	Line5Bit10011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 5, 19);
}

void	Line5Bit10100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 5, 20);
}

void	Line5Bit11011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 5, 27);
}

void	Line6Bit000011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 3);
}

void	Line6Bit000111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 7);
}

void	Line6Bit001000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 8);
}

void	Line6Bit010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 6, 23);
}

void	Line6Bit011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 6, 24);
}

void	Line6Bit101010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 42);
}

void	Line6Bit101011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 43);
}

void	Line6Bit110100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 52);
}

void	Line6Bit110101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 6, 53);
}

void	Line7Bit0000011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 3);
}

void	Line7Bit0000100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 4);
}

void	Line7Bit0001000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 8);
}

void	Line7Bit0001100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 12);
}

void	Line7Bit0010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 19);
}

void	Line7Bit0010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 23);
}

void	Line7Bit0011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 24);
}

void	Line7Bit0100100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 36);
}

void	Line7Bit0100111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 39);
}

void	Line7Bit0101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 40);
}

void	Line7Bit0101011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 7, 43);
}

void	Line7Bit0110111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 7, 55);
}

void	Line8Bit00000000(sc_convert_tiff * s)
{
	s->eol=0;
	s->zcnt=8;
}

void	Line8Bit00000010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 2);
}

void	Line8Bit00000011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 3);
}

void	Line8Bit00000100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 4);
}

void	Line8Bit00000101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 5);
}

void	Line8Bit00001010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 10);
}

void	Line8Bit00001011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 11);
}

void	Line8Bit00010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 18);
}

void	Line8Bit00010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 19);
}

void	Line8Bit00010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 20);
}

void	Line8Bit00010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 21);
}

void	Line8Bit00010110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 22);
}

void	Line8Bit00010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 23);
}

void	Line8Bit00011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 26);
}

void	Line8Bit00011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 27);
}

void	Line8Bit00100100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 36);
}

void	Line8Bit00100101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 37);
}

void	Line8Bit00101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 40);
}

void	Line8Bit00101001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 41);
}

void	Line8Bit00101010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 42);
}

void	Line8Bit00101011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 43);
}

void	Line8Bit00101100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 44);
}

void	Line8Bit00101101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 45);
}

void	Line8Bit00110010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 50);
}

void	Line8Bit00110011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 51);
}

void	Line8Bit00110100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 52);
}

void	Line8Bit00110101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 53);
}

void	Line8Bit00110110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 8, 54);
}

void	Line8Bit00110111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 8, 55);
}

void	Line8Bit01001010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 74);
}

void	Line8Bit01001011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 75);
}

void	Line8Bit01010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 82);
}

void	Line8Bit01010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 83);
}

void	Line8Bit01010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 84);
}

void	Line8Bit01010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 85);
}

void	Line8Bit01011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 88);
}

void	Line8Bit01011001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 89);
}

void	Line8Bit01011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 90);
}

void	Line8Bit01011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=1;
	ScAddTxBit(s, 8, 91);
}

void	Line8Bit01100100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 8, 100);
}

void	Line8Bit01100101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 8, 101);
}

void	Line8Bit01100111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 8, 103);
}

void	Line8Bit01101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 8, 104);
}

void	Line9Bit010011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 152);
}

void	Line9Bit010011001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 153);
}

void	Line9Bit010011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 154);
}

void	Line9Bit010011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 155);
}

void	Line9Bit011001100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 204);
}

void	Line9Bit011001101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 205);
}

void	Line9Bit011010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 210);
}

void	Line9Bit011010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 211);
}

void	Line9Bit011010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 212);
}

void	Line9Bit011010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 213);
}

void	Line9Bit011010110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 214);
}

void	Line9Bit011010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 215);
}

void	Line9Bit011011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 216);
}

void	Line9Bit011011001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 217);
}

void	Line9Bit011011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 218);
}

void	Line9Bit011011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 9, 219);
}

void	Line11Bit00000001000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 11, 8);
}

void	Line11Bit00000001100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 11, 12);
}

void	Line11Bit00000001101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 11, 13);
}

void	Line12Bit000000010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 18);
}

void	Line12Bit000000010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 19);
}

void	Line12Bit000000010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 20);
}

void	Line12Bit000000010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 21);
}

void	Line12Bit000000010110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 22);
}

void	Line12Bit000000010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 23);
}

void	Line12Bit000000011100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 28);
}

void	Line12Bit000000011101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 29);
}

void	Line12Bit000000011110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 30);
}

void	Line12Bit000000011111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 31);
}





void	Bine2Bit10(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 2, 2);
}

void	Bine2Bit11(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 2, 3);
}

void	Bine3Bit010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 3, 2);
}

void	Bine3Bit011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 3, 3);
}

void	Bine4Bit0010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 4, 2);
}

void	Bine4Bit0011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 4, 3);
}

void	Bine5Bit00011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 5, 3);
}

void	Bine6Bit000100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 6, 4);
}

void	Bine6Bit000101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 6, 5);
}

void	Bine7Bit0000100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 7, 4);
}

void	Bine7Bit0000101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 7, 5);
}

void	Bine7Bit0000111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 7, 7);
}

void	Bine8Bit00000000(sc_convert_tiff * s)
{
	s->eol=0;
	s->zcnt=8;
}

void	Bine8Bit00000100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 8, 4);
}

void	Bine8Bit00000111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 8, 7);
}

void	Bine9Bit000011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 9, 24);
}

void	Bine10Bit0000001000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 10, 8);
}

void	Bine10Bit0000001111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 10, 15);
}

void	Bine10Bit0000010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 10, 23);
}

void	Bine10Bit0000011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 10, 24);
}

void	Bine10Bit0000110111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 10, 55);
}

void	Bine11Bit00000001000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 11, 8);
}

void	Bine11Bit00000001100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 11, 12);
}

void	Bine11Bit00000001101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 11, 13);
}

void	Bine11Bit00000010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 23);
}

void	Bine11Bit00000011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 24);
}

void	Bine11Bit00000101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 40);
}

void	Bine11Bit00000110111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 55);
}

void	Bine11Bit00001100111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 103);
}

void	Bine11Bit00001101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 104);
}

void	Bine11Bit00001101100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 11, 108);
}

void	Bine12Bit000000010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 18);
}

void	Bine12Bit000000010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 19);
}

void	Bine12Bit000000010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 20);
}

void	Bine12Bit000000010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 21);
}

void	Bine12Bit000000010110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 22);
}

void	Bine12Bit000000010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 23);
}

void	Bine12Bit000000011100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 28);
}

void	Bine12Bit000000011101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 29);
}

void	Bine12Bit000000011110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 30);
}

void	Bine12Bit000000011111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 31);
}

void	Bine12Bit000000100100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 36);
}

void	Bine12Bit000000100111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 39);
}

void	Bine12Bit000000101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 40);
}

void	Bine12Bit000000101011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 43);
}

void	Bine12Bit000000101100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 44);
}

void	Bine12Bit000000110011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 51);
}

void	Bine12Bit000000110100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 52);
}

void	Bine12Bit000000110101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 53);
}

void	Bine12Bit000000110111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 55);
}

void	Bine12Bit000000111000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 56);
}

void	Bine12Bit000001010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 82);
}

void	Bine12Bit000001010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 83);
}

void	Bine12Bit000001010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 84);
}

void	Bine12Bit000001010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 85);
}

void	Bine12Bit000001010110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 86);
}

void	Bine12Bit000001010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 87);
}

void	Bine12Bit000001011000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 88);
}

void	Bine12Bit000001011001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 89);
}

void	Bine12Bit000001011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 90);
}

void	Bine12Bit000001011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 91);
}

void	Bine12Bit000001100100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 100);
}

void	Bine12Bit000001100101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 101);
}

void	Bine12Bit000001100110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 102);
}

void	Bine12Bit000001100111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 103);
}

void	Bine12Bit000001101000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 104);
}

void	Bine12Bit000001101001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 105);
}

void	Bine12Bit000001101010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 106);
}

void	Bine12Bit000001101011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 107);
}

void	Bine12Bit000001101100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 108);
}

void	Bine12Bit000001101101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 109);
}

void	Bine12Bit000011001000(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 200);
}

void	Bine12Bit000011001001(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 12, 201);
}

void	Bine12Bit000011001010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 202);
}

void	Bine12Bit000011001011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 203);
}

void	Bine12Bit000011001100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 204);
}

void	Bine12Bit000011001101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 205);
}

void	Bine12Bit000011010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 210);
}

void	Bine12Bit000011010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 211);
}

void	Bine12Bit000011010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 212);
}

void	Bine12Bit000011010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 213);
}

void	Bine12Bit000011010110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 214);
}

void	Bine12Bit000011010111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 215);
}

void	Bine12Bit000011011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 218);
}

void	Bine12Bit000011011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	s->black=0;
	ScAddTxBit(s, 12, 219);
}

void	Bine13Bit0000001001010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 74);
}

void	Bine13Bit0000001001011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 75);
}

void	Bine13Bit0000001001100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 76);
}

void	Bine13Bit0000001001101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 77);
}

void	Bine13Bit0000001010010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 82);
}

void	Bine13Bit0000001010011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 83);
}

void	Bine13Bit0000001010100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 84);
}

void	Bine13Bit0000001010101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 85);
}

void	Bine13Bit0000001011010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 90);
}

void	Bine13Bit0000001011011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 91);
}

void	Bine13Bit0000001100100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 100);
}

void	Bine13Bit0000001100101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 101);
}

void	Bine13Bit0000001101100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 108);
}

void	Bine13Bit0000001101101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 109);
}

void	Bine13Bit0000001110010(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 114);
}

void	Bine13Bit0000001110011(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 115);
}

void	Bine13Bit0000001110100(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 116);
}

void	Bine13Bit0000001110101(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 117);
}

void	Bine13Bit0000001110110(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 118);
}

void	Bine13Bit0000001110111(sc_convert_tiff * s)
{
	s->bcnt=0;
	s->bseq=0;
	ScAddTxBit(s, 13, 119);
}


void	InitHmpTiffConvert()
{
	int	i, j;

	sc_pid=getpid();

	for(i=0;i<256;i++)
		for(j=0;j<8;j++)
			if( i & (1<<j) )
				sc_bswap[i] |= 0x80>>j;

	LineWord4[7]=Line4Bit0111;
	LineWord4[8]=Line4Bit1000;
	LineWord4[11]=Line4Bit1011;
	LineWord4[12]=Line4Bit1100;
	LineWord4[14]=Line4Bit1110;
	LineWord4[15]=Line4Bit1111;
	LineWord5[7]=Line5Bit00111;
	LineWord5[8]=Line5Bit01000;
	LineWord5[18]=Line5Bit10010;
	LineWord5[19]=Line5Bit10011;
	LineWord5[20]=Line5Bit10100;
	LineWord5[27]=Line5Bit11011;
	LineWord6[3]=Line6Bit000011;
	LineWord6[7]=Line6Bit000111;
	LineWord6[8]=Line6Bit001000;
	LineWord6[23]=Line6Bit010111;
	LineWord6[24]=Line6Bit011000;
	LineWord6[42]=Line6Bit101010;
	LineWord6[43]=Line6Bit101011;
	LineWord6[52]=Line6Bit110100;
	LineWord6[53]=Line6Bit110101;
	LineWord7[3]=Line7Bit0000011;
	LineWord7[4]=Line7Bit0000100;
	LineWord7[8]=Line7Bit0001000;
	LineWord7[12]=Line7Bit0001100;
	LineWord7[19]=Line7Bit0010011;
	LineWord7[23]=Line7Bit0010111;
	LineWord7[24]=Line7Bit0011000;
	LineWord7[36]=Line7Bit0100100;
	LineWord7[39]=Line7Bit0100111;
	LineWord7[40]=Line7Bit0101000;
	LineWord7[43]=Line7Bit0101011;
	LineWord7[55]=Line7Bit0110111;
	LineWord8[0]=Line8Bit00000000;
	LineWord8[2]=Line8Bit00000010;
	LineWord8[3]=Line8Bit00000011;
	LineWord8[4]=Line8Bit00000100;
	LineWord8[5]=Line8Bit00000101;
	LineWord8[10]=Line8Bit00001010;
	LineWord8[11]=Line8Bit00001011;
	LineWord8[18]=Line8Bit00010010;
	LineWord8[19]=Line8Bit00010011;
	LineWord8[20]=Line8Bit00010100;
	LineWord8[21]=Line8Bit00010101;
	LineWord8[22]=Line8Bit00010110;
	LineWord8[23]=Line8Bit00010111;
	LineWord8[26]=Line8Bit00011010;
	LineWord8[27]=Line8Bit00011011;
	LineWord8[36]=Line8Bit00100100;
	LineWord8[37]=Line8Bit00100101;
	LineWord8[40]=Line8Bit00101000;
	LineWord8[41]=Line8Bit00101001;
	LineWord8[42]=Line8Bit00101010;
	LineWord8[43]=Line8Bit00101011;
	LineWord8[44]=Line8Bit00101100;
	LineWord8[45]=Line8Bit00101101;
	LineWord8[50]=Line8Bit00110010;
	LineWord8[51]=Line8Bit00110011;
	LineWord8[52]=Line8Bit00110100;
	LineWord8[53]=Line8Bit00110101;
	LineWord8[54]=Line8Bit00110110;
	LineWord8[55]=Line8Bit00110111;
	LineWord8[74]=Line8Bit01001010;
	LineWord8[75]=Line8Bit01001011;
	LineWord8[82]=Line8Bit01010010;
	LineWord8[83]=Line8Bit01010011;
	LineWord8[84]=Line8Bit01010100;
	LineWord8[85]=Line8Bit01010101;
	LineWord8[88]=Line8Bit01011000;
	LineWord8[89]=Line8Bit01011001;
	LineWord8[90]=Line8Bit01011010;
	LineWord8[91]=Line8Bit01011011;
	LineWord8[100]=Line8Bit01100100;
	LineWord8[101]=Line8Bit01100101;
	LineWord8[103]=Line8Bit01100111;
	LineWord8[104]=Line8Bit01101000;
	LineWord9[24]=Line9Bit010011000;
	LineWord9[25]=Line9Bit010011001;
	LineWord9[26]=Line9Bit010011010;
	LineWord9[27]=Line9Bit010011011;
	LineWord9[76]=Line9Bit011001100;
	LineWord9[77]=Line9Bit011001101;
	LineWord9[82]=Line9Bit011010010;
	LineWord9[83]=Line9Bit011010011;
	LineWord9[84]=Line9Bit011010100;
	LineWord9[85]=Line9Bit011010101;
	LineWord9[86]=Line9Bit011010110;
	LineWord9[87]=Line9Bit011010111;
	LineWord9[88]=Line9Bit011011000;
	LineWord9[89]=Line9Bit011011001;
	LineWord9[90]=Line9Bit011011010;
	LineWord9[91]=Line9Bit011011011;
	LineWord11[8]=Line11Bit00000001000;
	LineWord11[12]=Line11Bit00000001100;
	LineWord11[13]=Line11Bit00000001101;
	LineWord12[18]=Line12Bit000000010010;
	LineWord12[19]=Line12Bit000000010011;
	LineWord12[20]=Line12Bit000000010100;
	LineWord12[21]=Line12Bit000000010101;
	LineWord12[22]=Line12Bit000000010110;
	LineWord12[23]=Line12Bit000000010111;
	LineWord12[28]=Line12Bit000000011100;
	LineWord12[29]=Line12Bit000000011101;
	LineWord12[30]=Line12Bit000000011110;
	LineWord12[31]=Line12Bit000000011111;

	LineBord2[2]=Bine2Bit10;
	LineBord2[3]=Bine2Bit11;
	LineBord3[2]=Bine3Bit010;
	LineBord3[3]=Bine3Bit011;
	LineBord4[2]=Bine4Bit0010;
	LineBord4[3]=Bine4Bit0011;
	LineBord5[3]=Bine5Bit00011;
	LineBord6[4]=Bine6Bit000100;
	LineBord6[5]=Bine6Bit000101;
	LineBord7[4]=Bine7Bit0000100;
	LineBord7[5]=Bine7Bit0000101;
	LineBord7[7]=Bine7Bit0000111;
	LineBord8[0]=Bine8Bit00000000;
	LineBord8[4]=Bine8Bit00000100;
	LineBord8[7]=Bine8Bit00000111;
	LineBord9[24]=Bine9Bit000011000;
	LineBord10[8]=Bine10Bit0000001000;
	LineBord10[15]=Bine10Bit0000001111;
	LineBord10[23]=Bine10Bit0000010111;
	LineBord10[24]=Bine10Bit0000011000;
	LineBord10[55]=Bine10Bit0000110111;
	LineBord11[8]=Bine11Bit00000001000;
	LineBord11[12]=Bine11Bit00000001100;
	LineBord11[13]=Bine11Bit00000001101;
	LineBord11[23]=Bine11Bit00000010111;
	LineBord11[24]=Bine11Bit00000011000;
	LineBord11[40]=Bine11Bit00000101000;
	LineBord11[55]=Bine11Bit00000110111;
	LineBord11[103]=Bine11Bit00001100111;
	LineBord11[104]=Bine11Bit00001101000;
	LineBord11[108]=Bine11Bit00001101100;
	LineBord12[18]=Bine12Bit000000010010;
	LineBord12[19]=Bine12Bit000000010011;
	LineBord12[20]=Bine12Bit000000010100;
	LineBord12[21]=Bine12Bit000000010101;
	LineBord12[22]=Bine12Bit000000010110;
	LineBord12[23]=Bine12Bit000000010111;
	LineBord12[28]=Bine12Bit000000011100;
	LineBord12[29]=Bine12Bit000000011101;
	LineBord12[30]=Bine12Bit000000011110;
	LineBord12[31]=Bine12Bit000000011111;
	LineBord12[36]=Bine12Bit000000100100;
	LineBord12[39]=Bine12Bit000000100111;
	LineBord12[40]=Bine12Bit000000101000;
	LineBord12[43]=Bine12Bit000000101011;
	LineBord12[44]=Bine12Bit000000101100;
	LineBord12[51]=Bine12Bit000000110011;
	LineBord12[52]=Bine12Bit000000110100;
	LineBord12[53]=Bine12Bit000000110101;
	LineBord12[55]=Bine12Bit000000110111;
	LineBord12[56]=Bine12Bit000000111000;
	LineBord12[82]=Bine12Bit000001010010;
	LineBord12[83]=Bine12Bit000001010011;
	LineBord12[84]=Bine12Bit000001010100;
	LineBord12[85]=Bine12Bit000001010101;
	LineBord12[86]=Bine12Bit000001010110;
	LineBord12[87]=Bine12Bit000001010111;
	LineBord12[88]=Bine12Bit000001011000;
	LineBord12[89]=Bine12Bit000001011001;
	LineBord12[90]=Bine12Bit000001011010;
	LineBord12[91]=Bine12Bit000001011011;
	LineBord12[100]=Bine12Bit000001100100;
	LineBord12[101]=Bine12Bit000001100101;
	LineBord12[102]=Bine12Bit000001100110;
	LineBord12[103]=Bine12Bit000001100111;
	LineBord12[104]=Bine12Bit000001101000;
	LineBord12[105]=Bine12Bit000001101001;
	LineBord12[106]=Bine12Bit000001101010;
	LineBord12[107]=Bine12Bit000001101011;
	LineBord12[108]=Bine12Bit000001101100;
	LineBord12[109]=Bine12Bit000001101101;
	LineBord12[200]=Bine12Bit000011001000;
	LineBord12[201]=Bine12Bit000011001001;
	LineBord12[202]=Bine12Bit000011001010;
	LineBord12[203]=Bine12Bit000011001011;
	LineBord12[204]=Bine12Bit000011001100;
	LineBord12[205]=Bine12Bit000011001101;
	LineBord12[210]=Bine12Bit000011010010;
	LineBord12[211]=Bine12Bit000011010011;
	LineBord12[212]=Bine12Bit000011010100;
	LineBord12[213]=Bine12Bit000011010101;
	LineBord12[214]=Bine12Bit000011010110;
	LineBord12[215]=Bine12Bit000011010111;
	LineBord12[218]=Bine12Bit000011011010;
	LineBord12[219]=Bine12Bit000011011011;
	LineBord13[10]=Bine13Bit0000001001010;
	LineBord13[11]=Bine13Bit0000001001011;
	LineBord13[12]=Bine13Bit0000001001100;
	LineBord13[13]=Bine13Bit0000001001101;
	LineBord13[18]=Bine13Bit0000001010010;
	LineBord13[19]=Bine13Bit0000001010011;
	LineBord13[20]=Bine13Bit0000001010100;
	LineBord13[21]=Bine13Bit0000001010101;
	LineBord13[26]=Bine13Bit0000001011010;
	LineBord13[27]=Bine13Bit0000001011011;
	LineBord13[36]=Bine13Bit0000001100100;
	LineBord13[37]=Bine13Bit0000001100101;
	LineBord13[44]=Bine13Bit0000001101100;
	LineBord13[45]=Bine13Bit0000001101101;
	LineBord13[50]=Bine13Bit0000001110010;
	LineBord13[51]=Bine13Bit0000001110011;
	LineBord13[52]=Bine13Bit0000001110100;
	LineBord13[53]=Bine13Bit0000001110101;
	LineBord13[54]=Bine13Bit0000001110110;
	LineBord13[55]=Bine13Bit0000001110111;
}
