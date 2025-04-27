/**************************************************************
 * MIT License
 *
 * Copyright (c) 2022 Dugy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **************************************************************/
// See https://github.com/Dugy/EzGz

#ifndef EZGZ_HPP
#define EZGZ_HPP

#if defined(_MSC_VER)
static_assert(_MSVC_LANG == __cplusplus, "MSVC requires /Zc:__cplusplus for correct __cplusplus macro value");
#endif
static_assert(__cplusplus >= 201703L, "C++17 or higher is required");

#define EZGZ_HAS_CPP20 (__cplusplus >= 202002L)
#define EZGZ_HAS_CONCEPTS (__cpp_concepts >= 201907L)

#include <array>
#include <cstring>
#if EZGZ_HAS_CPP20
#include <span>
#include <bit>
#endif
#include <algorithm>
#include <charconv>
#include <vector>
#include <numeric>
#include <optional>
#include <functional>
#include <variant>
#include <memory>
#include <istream>
#include <chrono>
#ifndef EZGZ_NO_FILE
#include <fstream>
#endif

#if ! EZGZ_HAS_CPP20
namespace std {

// Custom span-like class for C++17
template <typename T>
class span {
public:
	span() : ptr(nullptr), length(0) {}
	span(T* ptr, std::size_t size) : ptr(ptr), length(size) {}
	span(T* begin, T* end) : ptr(begin), length(std::distance(begin, end)) {}
	template <typename It>
	span(It begin, It end) : ptr(begin == end ? nullptr : &(*begin)), length(std::distance(begin, end)) {}
	template <typename Source, std::enable_if_t<std::is_convertible_v<decltype(std::declval<Source>().data()), T*> && std::is_integral_v<decltype(std::declval<Source>().size())>>* = nullptr>
	span(Source& source) : ptr(source.data()), length(source.size()) {}

	template <std::size_t N>
	span(std::array<std::remove_const_t<T>, N>& arr) : ptr(arr.data()), length(N) {}
	template <std::size_t N>
	span(const std::array<const T, N>& arr) : ptr(arr.data()), length(N) {}

	T* data() { return ptr; }
	const T* data() const { return ptr; }

	std::size_t size() const { return length; }
 
	using iterator = T*;
    using const_iterator = const T*;

	iterator begin() { return ptr; }
	const_iterator begin() const { return ptr; }

	iterator end() { return ptr + length; }
	const_iterator end() const { return ptr + length; }

    span subspan(std::size_t offset, std::size_t count = std::size_t(-1)) const {
		if (offset >= length) {
            return span(); // Return an empty span if offset is out of bounds
        }
		return span(ptr + offset, std::min(count, length - offset));
    }

	const T& operator[](std::size_t index) const { return ptr[index]; }
	T& operator[](std::size_t index) { return ptr[index]; }

private:
	T* ptr;
	std::size_t length;
};

template <typename T>
constexpr auto ssize(const T& container) -> std::ptrdiff_t {
    return static_cast<std::ptrdiff_t>(container.size());
}

// Fallback for raw arrays (e.g., T[] or T[N])
template <typename T, std::size_t N>
constexpr std::ptrdiff_t ssize(const T(&)[N]) {
    return static_cast<std::ptrdiff_t>(N);
}

template <typename T>
constexpr int bit_width(T number) {
	static_assert(std::is_integral<T>::value && std::is_unsigned<T>::value,
		"bit_width requires an unsigned integral type");

   int result = 0;
   for (; number > 0; number >>= 1, result++);
   return result;
}
} // end namespace std
#endif // ! EZGZ_HAS_CPP20

namespace EzGz {

#if EZGZ_HAS_CPP20
static_assert(std::endian::native == std::endian::big || std::endian::native == std::endian::little);
constexpr bool IsBigEndian = std::endian::native == std::endian::big;
#elif defined(_WIN32) || defined(_WIN64)
constexpr bool IsBigEndian = false;
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
constexpr bool IsBigEndian = (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
#elif defined(__LITTLE_ENDIAN__)
constexpr bool IsBigEndian = false;
#elif defined(__BIG_ENDIAN__)
constexpr bool IsBigEndian = true;
#else
static_assert(false, "compiler does not support endian check");
#endif

#if EZGZ_HAS_CONCEPTS
template <typename T>
concept StreamSettings = requires() {
	int(T::minSize);
	int(T::maxSize);
};

template <typename T>
concept InputStreamSettings = StreamSettings<T> && requires() {
	int(T::lookAheadSize);
};

template <typename T>
concept DecompressionSettings = std::constructible_from<typename T::Checksum> && InputStreamSettings<typename T::Input>
			&& StreamSettings<typename T::Output> && requires(typename T::Checksum checksum) {
	int(checksum());
	int(checksum(std::span<const uint8_t>()));
	bool(T::verifyChecksum);
};
#else
#define StreamSettings typename
#define InputStreamSettings typename
#define DecompressionSettings typename
#endif

#if EZGZ_HAS_CONCEPTS
template <typename T>
concept BasicStringType = std::constructible_from<T> && requires(T value) {
	value += 'a';
	std::string_view(value);
};
#else
#define BasicStringType typename
#endif

struct NoChecksum { // Noop
	int operator() () { return 0; }
	int operator() (std::span<const uint8_t>) { return 0; }
};

struct MinDecompressionSettings {
	struct Output {
		constexpr static int maxSize = 32768 * 2 + 258;
		constexpr static int minSize = std::min(32768, maxSize / 2); // Max offset + max copy size
	};
	struct Input {
		constexpr static int lookAheadSize = sizeof(uint32_t);
		constexpr static int maxSize = 33000;
		constexpr static int minSize = 0;
	};

	using Checksum = NoChecksum;
	constexpr static bool verifyChecksum = false;
	using StringType = std::string;
};

namespace Detail {

constexpr std::array<uint32_t, 256> generateBasicCrc32LookupTable() {
	constexpr uint32_t reversedPolynomial = 0xedb88320;
	std::array<uint32_t, 256> result = {};
	for (int i = 0; i < std::ssize(result); i++) {
		result[i] = i;
		for (auto j = 0; j < 8; j++)
			result[i] = (result[i] >> 1) ^ ((result[i] & 0x1) * reversedPolynomial);
	}
	return result;
}

constexpr std::array<uint32_t, 256> basicCrc32LookupTable = generateBasicCrc32LookupTable();

static constexpr std::array<uint32_t, 256> generateNextCrc32LookupTableSlice(const std::array<uint32_t, 256>& previous) {
	std::array<uint32_t, 256> result = {};
	for (int i = 0; i < std::ssize(result); i++) {
		result[i] = (previous[i] >> 8) ^ (basicCrc32LookupTable[previous[i] & 0xff]);
	}
	return result;
}

template <int Slice>
struct CrcLookupTable {
	constexpr static std::array<uint32_t, 256> data = generateNextCrc32LookupTableSlice(CrcLookupTable<Slice - 1>::data);
};

template <>
struct CrcLookupTable<0> {
	constexpr static const std::array<uint32_t, 256> data = basicCrc32LookupTable;
};
}

class LightCrc32 {
	uint32_t state = 0xffffffffu;

public:
	uint32_t operator() () { return ~state; }
	uint32_t operator() (std::span<const uint8_t> input) {
		for (auto it : input) {
			const uint8_t tableIndex = uint8_t(state ^ it);
			state = (state >> 8) ^ Detail::basicCrc32LookupTable[tableIndex];
		}
		return ~state; // Invert all bits at the end
	}
};

// Inspired by https://create.stephan-brumme.com/crc32/
class FastCrc32 {
	uint32_t state = 0xffffffffu;

	constexpr static int chunkSize = 16;
	struct CodingChunk {
		std::array<uint8_t, 16> chunk;

		constexpr static std::array<const std::array<uint32_t, 256>, chunkSize> lookupTables = {
			Detail::CrcLookupTable<0>::data, Detail::CrcLookupTable<1>::data, Detail::CrcLookupTable<2>::data, Detail::CrcLookupTable<3>::data,
			Detail::CrcLookupTable<4>::data, Detail::CrcLookupTable<5>::data, Detail::CrcLookupTable<6>::data, Detail::CrcLookupTable<7>::data,
			Detail::CrcLookupTable<8>::data, Detail::CrcLookupTable<9>::data, Detail::CrcLookupTable<10>::data, Detail::CrcLookupTable<11>::data,
			Detail::CrcLookupTable<12>::data, Detail::CrcLookupTable<13>::data, Detail::CrcLookupTable<14>::data, Detail::CrcLookupTable<15>::data};

		template <int Index = 0, int Size = chunkSize>
		uint32_t process() {
			if constexpr(Size == 1) {
				return lookupTables[chunkSize - 1 - Index][chunk[Index]];
			} else {
				return process<Index, Size / 2>() ^ process<Index + Size / 2, Size / 2>();
			}
		}
	};

public:
	uint32_t operator() () { return ~state; }
	uint32_t operator() (std::span<const uint8_t> input) {
		ptrdiff_t position = 0;
		for ( ; position + chunkSize < std::ssize(input); position += chunkSize) {
			CodingChunk chunk;
			memcpy(chunk.chunk.data() + sizeof(state), input.data() + position + sizeof(state), chunkSize - sizeof(state));
			union {
				std::array<uint8_t, sizeof(state)> bytes;
				uint32_t number = 0;
			} stateBytes;
			stateBytes.number = state;
			if constexpr (IsBigEndian) {
				for (int i = 0; i < std::ssize(stateBytes.bytes) / 2; i++) {
					std::swap(stateBytes.bytes[i], stateBytes.bytes[std::ssize(stateBytes.bytes) - 1 - i]);
				}
			}
			stateBytes.number ^= *reinterpret_cast<const uint32_t*>(input.data() + position);
			memcpy(chunk.chunk.data(), reinterpret_cast<const uint8_t*>(&stateBytes.number), sizeof(state));

			state = chunk.process();
		}

		for ( ; position < std::ssize(input); position++) {
			const uint8_t tableIndex = uint8_t(state ^ input[position]);
			state = (state >> 8) ^ Detail::basicCrc32LookupTable[tableIndex];
		}
		return ~state; // Invert all bits at the end
	}
};

struct DefaultDecompressionSettings : MinDecompressionSettings {
	struct Output {
		constexpr static int maxSize = 100000;
		constexpr static int minSize = std::min(32768, maxSize / 2); // Max offset + max copy size
	};
	struct Input {
		constexpr static int lookAheadSize = sizeof(uint32_t);
		constexpr static int maxSize = 100000;
		constexpr static int minSize = 0;
	};

	using Checksum = FastCrc32;
	constexpr static bool verifyChecksum = true;
};

namespace Detail {

template <typename Builder>
struct ArrayFiller {
	Builder builder;
	using BuildingType = decltype(builder(0));
	constexpr ArrayFiller(Builder&& builder) : builder(std::move(builder)) {}

	template <size_t Size>
	constexpr operator std::array<BuildingType, Size>() {
		return build(std::make_index_sequence<Size>());
	}
private:
	template <size_t... Indexes>
	constexpr std::array<BuildingType, sizeof...(Indexes)> build(std::index_sequence<Indexes...>) {
		return std::array<BuildingType, sizeof...(Indexes)>{builder(Indexes)...};
	}
};

static constexpr std::array<uint8_t, 19> codeCodingReorder = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
static constexpr std::array<uint8_t, 19> codeCodingReorderInverse = {3, 17, 15, 13, 11, 9, 7, 5, 4, 6, 8, 10, 12, 14, 16, 18, 0, 1, 2};

static constexpr std::array<uint8_t, 256> reversedBytes = ArrayFiller([] (int uninverted) constexpr {
	return uint8_t((uninverted * uint64_t(0x0202020202) & uint64_t(0x010884422010)) % 0x3ff); // Reverse a byte
});

class ByteInput {
	std::span<uint8_t> buffer = {};
	std::function<int(std::span<uint8_t> batch)> readMore;
	int position = 0;
	int filled = 0;
	ptrdiff_t positionStart = 0;
	int minSize = 0;
	int lookAheadSize = 0;

	void ensureSize(int bytes) {
		while (position + bytes + lookAheadSize > filled) [[unlikely]] {
			int refilled = refillSome(readMore);
			if (refilled == 0 && lookAheadSize == 0) {
				throw std::runtime_error("Unexpected end of stream");
			}
		}
	}

public:

	ByteInput(std::span<uint8_t> buffer, std::function<int(std::span<uint8_t> batch)> readMoreFunction, int minSize, int lookAheadSize)
		: buffer(buffer), readMore(std::move(readMoreFunction)), minSize(minSize), lookAheadSize(lookAheadSize) {}

	// Note: May not get as many bytes as necessary, would need to be called multiple times
	template <typename ByteType = uint8_t>
	std::span<const ByteType> getRange(int size) {
		if (position + size + lookAheadSize > filled) {
			refillSome(readMore);
		}
		ptrdiff_t start = position;
		int available = std::min<int>(size, int(filled - start));
		position += available;
		return {reinterpret_cast<const ByteType*>(buffer.data()) + start, size_t(available)};
	}

	bool hasMoreDataInBuffer() const {
		return position + lookAheadSize < filled;
	}

	uint64_t getBytes(int amount) {
		return getInteger<int64_t>(amount);
	}

	template <typename IntType>
	IntType getInteger(int bytes = sizeof(IntType)) {
		IntType result = 0;
		ensureSize(bytes);
		memcpy(&result, &buffer[position], bytes);
		position += bytes;
		return result;
	}

	// Can return only up to the size of the last read
	void returnBytes(int amount) {
		position -= amount;
	}

	int getPosition() const {
		return position;
	}
	void advancePosition(int by = 1) {
		position += by;
	}
	ptrdiff_t getPositionStart() const {
		return positionStart;
	}
	uint8_t getAtPosition(int index) const {
		return buffer[index];
	}
	uint64_t getEightBytesFromCurrentPosition() {
		ensureSize(1);
		uint64_t got = getEightBytesAtPosition(position);
		position++;
		return got;
	}
	uint64_t getEightBytesAtPosition(int index) const {
		uint64_t obtained = 0;
		memcpy(&obtained, buffer.data() + index, sizeof(uint64_t));
		return obtained;
	}
	int availableAhead() const {
		return filled - position;
	}
	bool isAtEnd() {
		return lookAheadSize == 0 && !availableAhead();
	}

	int refillSome(const std::function<int(std::span<uint8_t> batch)>& readMoreFunction) {
		int added = readMoreFunction(startFilling());
		return doneFilling(added);
	}
	std::span<uint8_t> startFilling() {
		if (position + lookAheadSize >= filled) {
			int offset = std::max(0, position - minSize);
			positionStart += offset;
			filled -= offset;
			memmove(buffer.data(), buffer.data() + offset, filled);
			position -= offset;
		}
		return buffer.subspan(filled);
	}

	int doneFilling(int added) {
		addToChecksum(std::span<uint8_t>(reinterpret_cast<uint8_t*>(buffer.data() + filled), added));
		if (added == 0) {
			lookAheadSize = 0;
			return filled - position;
		}
		filled += added;
		return added;
	}

	template <int MaxTableSize>
	auto encodedTable(int realSize, const std::array<uint8_t, 256>& codeCodingLookup, const std::array<uint8_t, codeCodingReorder.size()>& codeCodingLengths);

protected:
	virtual void addToChecksum(std::span<uint8_t> batch) = 0;
};

// Provides access to input stream as chunks of contiguous data
template <InputStreamSettings Settings, typename Checksum>
class ByteInputWithBuffer : private std::array<uint8_t, Settings::maxSize + Settings::lookAheadSize>, public ByteInput {
	static_assert(Settings::minSize < Settings::maxSize);
	std::array<uint8_t, Settings::maxSize + Settings::lookAheadSize> buffer = {};
	Checksum crc = {};

	void addToChecksum(std::span<uint8_t> batch) override {
		crc(batch);
	}

public:
	ByteInputWithBuffer(std::function<int(std::span<uint8_t> batch)> readMoreFunction)
		: ByteInput(*this, std::move(readMoreFunction), Settings::minSize, Settings::lookAheadSize) {}

	uint32_t checksum() {
		return crc();
	}
};

constexpr static std::array<int, 29> lengthOffsets = {3, 4, 4, 5, 7, 8, 9, 10, 11, 13, 15,
		7, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
constexpr static std::array<int, 30> distanceOffsets = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33,
		49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

#if EZGZ_HAS_CONCEPTS
template <typename T>
concept ByteReader = requires(T reader) {
	reader.returnBytes(1);
	std::span<const uint8_t>(reader.getRange(6));
};
#else
#define ByteReader typename
#endif

static constexpr std::array<uint16_t, 17> upperRemovals = {0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
		0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff};

// Provides optimised access to data from a ByteInput by bits
class BitReader {
	ByteInput* input;
	int bitsLeft = 0;
	uint64_t data = 0; // Invariant - lowest bit is the first valid
	static constexpr int minimumBits = 16; // The specification doesn't require any reading by bits that are longer than 16 bits

	void refillIfNeeded() {
		if (bitsLeft < minimumBits) {
			std::span<const uint8_t> added = input->getRange(sizeof(data) - (minimumBits / 8));
			union {
				std::array<uint8_t, sizeof(uint64_t)> bytes;
				uint64_t number = 0;
			} dataAdded;
			if constexpr (!IsBigEndian) {
				memcpy(dataAdded.bytes.data(), added.data(), std::ssize(added));
			} else {
				for (int i = 0; i < std::ssize(added); i++) {
					dataAdded.bytes[sizeof(data) - i] = added[i];
				}
			};
			dataAdded.number <<= bitsLeft;
			data += dataAdded.number;
			bitsLeft += int(added.size() << 3);
		}
	}

public:

	BitReader(ByteInput* byteInput) : input(byteInput) {}
	BitReader(BitReader&& other) noexcept : input(other.input), bitsLeft(other.bitsLeft), data(other.data) {
		other.input = nullptr;
	}
	BitReader(const BitReader&) = delete;
	BitReader& operator=(BitReader&& other) {
		if (input)
			input->returnBytes(bitsLeft >> 3);
		input = other.input;
		other.input = nullptr;
		bitsLeft = other.bitsLeft;
		data = other.data;
		return *this;
	}
	BitReader& operator=(const BitReader&) = delete;
	~BitReader() {
		if (input)
			input->returnBytes(bitsLeft >> 3);
	}

	// Up to 16 bits, unwanted bits blanked
	uint16_t getBits(int amount) {
		refillIfNeeded();
		if (bitsLeft < amount) [[unlikely]] {
			throw std::runtime_error("Run out of data");
		}
		uint16_t result = uint16_t(data);
		data >>= amount;
		bitsLeft -= amount;
		result &= upperRemovals[amount];
		return result;
	}

	// Provides 8 bits, the functor must return how many of them were actually wanted
	template <typename ReadAndTellHowMuchToConsume>
	void peekAByteAndConsumeSome(const ReadAndTellHowMuchToConsume& readAndTellHowMuchToConsume) {
		refillIfNeeded();
		uint8_t pulled = uint8_t(data);
		auto consumed = readAndTellHowMuchToConsume(pulled);
		if (bitsLeft < consumed) [[unlikely]] {
			throw std::runtime_error("Run out of data");
		}
		data >>= consumed;
		bitsLeft -= consumed;
	}

	// Uses the table in the specification to determine how many bytes are copied
	int parseLongerSize(int partOfSize) {
		if (partOfSize != 31) {
			// Sizes in this range take several extra bits
			int size = partOfSize;
			int nextBits = (size - 7) >> 2;
			auto additionalBits = getBits(nextBits);
			size++; // Will ease the next line
			size = ((((size & 0x3) << nextBits) | additionalBits)) + ((1 << (size >> 2)) + 3); // This is a generalisation of the size table at 3.2.5
			return size;
		} else {
			return 258; // If the value is maximmum, it means the size is 258
		}
	}

	// Uses the table in the specification to determine distance from where bytes are copied
	int parseLongerDistance(int partOfDistance) {
		int readMore = (partOfDistance - 3) >> 1;
		auto moreBits = getBits(readMore);
		int distance = distanceOffsets[partOfDistance - 1] + moreBits;
		return distance;
	}
};

constexpr int maximumCopyLength = 258;

template <StreamSettings Settings>
class DeduplicatedStream {
	std::array<int16_t, Settings::maxSize> deduplicated = {};
	int position = 0;

public:
	struct Section {
		int position = 0; // public
		constexpr static std::array<int, 29> lengthLengthArray = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
		inline static const int* lengthLength = lengthLengthArray.data() - 257; // Accessing other words than copy words is undefined behaviour

		constexpr static std::array<int, 30> distanceLengthArray = {13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 8, 8, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0, 0, 0};
		inline static const int* distanceLength = distanceLengthArray.data() + distanceLengthArray.size(); // Accessing other indexes than between -30 and -1 is UB

		Section(std::span<const int16_t> section) : section(section) {}

		struct CodeRemainderWithLength {
			int remainder = 0;
			int length = 0;
		};

		template <typename OnDuplication> // Three arguments, copy length - 1, distance word (range between -1 and -30), distance length - 1
		int16_t readWord(const OnDuplication& onDuplication) {
			int16_t word = section[position];
			if (word >= 257) {
				CodeRemainderWithLength lengthRemainder{ (-section[position + 1] & upperRemovals[lengthLength[word]]), lengthLength[word] };
				int distanceWord = section[position + 2];
				CodeRemainderWithLength distanceRemainder{ (-section[position + 3] & upperRemovals[distanceLength[distanceWord]]), distanceLength[distanceWord] };
				onDuplication(lengthRemainder, section[position + 2], distanceRemainder);
				position += 4;
			} else {
				position++;
			}
			return word;
		}

		bool atEnd() const {
			return position == std::ssize(section);
		}
		int endPosition() const {
			return int(std::ssize(section));
		}

		constexpr static int MaxSize = Settings::maxSize;

	private:
		std::span<const int16_t> section = {};
	};

private:
	std::function<int(Section, bool lastCall)> submit = {};

	void add(int16_t value) {
		deduplicated[position] = value;
		position++;
	}

	void ensureSize(int size) {
		if (position + size > std::ssize(deduplicated)) [[unlikely]] {
			Section section(std::span<const int16_t>(deduplicated.begin(), deduplicated.begin() + position));
			int consumed = submit(section, false);
			if (consumed < std::ssize(deduplicated) - position - size)
				throw std::runtime_error("DeduplicatedStream must have a submit callback that consumes at least 4 bytes");
			memmove(deduplicated.data(), deduplicated.data() + consumed, (position - consumed) * sizeof(int16_t));
			position -= consumed;
		}
	}

public:
	DeduplicatedStream(std::function<int(Section, bool lastCall)> submit) : submit(std::move(submit)) {}
	~DeduplicatedStream() {
		flush();
	}

	void flush() {
		if (position > 0) [[likely]] {
			Section section(std::span<const int16_t>(deduplicated.begin(), deduplicated.begin() + position));
			submit(section, true);
		}
	}

	void addByte(uint8_t value) {
		ensureSize(1);
		add(value);
	}

	void addDuplication(int length, int distance) {
		ensureSize(4);

		if (length <= 10) {
			add(int16_t(254 + length));
			add(-1);
		} else if (length == maximumCopyLength) {
			add(285);
			add(-1);
		} else {
			unsigned int modifiedLength = length - 3;
			int width = std::bit_width(modifiedLength);
			int suffixWidth = width - 3;
			int prefix = modifiedLength >> suffixWidth;
			int code = 257 + prefix + (suffixWidth << 2);
			add(int16_t(code));
			add(-int16_t(modifiedLength));
		}

		if (distance <= 4) {
			add(int16_t(-distance));
			add(-1);
		} else {
			unsigned int modifiedDistance = distance - 1;
			int width = std::bit_width(modifiedDistance);
			int suffixWidth = width - 2;
			int prefix = modifiedDistance >> suffixWidth;
			int code = prefix + (suffixWidth << 1);
			add(int16_t(- code - 1));
			add(-int16_t(modifiedDistance));
		}
	}

};

// Handles output of decompressed data, filling bytes from past bytes and chunking. Consume needs to be called to empty it
template <StreamSettings Settings, typename Checksum>
class ByteOutput {
	std::array<char, Settings::maxSize> buffer = {};
	int used = 0; // Number of bytes filled in the buffer (valid data must start at index 0)
	int kept = 0;
	bool expectsMore = true; // If we expect more data to be present
	Checksum checksum = {};
	int writtenOut = 0; // TOOD: Remove

	void checkSize(int added = 1) {
		if (used + added > std::ssize(buffer)) [[unlikely]] {
			throw std::logic_error("Writing more bytes than available, probably an internal bug");
		}
	}

public:
	int available() {
		return int(buffer.size() - used);
	}

	int minSize() {
		return expectsMore ? Settings::minSize : 0;
	}

	std::span<const char> getBuffer() {
		return std::span<const char>(buffer.data() + kept, used - kept);
	}

	void cleanBuffer(int leave = 0) {
		leave = std::max(leave, expectsMore? Settings::minSize : 0);
		if (used - leave <= 0) {
			return;
		}
		if (leave == 0) {
			checksum(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer.data()), used));
			writtenOut += used;
			used = 0;
			kept = 0;
		} else {
			checksum(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer.data()), used - leave));
			memmove(buffer.data(), buffer.data() + used - leave, leave);
			writtenOut += used - leave;
			used = leave;
			kept = leave;
		}
	}

	void addByte(char byte) {
		checkSize();
		buffer[used] = byte;
		used++;
	}

	void addBytes(std::span<const char> bytes) {
		checkSize(int(bytes.size()));
		memcpy(buffer.data() + used, bytes.data(), bytes.size());
		used += int(bytes.size());
	}

	void repeatSequence(int length, int distance) {
		checkSize(length);
		int written = 0;
		while (written < length) {
			if (distance > used) {
				throw std::runtime_error("Looking back too many bytes, corrupted archive or insufficient buffer size");
			}
			int toWrite = std::min(distance, length - written);
			memmove(buffer.data() + used, buffer.data() + used - distance, toWrite);
			used += toWrite;
			written += toWrite;
		}
	}

	auto& getChecksum() {
		return checksum;
	}

	void done() { // Called when the whole buffer can be consumed because the data won't be needed anymore
		expectsMore = false;
	}
};

template <StreamSettings Settings, typename Checksum>
class BitOutput {
	uint64_t data = 0;
	int filled = 0;
	ByteOutput<Settings, Checksum>& output;

	void doEmpty(int bytes) {
		const char* dataAsBytes = reinterpret_cast<char*>(&data);
		if constexpr (!IsBigEndian) {
			std::array<char, 8> moved = {};
			for (int i = 0; i < bytes; i++)
				moved[i] = dataAsBytes[i];
//			output.addBytes(moved);
			output.addBytes(std::span<const char>(dataAsBytes, bytes));
		} else {
			std::array<char, 8> invertedBytes = {};
			for (int i = 0; i < bytes; i++) {
				invertedBytes[bytes - i] = dataAsBytes[i];
				output.addBytes(std::span<const char>(invertedBytes.data(), bytes));
			}
		}
	}

	void emptyIfNeeded() {
		if (filled > 48) [[unlikely]] { // Will always keep space for 16 bits
			int removingBytes = filled / 8; // Rounded down
			doEmpty(removingBytes);
			int removingBits = removingBytes * 8;
			data >>= removingBits;
			filled -= removingBits;
		}
	}

public:
	BitOutput(ByteOutput<Settings, Checksum>& output) : output(output) {}

	~BitOutput() {
		doEmpty((filled + 7) / 8); // Round up before dividing by 8
	}

	void addBits(uint64_t value, int size) { // More than 16 bits is actually not supported (there is no use case for more)
		data += value << filled;
		filled += size;
		emptyIfNeeded();
	}

	void addBitsAndCrop(uint64_t value, int size) { // More than 16 bits is actually not supported (there is no use case for more)
		data += (value & upperRemovals[size]) << filled;
		filled += size;
		emptyIfNeeded();
	}
};

// Represents a table encoding Huffman codewords and can parse the stream by bits
template <int MaxSize>
class EncodedTable {
	BitReader& reader;
	struct CodeIndexEntry {
		int16_t word = 0;
		int8_t length = 0;
		bool valid = false;
	};
	std::array<CodeIndexEntry, 256> codesIndex = {}; // If value is greater than MaxSize, it's a remainder at index value minus MaxSize
	struct CodeRemainder {
		uint8_t remainder = 0;
		uint8_t bitsLeft = 0;
		uint16_t index = 0; // bit or with 0x8000 if it's the last one in sequence
	};
	std::array<CodeRemainder, MaxSize> remainders = {};

public:
	EncodedTable(BitReader& reader, int realSize, std::array<uint8_t, 256> codeCodingLookup, std::array<uint8_t, codeCodingReorder.size()> codeCodingLengths)
	: reader(reader) {
		std::array<int, 17> quantities = {};
		struct CodeEntry {
			uint8_t start = 0;
			uint8_t ending = 0;
			uint8_t length = 0;
		};
		std::array<CodeEntry, MaxSize> codes = {};

		// Read the Huffman-encoded Huffman codes
		for (int i = 0; i < realSize; ) {
			int length = 0;
			reader.peekAByteAndConsumeSome([&] (uint8_t peeked) {
				length = codeCodingLookup[peeked];
				return codeCodingLengths[length];
			});
			if (length < 16) {
				codes[i].length = uint8_t(length);
				i++;
				quantities[length]++;
			} else if (length == 16) {
				if (i == 0) [[unlikely]]
					throw std::runtime_error("Invalid lookback position");
				int copy = reader.getBits(2) + 3;
				for (int j = i; j < i + copy; j++) {
					codes[j].length = codes[i - 1].length;
				}
				quantities[codes[i - 1].length] += copy;
				i += copy;
			} else if (length > 16) {
				int zeroCount = 0;
				if (length == 17) {
					zeroCount = reader.getBits(3) + 3;
				} else {
					int sevenBitsValue = reader.getBits(7);
					zeroCount = sevenBitsValue + 11;
				};
				for (int j = i; j < i + zeroCount; j++) {
					codes[j].length = 0;
				}
				i += zeroCount;
			}
		}

		struct UnindexedEntry {
			int quantity = 0;
			int startIndex = 0;
			int filled = 0;
		};
		std::array<UnindexedEntry, 256> unindexedEntries = {};

		// Generate the codes
		int nextCode = 0;
		for (int size = 1; size <= 16; size++) {
			if (quantities[size] > 0) {
				for (int i = 0; i <= realSize; i++) {
					if (codes[i].length == size) {
						if (nextCode >= (1 << size)) [[unlikely]]
								throw std::runtime_error("Bad Huffman encoding, run out of Huffman codes");
						uint8_t firstPart = uint8_t(nextCode);
						if (size <= 8) [[likely]] {
							codes[i].start = reversedBytes[firstPart];
							for (int code = codes[i].start >> (8 - size); code < std::ssize(codesIndex); code += (1 << size)) {
								codesIndex[code].word = int16_t(i);
								codesIndex[code].length = int8_t(size);
								codesIndex[code].valid = true;
							}
						} else {
							uint8_t start = reversedBytes[uint8_t(nextCode >> (size - 8))];
							codes[i].start = start;
							codesIndex[start].valid = true;
							unindexedEntries[start].quantity++;
							codes[i].ending = reversedBytes[uint8_t(nextCode)] >> (16 - size);
						}
						nextCode++;
					}
				}
			}
			nextCode <<= 1;
		}

		// Calculate ranges of the longer parts
		int currentStartIndex = 0;
		for (auto& entry : unindexedEntries) {
			entry.startIndex = currentStartIndex;
			currentStartIndex += entry.quantity;
		}

		// Index the longer parts
		for (int i = 0; i < std::ssize(codes); i++) {
			CodeEntry& code = codes[i];
			if (code.length > 8) {
				UnindexedEntry& unindexedEntry = unindexedEntries[code.start];
				CodeRemainder& remainder = remainders[unindexedEntry.startIndex + unindexedEntry.filled];
				codesIndex[code.start].word = int16_t(MaxSize + unindexedEntry.startIndex);
				unindexedEntry.filled++;
				remainder.remainder = code.ending; // The upper bits are cut
				remainder.bitsLeft = code.length - 8;
				remainder.index = uint16_t(i);
				if (unindexedEntry.filled == unindexedEntry.quantity)
					remainder.index |= 0x8000;
			}
		}
	}

	int readWord() {
		int word = 0;
		reader.peekAByteAndConsumeSome([&] (uint8_t peeked) {
			const CodeIndexEntry entry = codesIndex[peeked];
			word = entry.word;
			if (word >= MaxSize) {
				return 8;
			} else if (!entry.valid) {
				throw std::runtime_error("Unknown Huffman code (not even first 8 bits)");
			}
			return int(entry.length);
		});

		// Longer codes than a byte are indexed specially
		static constexpr std::array<uint8_t, 9> endMasks = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
		if (word >= MaxSize) {
			reader.peekAByteAndConsumeSome([&] (uint8_t peeked) {
				for (int i = word - MaxSize; i < MaxSize * 2; i++) {
					if ((peeked & endMasks[remainders[i].bitsLeft]) == remainders[i].remainder) {
						word = remainders[i].index & 0x7fff;
						return remainders[i].bitsLeft;
					}

					if (remainders[i].index & 0x8000) {
						throw std::runtime_error("Unknown Huffman code (ending bits don't fit)");
					}
				}
				throw std::runtime_error("Unknown Huffman code (bad prefix)");
			});
		}
		return word;
	}
};

template <int MaxTableSize>
auto ByteInput::encodedTable(int realSize, const std::array<uint8_t, 256>& codeCodingLookup, const std::array<uint8_t, codeCodingReorder.size()>& codeCodingLengths) {
	return EncodedTable<MaxTableSize>(*this, realSize, codeCodingLookup, codeCodingLengths);
}

#if EZGZ_HAS_CONCEPTS
template <typename Index>
concept DeduplicatingSearch = requires(Index index, Detail::ByteInput& input, int distance, int length, uint64_t sequence, int offset) {
	Index(input);
	std::tie(distance, length) = index.locate(sequence);
	index.moveBack(offset);
};
#else
#define DeduplicatingSearch typename
#endif

template <int IndexLength = 31237, int IndexLengthSize = 3, int ChunkSize = 30000, typename IndexType = uint16_t>
class MultiIndexBloomFilter {
	constexpr static std::array<int, 3> indexLengths = {4, 6, 8};
	ByteInput& input;

	struct LookbackIndex {
		std::array<IndexType, IndexLength> positions = {};
		uint64_t mask = 0;

		void moveBack(int offset) {
			for (IndexType& position : positions) {
				position -= IndexType(offset);
			}
		}
	};
	std::array<LookbackIndex, IndexLengthSize> lookbackIndexes = ArrayFiller([] (int index) constexpr {
		if constexpr (!IsBigEndian)
			return LookbackIndex{{}, 0xffffffffffffffff >> ((8 - indexLengths[index]) * 8)};
		else
			return LookbackIndex{{}, 0xffffffffffffffff << ((8 - indexLengths[index]) * 8)};
	});

	template <int Index> // Just check if we aren't indexing something that doesn't fit the numeric type
	constexpr static bool checkIfNoneGreaterThanEight() {
		if constexpr(Index < IndexLengthSize) {
			static_assert(indexLengths[Index] <= 8, "We can't have longer indexes than 8 bytes with uint64_t as pseudohash");
			return checkIfNoneGreaterThanEight<Index + 1>();
		} else return true;
	}
	constexpr static bool noneGreaterThanEight = checkIfNoneGreaterThanEight<0>();

public:
	MultiIndexBloomFilter(ByteInput& input) : input(input) {}

	void moveBack(int shift) {
		for (auto& index : lookbackIndexes)
			index.moveBack(shift);
	}

	std::pair<IndexType, int> locate(uint64_t sequence) {
		IndexType position = IndexType(input.getPosition() - 1);
		IndexType location = 0;
		int matchLength = 0;
		for (int index = IndexLengthSize - 1; index >= 0; index--) {
			uint64_t trimmedSequence = sequence & lookbackIndexes[index].mask;
			int sequenceHash = trimmedSequence % IndexLength;
			location = lookbackIndexes[index].positions[sequenceHash];
			if (location >= position) { // Clearly bad data
				continue;
			}
			uint64_t there = input.getEightBytesAtPosition(location);
			there &= lookbackIndexes[index].mask;
			if (there == trimmedSequence) {
				for (matchLength = indexLengths[index]; matchLength < maximumCopyLength; matchLength++) { // TODO: Don't go past end
					if (input.getAtPosition(location + matchLength) != input.getAtPosition(position + matchLength)) {
						return {location, matchLength};
					}
				}
				break;
			}
			lookbackIndexes[index].positions[sequenceHash] = static_cast<std::remove_reference_t<decltype(lookbackIndexes[0].positions[0])>>(position);
		}
		return {position, 0};
	}
};

template <StreamSettings DeduplicatedSettings, DeduplicatingSearch DuplicationIndex>
class Deduplicator {

	int positionStart = 0;

	ByteInput& input;
	DeduplicatedStream<DeduplicatedSettings>& output;

	DuplicationIndex search = {input};

public:
	Deduplicator(ByteInput& input, DeduplicatedStream<DeduplicatedSettings>& output) : input(input), output(output) {}

	void deduplicateSome() {
		do {
			uint64_t sequence = input.getEightBytesFromCurrentPosition();
			// Shift positions int the map to the new offset
			if (input.getPositionStart() != positionStart) [[unlikely]] {
				int newStart = int(input.getPositionStart());
				int shift = newStart - positionStart;
				search.moveBack(shift);
				positionStart = newStart;
			}

			int position = input.getPosition() - 1;
			int location = 0;
			int matchLength = 0;
			std::tie(location, matchLength) = search.locate(sequence);
			matchLength = std::min(matchLength, input.availableAhead());
			if (matchLength >= 3) {
				std::string copied;
				for (int i = 0; i < matchLength; i++) {
					copied += input.getAtPosition(location + i);
				}
				output.addDuplication(matchLength, position - location);
				input.advancePosition(matchLength - 1);
			} else {
				output.addByte(input.getAtPosition(position));
			}
		} while (input.hasMoreDataInBuffer());
	}
};

// Higher level class handling the overall state of parsing. Implemented as a state machine to allow pausing when output is full.
template <DecompressionSettings Settings>
class DeflateReader {
	ByteInput& input;
	ByteOutput<typename Settings::Output, typename Settings::Checksum>& output;

	struct CopyState {
		int copyDistance = 0;
		int copyLength = 0;

		bool restart(decltype(DeflateReader::output)& output) {
			int copying = int(std::min(output.available(), copyLength));
			output.repeatSequence(copying, copyDistance);
			copyLength -= copying;
			return (copyLength == 0);
		}
		bool copy(decltype(DeflateReader::output)& output, int length, int distance) {
			copyLength = length;
			copyDistance = distance;
			return restart(output);
		}
	};

	struct LiteralState {
		int bytesLeft = 0;
		LiteralState(DeflateReader* parent) {
			int length = int(parent->input.getBytes(2));
			int antiLength = int(parent->input.getBytes(2));
			if ((~length & 0xffff) != antiLength) {
				throw std::runtime_error("Corrupted data, inverted length of literal block is mismatching");
			}
			bytesLeft = length;
		}

		bool parseSome(DeflateReader* parent) {
			if (parent->output.available() > bytesLeft) {
				std::span<const uint8_t> chunk = parent->input.getRange(bytesLeft);
				parent->output.addBytes(std::span<const char>(reinterpret_cast<const char*>(chunk.data()), (chunk.size())));
				bytesLeft -= int(chunk.size());
				return (bytesLeft > 0);
			} else {
				std::span<const uint8_t> chunk = parent->input.getRange(parent->output.available());
				bytesLeft -= int(chunk.size());
				parent->output.addBytes(std::span<const char>(reinterpret_cast<const char*>(chunk.data()), (chunk.size())));
				return true;
			}
		}
	};

	struct FixedCodeState : CopyState {
		BitReader input;
		FixedCodeState(decltype(input)&& input) : input(std::move(input)) {}

		bool parseSome(DeflateReader* parent) {
			if (CopyState::copyLength > 0) { // Resume copying if necessary
				if (CopyState::restart(parent->output)) {
					return true; // Out of space
				}
			}

			struct CodeEntry {
				int8_t length = 7;
				int16_t code = 0;
			};
			constexpr static std::array<CodeEntry, 256> codeIndex = ArrayFiller([] (uint8_t oneByteReversed) constexpr {
				uint8_t oneByte = reversedBytes[oneByteReversed];
				if (oneByte < 0b00000010) {
					return CodeEntry{7, 256};
				} else if (oneByte < 0b00110000) {
					return CodeEntry{7, int16_t((oneByte >> 1) - 1 + 257)};
				} else if (oneByte < 0b11000000) {
					return CodeEntry{8, int16_t(oneByte - 0b00110000)};
				} else if (oneByte < 0b11001000) {
					return CodeEntry{8, int16_t(oneByte - 0b11000000 + 280)};
				} else {
					return CodeEntry{8, int16_t(oneByte - 0b11001000 + 144)};
				}
			});

			while (parent->output.available()) {
				CodeEntry code = {};
				input.peekAByteAndConsumeSome([&code] (uint8_t peeked) {
					code = codeIndex[peeked];
					return code.length;
				});

				if (code.code == 256) [[unlikely]] {
					break;
				} else if (code.code > 256) {
					int length = code.code - 254;
					if (length > 10) {
						length = input.parseLongerSize(length);
					}
					static constexpr std::array<uint8_t, 32> lengthDictionary = ArrayFiller([] (int uninverted) constexpr {
						uint8_t reversed = uint8_t((uninverted * uint64_t(0x0202020202) & uint64_t(0x010884422010)) % 0x3ff);
						return uint8_t((reversed >> 3) + 1); // Convert to length word
					});
					int distance = lengthDictionary[input.getBits(5)];
					if (distance > 4) {
						distance = input.parseLongerDistance(distance);
					}
					CopyState::copy(parent->output, length, distance);
				} else {
					if (code.code < 144) {
						parent->output.addByte(char(code.code));
					} else {
						uint8_t full = uint8_t((((code.code - 144)) << 1) + 144 + input.getBits(1));
						parent->output.addByte(full);
					}
				}
			}
			return (parent->output.available() == 0);
		}
	};

	struct DynamicCodeState : CopyState {
		BitReader input;
		EncodedTable<288> codes;
		EncodedTable<31> distanceCode;

		DynamicCodeState(decltype(input)&& inputMoved, int codeCount, int distanceCodeCount, const std::array<uint8_t, 256>& codeCodingLookup,
						const std::array<uint8_t, codeCodingReorder.size()>& codeCodingLengths)
			: input(std::move(inputMoved))
			, codes(input, codeCount, codeCodingLookup, codeCodingLengths)
			, distanceCode(input, distanceCodeCount, codeCodingLookup, codeCodingLengths)
		{ }

		bool parseSome(DeflateReader* parent) {
			if (CopyState::copyLength > 0) { // Resume copying if necessary
				if (CopyState::restart(parent->output)) {
					return true; // Out of space
				}
			}
			while (parent->output.available()) {
				int word = codes.readWord();
				if (word < 256) {
					parent->output.addByte(char(word));
				} else if (word == 256) [[unlikely]] {
					break;
				} else {
					int length = word - 254;
					if (length > 10) {
						length = input.parseLongerSize(length);
					}
					int distance = distanceCode.readWord() + 1;
					if (distance > 4) {
						distance = input.parseLongerDistance(distance);
					}
					CopyState::copy(parent->output, length, distance);
				}
			}
			return (parent->output.available() == 0);
		}
	};

	std::variant<std::monostate, LiteralState, FixedCodeState, DynamicCodeState> decodingState = {};
	bool wasLast = false;

public:
	DeflateReader(decltype(input)& input, decltype(output)& output) : input(input), output(output) {}

	// Returns whether there is more work to do
	bool parseSome() {
		while (true) {
			BitReader bitInput(nullptr);
			if (LiteralState* literalState = std::get_if<LiteralState>(&decodingState)) {
				if (literalState->parseSome(this)) {
					return true;
				}
				bitInput = decltype(bitInput)(&input);
			} else if (FixedCodeState* fixedState = std::get_if<FixedCodeState>(&decodingState)) {
				if (fixedState->parseSome(this)) {
					return true;
				}
				bitInput = std::move(fixedState->input);
			} else if (DynamicCodeState* dynamicState = std::get_if<DynamicCodeState>(&decodingState)) {
				if (dynamicState->parseSome(this)) {
					return true;
				}
				bitInput = std::move(dynamicState->input);
			} else {
				bitInput = decltype(bitInput)(&input);
			}
			decodingState = std::monostate();

			// No decoding state
			if (wasLast) {
				output.done();
				return false;
			}
			wasLast = bitInput.getBits(1);
			int compressionType = bitInput.getBits(2);
			if (compressionType == 0b00) {
				BitReader(std::move(bitInput)); // Move it to a temporary and destroy it
				decodingState.template emplace<LiteralState>(this);
			} else if (compressionType == 0b01) {
				decodingState.template emplace<FixedCodeState>(std::move(bitInput));
			} else if (compressionType == 0b10) {
				// Read lengths
				const int extraCodes = bitInput.getBits(5); // Will be used later
				constexpr int maximumExtraCodes = 29;
				if (extraCodes > maximumExtraCodes) [[unlikely]] {
					throw std::runtime_error("Impossible number of extra codes");
				}
				const int distanceCodes = bitInput.getBits(5) + 1; // Will be used later
				if (distanceCodes > 31) [[unlikely]] {
					throw std::runtime_error("Impossible number of distance codes");
				}
				const int codeLengthCount = bitInput.getBits(4) + 4;
				if (codeLengthCount > 19) [[unlikely]]
						throw std::runtime_error("Invalid distance code count");

				// Read Huffman code lengths
				std::array<uint8_t, codeCodingReorder.size()> codeCodingLengths = {};
				for (int i = 0; i < codeLengthCount; i++) {
					codeCodingLengths[codeCodingReorder[i]] = uint8_t(bitInput.getBits(3));
				}

				// Generate Huffman codes for lengths
				std::array<int, codeCodingReorder.size()> codeCoding = {};
				std::array<uint8_t, 256> codeCodingLookup = {};
				int nextCodeCoding = 0;
				for (int size = 1; size <= 8; size++) {
					for (int i = 0; i < std::ssize(codeCoding); i++)
						if (codeCodingLengths[i] == size) {

							for (int code = nextCodeCoding << (8 - size); code < (nextCodeCoding + 1) << (8 - size); code++) {
								codeCodingLookup[reversedBytes[code]] = uint8_t(i);
							}
							codeCoding[i] = reversedBytes[nextCodeCoding];

							nextCodeCoding++;
						}
					nextCodeCoding <<= 1;
				}

				decodingState.template emplace<DynamicCodeState>(std::move(bitInput), 257 + extraCodes, distanceCodes, codeCodingLookup, codeCodingLengths);
			} else {
				throw std::runtime_error("Unknown type of block compression");
			}
		}
	}
};

template <StreamSettings OutputSettings, StreamSettings DeduplicatedSettings, int BlockSize = 1000000>
class HuffmanWriter {
	ByteOutput<OutputSettings, NoChecksum>& byteOutput;
	std::optional<BitOutput<OutputSettings, NoChecksum>> bitOutput;

	template <int Size>
	struct HuffmanTable {
		struct Entry {
			uint16_t code;
			uint8_t length;

			Entry() = default;
			Entry(uint16_t codeUnreversed, uint8_t length) : length(length) {
				if (length <= 8) {
					code = reversedBytes[codeUnreversed] >> (8 - length);
				} else {
					code = (reversedBytes[codeUnreversed >> 8] >> (16 - length)) | (reversedBytes[codeUnreversed & 0xff] << (length - 8));
				}
			}
		};
		std::array<Entry, Size> codes = {};

		HuffmanTable() = default;
		struct UseDefaultLengthEncoding {};
		constexpr HuffmanTable(UseDefaultLengthEncoding) : codes(ArrayFiller([] (int index) {
			if (index <= 143) {
				return Entry(uint16_t(index + 0b00110000), 8);
			} else if (index <= 255) {
				return Entry(uint16_t(index - 144 + 0b110010000), 9);
			} else if (index == 256) {
				return Entry(0, 7);
			} else if (index < 279) {
				return Entry(uint16_t(index - 256), 7);
			} else {
				return Entry(uint16_t(index - 280 + 0b11000000), 8);
			}
		})) {}
		struct UseDefaultDistanceEncoding {};
		constexpr HuffmanTable(UseDefaultDistanceEncoding) : codes(ArrayFiller([] (int index) {
			return Entry(uint16_t(29 - index), 5);
		})) {}


		template <typename Applied>
		void runThroughCodeEncoding(int endAt, int increment, const Applied& applied) const {
			int previousLength = 0;
			int previousLengthRepeats = 0;
			auto doneRepeating = [&] {
				bool repeatAgain = false;
				do {
					repeatAgain = false;
					if (previousLengthRepeats == 1) {
						applied(previousLength, 0);
					} else if (previousLengthRepeats == 2) {
						applied(previousLength, 0);
						applied(previousLength, 0);
					} else {
						// Note: not the most efficient, it may end with having to repeat the previous value up to 2 times at the end
						if (previousLength == 0) {
							if (previousLengthRepeats > 10) {
								applied(18, std::min(previousLengthRepeats, 138));
								previousLengthRepeats = std::max(0, previousLengthRepeats - 138);
								if (previousLengthRepeats > 0) {
									repeatAgain = true;
								}
							} else {
								applied(17, previousLengthRepeats);
								previousLengthRepeats = 0;
							}
						} else {
							applied(previousLength, 0);
							previousLengthRepeats--;
							while (previousLengthRepeats > 0) {
								if (previousLengthRepeats < 3) {
									for (int i = 0; i < previousLengthRepeats; i++)
										applied(previousLength, 0);
									break;
								} else {
									applied(16, std::min(6, previousLengthRepeats));
									previousLengthRepeats = std::max(0, previousLengthRepeats - 6);
								}
							}
						}
					}
				} while (repeatAgain);
			};
			auto checkOne = [&] (Entry code) {
				if (code.length == previousLength) {
					previousLengthRepeats++;
				} else {
					if (previousLengthRepeats > 0) [[likely]] { // Startup
						doneRepeating();
					}
					previousLength = code.length;
					previousLengthRepeats = 1;
				}
			};
			if (increment > 0) {
				for (int i = 0; i < endAt; i += increment) {
					checkOne(codes[i]);
				}
			} else {
				for (int i = Size - 1; i >= endAt; i += increment) {
					checkOne(codes[i]);
				}
			}
			doneRepeating();
		}
	};

	template <int Size>
	struct FrequencyCounts {
		struct Entry {
			int index = 0;
			int count = 0;
			int length = 0;
		};

		std::array<Entry, Size> counts = ArrayFiller([] (int index) { return Entry{ index, 0 }; });

		HuffmanTable<Size> generateEncoding(int left, bool ascending) { // Trying to do this without the bool flag will bloat the code a lot
			std::array<Entry, Size> sortedCounts = counts;
			// Make sure we have enough elements to form a Huffman table
			int usedCodes = 0;
			for (Entry& entry : sortedCounts) {
				if (entry.count > 0)
					usedCodes++;
			}
			HuffmanTable<Size> made = {};
			if (usedCodes == 0) {
				return made;
			}

			int sizeIncrement = 1;
			int64_t capacity = 0x10000;

			std::sort(sortedCounts.begin(), sortedCounts.end(), [] (Entry first, Entry second) {
				return first.count > second.count;
			});

			// Assign lengths, assign as much capacity as possible but not more than the word's proportion in the total number of words
			for (Entry& word : sortedCounts) {
				if (word.count == 0) break; // In this case, we're done sooner
				while (int(0x10000 >> sizeIncrement) * left > word.count * capacity) {
					// Switch to longer codes
					sizeIncrement++;
				}
				word.length = sizeIncrement;
				left -= word.count;
				capacity -= (0x10000 >> sizeIncrement);
			}

			// Assign leftover capacity to the shortest lengths that can be shortened by taking this capacity
			int sameLengthRangeBegin = 0;
			int sameLengthRangeEnd = 0;
			uint16_t currentCode = 0;
			auto sortLastLength = [&] () {
				// Ensure codes of the same length start from the lowest index code
				if (ascending) {
					std::sort(sortedCounts.begin() + sameLengthRangeBegin, sortedCounts.begin() + sameLengthRangeEnd,
							  [] (Entry first, Entry second) {
						return first.index < second.index;
					});
				} else {
					std::sort(sortedCounts.begin() + sameLengthRangeBegin, sortedCounts.begin() + sameLengthRangeEnd,
							  [] (Entry first, Entry second) {
						return first.index > second.index;
					});
				}
				for (int i = sameLengthRangeBegin; i < sameLengthRangeEnd; i++) {
					made.codes[sortedCounts[i].index] = typename HuffmanTable<Size>::Entry(currentCode, uint8_t(sortedCounts[i].length));
					currentCode += 1;
				}
				sameLengthRangeBegin = sameLengthRangeEnd;
			};
			int previousLength = sortedCounts.front().length;
			for (Entry& word : sortedCounts) {
				if (word.count == 0) break; // In this case, we're done sooner
				if (usedCodes != 1) {
					int neededToUpgrade = 0x10000 >> word.length;
					if (neededToUpgrade <= capacity) {
						capacity -= neededToUpgrade;
						word.length--;
					}
				}
				if (word.length != previousLength) {
					sortLastLength();
					currentCode <<= (word.length) - previousLength;
					previousLength = word.length;
				}
				sameLengthRangeEnd++;
			}
			sortLastLength();

			if (capacity < 0) {
				throw std::logic_error("Didn't generate the Huffman code correctly");
			}
			if (capacity > 0 && usedCodes != 1) { // This is not only suboptimal, gunzip requires every Huffman code to be valid (the standard does not)
				throw std::logic_error("Didn't use all capacity available for Huffman coding");
			}

			return made;
		}

		int sizeWithEncoding(const HuffmanTable<Size>& encoding) {
			int total = 0;
			for (Entry letter : counts) {
				total += letter.count * encoding.codes[letter.index].length;
			}
			return total;
		}

		template <int OtherSize>
		int addToHuffmanTableLengths(HuffmanTable<OtherSize>& encoding, int endAt, int increment) {
			int totalWords = 0;
			encoding.runThroughCodeEncoding(endAt, increment, [this, &totalWords] (int word, int) {
				counts[word].count++;
				totalWords++;
			});
			return totalWords;
		}
	};

public:
	HuffmanWriter(ByteOutput<OutputSettings, NoChecksum>& output) : byteOutput(output) {}

	~HuffmanWriter() {
		finalFlush();
	}

	void finalFlush() {
		bitOutput.reset();
	}

	void writeBatch(typename DeduplicatedStream<DeduplicatedSettings>::Section& section, bool isLast) {
		// Baseline, dynamic Hufffman coding won't be used if it's better; TODO: Why not constexpr?
		static HuffmanTable<286> staticWordEncoding = HuffmanTable<286>(typename HuffmanTable<286>::UseDefaultLengthEncoding());
		static HuffmanTable<30> staticDistanceEncoding = HuffmanTable<30>(typename HuffmanTable<30>::UseDefaultDistanceEncoding());
		// Avoid having to convert the oddly positioned distance codes in the range of -30 to -1
		static typename HuffmanTable<30>::Entry* staticDistanceZero = staticDistanceEncoding.codes.data() + 30;

		struct Counts {
			FrequencyCounts<286> wordCounts = {};
			FrequencyCounts<30> distanceCounts = {};
			int lengthsAfter256 = 1;
			int lowestDistanceWord = 29;
			int properEndPos = 0;
			int words = 0;
			int distances = 0;
			int staticLength = 0;

			typename FrequencyCounts<30>::Entry* frequencyDistanceZero() {
				return distanceCounts.counts.data() + 30;
			}

			Counts(typename DeduplicatedStream<DeduplicatedSettings>::Section& section, int startPos, int endPos) {
				section.position = startPos;
				while (section.position < endPos) {
					int word = section.readWord([&] (auto, int distanceWord, auto) {
						frequencyDistanceZero()[distanceWord].count++;
						staticLength += staticDistanceZero[distanceWord].length;
						distances++;
					});
					if (word < 0) {
						throw std::runtime_error("Corrupted stream boundaries");
					}
					wordCounts.counts[word].count++;
					staticLength += staticWordEncoding.codes[word].length;
					words++;
				}
				properEndPos = section.position; // We may have run through some trailers, need to update the start of the following block
				wordCounts.counts[256].count++; // We need to end the block somewhere
				words++;

				// We'll need to stop writing lengths from some point
				for (int i = int(std::ssize(wordCounts.counts)) - 1; i > 257; i--) {
					if (wordCounts.counts[i].count != 0) {
						lengthsAfter256 = i - 256;
						break;
					}
				}
				for (int i = 0; i < std::ssize(distanceCounts.counts); i++) {
					if (distanceCounts.counts[i].count != 0) {
						lowestDistanceWord = i;
						break;
					}
				}
			}

			Counts(const Counts& first, const Counts& second) // MUST be adjacent
			: lengthsAfter256(std::max(first.lengthsAfter256, second.lengthsAfter256))
			, lowestDistanceWord(std::min(first.lowestDistanceWord, second.lowestDistanceWord))
			, properEndPos(std::max(first.properEndPos, second.properEndPos))
			, words(first.words + second.words)
			, distances(first.distances + second.distances)
			, staticLength(first.staticLength + second.staticLength) {
				for (int i = 0; i <= 256 + lengthsAfter256; i++) {
					wordCounts.counts[i].count = first.wordCounts.counts[i].count + second.wordCounts.counts[i].count;
				}
				for (int i = int(std::ssize(distanceCounts.counts) - 1); i >= lowestDistanceWord; i--) {
					distanceCounts.counts[i].count = first.distanceCounts.counts[i].count + second.distanceCounts.counts[i].count;
				}

				wordCounts.counts[256].count--; // One less ending
				words--;
			}
		};

		class Block {
			typename DeduplicatedStream<DeduplicatedSettings>::Section* section = nullptr; // Pointer just to allow default assignment

			bool enabled = true;
			bool last = false;
			Counts counts;
			int startPos = 0;
			int endPos = 0;
			int total = 0;
			int codeCodingTableLength = 0;

			bool usesDynamic = false;
			HuffmanTable<19> codeEncoding = {};
			HuffmanTable<286> dynamicWordEncoding = {};
			HuffmanTable<30> dynamicDistanceEncoding = {};

		public:
			Block(typename DeduplicatedStream<DeduplicatedSettings>::Section& section, Counts&& counts, int startPos, bool last)
			: section(&section), last(last), counts(std::move(counts)), startPos(startPos), endPos(counts.properEndPos) {
				if (startPos >= endPos) {
					enabled = false;
					return;
				}

				dynamicWordEncoding = counts.wordCounts.generateEncoding(counts.words, true /*ascending*/);
				dynamicDistanceEncoding = counts.distanceCounts.generateEncoding(counts.distances, false /*descending*/);

				// Find distribution and generate encodings for the definition of the Huffman code
				FrequencyCounts<19> codeCounts = {};
				int totalCodes = 0;
				totalCodes += codeCounts.addToHuffmanTableLengths(dynamicWordEncoding, 257 + counts.lengthsAfter256, 1);
				totalCodes += codeCounts.addToHuffmanTableLengths(dynamicDistanceEncoding, counts.lowestDistanceWord, -1);
				codeEncoding = codeCounts.generateEncoding(totalCodes, true /*ascending*/);

				for (int i = 0; i < std::ssize(codeCodingReorder); i++) {
					if (codeEncoding.codes[i].length > 0) {
						codeCodingTableLength = std::max<int>(codeCodingTableLength, codeCodingReorderInverse[i]);
					}
				}
				codeCodingTableLength = std::max(codeCodingTableLength + 1, 4); // At least 4 codes must be written

				// Now, we can estimate the size of the encoded part (minus the extra bits after some numbers because they're always the same)
				int dynamicLength = 12 + codeCodingTableLength * 3; // Intro length
				dynamicLength += codeCounts.sizeWithEncoding(codeEncoding); // Encoded codes
				dynamicLength += 2 * codeCounts.counts[16].count + 3 * codeCounts.counts[17].count + 7 * codeCounts.counts[18].count; // Extra bits
				dynamicLength += counts.wordCounts.sizeWithEncoding(dynamicWordEncoding); // Encoded words
				dynamicLength += counts.distanceCounts.sizeWithEncoding(dynamicDistanceEncoding); // Encoded distances

				// Pick the encoding
				usesDynamic = (dynamicLength < counts.staticLength);
				total = std::min(dynamicLength, counts.staticLength);
			}

			void writeOut(BitOutput<OutputSettings, NoChecksum>& bitOutput) {
				if (!enabled) {
					return;
				}

				const HuffmanTable<286>* wordEncoding = (usesDynamic) ? &dynamicWordEncoding : &staticWordEncoding;
				const HuffmanTable<30>* distanceEncoding = (usesDynamic) ? &dynamicDistanceEncoding : &staticDistanceEncoding;
				const typename HuffmanTable<30>::Entry* distanceZero = distanceEncoding->codes.data() + 30;

				bitOutput.addBits(last, 1);
				if (usesDynamic) {
					// Intro
					bitOutput.addBits(0b10, 2);
					bitOutput.addBits(counts.lengthsAfter256, 5);
					bitOutput.addBits(29 - counts.lowestDistanceWord, 5);
					bitOutput.addBits(codeCodingTableLength - 4, 4);

					// Code encoding
					for (int i = 0; i < codeCodingTableLength; i++) {
						bitOutput.addBits(codeEncoding.codes[codeCodingReorder[i]].length, 3); // Writing some bullshit here
					}

					// Code declaration
					auto encodeCode = [this, &bitOutput] (const auto& code, int endAt, int increment) {
						code.runThroughCodeEncoding(endAt, increment, [this, &bitOutput] (int word, int extra) {
							bitOutput.addBits(codeEncoding.codes[word].code, codeEncoding.codes[word].length);
							if (word == 16) {
								bitOutput.addBits(extra - 3, 2);
							} else if (word == 17) {
								bitOutput.addBits(extra - 3, 3);
							} else if (word == 18) {
								bitOutput.addBits(extra - 11, 7);
							}
						});
					};

					encodeCode(dynamicWordEncoding, 257 + counts.lengthsAfter256, 1);
					encodeCode(dynamicDistanceEncoding, counts.lowestDistanceWord, -1);
				} else {
					bitOutput.addBits(0b01, 2);
				}

				// Encode the actual block
				section->position = startPos;
				while (section->position < endPos) {
					bool alsoOthers = false;
					typename std::decay_t<decltype(*section)>::CodeRemainderWithLength length = {};
					int distanceWord = 0;
					typename std::decay_t<decltype(*section)>::CodeRemainderWithLength distance = {};
					int word = section->readWord([&] (auto lengthExtraAssigned, int distanceWordAssigned, auto distanceExtraAssigned) {
						alsoOthers = true;
						length = lengthExtraAssigned;
						distanceWord = distanceWordAssigned;
						distance = distanceExtraAssigned;
					});
					bitOutput.addBits(wordEncoding->codes[word].code, wordEncoding->codes[word].length);
					if (alsoOthers) { // Hopefully this will be optimised out
						if (word >= 265) {
							bitOutput.addBitsAndCrop(length.remainder, length.length);
						}
						bitOutput.addBits(distanceZero[distanceWord].code, distanceZero[distanceWord].length);
						if (distanceWord < -4) {
							bitOutput.addBitsAndCrop(distance.remainder, distance.length);
						}
					}
				}
				bitOutput.addBits(wordEncoding->codes[256].code, wordEncoding->codes[256].length); // Ending
			}

			bool getEnabled() const {
				return enabled;
			}
			void setEnabled(bool value) {
				enabled = value;
			}
			int getStart() const {
				return startPos;
			}
			int getEnd() const {
				return endPos;
			}
			int totalSize() const {
				return total;
			}
			bool isLast() const {
				return last;
			}
			const Counts& getCounts() const {
				return counts;
			}
		};

		constexpr static int BlockCount = std::max(DeduplicatedStream<DeduplicatedSettings>::Section::MaxSize / BlockSize, 1);
		int previousEnd = 0;
		std::array<Block, BlockCount> blocks = ArrayFiller([&] (int index) -> Block {
			int end = (index + 1) * BlockSize;
			bool thisOneIsLast = isLast;
			if (end > section.endPosition()) {
				end = section.endPosition();
			} else thisOneIsLast = false;
			Block made(section, Counts(section, previousEnd, std::min((index + 1) * BlockSize, section.endPosition())), previousEnd, thisOneIsLast);
			previousEnd = made.getEnd();
			return made;
		});

		auto considerMerge = [&section] (Block& first, Block& second) {
			Block merged(section, Counts(first.getCounts(), second.getCounts()), first.getStart(), first.isLast() || second.isLast());
			if (merged.totalSize() < first.totalSize() + second.totalSize()) {
				first = merged;
				second.setEnabled(false);
			}
		};
		for (int width = 1; width < std::ssize(blocks) / 2; width *= 2) {
			for (int start = 0; start < std::ssize(blocks); start += width * 2) {
				int mid = start + width;
				if (mid >= std::ssize(blocks))
					break; // Past the end
				int end = std::min<int>(start + width * 2, std::ssize(blocks));
				int startBlockIndex = mid - 1;
				for ( ; startBlockIndex >= start; startBlockIndex--) {
					if (blocks[startBlockIndex].getEnabled())
						break;
				}
				if (startBlockIndex < start)
					continue; // Found nothing
				int endBlockIndex = mid;
				for ( ; endBlockIndex < end; endBlockIndex++) {
					if (blocks[startBlockIndex].getEnabled())
						break;
				}
				if (endBlockIndex >= end)
					continue; // Foud nothing

				considerMerge(blocks[startBlockIndex], blocks[endBlockIndex]);
			}
		}

		if (!bitOutput) {
			bitOutput.emplace(byteOutput);
		}

		for (int i = 0; i < std::ssize(blocks); i++) {
			blocks[i].writeOut(*bitOutput);
		}
	}
};

} // namespace Detail

#if EZGZ_HAS_CONCEPTS
template <typename T>
concept CompressionSettings = std::constructible_from<typename T::Checksum> && InputStreamSettings<typename T::Input>
			&& StreamSettings<typename T::Output> && StreamSettings<typename T::DeduplicationProperties>
			 && Detail::DeduplicatingSearch<typename T::DeduplicationIndex> && requires(typename T::Checksum checksum) {
	int(checksum());
	int(checksum(std::span<const uint8_t>()));
	int(T::HuffmanSectionSize);
};
#else
#define CompressionSettings typename
#endif

struct DefaultCompressionSettings {
	struct Input {
		constexpr static int maxSize = 30000;
		constexpr static int minSize = 10000;
		constexpr static int lookAheadSize = 300;
	};
	struct DeduplicationProperties {
		constexpr static int maxSize = 50000;
		constexpr static int minSize = 10000;
	};
	struct Output {
		constexpr static int maxSize = 100000;
		constexpr static int minSize = 0;
	};

	constexpr static int HuffmanSectionSize = 1000000;
	using Checksum = FastCrc32;
	using DeduplicationIndex = Detail::MultiIndexBloomFilter<>;
};

struct BestCompressionSettings : DefaultCompressionSettings {
	constexpr static int HuffmanSectionSize = 1000;
};

// Handles decompression of a deflate-compressed archive, no headers
template <DecompressionSettings Settings = DefaultDecompressionSettings>
std::vector<char> readDeflateIntoVector(std::function<int(std::span<uint8_t> batch)> readMoreFunction) {
	std::vector<char> result;
	Detail::ByteInputWithBuffer<typename Settings::Input, typename Settings::Checksum> input(readMoreFunction);
	Detail::ByteOutput<typename Settings::Output, typename Settings::Checksum> output;
	Detail::DeflateReader<Settings> reader(input, output);
	bool workToDo = false;
	do {
		workToDo = reader.parseSome();
		std::span<const char> batch = output.getBuffer();
		result.insert(result.end(), batch.begin(), batch.end());
		output.cleanBuffer();
	} while (workToDo);
	return result;
}

template <DecompressionSettings Settings = DefaultDecompressionSettings>
std::vector<char> readDeflateIntoVector(std::span<const uint8_t> allData) {
	return readDeflateIntoVector<Settings>([allData, position = 0] (std::span<uint8_t> toFill) mutable -> int {
		int filling = int(std::min(allData.size() - position, toFill.size()));
		if(filling != 0)
			memcpy(toFill.data(), &allData[position], filling);
		position += filling;
		return filling;
	});
}

template <CompressionSettings Settings>
std::vector<uint8_t> writeDeflateIntoVector(std::function<int(std::span<char> batch)> readMoreFunction) {
	std::vector<uint8_t> result;
	{
		Detail::ByteOutput<typename Settings::Output, NoChecksum> output;
		Detail::HuffmanWriter<typename Settings::Output, typename Settings::DeduplicationProperties> writer(output);
		auto connector = [&] (typename Detail::DeduplicatedStream<typename Settings::DeduplicationProperties>::Section section, bool lastCall) {
			writer.writeBatch(section, lastCall);
			return section.position;
		};
		Detail::ByteInputWithBuffer<typename Settings::Input, typename Settings::Checksum> input([&readMoreFunction] (std::span<uint8_t> batch) {
			return readMoreFunction(std::span<char>(reinterpret_cast<char*>(batch.data()), batch.size()));
		});
		Detail::DeduplicatedStream<typename Settings::DeduplicationProperties> deduplicated(connector);
		Detail::Deduplicator<typename Settings::DeduplicationProperties, typename Settings::DeduplicationIndex> deduplicator(input, deduplicated);

		do {
			deduplicator.deduplicateSome();
			std::span<const char> batch = output.getBuffer();
			result.insert(result.end(), batch.begin(), batch.end());
			output.cleanBuffer();
		} while (!input.isAtEnd());

		deduplicated.flush();
		writer.finalFlush();
		output.done();
		std::span<const char> batch = output.getBuffer();
		result.insert(result.end(), batch.begin(), batch.end());
	}
	return result;
}

template <CompressionSettings Settings>
std::vector<uint8_t> writeDeflateIntoVector(std::span<const char> allData) {
	return writeDeflateIntoVector<Settings>([allData, position = 0] (std::span<char> toFill) mutable -> int {
		int filling = std::min(int(allData.size() - position), int(toFill.size()));
		if(filling != 0)
			memcpy(toFill.data(), &allData[position], filling);
		position += filling;
		return filling;
	});
}


// Handles decompression of a deflate-compressed archive, no headers
template <DecompressionSettings Settings = DefaultDecompressionSettings>
class IDeflateArchive {
protected:
	Detail::ByteInputWithBuffer<typename Settings::Input, typename Settings::Checksum> input;
	Detail::ByteOutput<typename Settings::Output, typename Settings::Checksum> output;
	Detail::DeflateReader<Settings> deflateReader = {input, output};
	bool done = false;
	int bytesKept = 0;

	bool bufferNeedsCleaning = false;
	void cleanBufferIfNeeded() {
		if (bufferNeedsCleaning) {
			output.cleanBuffer(bytesKept);
			bufferNeedsCleaning = false;
		}
	}

	virtual void onFinish() {}

public:
	IDeflateArchive(std::function<int(std::span<uint8_t> batch)> readMoreFunction) : input(readMoreFunction) {}

#ifndef EZGZ_NO_FILE
	IDeflateArchive(const std::string& fileName) : input([file = std::make_shared<std::ifstream>(fileName, std::ios::binary)] (std::span<uint8_t> batch) mutable {
		if (!file->good()) {
			throw std::runtime_error("Can't read file");
		}
		file->read(reinterpret_cast<char*>(batch.data()), batch.size());
		int bytesRead = int(file->gcount());
		return bytesRead;
	}) {}
#endif

	IDeflateArchive(std::span<const uint8_t> data) : input([data] (std::span<uint8_t> batch) mutable {
		int copying = int(std::min(batch.size(), data.size()));
		if (copying == 0) {
			return 0;
		}
		memcpy(batch.data(), data.data(), copying);
		data = data.subspan(copying);
		return copying;
	}) {}

	// Returns whether there are more bytes to read
	std::optional<std::span<const char>> readSome(int bytesToKeep = 0) {
		cleanBufferIfNeeded();
		if (done) {
			return std::nullopt;
		}
		bool moreStuffToDo = deflateReader.parseSome();
		std::span<const char> batch = output.getBuffer();
		bytesKept = bytesToKeep;
		if (!moreStuffToDo) {
			onFinish();
			done = true;
		}
		bufferNeedsCleaning = true;
		return batch;
	}

	void readByLines(const std::function<void(std::span<const char>)> reader, char separator = '\n') {
		int keeping = 0;
		bool wasSeparator = false;
		std::span<const char> batch = {};
		while (!done) {
			batch = *readSome(keeping);
			std::span<const char>::iterator start = batch.begin();
			for (std::span<const char>::iterator it = start; it != batch.end(); ++it) {
				if (wasSeparator) {
					wasSeparator = false;
					start = it;
				}
				if (*it == separator) {
					reader(std::span<const char>(&*start, std::distance(start, it)));
					wasSeparator = true;
				}
			}
			keeping = int(batch.end() - start);
		}
		if (keeping > 0) {
			if (wasSeparator)
				reader(std::span<const char>());
			else
				reader(std::span<const char>(&*(batch.end() - keeping), keeping));
		}
	}

	void readAll(const std::function<void(std::span<const char>)>& reader) {
		while (std::optional<std::span<const char>> batch = readSome()) {
			reader(*batch);
		}
	}

	std::vector<char> readAll() {
		std::vector<char> returned;
		while (std::optional<std::span<const char>> batch = readSome()) {
			returned.insert(returned.end(), batch->begin(), batch->end());
		};
		return returned;
	}
};

template <CompressionSettings Settings, typename Checksum = NoChecksum>
class ODeflateArchive {
protected:
	Detail::ByteOutput<typename Settings::Output, NoChecksum> output;
	Detail::ByteInputWithBuffer<typename Settings::Input, Checksum> input = {[this] (std::span<uint8_t>) -> int {
		return 0; // TODO: The input buffer should be able to renew from this
	}};

	void writeAtEnd(std::function<void()> trailerWriter) {
		writeTrailer = trailerWriter;
	}
private:
	Detail::HuffmanWriter<typename Settings::Output, typename Settings::DeduplicationProperties, Settings::HuffmanSectionSize> writer = {output};
	std::function<void(std::span<const char> batch)> consumeFunction;
	Detail::DeduplicatedStream<typename Settings::DeduplicationProperties> deduplicated = {[this]
				(typename Detail::DeduplicatedStream<typename Settings::DeduplicationProperties>::Section section, bool lastCall) {
		writer.writeBatch(section, lastCall);
		return section.position;
	}};
	Detail::Deduplicator<typename Settings::DeduplicationProperties, typename Settings::DeduplicationIndex> deduplicator = {input, deduplicated};

	void consume() {
		std::span<const char> batch = output.getBuffer();
		consumeFunction(batch);
		output.cleanBuffer();
	}

	std::function<void()> writeTrailer;

public:
	ODeflateArchive(std::function<void(std::span<const char> batch)> consumeFunction) : consumeFunction(std::move(consumeFunction)) { }

#ifndef EZGZ_NO_FILE
	// FIXME: We are saving output, not input!
	ODeflateArchive(const std::string& fileName) : consumeFunction([file = std::make_shared<std::ofstream>(fileName + ".gz", std::ios::binary)] (std::span<const char> batch) mutable {
		if (!file->good()) {
			throw std::runtime_error("Can't write file");
		}
		file->write(batch.data(), batch.size());
	}) {}
#endif

	ODeflateArchive(std::vector<char>& outVector) : consumeFunction([&outVector] (std::span<const char> batch) mutable {
		outVector.insert(outVector.end(), batch.begin(), batch.end());
	}) {}

	~ODeflateArchive() {
		flush();
	}

	void flush() {
		input.doneFilling(0);
		if (input.hasMoreDataInBuffer()) {
			deduplicator.deduplicateSome();
		}
		deduplicated.flush();
		writer.finalFlush();
		writeTrailer();
		output.done();
		consume();
	}

	void writeSome(std::span<const char> section) {
		int position = 0;
		while (position < std::ssize(section)) {
			bool doDeduplicate = false;
			input.refillSome([&] (std::span<uint8_t> outSection) -> int {
				int copied = std::min(int(section.size() - position), int(outSection.size()));
				doDeduplicate = (std::ssize(outSection) <= std::ssize(section) * 0.8);
				memcpy(outSection.data(), section.data() + position, copied);
				position += copied;
				return copied;
			});
			if (doDeduplicate) {
				deduplicator.deduplicateSome();
				consume();
			}
		}
	}
	void writeSome(std::string_view section) {
		writeSome(std::span<const char>(section.data(), section.size()));
	}

	class OpenWritingBuffer {
		ODeflateArchive* parent = nullptr;
		std::span<uint8_t> range = {};
		OpenWritingBuffer(ODeflateArchive* parent) : parent(parent), range(parent->input.startFilling()) { }
		friend class ODeflateArchive;

	public:
		OpenWritingBuffer() = default;
		OpenWritingBuffer(OpenWritingBuffer&& another) {
			operator=(std::move(another));
		}
		OpenWritingBuffer& operator=(OpenWritingBuffer&& another) {
			if (parent)
				throw std::logic_error("OpenWritingBuffer must be finished with doneFilling() before destruction");
			parent = another.parent;
			range = another.range;
			another.parent = nullptr;
			return *this;
		}
		std::span<uint8_t> accessRange() const {
			return range;
		}
		void finish(int added) {
			parent->input.doneFilling(added);
			parent->deduplicator.deduplicateSome();
			parent->consume();
			parent = nullptr;
		}
		~OpenWritingBuffer() noexcept(false) {
			if (parent)
				throw std::logic_error("OpenWritingBuffer must be finished with doneFilling() before destruction");
		}
	};
	OpenWritingBuffer openWritingBuffer() {
		return OpenWritingBuffer(this);
	}
};

enum class CreatingOperatingSystem {
	UNIX_BASED,
	WINDOWS,
	OTHER
};

// File information in the .gz file
template <BasicStringType StringType>
struct GzFileInfo {
	int32_t modificationTime = int32_t(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	CreatingOperatingSystem operatingSystem = CreatingOperatingSystem::OTHER;
	bool fastestCompression = false;
	bool densestCompression = false;
	std::optional<std::vector<uint8_t>> extraData = {};
	StringType name;
	StringType comment;
	bool probablyText = false;

	GzFileInfo(std::string_view name) : name(name) {}

	void writeOut(std::function<void(std::span<const char> batch)> writer) const {
		LightCrc32 crc = {};
		auto writeInteger = [&writer, &crc] (auto integer) {
			std::array<char, sizeof(integer)> buffer = {};
			memcpy(buffer.data(), &integer, buffer.size());
			writer(buffer);
			crc(std::span<const uint8_t>(reinterpret_cast<uint8_t*>(buffer.data()), buffer.size()));
		};

		writeInteger(uint8_t(0x1f));
		writeInteger(uint8_t(0x8b));
		writeInteger(uint8_t(0x08));
		uint8_t flags = 0x02; // It will have CRC32
		if (extraData) flags |= 0x04;
		if (!std::string_view(name).empty()) flags |= 0x08;
		if (!std::string_view(comment).empty()) flags |= 0x10;
		if (probablyText) flags |= 0x01;
		writeInteger(flags);
		writeInteger(modificationTime);
		writeInteger(uint8_t((densestCompression ? 4 : 0) | (fastestCompression ? 8 : 0)));
		writeInteger(uint8_t(operatingSystem == CreatingOperatingSystem::UNIX_BASED ? 3 : (operatingSystem == CreatingOperatingSystem::WINDOWS ? 0 : 255)));
		if (extraData) {
			if (extraData->size() > std::numeric_limits<uint16_t>::max())
				throw std::runtime_error("Cannot save so many extra data in the archive");
			writeInteger(uint16_t(extraData->size()));
			for (uint8_t letter : *extraData) {
				writeInteger(letter);
			}
		}
		auto writeNullTerminatedString = [&] (const StringType& written) {
			if (!std::string_view(written).empty()) {
				for (uint8_t letter : std::string_view(written)) {
					writeInteger(letter);
				}
				writeInteger(char(0));
			}
		};
		writeNullTerminatedString(name);
		writeNullTerminatedString(comment);
		writeInteger(uint16_t(crc()));
	}

	GzFileInfo(Detail::ByteInput& input) {
		LightCrc32 checksum = {};
		auto check = [&checksum] (auto num) -> uint32_t {
			std::array<uint8_t, sizeof(num)> asBytes = {};
			memcpy(asBytes.data(), &num, asBytes.size());
			return checksum(asBytes);
		};
		if (input.template getInteger<uint8_t>() != 0x1f || input.template getInteger<uint8_t>() != 0x8b || input.template getInteger<uint8_t>() != 0x08)
			throw std::runtime_error("Trying to parse something that isn't a Gzip archive");
		check(0x1f);
		check(0x8b);
		check(0x08);
		uint8_t flags = input.template getInteger<uint8_t>();
		check(flags);
		modificationTime = input.template getInteger<uint32_t>();
		check(modificationTime);
		uint8_t extraFlags = input.template getInteger<uint8_t>();
		check(extraFlags);

		if (extraFlags == 4) {
			densestCompression = true;
		} else if (extraFlags == 8) {
			fastestCompression = true;
		}
		uint8_t creatingOperatingSystem = input.template getInteger<uint8_t>(); // Was at input[9]
		check(creatingOperatingSystem);
		if (creatingOperatingSystem == 0) {
			operatingSystem = CreatingOperatingSystem::WINDOWS;
		} else if (creatingOperatingSystem == 3) {
			operatingSystem = CreatingOperatingSystem::UNIX_BASED;
		}

		if (flags & 0x04) {
			uint16_t extraHeaderSize = input.template getInteger<uint16_t>();
			check(extraHeaderSize);
			int readSoFar = 0;
			extraData.emplace();
			while (readSoFar < extraHeaderSize) {
				std::span<const uint8_t> taken = input.getRange(extraHeaderSize - readSoFar);
				checksum(taken);
				extraData->insert(extraData->end(), taken.begin(), taken.end());
				readSoFar += int(taken.size());
			}
		}
		if (flags & 0x08) {
			char letter = input.template getInteger<uint8_t>();
			check(letter);
			while (letter != '\0') {
				name += letter;
				letter = input.template getInteger<uint8_t>();
				check(letter);
			}
		}
		if (flags & 0x10) {
			char letter = input.template getInteger<uint8_t>();
			check(letter);
			while (letter != '\0') {
				comment += letter;
				letter = input.template getInteger<uint8_t>();
				check(letter);
			}
		}
		if (flags & 0x01) {
			probablyText = true;
		}
		if (flags & 0x02) {
			uint16_t expectedHeaderCrc = uint16_t(input.template getInteger<uint16_t>());
			check(expectedHeaderCrc);
//			uint16_t realHeaderCrc = checksum(); // Probably bugged
//			if (expectedHeaderCrc != realHeaderCrc)
//				throw std::runtime_error("Gzip archive's headers crc32 checksum doesn't match the actual header's checksum");
		}
	}
};

// Parses a .gz file, only takes care of the header, the rest is handled by its parent class IDeflateArchive
template <DecompressionSettings Settings = DefaultDecompressionSettings>
class IGzFile : public IDeflateArchive<Settings> {
	GzFileInfo<typename Settings::StringType> parsedHeader;
	using Deflate = IDeflateArchive<Settings>;

	void onFinish() override {
		Deflate::output.done();
		Deflate::output.cleanBuffer();
		uint32_t expectedCrc = Deflate::input.template getInteger<uint32_t>();
		if constexpr(Settings::verifyChecksum) {
			auto realCrc = Deflate::output.getChecksum()();
			if (expectedCrc != realCrc)
				throw std::runtime_error("Gzip archive's crc32 checksum doesn't match the calculated checksum");
		}
	}

public:
	IGzFile(std::function<int(std::span<uint8_t> batch)> readMoreFunction) : Deflate(readMoreFunction), parsedHeader(Deflate::input) {}
	IGzFile(const std::string& fileName) : Deflate(fileName), parsedHeader(Deflate::input) {}
	IGzFile(std::span<const uint8_t> data) : Deflate(data), parsedHeader(Deflate::input) {}

	const GzFileInfo<typename Settings::StringType>& info() const {
		return parsedHeader;
	}
};

// Writes a .gz file, only takes care of the header, the rest is handled by its parent class ODeflateArchive
template <CompressionSettings Settings, BasicStringType StringType>
class OGzFile : public ODeflateArchive<Settings, FastCrc32> {
	void fillTrailerWriter() {
		ODeflateArchive<Settings, FastCrc32>::writeAtEnd([this] {
			auto writeInteger = [this] (uint32_t value) {
				std::array<char, sizeof(uint32_t)> bytes = {};
				memcpy(bytes.data(), &value, bytes.size());
				ODeflateArchive<Settings, FastCrc32>::output.addBytes(bytes);
			};
			writeInteger(uint32_t(ODeflateArchive<Settings, FastCrc32>::input.checksum()));
			writeInteger(uint32_t(ODeflateArchive<Settings, FastCrc32>::input.getPosition() + ODeflateArchive<Settings, FastCrc32>::input.getPositionStart()));
		});
	}

public:
	OGzFile(const GzFileInfo<StringType>& header, std::function<void(std::span<const char> batch)> consumeFunction)
	: ODeflateArchive<Settings, FastCrc32>(consumeFunction) {
		header.writeOut(consumeFunction);
		fillTrailerWriter();
	}

#ifndef EZGZ_NO_FILE
	OGzFile(const GzFileInfo<StringType>& header) : ODeflateArchive<Settings, FastCrc32>(header.name) {
		header.writeOut([this] (std::span<const char> batch) {
			ODeflateArchive<Settings, FastCrc32>::output.addBytes(batch);
		});
		fillTrailerWriter();
	}
#endif
};


namespace Detail {
template <DecompressionSettings Settings = DefaultDecompressionSettings>
class IGzStreamBuffer : public std::streambuf {
	IGzFile<Settings> inputFile;
	int bytesToKeep = 10;
	ptrdiff_t produced = 0;
public:
	template<typename Arg>
	IGzStreamBuffer(const Arg& arg, int bytesToKeep) : inputFile(arg), bytesToKeep(bytesToKeep) {}

	int underflow() override {
		std::optional<std::span<const char>> batch = inputFile.readSome(bytesToKeep);
		if (batch.has_value()) {
			// We have to believe std::istream that it won't edit the data, otherwise it would be necessary to copy the data
			char* start = const_cast<char*>(batch->data());
			setg(start - std::min<ptrdiff_t>(bytesToKeep, produced), start, start + batch->size());
			produced += batch->size();
			return traits_type::to_int_type(*gptr());
		} else {
			return traits_type::eof();
		}
	}

	const GzFileInfo<typename Settings::StringType>& info() const {
		return inputFile.info();
	}
};

template <CompressionSettings Settings, BasicStringType StringType>
class OGzStreamBuffer : public std::streambuf {
	OGzFile<Settings, StringType> outputFile;
	typename OGzFile<Settings, StringType>::OpenWritingBuffer writeBuffer = {};

	void prepareBuffer() {
		writeBuffer = outputFile.openWritingBuffer();
		auto range = writeBuffer.accessRange();
		setp(reinterpret_cast<char*>(range.data()), reinterpret_cast<char*>(range.data()) + range.size());
	}
	void clearBuffer() {
		writeBuffer.finish(int(pptr() - pbase()));
	}
public:

public:
	OGzStreamBuffer(const GzFileInfo<StringType>& header, std::function<void(std::span<const char> batch)> consumeFunction)
		: outputFile(header, consumeFunction) {
		prepareBuffer();
	}

#ifndef EZGZ_NO_FILE
	OGzStreamBuffer(const GzFileInfo<StringType>& header) : outputFile(header) {
		prepareBuffer();
	}
#endif

	int_type overflow(int_type added) override {
		clearBuffer();
		prepareBuffer();
		writeBuffer.accessRange()[0] = static_cast<std::remove_reference_t<decltype(writeBuffer.accessRange()[0])>>(added);
		pbump(1);
		return 0;
	}
	virtual ~OGzStreamBuffer() noexcept(true) {
		clearBuffer();
	};
};
}

// Using IGzFile as std::istream, configurable
template <DecompressionSettings Settings = DefaultDecompressionSettings>
class BasicIGzStream : private Detail::IGzStreamBuffer<Settings>, public std::istream
{
public:
#ifndef EZGZ_NO_FILE
	// Open and read a file, always keeping the given number of characters specified in the second character (10 by default)
	BasicIGzStream(const std::string& sourceFile, int bytesToKeep = 10) : Detail::IGzStreamBuffer<Settings>(sourceFile, bytesToKeep), std::istream(this) {}
#endif
	// Read from a buffer
	BasicIGzStream(std::span<const uint8_t> data, int bytesToKeep = 10) : Detail::IGzStreamBuffer<Settings>(data, bytesToKeep),  std::istream(this) {}
	// Use a function that fills a buffer of data and returns how many bytes it wrote
	BasicIGzStream(std::function<int(std::span<uint8_t> batch)> readMoreFunction, int bytesToKeep = 10) : Detail::IGzStreamBuffer<Settings>(readMoreFunction, bytesToKeep), std::istream(this) {}
	// Read from an existing stream
	BasicIGzStream(std::istream& input, int bytesToKeep = 10) : Detail::IGzStreamBuffer<Settings>([&input] (std::span<uint8_t> batch) -> int {
		input.read(reinterpret_cast<char*>(batch.data()), batch.size());
		return input.gcount();
	}, bytesToKeep), std::istream(this) {}

	using Detail::IGzStreamBuffer<Settings>::info;
};

template <CompressionSettings Settings = DefaultCompressionSettings, BasicStringType StringType = std::string>
class BasicOGzStream : private Detail::OGzStreamBuffer<Settings, StringType>, public std::ostream {

public:
	BasicOGzStream(const GzFileInfo<StringType>& header, std::function<void(std::span<const char> batch)> consumeFunction)
		: Detail::OGzStreamBuffer<Settings, StringType>(header, consumeFunction), std::ostream(this) { }

#ifndef EZGZ_NO_FILE
	BasicOGzStream(const GzFileInfo<StringType>& header) : Detail::OGzStreamBuffer<Settings, StringType>(header), std::ostream(this) {	}
#endif
};


// Most obvious usage, default settings
using IGzStream = BasicIGzStream<>;
using OGzStream = BasicOGzStream<>;
} // namespace EzGz

#if ! EZGZ_HAS_CONCEPTS
#undef StreamSettings
#undef InputStreamSettings
#undef DecompressionSettings
#undef BasicStringType
#undef ByteReader
#undef DeduplicatingSearch
#undef CompressionSettings
#endif

#endif // EZGZ_HPP
