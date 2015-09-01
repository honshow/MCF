// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2015, LH_Mouse. All wrongs reserved.

#ifndef MCF_CORE_STRING_OBSERVER_HPP_
#define MCF_CORE_STRING_OBSERVER_HPP_

#include "../Utilities/Assert.hpp"
#include "../Utilities/CountOf.hpp"
#include <algorithm>
#include <utility>
#include <iterator>
#include <type_traits>
#include <initializer_list>
#include <functional>
#include <cstddef>

namespace MCF {

enum class StringType {
	NARROW,
	WIDE,
	UTF8,
	UTF16,
	UTF32,
	CESU8,
	ANSI,
};

template<StringType>
struct StringEncodingTrait;

template<>
struct StringEncodingTrait<StringType::NARROW> {
	using Char = char;
};
template<>
struct StringEncodingTrait<StringType::WIDE> {
	using Char = wchar_t;
};

template<>
struct StringEncodingTrait<StringType::UTF8> {
	using Char = char;
};
template<>
struct StringEncodingTrait<StringType::UTF16> {
	using Char = char16_t;
};
template<>
struct StringEncodingTrait<StringType::UTF32> {
	using Char = char32_t;
};
template<>
struct StringEncodingTrait<StringType::CESU8> {
	using Char = char;
};
template<>
struct StringEncodingTrait<StringType::ANSI> {
	using Char = char;
};

namespace Impl_StringObserver {
	enum : std::size_t {
		kNpos = static_cast<std::size_t>(-1)
	};

	template<typename CharT>
	const CharT *StrEndOf(const CharT *pszBegin) noexcept {
		ASSERT(pszBegin);

		auto pchEnd = pszBegin;
		while(*pchEnd != CharT()){
			++pchEnd;
		}
		return pchEnd;
	}

	template<typename CharT, typename IteratorT>
	std::size_t StrChrRep(IteratorT itBegin, std::common_type_t<IteratorT> itEnd,
		CharT chToFind, std::size_t uRepCount) noexcept
	{
		ASSERT(uRepCount != 0);
		ASSERT((std::size_t)(itEnd - itBegin) >= uRepCount);

		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uRepCount - 1);

		std::size_t uFound = kNpos;

		auto itCur = itBegin;
		do {
			const auto itPartBegin = std::find_if(itCur, itSearchEnd,
				[chToFind](CharT ch) noexcept { return ch == chToFind; });
			if(itPartBegin == itSearchEnd){
				break;
			}
			const auto itPartEnd = itPartBegin + (std::ptrdiff_t)uRepCount;
			itCur = std::find_if(itPartBegin, itPartEnd,
				[chToFind](CharT ch) noexcept { return ch != chToFind; });
			if(itCur == itPartEnd){
				uFound = (std::size_t)(itPartBegin - itBegin);
				break;
			}
			++itCur;
		} while(itCur < itSearchEnd);

		return uFound;
	}

	template<typename IteratorT, typename ToFindIteratorT>
	std::size_t StrStr(IteratorT itBegin, std::common_type_t<IteratorT> itEnd,
		ToFindIteratorT itToFindBegin, std::common_type_t<ToFindIteratorT> itToFindEnd) noexcept
	{
		ASSERT(itToFindEnd >= itToFindBegin);
		ASSERT(itEnd - itBegin >= itToFindEnd - itToFindBegin);

		const auto uToFindLen = (std::size_t)(itToFindEnd - itToFindBegin);
		const auto itSearchEnd = itEnd - (std::ptrdiff_t)(uToFindLen - 1);

		std::size_t *puKmpTable;

		std::size_t auSmallTable[256];
		if(uToFindLen <= COUNT_OF(auSmallTable)){
			puKmpTable = auSmallTable;
		} else {
			puKmpTable = new(std::nothrow) std::size_t[uToFindLen];
			if(!puKmpTable){
				// 内存不足，使用暴力搜索方法。
				for(auto itCur = itBegin; itCur != itSearchEnd; ++itCur){
					if(std::equal(itToFindBegin, itToFindEnd, itCur)){
						return (std::size_t)(itCur - itBegin);
					}
				}
				return kNpos;
			}
		}

		std::size_t uFound = kNpos;

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
				itToFindBegin + (std::ptrdiff_t)uToSkip, itToFindEnd, itCur + (std::ptrdiff_t)uToSkip);
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
			delete[] puKmpTable;
		}
		return uFound;
	}
}

template<StringType kTypeT>
class StringObserver {
public:
	using Char = typename StringEncodingTrait<kTypeT>::Char;

	enum : std::size_t {
		kNpos = Impl_StringObserver::kNpos
	};

	static_assert(std::is_integral<Char>::value, "Char must be an integral type.");

private:
	// 为了方便理解，想象此处使用的是所谓“插入式光标”：

	// 字符串内容：    a   b   c   d   e   f   g
	// 正光标位置：  0   1   2   3   4   5   6   7
	// 负光标位置： -8  -7  -6  -5  -4  -3  -2  -1

	// 以下均以此字符串为例。
	static std::size_t xTranslateOffset(std::ptrdiff_t nOffset, std::size_t uLength) noexcept {
		auto uRet = (std::size_t)nOffset;
		if(nOffset < 0){
			uRet += uLength + 1;
		}
		ASSERT_MSG(uRet <= uLength, L"索引越界。");
		return uRet;
	}

private:
	const Char *x_pchBegin;
	const Char *x_pchEnd;

public:
	constexpr StringObserver() noexcept
		: x_pchBegin(nullptr), x_pchEnd(nullptr)
	{
	}
	constexpr StringObserver(const Char *pchBegin, const Char *pchEnd) noexcept
		: x_pchBegin(pchBegin), x_pchEnd(pchEnd)
	{
	}
	constexpr StringObserver(std::nullptr_t, std::nullptr_t = nullptr) noexcept
		: StringObserver()
	{
	}
	constexpr StringObserver(const Char *pchBegin, std::size_t uLen) noexcept
		: x_pchBegin(pchBegin), x_pchEnd(pchBegin + uLen)
	{
	}
	constexpr StringObserver(std::initializer_list<Char> rhs) noexcept
		: StringObserver(rhs.begin(), rhs.size())
	{
	}
	explicit StringObserver(const Char *pszBegin) noexcept
		: StringObserver(pszBegin, Impl_StringObserver::StrEndOf(pszBegin))
	{
	}

public:
	const Char *GetBegin() const noexcept {
		return x_pchBegin;
	}
	const Char *GetEnd() const noexcept {
		return x_pchEnd;
	}
	std::size_t GetSize() const noexcept {
		return (std::size_t)(x_pchEnd - x_pchBegin);
	}
	std::size_t GetLength() const noexcept {
		return GetSize();
	}

	bool IsEmpty() const noexcept {
		return GetSize() == 0;
	}
	void Clear() noexcept {
		x_pchEnd = x_pchBegin;
	}

	void Swap(StringObserver &rhs) noexcept {
		std::swap(x_pchBegin, rhs.x_pchBegin);
		std::swap(x_pchEnd, rhs.x_pchEnd);
	}

	int Compare(const StringObserver &rhs) const noexcept {
		using UChar = std::make_unsigned_t<Char>;

		auto pLRead = GetBegin();
		const auto pLEnd = GetEnd();
		auto pRRead = rhs.GetBegin();
		const auto pREnd = rhs.GetEnd();
		for(;;){
			const int nLAtEnd = (pLRead == pLEnd) ? 3 : 0;
			const int nRAtEnd = (pRRead == pREnd) ? 1 : 0;
			const int nResult = 2 - (nLAtEnd ^ nRAtEnd);
			if(nResult != 2){
				return nResult;
			}

			const auto uchLhs = static_cast<UChar>(*pLRead);
			const auto uchRhs = static_cast<UChar>(*pRRead);
			if(uchLhs != uchRhs){
				return (uchLhs < uchRhs) ? -1 : 1;
			}
			++pLRead;
			++pRRead;
		}
	}

	void Assign(const Char *pchBegin, const Char *pchEnd) noexcept {
		x_pchBegin = pchBegin;
		x_pchEnd = pchEnd;
	}
	void Assign(std::nullptr_t, std::nullptr_t = nullptr) noexcept {
		x_pchBegin = nullptr;
		x_pchEnd = nullptr;
	}
	void Assign(const Char *pchBegin, std::size_t uLen) noexcept {
		Assign(pchBegin, pchBegin + uLen);
	}
	void Assign(std::initializer_list<Char> rhs) noexcept {
		Assign(rhs.begin(), rhs.end());
	}
	void Assign(const Char *pszBegin) noexcept {
		Assign(pszBegin, Impl_StringObserver::StrEndOf(pszBegin));
	}

	// 举例：
	//   Slice( 1,  5)   返回 "bcde"；
	//   Slice( 1, -5)   返回 "bc"；
	//   Slice( 5, -1)   返回 "fg"；
	//   Slice(-5, -1)   返回 "defg"。
	StringObserver Slice(std::ptrdiff_t nBegin, std::ptrdiff_t nEnd) const noexcept {
		const auto uLength = GetLength();
		return StringObserver(x_pchBegin + xTranslateOffset(nBegin, uLength), x_pchBegin + xTranslateOffset(nEnd, uLength));
	}

	// 举例：
	//   Find("def", 3)				返回 3；
	//   Find("def", 4)				返回 kNpos；
	//   FindBackward("def", 5)		返回 kNpos；
	//   FindBackward("def", 6)		返回 3。
	std::size_t Find(const StringObserver &obsToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(nBegin, uLength);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealBegin;
		}
		if(uLength < uLenToFind){
			return kNpos;
		}
		if(uRealBegin + uLenToFind > uLength){
			return kNpos;
		}
		const auto uPos = Impl_StringObserver::StrStr(GetBegin() + uRealBegin, GetEnd(), obsToFind.GetBegin(), obsToFind.GetEnd());
		if(uPos == kNpos){
			return kNpos;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindBackward(const StringObserver &obsToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(nEnd, uLength);
		const auto uLenToFind = obsToFind.GetLength();
		if(uLenToFind == 0){
			return uRealEnd;
		}
		if(uLength < uLenToFind){
			return kNpos;
		}
		if(uRealEnd < uLenToFind){
			return kNpos;
		}
		std::reverse_iterator<const Char *> itBegin(GetBegin() + uRealEnd), itEnd(GetBegin()),
			itToFindBegin(obsToFind.GetEnd()), itToFindEnd(obsToFind.GetBegin());
		const auto uPos = Impl_StringObserver::StrStr(itBegin, itEnd, itToFindBegin, itToFindEnd);
		if(uPos == kNpos){
			return kNpos;
		}
		return uRealEnd - uPos - uLenToFind;
	}

	// 举例：
	//   Find('c', 3)			返回 kNpos；
	//   Find('d', 3)			返回 3；
	//   FindBackward('c', 3)	返回 2；
	//   FindBackward('d', 3)	返回 kNpos。
	std::size_t FindRep(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nBegin = 0) const noexcept {
		const auto uLength = GetLength();
		const auto uRealBegin = xTranslateOffset(nBegin, uLength);
		if(uRepCount == 0){
			return uRealBegin;
		}
		if(uLength < uRepCount){
			return kNpos;
		}
		if(uRealBegin < uRepCount){
			return kNpos;
		}
		const auto uPos = Impl_StringObserver::StrChrRep(GetBegin() + uRealBegin, GetEnd(), chToFind, uRepCount);
		if(uPos == kNpos){
			return kNpos;
		}
		return uPos + uRealBegin;
	}
	std::size_t FindRepBackward(Char chToFind, std::size_t uRepCount, std::ptrdiff_t nEnd = -1) const noexcept {
		const auto uLength = GetLength();
		const auto uRealEnd = xTranslateOffset(nEnd, uLength);
		if(uRepCount == 0){
			return uRealEnd;
		}
		if(uLength < uRepCount){
			return kNpos;
		}
		if(uRealEnd < uRepCount){
			return kNpos;
		}
		std::reverse_iterator<const Char *> itBegin(GetBegin() + uRealEnd), itEnd(GetBegin());
		const auto uPos = Impl_StringObserver::StrChrRep(itBegin, itEnd, chToFind, uRepCount);
		if(uPos == kNpos){
			return kNpos;
		}
		return uRealEnd - uPos - uRepCount;
	}
	std::size_t Find(Char chToFind, std::ptrdiff_t nBegin = 0) const noexcept {
		return FindRep(chToFind, 1, nBegin);
	}
	std::size_t FindBackward(Char chToFind, std::ptrdiff_t nEnd = -1) const noexcept {
		return FindRepBackward(chToFind, 1, nEnd);
	}

	bool DoesOverlapWith(const StringObserver &rhs) const noexcept {
		return std::less<void>()(x_pchBegin, rhs.x_pchEnd) && std::less<void>()(rhs.x_pchBegin, x_pchEnd);
	}

public:
	explicit operator bool() const noexcept {
		return !IsEmpty();
	}
	const Char &operator[](std::size_t uIndex) const noexcept {
		ASSERT_MSG(uIndex < GetSize(), L"索引越界。");

		return GetBegin()[uIndex];
	}
};

template<StringType kTypeT>
bool operator==(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return false;
	}
	return lhs.Compare(rhs) == 0;
}
template<StringType kTypeT>
bool operator!=(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	if(lhs.GetSize() != rhs.GetSize()){
		return true;
	}
	return lhs.Compare(rhs) != 0;
}
template<StringType kTypeT>
bool operator<(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) < 0;
}
template<StringType kTypeT>
bool operator>(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) > 0;
}
template<StringType kTypeT>
bool operator<=(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) <= 0;
}
template<StringType kTypeT>
bool operator>=(const StringObserver<kTypeT> &lhs, const StringObserver<kTypeT> &rhs) noexcept {
	return lhs.Compare(rhs) >= 0;
}

template<StringType kTypeT>
void swap(StringObserver<kTypeT> &lhs, StringObserver<kTypeT> &rhs) noexcept {
	lhs.Swap(rhs);
}

template<StringType kTypeT>
auto begin(const StringObserver<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
auto cbegin(const StringObserver<kTypeT> &rhs) noexcept {
	return rhs.GetBegin();
}
template<StringType kTypeT>
auto end(const StringObserver<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}
template<StringType kTypeT>
auto cend(const StringObserver<kTypeT> &rhs) noexcept {
	return rhs.GetEnd();
}

extern template class StringObserver<StringType::NARROW>;
extern template class StringObserver<StringType::WIDE>;
extern template class StringObserver<StringType::UTF8>;
extern template class StringObserver<StringType::UTF16>;
extern template class StringObserver<StringType::UTF32>;
extern template class StringObserver<StringType::CESU8>;
extern template class StringObserver<StringType::ANSI>;

using NarrowStringObserver = StringObserver<StringType::NARROW>;
using WideStringObserver   = StringObserver<StringType::WIDE>;
using Utf8StringObserver   = StringObserver<StringType::UTF8>;
using Utf16StringObserver  = StringObserver<StringType::UTF16>;
using Utf32StringObserver  = StringObserver<StringType::UTF32>;
using Cesu8StringObserver  = StringObserver<StringType::CESU8>;
using AnsiStringObserver   = StringObserver<StringType::ANSI>;

// 字面量运算符。
// 注意 StringObserver 并不是所谓“零结尾的字符串”。
// 这些运算符经过特意设计防止这种用法。
template<typename CharT, CharT ...kCharsT>
[[deprecated("Be warned that encodings of narrow string literals vary from compilers to compilers and might even depend on encodings of source files on g++.")]]
extern inline auto operator""_nso() noexcept {
	static constexpr char s_achData[] = { kCharsT..., '$' };
	return NarrowStringObserver(s_achData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_wso() noexcept {
	static constexpr wchar_t s_awcData[] = { kCharsT..., '$' };
	return WideStringObserver(s_awcData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u8so() noexcept {
	static constexpr char s_au8cData[] = { kCharsT..., '$' };
	return Utf8StringObserver(s_au8cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u16so() noexcept {
	static constexpr char16_t s_au16cData[] = { kCharsT..., '$' };
	return Utf16StringObserver(s_au16cData, sizeof...(kCharsT));
}
template<typename CharT, CharT ...kCharsT>
extern inline auto operator""_u32so() noexcept {
	static constexpr char32_t s_au32cData[] = { kCharsT..., '$' };
	return Utf32StringObserver(s_au32cData, sizeof...(kCharsT));
}

}

using ::MCF::operator""_nso;
using ::MCF::operator""_wso;
using ::MCF::operator""_u8so;
using ::MCF::operator""_u16so;
using ::MCF::operator""_u32so;

#endif
