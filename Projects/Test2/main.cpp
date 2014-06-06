#include <MCF/StdMCF.hpp>
#include <MCF/Core/Exception.hpp>
#include <iostream>
using namespace std;
using namespace MCF;

unsigned int MCFMain()
try {
	double d1 = 1.0, d2 = 2.0;
	BSwap(d1, d2);
	cout <<d1 << ',' <<d2 <<endl;

	return 0;
} catch(exception &e){
	printf("exception '%s'\n", e.what());
	auto *const p = dynamic_cast<const Exception *>(&e);
	if(p){
		printf("  err  = %lu\n", p->m_ulErrorCode);
		printf("  desc = %s\n", AnsiString(GetWin32ErrorDesc(p->m_ulErrorCode)).GetCStr());
		printf("  func = %s\n", p->m_pszFunction);
		printf("  line = %lu\n", p->m_ulLine);
		printf("  msg  = %s\n", AnsiString(WideString(p->m_wcsMessage)).GetCStr());
	}
	return 0;
}
