#ifndef BTFAX_CONVERT_H
#define BTFAX_CONVERT_H


#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	Sc_Err_File_Open=-9,
	Sc_Err_Decode_Tiff,
	Sc_Err_Decode_Ifd,
	Sc_Err_Unsupported_Encoding,
	Sc_Convert_Success=0,
	Sc_Nothing_To_Convert,
}sc_convert_error;


void	InitHmpTiffConvert();
int	HmpTiffConvert2High(char * infile, char * outfile);


#ifdef __cplusplus
}
#endif

#endif
