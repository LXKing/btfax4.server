#pragma once



void printSIPMessage( BTSIPMessage* pMessage, bool bReceived = false, bool bDisplayLHOPE = false );
void printSDPMessage( BTSDPMessage* pSDPMessage, bool bDisplayLHOPE = false  );

void setLocation( CString& p_strLocation, const char* p_szIp, int p_nPort, const char* p_szUser, const char* p_szName = NULL );
void setUri( CString& p_strUri, const char* p_szIp, int p_nPort, const char* p_szUser );
void setFrom( char* strFrom, const char* strURI, const char* strName );
void setTo( char* strTo, const char* strURI, const char* strName );
void setContact( BTSIPMessage* pMessage, const char* strURI, int nPort = 0 );
void setTimestamp( BTSIPMessage* pMessage, int nTimestamp );
void setExpires( BTSIPMessage* pMessage, int nExpires );
void setSupported( BTSIPMessage* pMessage, const char* strSupported );
void setAllowed( BTSIPMessage* pMessage );
bool checkMethod( BTSIPMessage* pMessage );


void	HexaDump(int chan, int len, uchar * buf);

