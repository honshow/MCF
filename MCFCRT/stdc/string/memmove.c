// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"
#include "_string_asm.h"

void *memmove(void *dst, const void *src, size_t cb){
	uintptr_t unused;
	__asm__ __volatile__(
		"cmp " __RSI ", " __RDI " \n"
		"jbe 5f \n"
		"	cmp %2, 64 \n"
		"	jb 1f \n"
		"		mov " __RCX ", " __RDI " \n"
		"		neg " __RCX " \n"
		"		and " __RCX ", 0x0F \n"
		"		sub %2, " __RCX " \n"
		"		rep movsb \n"
		"		mov " __RCX ", %2 \n"
		"		shr " __RCX ", 4 \n"
		"		and %2, 0x0F \n"
		"		cmp " __RCX ", 64 * 1024 * 16 \n" // 16 MiB
		"		jb 4f \n"
		"			test " __RSI ", 0x0F \n"
		"			jnz 3f \n"
		"				2: \n"
		"				movdqa xmm0, xmmword ptr[" __RSI "] \n"
		"				movntdq xmmword ptr[" __RDI "], xmm0 \n"
		"				add " __RSI ", 16 \n"
		"				dec " __RCX " \n"
		"				lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"				jnz 2b \n"
		"				jmp 1f \n"
		"			3: \n"
		"			movdqu xmm0, xmmword ptr[" __RSI "] \n"
		"			movntdq xmmword ptr[" __RDI "], xmm0 \n"
		"			add " __RSI ", 16 \n"
		"			dec " __RCX " \n"
		"			lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"			jnz 3b \n"
		"			jmp 1f \n"
		"		4: \n"
		"		test " __RSI ", 0x0F \n"
		"		jnz 3f \n"
		"			2: \n"
		"			movdqa xmm0, xmmword ptr[" __RSI "] \n"
		"			movdqa xmmword ptr[" __RDI "], xmm0 \n"
		"			add " __RSI ", 16 \n"
		"			dec " __RCX " \n"
		"			lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"			jnz 2b \n"
		"			jmp 1f \n"
		"		3: \n"
		"		movdqu xmm0, xmmword ptr[" __RSI "] \n"
		"		movdqa xmmword ptr[" __RDI "], xmm0 \n"
		"		add " __RSI ", 16 \n"
		"		dec " __RCX " \n"
		"		lea " __RDI ", dword ptr[" __RDI " + 16] \n"
		"		jnz 3b \n"
		"	1: \n"
		"	mov " __RCX ", %2 \n"
#ifdef _WIN64
		"	shr rcx, 3 \n"
		"	rep movsq \n"
		"	mov rcx, %2 \n"
		"	and rcx, 7 \n"
#else
		"	shr ecx, 2 \n"
		"	rep movsd \n"
		"	mov ecx, %2 \n"
		"	and ecx, 3 \n"
#endif
		"	rep movsb \n"
		"	jmp 6f \n"
		"	.align 16 \n"
		"5: \n"
		"je 6f \n"
		"std \n"
		"cmp %2, 64 \n"
		"lea " __RSI ", dword ptr[" __RSI " + %2] \n"
		"lea " __RDI ", dword ptr[" __RDI " + %2] \n"
		"jb 1f \n"
		"	lea " __RSI ", dword ptr[" __RSI " - 1] \n"
		"	mov " __RCX ", " __RDI " \n"
		"	and " __RCX ", 0x0F \n"
		"	lea " __RDI ", dword ptr[" __RDI " - 1] \n"
		"	sub %2, " __RCX " \n"
		"	rep movsb \n"
		"	lea " __RSI ", dword ptr[" __RSI " + 1] \n"
		"	mov " __RCX ", %2 \n"
		"	shr " __RCX ", 4 \n"
		"	lea " __RDI ", dword ptr[" __RDI " + 1] \n"
		"	and %2, 0x0F \n"
		"	cmp " __RCX ", 64 * 1024 * 16 \n" // 16 MiB
		"	jb 4f \n"
		"		test " __RSI ", 0x0F \n"
		"		jnz 3f \n"
		"			2: \n"
		"			movdqa xmm0, xmmword ptr[" __RSI " - 16] \n"
		"			movntdq xmmword ptr[" __RDI " - 16], xmm0 \n"
		"			sub " __RSI ", 16 \n"
		"			dec " __RCX " \n"
		"			lea " __RDI ", dword ptr[" __RDI " - 16] \n"
		"			jnz 2b \n"
		"			jmp 1f \n"
		"		3: \n"
		"		movdqu xmm0, xmmword ptr[" __RSI " - 16] \n"
		"		movntdq xmmword ptr[" __RDI " - 16], xmm0 \n"
		"		sub " __RSI ", 16 \n"
		"		dec " __RCX " \n"
		"		lea " __RDI ", dword ptr[" __RDI " - 16] \n"
		"		jnz 3b \n"
		"		jmp 1f \n"
		"	4: \n"
		"	test " __RSI ", 0x0F \n"
		"	jnz 3f \n"
		"		2: \n"
		"		movdqa xmm0, xmmword ptr[" __RSI " - 16] \n"
		"		movdqa xmmword ptr[" __RDI " - 16], xmm0 \n"
		"		sub " __RSI ", 16 \n"
		"		dec " __RCX " \n"
		"		lea " __RDI ", dword ptr[" __RDI " - 16] \n"
		"		jnz 2b \n"
		"		jmp 1f \n"
		"	3: \n"
		"	movdqu xmm0, xmmword ptr[" __RSI " - 16] \n"
		"	movdqa xmmword ptr[" __RDI " - 16], xmm0 \n"
		"	sub " __RSI ", 16 \n"
		"	dec " __RCX " \n"
		"	lea " __RDI ", dword ptr[" __RDI " - 16] \n"
		"	jnz 3b \n"
		"1: \n"
		"mov " __RCX ", %2 \n"
#ifdef _WIN64
		"lea rsi, dword ptr[rsi - 8] \n"
		"shr rcx, 3 \n"
		"lea rdi, dword ptr[rdi - 8] \n"
		"rep movsq \n"
		"lea rsi, dword ptr[rsi + 7] \n"
		"mov rcx, %2 \n"
		"and rcx, 7 \n"
		"lea rdi, dword ptr[" __RDI " + 7] \n"
#else
		"lea esi, dword ptr[esi - 4] \n"
		"shr ecx, 2 \n"
		"lea edi, dword ptr[edi - 4] \n"
		"rep movsd \n"
		"lea esi, dword ptr[esi + 3] \n"
		"mov ecx, %2 \n"
		"and ecx, 3 \n"
		"lea edi, dword ptr[edi + 3] \n"
#endif
		"rep movsb \n"
		"cld \n"
		"6: \n"
		: "=D"(unused), "=S"(unused), "=r"(unused)
		: "0"(dst), "1"(src), "2"(cb)
		: "cx", "xmm0"
	);
	return dst;
}
