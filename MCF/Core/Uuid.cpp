// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../StdMCF.hpp"
#include "Uuid.hpp"
#include "Exception.hpp"
#include "Clocks.hpp"
#include "Random.hpp"
#include "../Utilities/Endian.hpp"
#include "../Thread/Atomic.hpp"

namespace MCF {

namespace {
	const auto g_u16Pid = (std::uint16_t)::GetCurrentProcessId();

	Atomic<std::uint32_t> g_u32AutoInc(0);
}

// 静态成员函数。
Uuid Uuid::Generate(){
	const auto u64Now = GetUtcClock();
	const auto u32Unique = g_u16Pid | ((g_u32AutoInc.Increment(kAtomicRelaxed) << 16) & 0x3FFFFFFFu);

	Uuid vRet;
	StoreBe(vRet.x_unData.au32[0], (std::uint32_t)(u64Now >> 12));
	StoreBe(vRet.x_unData.au16[2], (std::uint16_t)((u64Now << 4) | (u32Unique >> 26)));
	StoreBe(vRet.x_unData.au16[3], (std::uint16_t)((u32Unique >> 14) & 0x0FFFu)); // 版本 = 0
	StoreBe(vRet.x_unData.au16[4], (std::uint16_t)(0xC000u | (u32Unique & 0x3FFFu))); // 变种 = 3
	StoreBe(vRet.x_unData.au16[5], (std::uint16_t)GetRandomUint32());
	StoreBe(vRet.x_unData.au32[3], (std::uint32_t)GetRandomUint32());
	return vRet;
}

// 构造函数和析构函数。
Uuid::Uuid(const Array<char, 36> &achHex){
	if(!Scan(achHex)){
		DEBUG_THROW(Exception, ERROR_INVALID_PARAMETER, "Invalid UUID string"_rcs);
	}
}

// 其他非静态成员函数。
void Uuid::Print(Array<char, 36> &achHex, bool bUpperCase) const noexcept {
	auto pbyRead = x_unData.aby;
	auto pchWrite = achHex.GetData();

#define PRINT(count_)	\
	for(std::size_t i = 0; i < count_; ++i){	\
		const unsigned uByte = *(pbyRead++);	\
		unsigned uChar = uByte >> 4;	\
		if(uChar <= 9){	\
			uChar += '0';	\
		} else if(bUpperCase){	\
			uChar += 'A' - 0x0A;	\
		} else {	\
			uChar += 'a' - 0x0A;	\
		}	\
		*(pchWrite++) = (char)uChar;	\
		uChar = uByte & 0x0F;	\
		if(uChar <= 9){	\
			uChar += '0';	\
		} else if(bUpperCase){	\
			uChar += 'A' - 0x0A;	\
		} else {	\
			uChar += 'a' - 0x0A;	\
		}	\
		*(pchWrite++) = (char)uChar;	\
	}

	PRINT(4) *(pchWrite++) = '-';
	PRINT(2) *(pchWrite++) = '-';
	PRINT(2) *(pchWrite++) = '-';
	PRINT(2) *(pchWrite++) = '-';
	PRINT(6)
}
bool Uuid::Scan(const Array<char, 36> &achHex) noexcept {
	unsigned char abyTemp[16];

	auto pchRead = achHex.GetData();
	auto pbyWrite = abyTemp;

#define SCAN(count_)	\
	for(std::size_t i = 0; i < count_; ++i){	\
		unsigned uByte;	\
		unsigned uChar = (unsigned char)*(pchRead++);	\
		if(('0' <= uChar) && (uChar <= '9')){	\
			uChar -= '0';	\
		} else if(('A' <= uChar) && (uChar <= 'F')){	\
			uChar -= 'A' - 0x0A;	\
		} else if(('a' <= uChar) && (uChar <= 'f')){	\
			uChar -= 'a' - 0x0A;	\
		} else {	\
			return false;	\
		}	\
		uByte = uChar << 4;	\
		uChar = (unsigned char)*(pchRead++);	\
		if(('0' <= uChar) && (uChar <= '9')){	\
			uChar -= '0';	\
		} else if(('A' <= uChar) && (uChar <= 'F')){	\
			uChar -= 'A' - 0x0A;	\
		} else if(('a' <= uChar) && (uChar <= 'f')){	\
			uChar -= 'a' - 0x0A;	\
		} else {	\
			return false;	\
		}	\
		uByte |= uChar;	\
		*(pbyWrite++) = (unsigned char)uByte;	\
	}

	SCAN(4) if(*(pchRead++) != '-'){ return false; }
	SCAN(2) if(*(pchRead++) != '-'){ return false; }
	SCAN(2) if(*(pchRead++) != '-'){ return false; }
	SCAN(2) if(*(pchRead++) != '-'){ return false; }
	SCAN(6)

	BCopy(x_unData, abyTemp);
	return true;
}

}
