// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "stpcpy.h"
#include "../env/expect.h"
#include "../env/xassert.h"
#include "../stdc/string/_sse3.h"
#include "rep_movs.h"

char *_MCFCRT_stppcpy(char *s1, char *es1, const char *restrict s2){
	_MCFCRT_ASSERT(s1 + 1 <= es1);

	register char *wp = s1;
	char *const wend = es1 - 1;
	register const char *rp = s2;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & 31) != 0){
#define PCPY_GEN()	\
		{	\
			if(wp == wend){	\
				*wp = 0;	\
				return wp;	\
			}	\
			const char rc = *rp;	\
			*wp = rc;	\
			if(rc == 0){	\
				return wp;	\
			}	\
			++rp;	\
			++wp;	\
		}
		PCPY_GEN()
	}
	if((size_t)(wend - wp) >= 64){
#define PCPY_SSE3(store2_)	\
		{	\
			const __m128i xz = _mm_setzero_si128();	\
			do {	\
				const __m128i xw0 = _mm_load_si128((const __m128i *)rp + 0);	\
				const __m128i xw1 = _mm_load_si128((const __m128i *)rp + 1);	\
				__m128i xt = _mm_cmpeq_epi8(xw0, xz);	\
				uint32_t mask = (uint32_t)_mm_movemask_epi8(xt);	\
				xt = _mm_cmpeq_epi8(xw1, xz);	\
				mask += (uint32_t)_mm_movemask_epi8(xt) << 16;	\
				if(_MCFCRT_EXPECT_NOT(mask != 0)){	\
					const unsigned tz = (unsigned)__builtin_ctzl(mask);	\
					_MCFCRT_rep_movsb(wp, rp, tz);	\
					wp += tz;	\
					*wp = 0;	\
					return wp;	\
				}	\
				store2_((__m128i *)wp + 0, xw0);	\
				store2_((__m128i *)wp + 1, xw1);	\
				rp += 32;	\
				wp += 32;	\
			} while((size_t)(wend - wp) >= 32);	\
		}
		if(((uintptr_t)wp & 15) == 0){
			PCPY_SSE3(_mm_store_si128)
		} else {
			PCPY_SSE3(_mm_storeu_si128)
		}
	}
	for(;;){
		PCPY_GEN()
	}
}
