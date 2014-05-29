// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2014. LH_Mouse. All wrongs reserved.

#ifndef MCF_STRING_OBSERVER_HPP_
#define MCF_STRING_OBSERVER_HPP_

#include "../../MCFCRT/ext/assert.h"
#include "VVector.hpp"
#include "Utilities.hpp"
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <cstddef>

namespace MCF {

template<typename Char_t>
class StringObserver;

template<typename Char_t>
class StringObserver {
public:
	enum : std::size_t {
		NPOS = (std::size_t)-1
	};

private:
	static const Char_t *xEndOf(const Char_t *pszBegin) noexcept {
		if(pszBegin == nullptr){
			return nullptr;
		}

		const Char_t *pchEnd = pszBegin;
		while(*pchEnd != Char_t()){
			++pchEnd;
		}
		return pchEnd;
	}

	static std::size_t xTranslateOffset(std::size_t uLength, std::ptrdiff_t nRaw) noexcept {
		std::ptrdiff_t nOffset = nRaw;
		if(nOffset < 0){
			nOffset += (std::ptrdiff_t)(uLength + 1);
		}

		ASSERT_MSG(nOffset >= 0, L"索引越界。");
		ASSERT_MSG((std::size_t)nOffset <= uLength, L"索引越界。");

		return (std::size_t)nOffset;
	}

	template<typename Iterator_t>
	static std::size_t xFindRep(const Iterator_t &itBegin, const Iterator_t &itEnd, Char_t chToFind, std::size_t uRepCount) noexcept {
		ASSERT(uRepCount != 0);
		ASSERT((std::size_t)(itEnd - itBegin) >= uRepCount);

		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uRepCount - 1);

		std::size_t uFound = NPOS;

		auto itCur = itBegin;
		do {
			const auto itPartBegin = std::find_if(itCur, itSearchEnd, [chToFind](Char_t ch) noexcept { return ch == chToFind; });
			if(itPartBegin == itSearchEnd){
				break;
			}
			const auto itPartEnd = itPartBegin + (std::ptrdiff_t)uRepCount;
			itCur = std::find_if(itPartBegin, itPartEnd, [chToFind](Char_t ch) noexcept { return ch != chToFind; });
			if(itCur == itPartEnd){
				uFound = (std::size_t)(itPartBegin - itBegin);
				break;
			}
			++itCur;
		} while(itCur < itSearchEnd);

		return uFound;
	}

	template<typename Iterator_t>
	static std::size_t xKmpSearch(
		const Iterator_t &itBegin,
		const Iterator_t &itEnd,
		const Iterator_t &itToFindBegin,
		const Iterator_t &itToFindEnd
	) noexcept {
		ASSERT(itToFindEnd >= itToFindBegin);
		ASSERT(itEnd - itBegin >= itToFindEnd - itToFindBegin);

		const auto uToFindLen = (std::size_t)(itToFindEnd - itToFindBegin);
		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uToFindLen - 1);

		std::size_t *puKmpTable;

		std::size_t auSmallTable[256];
		if(uToFindLen <= COUNT_OF(auSmallTable)){
			puKmpTable = auSmallTable;
		} else {
			puKmpTable = (std::size_t *)::operator new(sizeof(std::size_t) * uToFindLen, std::nothrow);
			if(!puKmpTable){
				// 内存不足，使用暴力搜索方法。
				for(auto itCur = itBegin; itCur != itSearchEnd; ++itCur){
					if(std::equal(itToFindBegin, itToFindEnd, itCur)){
						return (std::size_t)(itCur - itBegin);
					}
				}
				return NPOS;
			}
		}

		std::size_t uFound = NPOS;

		puKmpTable[0] = 0;
		puKmpTable[1] = 0;

		std::size_t uPos = 2;
		std::size_t uCand = 0;
		while(uPos < uToFindLen){
			if(itToFindBegin[(std::ptrdiff_t)(uPos - 1)] == itToFindBegin[(std::ptrdiff_t)uCand]){
				puKmpTable[uPos++] = ++uCand;
			} else if(uCand != 0){
				uCand = puKmpTable[uCand];
			} else {
				puKmpTable[uPos++] = 0;
			}
		}

		auto itCur = itBegin;
		std::size_t uToSkip = 0;
		do {
			const auto vResult = std::mismatch(
				itToFindBegin + (std::ptrdiff_t)uToSkip,
				itToFindEnd,
				itCur + (std::ptrdiff_t)uToSkip
			);
			if(vResult.first == itToFindEnd){
				uFound = (std::size_t)(itCur - itBegin);
				break;
			}
			auto uDelta = (std::size_t)(vResult.first - itToFindBegin);
			uToSkip = puKmpTable[uDelta];
			uDelta -= uToSkip;
			uDelta += (std::size_t)(*vResult.second != *itToFindBegin);
			itCur += (std::ptrdiff_t)uDelta;
		} while(itCur < itSearchEnd);

		if(puKmpTable != auSmallTable){
			::operator delete(puKmpTable);
		}
		return uFound;
	}

private:
	const Char_t *xm_pchBegin;
	const Char_t *xm_pchEnd;

public:
#ifdef NDEBUG
	constexpr
#endif
	StringObserver(const Char_t *pchBegin, const Char_t *pchEnd) noexcept
		: xm_pchBegin(pchBegin), xm_pchEnd(pchEnd)
	{
#ifndef NDEBUG
		ASSERT(pchBegin <= pchEnd);
#endif
	}
	constexpr StringObserver() noexcept
		: StringObserver((const Char_t *)nullptr, nullptr)
	{
	}
	constexpr StringObserver(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringObserver()
	{
	}
	constexpr StringObserver(const Char_t *pchBegin, std::size_t uLen) noexcept
		: StringObserver(pchBegin, pchBegin + uLen)
	{
	}
	constexpr StringObserver(std::initializer_list<Char_t> vInitList) noexcept
		: StringObserver(vInitList.begin(), vInitList.end())
	{
	}
	template<typename Test_t, typename = typename std::enable_if<std::is_same<Test_t, Char_t>::value>::type>
	constexpr StringObserver(const Test_t *const &pszBegin) noexcept
		: StringObserver(pszBegin, xEndOf(pszBegin))
	{
	}
	template<std::size_t N>
	constexpr StringObserver(const Char_t (&achLiteral)[N]) noexcept
		: StringObserver(achLiteral, achLiteral + N - 1)
	{
	}
	StringObserver &operator=(std::nullptr_t) noexcept {
		xm_pchBegin = nullptr;
		xm_pchEnd = nullptr;
		return *this;
	}
	StringObserver &operator=(std::initializer_list<Char_t> vInitList) noexcept {
		xm_pchBegin = vInitList.begin();
		xm_pchEnd = vInitList.end();
		return *this;
	}
	template<typename Test_t, typename = typename std::enable_if<std::is_same<Test_t, Char_t>::value>::type>
	StringObserver &operator=(const Test_t *const &pszBegin) noexcept {
		xm_pchBegin = pszBegin;
		xm_pchEnd = xEndOf(pszBegin);
		return *this;
	}
	template<std::size_t N>
	StringObserver &operator=(const Char_t (&achLiteral)[N]) noexcept {
		xm_pchBegin = achLiteral;
		xm_pchEnd = achLiteral + N - 1;
		return *this;
	}

public:
	const Char_t *GetBegin() const noexcept {
		return xm_pchBegin;
	}
	const Char_t *GetCBegin() const noexcept {
		return xm_pchBegin;
	}
	const Char_t *GetEnd() const noexcept {
		return xm_pchEnd;
	}
	const Char_t *GetCEnd() const noexcept {
		return xm_pchEnd;
	}
	std::size_t GetSize() const noexcept {
		return (std::size_t)(GetEnd() - GetBegin());
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		Assign(nullptr, nullptr);
	}

	void Swap(StringObserver &rhs) noexcept {
		std::swap(xm_pchBegin, rhs.xm_pchBegin);
		std::swap(xm_pchEnd, rhs.xm_pchEnd);
	}

	int Compare(const StringObserver &rhs) const noexcept {
		auto itLRead = GetBegin();
		const auto itLEnd = GetEnd();
		auto itRRead = rhs.GetBegin();
		const auto itREnd = rhs.GetEnd();
		for(;;){
			const int nResult = 2 - (((itLRead == itLEnd) ? 3 : 0) ^ ((itRRead == itREnd) ? 1 : 0));
			if(nResult != 2){
				return nResult;
			}

			typedef typename std::make_unsigned<Char_t>::type UChar;

			const auto uchL = (UChar)*itLRead;
			const auto uchR = (UChar)*itRRead;
			if(uchL != uchR){
				return (uchL < uchR) ? -1 : 1;
			}
			++itLRead;
			++itRRead;
		}
	}

	void Assign(const Char_t *pchBegin, const Char_t *pchEnd) noexcept {
		ASSERT(pchBegin <= pchEnd);

		xm_pchBegin = pchBegin;
		xm_pchEnd = pchEnd;
	}
	void Assign(std::nullptr_t, std::nullptr_t = nullptr) noexcept {
		Assign((const Char_t *)nullptr, nullptr);
	}
	void Assign(const Char_t *pchBegin, std::size_t uLen) noexcept {
		Assign(pchBegin, pchBegin + uLen);
	}
	template<typename Test_t, typename = typename std::enable_if<std::is_same<Test_t, Char_t>::value>::type>
	void Assign(const Test_t *const &pszBegin) noexcept {
		Assign(pszBegin, xEndOf(pszBegin));
	}
	template<std::size_t N>
	void Assign(const Char_t (&achLiteral)[N]) noexcept {
		Assign(achLiteral, achLiteral + N - 1);
	}
	template<std::size_t N>
	void Assign(Char_t (&achNonLiteral)[N]) noexcept {
		Assign(achNonLiteral, xEndOf(achNonLiteral));
	}

	// 为了方便理解，想象此处使用的是所谓“插入式光标”：

	// 字符串内容：    a   b   c   d   e   f   g  \0
	// 正光标位置：  0   1   2   3   4   5   6   7
	// 负光标位置： -8  -7  -6  -5  -4  -3  -2  -1

	// 以下均以此字符串为例。

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringObserver Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto pchBegin = GetBegin();
		const auto uLength = GetLength();
		return StringObserver(
			pchBegin + xTranslateOffset(uLength, nBegin),
			pchBegin + xTranslateOffset(uLength, nEnd)
		);
	}

	// 举例：
	//   Find("def", 3)				返回 3；
	//   Find("def", 4)				返回 NPOS；
	//   FindBackward("def", 5)		返回 NPOS；
	//   FindBackward("def", 6)		返回 3。
	std::size_t Find(const StringObserver &obsToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealBegin;
		}
		if(uLength < uLenToFind){
			return NPOS;
		}
		if(uRealBegin + uLenToFind > uLength){
			return NPOS;
		}

		const auto uPos = xKmpSearch(
			GetBegin() + uRealBegin,
			GetEnd(),
			obsToFind.GetBegin(),
			obsToFind.GetEnd()
		);
		return (uPos == NPOS) ? NPOS : (uPos + uRealBegin);
	}
	std::size_t FindBackward(const StringObserver &obsToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealEnd;
		}
		if(uLength < uLenToFind){
			return NPOS;
		}
		if(uRealEnd < uLenToFind){
			return NPOS;
		}

		typedef std::reverse_iterator<const Char_t *> RevIterator;

		const auto uPos = xKmpSearch(
			RevIterator(GetBegin() + uRealEnd),
			RevIterator(GetBegin()),
			RevIterator(obsToFind.GetBegin()),
			RevIterator(obsToFind.GetEnd())
		);
		return (uPos == NPOS) ? NPOS : (uRealEnd - uPos - uLenToFind);
	}

	// 举例：
	//   Find('c', 3)			返回 NPOS；
	//   Find('d', 3)			返回 3；
	//   FindBackward('c', 3)	返回 2；
	//   FindBackward('d', 3)	返回 NPOS。
	std::size_t FindRep(Char_t chToFind, std::size_t uRepCount, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(uLength, nOffsetBegin);
		if(uRepCount == 0){
			return uRealBegin;
		}
		if(uLength < uRepCount){
			return NPOS;
		}
		if(uRealBegin + uRepCount > uLength){
			return NPOS;
		}

		const auto uPos = xFindRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uRepCount);
		return (uPos == NPOS) ? NPOS : (uPos + uRealBegin);
	}
	std::size_t FindRepBackward(Char_t chToFind, std::size_t uRepCount, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(uLength, nOffsetEnd);
		if(uRepCount == 0){
			return uRealEnd;
		}
		if(uLength < uRepCount){
			return NPOS;
		}
		if(uRealEnd < uRepCount){
			return NPOS;
		}

		typedef std::reverse_iterator<const Char_t *> RevIterator;

		const auto uPos = xFindRep(RevIterator(GetBegin() + uRealEnd), RevIterator(GetBegin()), chToFind, uRepCount);
		return (uPos == NPOS) ? NPOS : (uRealEnd - uPos - uRepCount);
	}
	std::size_t Find(Char_t chToFind, std::ptrdiff_t nOffsetBegin = 0) const noexcept {
		return FindRep(chToFind, 1, nOffsetBegin);
	}
	std::size_t FindBackward(Char_t chToFind, std::ptrdiff_t nOffsetEnd = -1) const noexcept {
		return FindRepBackward(chToFind, 1, nOffsetEnd);
	}

	bool DoesOverlapWith(const StringObserver &obs) const noexcept {
		const Char_t *pchBegin1, *pchEnd1, *pchBegin2, *pchEnd2;
		if(xm_pchBegin <= xm_pchEnd){
			pchBegin1 = xm_pchBegin;
			pchEnd1 = xm_pchEnd;
		} else {
			pchBegin1 = xm_pchEnd + 1;
			pchEnd1 = xm_pchBegin + 1;
		}
		if(obs.xm_pchBegin <= obs.xm_pchEnd){
			pchBegin2 = obs.xm_pchBegin;
			pchEnd2 = obs.xm_pchEnd;
		} else {
			pchBegin2 = obs.xm_pchEnd + 1;
			pchEnd2 = obs.xm_pchBegin + 1;
		}
		return (pchBegin1 < pchEnd2) && (pchBegin2 < pchEnd1);
	}

	template<std::size_t SIZE_HINT>
	VVector<Char_t, SIZE_HINT> GetNullTerminated() const {
		VVector<Char_t, SIZE_HINT> vecRet;
		vecRet.Reserve(GetLength() + 1);
		vecRet.CopyToEnd(GetBegin(), GetEnd());
		vecRet.Push(Char_t());
		return std::move(vecRet);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char_t &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex <= GetSize(), L"索引越界。");

		return GetBegin()[uIndex];
	}

	bool operator==(const StringObserver &rhs) const noexcept {
		if(GetLength() != rhs.GetLength()){
			return false;
		}
		return Compare(rhs) == 0;
	}
	bool operator!=(const StringObserver &rhs) const noexcept {
		if(GetLength() != rhs.GetLength()){
			return true;
		}
		return Compare(rhs) != 0;
	}
	bool operator<(const StringObserver &rhs) const noexcept {
		return Compare(rhs) < 0;
	}
	bool operator>(const StringObserver &rhs) const noexcept {
		return Compare(rhs) > 0;
	}
	bool operator<=(const StringObserver &rhs) const noexcept {
		return Compare(rhs) <= 0;
	}
	bool operator>=(const StringObserver &rhs) const noexcept {
		return Compare(rhs) >= 0;
	}
};

template<typename Comparand_t, typename Char_t>
bool operator==(const Comparand_t &lhs, const StringObserver<Char_t> &rhs) noexcept {
	return rhs == lhs;
}
template<typename Comparand_t, typename Char_t>
bool operator!=(const Comparand_t &lhs, const StringObserver<Char_t> &rhs) noexcept {
	return rhs != lhs;
}
template<typename Comparand_t, typename Char_t>
bool operator<(const Comparand_t &lhs, const StringObserver<Char_t> &rhs) noexcept {
	return rhs > lhs;
}
template<typename Comparand_t, typename Char_t>
bool operator>(const Comparand_t &lhs, const StringObserver<Char_t> &rhs) noexcept {
	return rhs < lhs;
}
template<typename Comparand_t, typename Char_t>
bool operator<=(const Comparand_t &lhs, const StringObserver<Char_t> &rhs) noexcept {
	return rhs >= lhs;
}
template<typename Comparand_t, typename Char_t>
bool operator>=(const Comparand_t &lhs, const StringObserver<Char_t> &rhs) noexcept {
	return rhs <= lhs;
}

template<typename Char_t>
const Char_t *begin(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetBegin();
}
template<typename Char_t>
const Char_t *cbegin(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetCBegin();
}

template<typename Char_t>
const Char_t *end(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetEnd();
}
template<typename Char_t>
const Char_t *cend(const StringObserver<Char_t> &obs) noexcept {
	return obs.GetCEnd();
}

template class StringObserver<char>;
template class StringObserver<wchar_t>;

template class StringObserver<char16_t>;
template class StringObserver<char32_t>;

typedef StringObserver<char>		NarrowStringObserver;
typedef StringObserver<wchar_t>		WideStringObserver;

typedef StringObserver<char>		Utf8StringObserver;
typedef StringObserver<char16_t>	Utf16StringObserver;
typedef StringObserver<char32_t>	Utf32StringObserver;

}

constexpr inline const MCF::NarrowStringObserver operator""_NSO(const char *pchStr, std::size_t uLength) noexcept {
	return MCF::NarrowStringObserver(pchStr, uLength);
}
constexpr inline const MCF::WideStringObserver operator""_WSO(const wchar_t *pwchStr, std::size_t uLength) noexcept {
	return MCF::WideStringObserver(pwchStr, uLength);
}

constexpr inline const MCF::Utf8StringObserver operator""_U8SO(const char *pchStr, std::size_t uLength) noexcept {
	return MCF::Utf8StringObserver(pchStr, uLength);
}
constexpr inline const MCF::Utf16StringObserver operator""_U16SO(const char16_t *pc16Str, std::size_t uLength) noexcept {
	return MCF::Utf16StringObserver(pc16Str, uLength);
}
constexpr inline const MCF::Utf32StringObserver operator""_U32SO(const char32_t *pc32Str, std::size_t uLength) noexcept {
	return MCF::Utf32StringObserver(pc32Str, uLength);
}

#endif
