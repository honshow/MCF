// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2017, LH_Mouse. All wrongs reserved.

#include "Crc32OutputStream.hpp"
#include "../Core/Array.hpp"
#include "../Core/Endian.hpp"

// http://www.relisoft.com/science/CrcOptim.html
// 1. 原文提供的是正序（权较大位向权较小位方向）的 CRC 计算，而这里使用的是反序（权较小位向权较大位方向）。
// 2. 原文的 CRC 余数的初始值是 0；此处以 -1 为初始值，计算完成后进行按位反。

// 按照 IEEE 802.3 描述的算法，除数为 0xEDB88320。

namespace MCF {

namespace {
	template<unsigned kRoundT, std::uint32_t kRegT>
	struct Generator
		: std::integral_constant<std::uint32_t, Generator<kRoundT + 1, (kRegT >> 1) ^ ((kRegT & 1) ? 0xEDB88320 : 0)>::value>
	{ };
	template<std::uint32_t kRegT>
	struct Generator<8, kRegT>
		: std::integral_constant<std::uint32_t, kRegT>
	{ };

	template<std::uint32_t ...kIndices>
	constexpr Array<std::uint32_t, sizeof...(kIndices)> GenerateTable(const std::integer_sequence<std::uint32_t, kIndices...> &) noexcept {
		return { Generator<0, kIndices>::value... };
	}

	constexpr auto kCrcTable = GenerateTable(std::make_integer_sequence<std::uint32_t, 256>());
}

Crc32OutputStream::~Crc32OutputStream(){ }

void Crc32OutputStream::X_Initialize() noexcept {
	x_u32Reg = static_cast<std::uint32_t>(-1);
}
void Crc32OutputStream::X_Update(const std::uint8_t (&abyChunk)[8]) noexcept {
	auto u64Word = LoadLe(reinterpret_cast<const std::uint64_t *>(abyChunk)[0]);
	for(unsigned uIndex = 0; uIndex < sizeof(u64Word); ++uIndex){
		const unsigned uLow = static_cast<unsigned char>(u64Word);
		u64Word >>= 8;
		x_u32Reg = kCrcTable[(x_u32Reg ^ uLow) & 0xFF] ^ (x_u32Reg >> 8);
	}
}
void Crc32OutputStream::X_Finalize(std::uint8_t (&abyChunk)[8], unsigned uBytesInChunk) noexcept {
	for(unsigned uIndex = 0; uIndex < uBytesInChunk; ++uIndex){
		const unsigned uLow = abyChunk[uIndex];
		x_u32Reg = kCrcTable[(x_u32Reg ^ uLow) & 0xFF] ^ (x_u32Reg >> 8);
	}
	x_u32Reg = ~x_u32Reg;
}

void Crc32OutputStream::Put(unsigned char byData) noexcept {
	Put(&byData, 1);
}
void Crc32OutputStream::Put(const void *pData, std::size_t uSize) noexcept {
	if(x_nChunkOffset < 0){
		X_Initialize();
		x_nChunkOffset = 0;
	}

	auto pbyRead = static_cast<const unsigned char *>(pData);
	auto uBytesRemaining = uSize;
	const auto uChunkAvail = sizeof(x_abyChunk) - static_cast<unsigned>(x_nChunkOffset);
	if(uBytesRemaining >= uChunkAvail){
		if(x_nChunkOffset != 0){
			std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uChunkAvail);
			pbyRead += uChunkAvail;
			uBytesRemaining -= uChunkAvail;
			X_Update(x_abyChunk);
			x_nChunkOffset = 0;
		}
		while(uBytesRemaining >= sizeof(x_abyChunk)){
			X_Update(reinterpret_cast<const decltype(x_abyChunk) *>(pbyRead)[0]);
			pbyRead += sizeof(x_abyChunk);
			uBytesRemaining -= sizeof(x_abyChunk);
		}
	}
	if(uBytesRemaining != 0){
		std::memcpy(x_abyChunk + x_nChunkOffset, pbyRead, uBytesRemaining);
		x_nChunkOffset += static_cast<int>(uBytesRemaining);
	}
}
void Crc32OutputStream::Flush(bool bHard) noexcept {
	(void)bHard;
}

void Crc32OutputStream::Reset() noexcept {
	x_nChunkOffset = -1;
}
std::uint32_t Crc32OutputStream::Finalize() noexcept {
	if(x_nChunkOffset >= 0){
		X_Finalize(x_abyChunk, static_cast<unsigned>(x_nChunkOffset));
	} else {
		X_Initialize();
		X_Finalize(x_abyChunk, 0);
	}
	x_nChunkOffset = -1;

	return x_u32Reg;
}

}