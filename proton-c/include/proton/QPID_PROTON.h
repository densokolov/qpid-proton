#ifndef qpid_proton_H
#define qpid_proton_H 1
#include <malloc.h>
#ifdef	SWIGWIN
	#define QPID_PROTON_API  
	#define QPID_PROTON_PY  
#else
	#ifdef  _WINDOWS
		#ifdef qpid_proton_EXPORTS
			#define QPID_PROTON_API __declspec(dllexport)
			#ifdef qpid_proton_python_EXPORTS
				#define QPID_PROTON_PY  __declspec(dllexport)
			#else
				#define QPID_PROTON_PY 
			#endif
		#else
			#define QPID_PROTON_API __declspec(dllimport)
			#ifdef qpid_proton_python_IMPORTS
				#define QPID_PROTON_PY	__declspec(dllimport)
			#else
				#define QPID_PROTON_PY
			#endif
		#endif
	#else
		#define QPID_PROTON_API
	#endif
#endif


#ifdef C99
	   #define PN_VLA(TYPE, buf, size)  TYPE buf[size]
	   #define PN_VLA_FREE
#else

		#define PN_VLA(TYPE, buf, size) TYPE *buf = (TYPE*) _malloca((size) * sizeof(TYPE))
		#define PN_VLA_FREE(buf) 		_freea(buf)	
#endif

#endif