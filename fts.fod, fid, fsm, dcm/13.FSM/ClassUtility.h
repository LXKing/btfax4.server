#pragma once


#define SINGLETON_DECLARE( __TYPE__ ) \
	protected: \
		static __TYPE__* s_pInstance; \
	public: \
		static __TYPE__* Inst();

#define SINGLETON_IMPLEMENT( __TYPE__ ) \
	__TYPE__* __TYPE__::s_pInstance = NULL; \
	__TYPE__* __TYPE__::Inst() \
	{ \
		if( s_pInstance == NULL ) \
			s_pInstance = new __TYPE__(); \
		return s_pInstance; \
	}

