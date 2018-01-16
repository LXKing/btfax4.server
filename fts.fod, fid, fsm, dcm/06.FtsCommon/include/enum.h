// -------------------------------------------------------------------
// Header File	: enum.h
// -------------------------------------------------------------------
// Descriotion	: Btfax CommonLib 공통 : enum.h
// -------------------------------------------------------------------

#ifndef __COMMON_LIB_ENUM_H__
#define __COMMON_LIB_ENUM_H__

namespace COMMON_LIB
{

	// System Type
    enum S_TYPE
    {
        S_NONE,
        S_FCS,
        S_FOD
    };
	inline const char* SystemType(S_TYPE type)
	{
		switch(type)
		{
		case S_FCS:	return "FCS";
		case S_FOD:	return "FOD";
		}
		return "NON";
	}

    // Process Type
    enum P_TYPE
    {
        P_NONE,
        P_AOB,
        P_AOX,
        P_WRH,
        P_PRC,
        P_DCV,
        P_TPC,
        P_TMK,
        P_PSC,
		P_FSM,
        P_FOD,
        P_FID,
		P_DCM,
        P_PSI,
    };
	inline const char* ProcessType(P_TYPE type)
	{
		switch(type)
		{
		case P_AOB:	return "AOB";
        case P_AOX:	return "AOX";
        case P_WRH:	return "WRH";
        case P_PRC:	return "PRC";
        case P_DCV:	return "DCV";
        case P_TPC:	return "TPC";
        case P_TMK:	return "TMK";
        case P_PSC:	return "PSC";
		case P_FSM:	return "FSM";
        case P_FOD:	return "FOD";
        case P_FID:	return "FID";
		case P_DCM:	return "DCM";
        case P_PSI:	return "PSI";
		}

		return "NON";
	}

    // Request Item State
    enum R_STATE
    {
        /// <summary>
        /// 초기상태
        /// </summary>
        INIT = 0,

        /// <summary>
        /// 전처리 대기
        /// </summary>
        PRC_W = 1,
        /// <summary>
        /// 전처리 진행중
        /// </summary>
        PRC_R = 2,
        /// <summary>
        /// 전처리 실패
        /// </summary>
        PRC_F = 9,

        /// <summary>
        /// 변환대기
        /// </summary>
        DCV_W = 11,
        /// <summary>
        /// 변환진행중
        /// </summary>
        DCV_R = 12,
        /// <summary>
        /// 변환실패
        /// </summary>
        DCV_F = 19,

        /// <summary>
        /// 생성대기
        /// </summary>
        TMK_W = 21,
        /// <summary>
        /// 생성진행중
        /// </summary>
        TMK_R = 22,
        /// <summary>
        /// 생성실패
        /// </summary>
        TMK_F = 29,

        /// <summary>
        /// 병합추출대기
        /// </summary>
        TPC_W = 31,
        /// <summary>
        /// 병합추출진행중
        /// </summary>
        TPC_R = 32,
        /// <summary>
        /// 병합추출실패
        /// </summary>
        TPC_F = 39,

        /// <summary>
        /// 발송대기
        /// </summary>
        FOD_W = 41,
        /// <summary>
        /// 발송중
        /// </summary>
        FOD_R = 42,

        /// <summary>
        /// 변환후처리대기
        /// </summary>
        PSC_W = 71,
        /// <summary>
        /// 변환후처리진행중
        /// </summary>
        PSC_R = 72,
        /// <summary>
        /// 미리보기대기
        /// </summary>
        PSC_WP = 76,
        /// <summary>
        /// 승인대기
        /// </summary>
        PSC_WA = 77,
        /// <summary>
        /// 승인취소
        /// </summary>
        PSC_C = 87,
        /// <summary>
        /// 승인보류
        /// </summary>
        PSC_H = 88,
        /// <summary>
        /// 처리실패
        /// </summary>
        PSC_F = 89,

        /// <summary>
        /// 최종성공
        /// </summary>
        FINISH_S = 91,
        /// <summary>
        /// 최종실패
        /// </summary>
        FINISH_F = 99,

        /// <summary>
        /// 테스트완료
        /// </summary>
        FINISH_TEST = 90,
    };
    
    enum RESULT
    {
        EMPTY											= 0,
        SUCCESS											= 1,

        F_DB_ERROR										= 101,
        F_DB_OPENERROR									= 102,
        F_DB_GET_FAX_ID_ERROR							= 103,
        F_DB_GET_FAX_DETAIL_ID_ERROR					= 104,
        F_DB_BEGIN_TRANSACTION_ERROR					= 105,
        F_DB_COMMIT_TRANSACTION_ERROR					= 106,
        F_DB_ROLLBACK_TRANSACTION_ERROR					= 107,
		F_DB_UPDATE_ERROR								= 108,
		F_DB_INSERT_ERROR								= 109,

        F_DB_NOTEXIST_FAX								= 111, // LHOPE
        F_DB_NOTEXIST_FAXFORM							= 112,
        F_DB_NOTEXIST_SENDDATA							= 113,
        F_DB_NOTEXIST_SEND_EXT_SIGNFILE					= 114,
        F_DB_ADO_NOTEXPECTED_RESULT						= 121,

        F_SOCK_RECEIVE_ERROR							= 130,

        F_MAKE_XML_ERROR								= 140,

        F_FILE_CNT_ZERO									= 201,
        F_FILE_NOT_EXIST								= 202,
        F_FILE_FAIL_TO_DOWNLOAD							= 211,
        F_FILE_FAIL_TO_UPLOAD							= 212,
        F_FILE_FAIL_TO_DELETE							= 213,
        F_FILE_FAIL_TO_DOPY_FINISHED_DIR				= 214,
        
        F_DIR_FAIL_TO_CREATE_DIR						= 261,

        F_SESSION_FULL									= 301,
        
        F_PARSE_ERROR									= 321,
        F_PARSE_ERROR_PACKETSIZ_ENOTENOUGH				= 322,
        F_PARSE_ERROR_REPEAT_COUNT						= 323,
        F_PARSE_ERROR_REPEATPART						= 324,
        F_PARSE_ERROR_HEADER							= 331, // LHOPE
        F_PARSE_ERROR_BODY								= 332, // LHOPE
        
        F_SYSTEMHEADER_FIELD_INVALID					= 351,
        F_SYSTEMHEADER_FAXNO_INVALID					= 352,
        F_SYSTEMHEADER_PRIORITY_INVALID					= 353,
        F_SYSTEMHEADER_REQTYPE_INVALID					= 354,
        F_SYSTEMHEADER_RESERVEYN_INVALID				= 355,
        F_SYSTEMHEADER_RESERVEDATE_INVALID				= 356,
        F_SYSTEMHEADER_RESERVETIME_INVALID				= 357,
        F_SYSTEMHEADER_APPROVEYN_INVALID				= 358,
        F_SYSTEMHEADER_DOCPATH_INVALIDE					= 359,
        F_SYSTEMHEADER_DOCFILELIST_FORMAT_INVALID		= 360,
        F_SYSTEMHEADER_DOCFILE_FORMAT_INVALID			= 362,
        F_SYSTEMHEADER_DOCMODE_NOTSUPPORTED				= 363,
        F_SYSTEMHEADER_DOCFILE_EXTENSION_NOTSUPPORTED	= 364,
        F_SYSTEMHEADER_DOCFILE_INVALIED_DOC_INFO		= 365,
        F_SYSTEMHEADER_INVALID_CATEGORY					= 366, // LHOPE
        F_SYSTEMHEADER_INVALID_COMMAND_NO				= 367, // LHOPE
        F_SYSTEMHEADER_INVALID_COMMAND					= 368, // LHOPE

        F_MAKE                                          = 401,
        F_MAKE_NOT_EXIST_FAXFORMFILE					= 405,
        F_MAKE_IMAGEFILE_INFO_NOT_IN_PACKET				= 411,
        F_MAKE_IMAGEFILE_NOT_EXIST						= 412,
        F_MAKE_IMAGEFILE_PAGECOUNT_NOT_EXIST			= 413,
        F_MAKE_FAIL_TO_BINDING_DATA						= 421,

        F_CONV											= 501,
        F_CONV_TIMEOUT									= 502,
        F_CONV_DOCLIST_NOTEXIST_IN_DB					= 511,

        F_TIFPROCESS_PREJOB_FAIL						= 601,
        F_TIFPROCESS_PREJOB_NOTDONE						= 602,
        F_TIFPROCESS_NOT_TIFFFILE						= 603,
        F_TIFPROCESS_DOC_STATE_INVALID					= 604,
        F_TIFPROCESS_DOCLIST_NOTEXIST_IN_DB				= 611,
        F_TIFPROCESS_NOTEXIST_PAGE_COUNT				= 622,
        F_TIFFPROCESS_FAIL_TO_MERG						= 631,
        F_TIFFPROCESS_FAIL_TO_EXTRACT					= 632,
		F_FAIL_TO_DELIVER								= 641,
        

        // HmpFaxSendSync() RESULT
        F_FAX_HMP_NOT_OPENED                            = 701, // 발송오류 : 팩스 채널 안열림
        F_FAX_LICENSE                                   = 702, // 발송오류 : 팩스 라이선스 오류
        F_FAX_INVALID_CHANNEL                           = 703, // 발송오류 : 팩스 잘못된 채널
        F_FAX_RESET                                     = 704, // 발송오류 : 팩스 재설정
        F_FAX_ALREADY_OPENED                            = 705, // 발송오류 : 팩스 채널 이미 열림
        F_FAX_ALREADY_STOPPED                           = 706, // 발송오류 : 팩스 채널 이미 닫힘
        F_FAX_DEVICE_BUSY                               = 707, // 발송오류 : 통화중
        F_FAX_SOCKET_FAIL                               = 708, // 발송오류 : 팩스 소켓 오류
        F_FAX_BAD_FILE_FORMAT                           = 709, // 발송오류 : 팩스 파일 포멧오류
        F_FAX_ABORT                                     = 710, // 발송오류 : 팩스 취소
        F_FAX_OPEN_TO_STOP_ACTIVE_CHANNEL               = 711, // 발송오류 : 채널 열기위한 활성화 채널 중지
        F_FAX_CLOSE_TO_STOP_ACTIVE_CHANNEL              = 712, // 발송오류 : 채널 닫기위한 활성화 채널 중지
        F_FAX_TRANS_ERROR                               = 713, // 발송오류 : 통신오류
        F_FAX_PARTIAL_PAGE_SENT                         = 714, // 발송오류 : 일부페이지 전송
        
        // FaxState error
        F_FAX_TX_FAIL_DIS_TIMEOVER		                = 715 , // 발송오류 : 팩스신호 감지 실패
        F_FAX_TX_FAIL_TRAIN                             = 716 , // 발송오류 : 전송속도 협상 실패
        F_FAX_TX_FAIL_RTN                               = 717 , // 발송오류 : 전송속도 협상 거부
        F_FAX_TX_FAIL_CTC_ERROR                         = 718 , // 발송오류 : 전송속도 하향조절 실패
        F_FAX_TX_FAIL_CFR_TIMEOUT                       = 719 , // 발송오류 : 수신확인 대기시간 초과
        F_FAX_TX_FAIL_MCF_TIMEOUT                       = 720 , // 발송오류 : TIFF송신후 수신확인 메시지 대기시간 초과
        F_FAX_TX_FAIL_MCF_LAST_TIMEOUT                  = 721 , // 발송오류 : 마지막 TIFF송신후 수신확인 메시지 대기시간 초과
        F_FAX_TX_FAIL_CTR_TIMEOUT                       = 722 , // 발송오류 : CTC신호 전송후 CTR신호 대기시간 초과
        F_FAX_TX_FAIL_DIS_IN_IMAGE                      = 723 , // 발송오류 : TIFF 발송중 DIS(Digital Identification Signal) 신호가 옴  
        F_FAX_TX_FAIL_DIS_IN_WAITING_MCF                = 724 , // 발송오류 : MCF를 기다리는데 DIS 신호가 옴  
        F_FAX_TX_FAIL_DCN_RECV         	                = 725 , // 발송오류 : DCN신호감지
        F_FAX_TX_FAIL_LOCAL_INTERRUPT                   = 726 , // 발송오류 : (팩스 송신중) 운용자 interrupt로 실패
        F_FAX_TX_FAIL_PIN_RECV                          = 727 , // 발송오류 : procedure interrupt negative 수신  
        F_FAX_TX_FAIL_PIP_RECV                          = 728 , // 발송오류 : procedure interrupt positive 수신  
        F_FAX_TX_FAIL_FLOW_CONTROL                      = 729 , // 발송오류 : receive not ready 수신후 시간내에 receive ready가 오지 않음  
        F_FAX_TX_FAIL_STOP             	                = 730 , // 발송오류 : faxstop() 함수에 의한 종료  
        
        // sip error
        F_FAX_SIP_BAD_REQUEST                           = 731, // 400 : 잘못된요청
        F_FAX_SIP_UNAUTHORIZED                          = 732, // 401 : 권한없음
        F_FAX_SIP_PAYMENT_REQUIRED                      = 733, // 402
        F_FAX_SIP_FORBIDDEN                             = 734, // 403
        F_FAX_SIP_NOT_FOUND                             = 735, // 404 : 잘못된팩스번호
        F_FAX_SIP_METHOD_NOT_ALLOWED                    = 736, // 405 : 허용불가함수
        F_FAX_SIP_NOT_ACCEPTABLE                        = 737, // 406
        F_FAX_SIP_PROXY_AUTHENTICATION_REQUIRED         = 738, // 407
        F_FAX_SIP_REQUEST_TIMEOUT                       = 739, // 408 : 무응답
        F_FAX_SIP_CONFLICT                              = 740, // 409
        F_FAX_SIP_GONE                                  = 741, // 410
        F_FAX_SIP_LENGTH_REQUIRED                       = 742, // 411
        F_FAX_SIP_CONDITIONAL_REQUEST_FAILED            = 743, // 412
        F_FAX_SIP_REQUEST_ENTITY_TOO_LARGE              = 744, // 413
        F_FAX_SIP_REQUEST_URI_TOO_LONG                  = 745, // 414
        F_FAX_SIP_UNSUPPORTED_MEDIA_TYPE                = 746, // 415
        F_FAX_SIP_UNSUPPORTED_URI_SCHEME                = 747, // 416
        F_FAX_SIP_UNKNOWN_RESOURCE_PRIORITY             = 748, // 417
        F_FAX_SIP_BAD_EXTENSION                         = 749, // 420
        F_FAX_SIP_EXTENSION_REQUIRED                    = 750, // 421
        F_FAX_SIP_SESSION_INTERVAL_TOO_SMALL            = 751, // 422
        F_FAX_SIP_INTERVAL_TOO_BRIEF                    = 752, // 423
        F_FAX_SIP_BAD_LOCATION_INFORMATION              = 753, // 424
        F_FAX_SIP_USE_IDENTITY_HEADER                   = 754, // 428
        F_FAX_SIP_PROVIDE_REFERRER_IDENTITY             = 755, // 429
        F_FAX_SIP_FLOW_FAILED                           = 756, // 430
        F_FAX_SIP_ANONYMITY_DISALLOWED                  = 757, // 433
        F_FAX_SIP_BAD_IDENTITY_INFO                     = 758, // 436
        F_FAX_SIP_UNSUPPORTED_CERTIFICATE               = 759, // 437
        F_FAX_SIP_INVALID_IDENTITY_HEADER               = 760, // 438
        F_FAX_SIP_FIRST_HOP_LACKS_OUTBOUND_SUPPORT      = 761, // 439
        F_FAX_SIP_CONSENT_NEEDED                        = 762, // 470
        F_FAX_SIP_TEMPORARILY_UNAVAILABLE               = 763, // 480
        F_FAX_SIP_CALL_TRANSACTION_DOES_NOT_EXIST       = 764, // 481
        F_FAX_SIP_LOOP_DETECTED                         = 765, // 482
        F_FAX_SIP_TOO_MANY_HOPS                         = 766, // 483
        F_FAX_SIP_ADDRESS_INCOMPLETE                    = 767, // 484
        F_FAX_SIP_AMBIGUOUS                             = 768, // 485
        F_FAX_SIP_BUSY_HERE                             = 769, // 486 : 통화중
        F_FAX_SIP_REQUEST_TERMINATED                    = 770, // 487 : 무응답 - 통화연결 안됨
        F_FAX_SIP_NOT_ACCEPTABLE_HERE                   = 771, // 488
        F_FAX_SIP_BAD_EVENT                             = 772, // 489
        F_FAX_SIP_REQUEST_PENDING                       = 773, // 491
        F_FAX_SIP_UNDECIPHERABLE                        = 774, // 493
        F_FAX_SIP_SECURITY_AGREEMENT_REQUIRED           = 775, // 494
        F_FAX_SIP_SERVER_INTERNAL_ERROR                 = 776, // 500 : MGW내부오류
        F_FAX_SIP_NOT_IMPLEMENTED                       = 777, // 501
        F_FAX_SIP_BAD_GATEWAY                           = 778, // 502
        F_FAX_SIP_SERVICE_UNAVAILABLE                   = 779, // 503 : MGW서비스불가
        F_FAX_SIP_SERVER_TIME_OUT                       = 780, // 504
        F_FAX_SIP_VERSION_NOT_SUPPORTED                 = 781, // 505
        F_FAX_SIP_MESSAGE_TOO_LARGE                     = 782, // 513
        F_FAX_SIP_PRECONDITION_FAILURE                  = 783, // 580
        

		//F_CHANNEL_HUNT_ERROR							= 710, // 가용채널 점유 에러
		//F_FAX_HMP_NOT_OPENED							= 711, // 팩스 채널 안열림
		//F_FAX_LICENSE									= 712, // 팩스 라이선스 오류
		//F_FAX_INVALID_CHANNEL							= 713, // 팩스 잘못된 채널
		//F_FAX_RESET										= 714, // 팩스 재설정
		//F_FAX_ALREADY_OPENED							= 715, // 팩스 채널 이미 열림
		//F_FAX_ALREADY_STOPPED							= 716, // 팩스 채널 이미 닫힘
		//F_FAX_BUSY										= 717, // 통화중
		//F_FAX_SOCKET_FAIL								= 718, // 팩스 소켓 오류
		//F_FAX_BAD_FILE_FORMAT							= 719, // 팩스 파일 포멧오류
		//F_FAX_ABORT										= 720, // 팩스 취소
		//F_FAX_OPEN_TO_STOP_ACTIVE_CHANNEL				= 721, // 채널 열기위한 활성화 채널 중지
		//F_FAX_CLOSE_TO_STOP_ACTIVE_CHANNEL				= 722, // 채널 닫기위한 활성화 채널 중지
		//F_FAX_TRANS_ERROR								= 723, // 통신오류
		//F_FAX_NOT_ACCESS_NO								= 724, // 잘못된 번호
		//F_FAX_PARTIAL_TRANSFER							= 725, // 부분전송

		//F_FAX_NOT_FAX									= 731, 	// 팩스 아님             
		//F_FAX_NOANSWER									= 732, 	// 무응답              
		//F_FAX_REJECT									= 733, 	// 팩스 수신 거부        
		//F_FAX_INVALID_BITRATE							= 734, 	// 팩스 전송 불가
		//F_FAX_CFR_TIMEOUT								= 735, 	// 무응답            
		//F_FAX_MCF_TIMEOUT								= 736, 	// 무응답            
		//F_FAX_LAST_MCF_TIMEOUT							= 737, 	// 무응답      
		//F_FAX_CTR_TIMEOUT								= 738, 	// 무응답            
		//F_FAX_DIS_IN_IMAGE								= 739, 	// 통신 오류       
		//F_FAX_DIS_IN_WATTING_MCF						= 740, 	// 통신 오류 
		//F_FAX_DCN_RECV									= 741, 	// 팩스 연결 종료      
		//F_FAX_LOCAL_INTERRUPT							= 742, 	// 팩스 연결 종료
		//F_FAX_PIN_RECV									= 743, 	// 팩스 연결 종료      
		//F_FAX_PIP_RECV									= 744, 	// 팩스 연결 종료      
		//F_FAX_FLOW_CONTROL								= 745, 	// 무응답          
		//F_FAX_FAXSTOP									= 746, 	// 팩스 연결 종료        
		//F_FAX_CALL										= 747,	// 다이얼 오류
		//
        F_UPLOAD										= 801,	// LHOPE - 삭제 필요
		F_BLOCKED_FAX_NO								= 802,	// 송신차단 번호
		F_RECV_FAX_ERROR								= 821,	// 수신오류
        F_SYSTEM_ERROR									= 901
    };

	enum EN_IO_FLAG
	{
		IO_NONE = 0,
		IO_OUTBOUND,
		IO_INBOUND
	};

	enum EN_STATUS
	{
		FAX_INIT = -1,										// 초기상태
		FAX_IDLE,											// HmpFaxOpenSync()를 Call한 상태
		FAX_OCCUPY,											// FAX 발송전 CH 점유 상태
		FAX_DIAL,											// FAX DIALING 중
		FAX_SEND,											// FAX 발송중 상태
		FAX_RECV,											// FAX 수신중 상태F
		FAX_SUCC_SEND,										// FAX 발송 성공 상태
		FAX_FAIL_SEND,										// FAX 발송 실패 상태
		FAX_SUCC_RECV,										// FAX 수신 성공 상태
		FAX_FAIL_RECV,										// FAX 수신 실패 상태
		FAX_ABORT											// FAX Abort

	};
}


#endif
