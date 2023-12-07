#pragma once

#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

typedef signed		__int64 int64_t;
typedef unsigned	__int64 uint64_t;

#define BITSTREAM_STACK_ALLOCATION_SIZE 256

namespace RakNet
{
	class BitStream
	{
	public:
		~BitStream();
		BitStream();
		BitStream(int initialBytesToAllocate);
		BitStream(char* _data, unsigned int lengthInBytes, bool _copyData);

		void Reset(void);

		template <class templateType>
		void Write(templateType var);

		template <class templateType>
		void WriteCompressed(templateType var);

		template <class templateType>
		bool Read(templateType& var);

		template <class templateType>
		bool ReadCompressed(templateType& var);

		void WriteBool(const bool input);
		void WriteFloat(const float input);
		void WriteString(const char* input, const int numberOfBytes);
		void WriteBitstream(const BitStream* bitStream);

		bool ReadFloat(float& output);
		bool ReadBool(bool& output);
		bool ReadString(char* output, const int numberOfBytes);

		bool ReadCompressedFloat(float& output);
		bool ReadCompressed(double& output);

		void ResetReadPointer(void);
		void ResetWritePointer(void);

		void IgnoreBits(const int numberOfBits);
		void SetWriteOffset(const int offset);

		int GetNumberOfBitsUsed(void) const;
		int GetNumberOfBytesUsed(void) const;
		int GetReadOffset(void) const;
		int GetNumberOfUnreadBits(void) const;

		void SetData(const unsigned char* input, const int numberOfBits);
		unsigned char* GetData(void) const;

		void WriteBits(const unsigned char* input, int numberOfBitsToWrite, const bool rightAlignedBits = true);
		void WriteAlignedBytes(const unsigned char* input, const int numberOfBytesToWrite);

		bool ReadAlignedBytes(unsigned char* output, const int numberOfBytesToRead);

		void AlignWriteToByteBoundary(void);
		void AlignReadToByteBoundary(void);

		bool ReadBits(unsigned char* output, int numberOfBitsToRead, const bool alignBitsToRight = true);
		bool ReadBit(void);

		void SetNumberOfBitsAllocated(const unsigned int lengthInBits);

	private:
		void WriteCompressed(const unsigned char* input, const int size, const bool unsignedData);
		bool ReadCompressed(unsigned char* output, const int size, const bool unsignedData);
		void AddBitsAndReallocate(const int numberOfBitsToWrite);

		int numberOfBitsUsed;
		int numberOfBitsAllocated;
		int readOffset;

		unsigned char* data;

		bool copyData;

		unsigned char stackData[BITSTREAM_STACK_ALLOCATION_SIZE]{};
	};

	template <class templateType>
	inline void BitStream::Write(templateType var)
	{
		WriteBits((unsigned char*)&var, sizeof(templateType) * 8, true);
	}

	template <class templateType>
	inline void BitStream::WriteCompressed(templateType var)
	{
		WriteCompressed((unsigned char*)&var, sizeof(templateType) * 8, true);
	}

	template <class templateType>
	inline bool BitStream::Read(templateType& var)
	{
		return ReadBits((unsigned char*)&var, sizeof(templateType) * 8, true);
	}

	template <class templateType>
	inline bool BitStream::ReadCompressed(templateType& var)
	{
		return ReadCompressed((unsigned char*)&var, sizeof(templateType) * 8, true);
	}
}