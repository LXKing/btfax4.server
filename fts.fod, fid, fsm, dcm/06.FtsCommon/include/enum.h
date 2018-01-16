// -------------------------------------------------------------------
// Header File	: enum.h
// -------------------------------------------------------------------
// Descriotion	: Btfax CommonLib ���� : enum.h
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
        /// �ʱ����
        /// </summary>
        INIT = 0,

        /// <summary>
        /// ��ó�� ���
        /// </summary>
        PRC_W = 1,
        /// <summary>
        /// ��ó�� ������
        /// </summary>
        PRC_R = 2,
        /// <summary>
        /// ��ó�� ����
        /// </summary>
        PRC_F = 9,

        /// <summary>
        /// ��ȯ���
        /// </summary>
        DCV_W = 11,
        /// <summary>
        /// ��ȯ������
        /// </summary>
        DCV_R = 12,
        /// <summary>
        /// ��ȯ����
        /// </summary>
        DCV_F = 19,

        /// <summary>
        /// �������
        /// </summary>
        TMK_W = 21,
        /// <summary>
        /// ����������
        /// </summary>
        TMK_R = 22,
        /// <summary>
        /// ��������
        /// </summary>
        TMK_F = 29,

        /// <summary>
        /// ����������
        /// </summary>
        TPC_W = 31,
        /// <summary>
        /// ��������������
        /// </summary>
        TPC_R = 32,
        /// <summary>
        /// �����������
        /// </summary>
        TPC_F = 39,

        /// <summary>
        /// �߼۴��
        /// </summary>
        FOD_W = 41,
        /// <summary>
        /// �߼���
        /// </summary>
        FOD_R = 42,

        /// <summary>
        /// ��ȯ��ó�����
        /// </summary>
        PSC_W = 71,
        /// <summary>
        /// ��ȯ��ó��������
        /// </summary>
        PSC_R = 72,
        /// <summary>
        /// �̸�������
        /// </summary>
        PSC_WP = 76,
        /// <summary>
        /// ���δ��
        /// </summary>
        PSC_WA = 77,
        /// <summary>
        /// �������
        /// </summary>
        PSC_C = 87,
        /// <summary>
        /// ���κ���
        /// </summary>
        PSC_H = 88,
        /// <summary>
        /// ó������
        /// </summary>
        PSC_F = 89,

        /// <summary>
        /// ��������
        /// </summary>
        FINISH_S = 91,
        /// <summary>
        /// ��������
        /// </summary>
        FINISH_F = 99,

        /// <summary>
        /// �׽�Ʈ�Ϸ�
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
        F_FAX_HMP_NOT_OPENED                            = 701, // �߼ۿ��� : �ѽ� ä�� �ȿ���
        F_FAX_LICENSE                                   = 702, // �߼ۿ��� : �ѽ� ���̼��� ����
        F_FAX_INVALID_CHANNEL                           = 703, // �߼ۿ��� : �ѽ� �߸��� ä��
        F_FAX_RESET                                     = 704, // �߼ۿ��� : �ѽ� �缳��
        F_FAX_ALREADY_OPENED                            = 705, // �߼ۿ��� : �ѽ� ä�� �̹� ����
        F_FAX_ALREADY_STOPPED                           = 706, // �߼ۿ��� : �ѽ� ä�� �̹� ����
        F_FAX_DEVICE_BUSY                               = 707, // �߼ۿ��� : ��ȭ��
        F_FAX_SOCKET_FAIL                               = 708, // �߼ۿ��� : �ѽ� ���� ����
        F_FAX_BAD_FILE_FORMAT                           = 709, // �߼ۿ��� : �ѽ� ���� �������
        F_FAX_ABORT                                     = 710, // �߼ۿ��� : �ѽ� ���
        F_FAX_OPEN_TO_STOP_ACTIVE_CHANNEL               = 711, // �߼ۿ��� : ä�� �������� Ȱ��ȭ ä�� ����
        F_FAX_CLOSE_TO_STOP_ACTIVE_CHANNEL              = 712, // �߼ۿ��� : ä�� �ݱ����� Ȱ��ȭ ä�� ����
        F_FAX_TRANS_ERROR                               = 713, // �߼ۿ��� : ��ſ���
        F_FAX_PARTIAL_PAGE_SENT                         = 714, // �߼ۿ��� : �Ϻ������� ����
        
        // FaxState error
        F_FAX_TX_FAIL_DIS_TIMEOVER		                = 715 , // �߼ۿ��� : �ѽ���ȣ ���� ����
        F_FAX_TX_FAIL_TRAIN                             = 716 , // �߼ۿ��� : ���ۼӵ� ���� ����
        F_FAX_TX_FAIL_RTN                               = 717 , // �߼ۿ��� : ���ۼӵ� ���� �ź�
        F_FAX_TX_FAIL_CTC_ERROR                         = 718 , // �߼ۿ��� : ���ۼӵ� �������� ����
        F_FAX_TX_FAIL_CFR_TIMEOUT                       = 719 , // �߼ۿ��� : ����Ȯ�� ���ð� �ʰ�
        F_FAX_TX_FAIL_MCF_TIMEOUT                       = 720 , // �߼ۿ��� : TIFF�۽��� ����Ȯ�� �޽��� ���ð� �ʰ�
        F_FAX_TX_FAIL_MCF_LAST_TIMEOUT                  = 721 , // �߼ۿ��� : ������ TIFF�۽��� ����Ȯ�� �޽��� ���ð� �ʰ�
        F_FAX_TX_FAIL_CTR_TIMEOUT                       = 722 , // �߼ۿ��� : CTC��ȣ ������ CTR��ȣ ���ð� �ʰ�
        F_FAX_TX_FAIL_DIS_IN_IMAGE                      = 723 , // �߼ۿ��� : TIFF �߼��� DIS(Digital Identification Signal) ��ȣ�� ��  
        F_FAX_TX_FAIL_DIS_IN_WAITING_MCF                = 724 , // �߼ۿ��� : MCF�� ��ٸ��µ� DIS ��ȣ�� ��  
        F_FAX_TX_FAIL_DCN_RECV         	                = 725 , // �߼ۿ��� : DCN��ȣ����
        F_FAX_TX_FAIL_LOCAL_INTERRUPT                   = 726 , // �߼ۿ��� : (�ѽ� �۽���) ����� interrupt�� ����
        F_FAX_TX_FAIL_PIN_RECV                          = 727 , // �߼ۿ��� : procedure interrupt negative ����  
        F_FAX_TX_FAIL_PIP_RECV                          = 728 , // �߼ۿ��� : procedure interrupt positive ����  
        F_FAX_TX_FAIL_FLOW_CONTROL                      = 729 , // �߼ۿ��� : receive not ready ������ �ð����� receive ready�� ���� ����  
        F_FAX_TX_FAIL_STOP             	                = 730 , // �߼ۿ��� : faxstop() �Լ��� ���� ����  
        
        // sip error
        F_FAX_SIP_BAD_REQUEST                           = 731, // 400 : �߸��ȿ�û
        F_FAX_SIP_UNAUTHORIZED                          = 732, // 401 : ���Ѿ���
        F_FAX_SIP_PAYMENT_REQUIRED                      = 733, // 402
        F_FAX_SIP_FORBIDDEN                             = 734, // 403
        F_FAX_SIP_NOT_FOUND                             = 735, // 404 : �߸����ѽ���ȣ
        F_FAX_SIP_METHOD_NOT_ALLOWED                    = 736, // 405 : ���Ұ��Լ�
        F_FAX_SIP_NOT_ACCEPTABLE                        = 737, // 406
        F_FAX_SIP_PROXY_AUTHENTICATION_REQUIRED         = 738, // 407
        F_FAX_SIP_REQUEST_TIMEOUT                       = 739, // 408 : ������
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
        F_FAX_SIP_BUSY_HERE                             = 769, // 486 : ��ȭ��
        F_FAX_SIP_REQUEST_TERMINATED                    = 770, // 487 : ������ - ��ȭ���� �ȵ�
        F_FAX_SIP_NOT_ACCEPTABLE_HERE                   = 771, // 488
        F_FAX_SIP_BAD_EVENT                             = 772, // 489
        F_FAX_SIP_REQUEST_PENDING                       = 773, // 491
        F_FAX_SIP_UNDECIPHERABLE                        = 774, // 493
        F_FAX_SIP_SECURITY_AGREEMENT_REQUIRED           = 775, // 494
        F_FAX_SIP_SERVER_INTERNAL_ERROR                 = 776, // 500 : MGW���ο���
        F_FAX_SIP_NOT_IMPLEMENTED                       = 777, // 501
        F_FAX_SIP_BAD_GATEWAY                           = 778, // 502
        F_FAX_SIP_SERVICE_UNAVAILABLE                   = 779, // 503 : MGW���񽺺Ұ�
        F_FAX_SIP_SERVER_TIME_OUT                       = 780, // 504
        F_FAX_SIP_VERSION_NOT_SUPPORTED                 = 781, // 505
        F_FAX_SIP_MESSAGE_TOO_LARGE                     = 782, // 513
        F_FAX_SIP_PRECONDITION_FAILURE                  = 783, // 580
        

		//F_CHANNEL_HUNT_ERROR							= 710, // ����ä�� ���� ����
		//F_FAX_HMP_NOT_OPENED							= 711, // �ѽ� ä�� �ȿ���
		//F_FAX_LICENSE									= 712, // �ѽ� ���̼��� ����
		//F_FAX_INVALID_CHANNEL							= 713, // �ѽ� �߸��� ä��
		//F_FAX_RESET										= 714, // �ѽ� �缳��
		//F_FAX_ALREADY_OPENED							= 715, // �ѽ� ä�� �̹� ����
		//F_FAX_ALREADY_STOPPED							= 716, // �ѽ� ä�� �̹� ����
		//F_FAX_BUSY										= 717, // ��ȭ��
		//F_FAX_SOCKET_FAIL								= 718, // �ѽ� ���� ����
		//F_FAX_BAD_FILE_FORMAT							= 719, // �ѽ� ���� �������
		//F_FAX_ABORT										= 720, // �ѽ� ���
		//F_FAX_OPEN_TO_STOP_ACTIVE_CHANNEL				= 721, // ä�� �������� Ȱ��ȭ ä�� ����
		//F_FAX_CLOSE_TO_STOP_ACTIVE_CHANNEL				= 722, // ä�� �ݱ����� Ȱ��ȭ ä�� ����
		//F_FAX_TRANS_ERROR								= 723, // ��ſ���
		//F_FAX_NOT_ACCESS_NO								= 724, // �߸��� ��ȣ
		//F_FAX_PARTIAL_TRANSFER							= 725, // �κ�����

		//F_FAX_NOT_FAX									= 731, 	// �ѽ� �ƴ�             
		//F_FAX_NOANSWER									= 732, 	// ������              
		//F_FAX_REJECT									= 733, 	// �ѽ� ���� �ź�        
		//F_FAX_INVALID_BITRATE							= 734, 	// �ѽ� ���� �Ұ�
		//F_FAX_CFR_TIMEOUT								= 735, 	// ������            
		//F_FAX_MCF_TIMEOUT								= 736, 	// ������            
		//F_FAX_LAST_MCF_TIMEOUT							= 737, 	// ������      
		//F_FAX_CTR_TIMEOUT								= 738, 	// ������            
		//F_FAX_DIS_IN_IMAGE								= 739, 	// ��� ����       
		//F_FAX_DIS_IN_WATTING_MCF						= 740, 	// ��� ���� 
		//F_FAX_DCN_RECV									= 741, 	// �ѽ� ���� ����      
		//F_FAX_LOCAL_INTERRUPT							= 742, 	// �ѽ� ���� ����
		//F_FAX_PIN_RECV									= 743, 	// �ѽ� ���� ����      
		//F_FAX_PIP_RECV									= 744, 	// �ѽ� ���� ����      
		//F_FAX_FLOW_CONTROL								= 745, 	// ������          
		//F_FAX_FAXSTOP									= 746, 	// �ѽ� ���� ����        
		//F_FAX_CALL										= 747,	// ���̾� ����
		//
        F_UPLOAD										= 801,	// LHOPE - ���� �ʿ�
		F_BLOCKED_FAX_NO								= 802,	// �۽����� ��ȣ
		F_RECV_FAX_ERROR								= 821,	// ���ſ���
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
		FAX_INIT = -1,										// �ʱ����
		FAX_IDLE,											// HmpFaxOpenSync()�� Call�� ����
		FAX_OCCUPY,											// FAX �߼��� CH ���� ����
		FAX_DIAL,											// FAX DIALING ��
		FAX_SEND,											// FAX �߼��� ����
		FAX_RECV,											// FAX ������ ����F
		FAX_SUCC_SEND,										// FAX �߼� ���� ����
		FAX_FAIL_SEND,										// FAX �߼� ���� ����
		FAX_SUCC_RECV,										// FAX ���� ���� ����
		FAX_FAIL_RECV,										// FAX ���� ���� ����
		FAX_ABORT											// FAX Abort

	};
}


#endif
