// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013. LH_Mouse. All wrongs reserved.

#ifndef __MCF_CRT_THREAD_H__
#define __MCF_CRT_THREAD_H__

#include "_crtdef.h"

__MCF_EXTERN_C_BEGIN

extern unsigned long __MCF_CRT_TlsEnvInitialize(void);
extern void __MCF_CRT_TlsEnvUninitialize(void);

extern unsigned long __MCF_CRT_ThreadInitialize(void);
extern void __MCF_CRT_ThreadUninitialize(void);

extern void *__MCF_CreateCRTThread(
	unsigned int (*pfnProc)(__MCF_STD intptr_t),
	__MCF_STD intptr_t nParam,
	unsigned long ulFlags,
	unsigned long *pulThreadId
);

extern int __MCF_AtCRTThreadExit(
	void (*pfnProc)(__MCF_STD intptr_t),
	__MCF_STD intptr_t nContext
);

extern void *__MCF_CRT_RetrieveTls(
	__MCF_STD intptr_t nKey,
	__MCF_STD size_t uSizeToAlloc,
	void (*pfnConstructor)(void *, __MCF_STD intptr_t),
	__MCF_STD intptr_t nParam,
	void (*pfnDestructor)(void *)
);
extern void __MCF_CRT_DeleteTls(
	__MCF_STD intptr_t nKey
);

__MCF_EXTERN_C_END

#endif
