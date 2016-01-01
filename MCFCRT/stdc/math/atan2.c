// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float atan2f(float y, float x){
	register float ret;
	__asm__ __volatile__(
		"fld dword ptr[%1] \n"
		"fld dword ptr[%2] \n"
		"fpatan \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(y), "m"(x)
	);
	return ret;
}

double atan2(double y, double x){
	register double ret;
	__asm__ __volatile__(
		"fld qword ptr[%1] \n"
		"fld qword ptr[%2] \n"
		"fpatan \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(y), "m"(x)
	);
	return ret;
}

long double atan2l(long double y, long double x){
	register long double ret;
	__asm__ __volatile__(
		"fld tbyte ptr[%1] \n"
		"fld tbyte ptr[%2] \n"
		"fpatan \n"
		__LDBL_RET_ST("%1")
		: __LDBL_RET_CONS(ret)
		: "m"(y), "m"(x)
	);
	return ret;
}
