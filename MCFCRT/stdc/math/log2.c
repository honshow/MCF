// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_math_asm.h"

float log2f(float x){
	register float ret;
	__asm__ __volatile__(
		"fld1 \n"
		"fld dword ptr[%1] \n"
		"fyl2x \n"
		__FLT_RET_ST("%1")
		: __FLT_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

double log2(double x){
	register double ret;
	__asm__ __volatile__(
		"fld1 \n"
		"fld qword ptr[%1] \n"
		"fyl2x \n"
		__DBL_RET_ST("%1")
		: __DBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}

long double log2l(long double x){
	register long double ret;
	__asm__ __volatile__(
		"fld1 \n"
		"fld tbyte ptr[%1] \n"
		"fyl2x \n"
		__LDBL_RET_ST("%1")
		: __LDBL_RET_CONS(ret)
		: "m"(x)
	);
	return ret;
}
