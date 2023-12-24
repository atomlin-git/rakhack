#include "../../rakhack/headers/rakclass.hpp"

using namespace RakNet;

#define B32_3 3
#define B32_2 2
#define B32_1 1
#define B32_0 0

BitStream::BitStream()
{
	numberOfBitsUsed = 0;
	numberOfBitsAllocated = BITSTREAM_STACK_ALLOCATION_SIZE * 8;

	readOffset = 0;
	data = (unsigned char*)stackData;

	copyData = true;
}

BitStream::BitStream(int initialBytesToAllocate)
{
	numberOfBitsUsed = 0;
	readOffset = 0;

	if (initialBytesToAllocate <= BITSTREAM_STACK_ALLOCATION_SIZE)
	{
		data = (unsigned char*)stackData;
		numberOfBitsAllocated = BITSTREAM_STACK_ALLOCATION_SIZE * 8;
	}
	else
	{
		data = (unsigned char*)malloc(initialBytesToAllocate);
		numberOfBitsAllocated = initialBytesToAllocate << 3;
	}

	copyData = true;
}

BitStream::BitStream(char* _data, unsigned int lengthInBytes, bool _copyData)
{
	if (lengthInBytes <= 0) return;

	numberOfBitsUsed = lengthInBytes << 3;
	readOffset = 0;
	copyData = _copyData;
	numberOfBitsAllocated = lengthInBytes << 3;

	if (copyData)
	{
		if (lengthInBytes > 0)
		{
			if (lengthInBytes < BITSTREAM_STACK_ALLOCATION_SIZE)
			{
				data = (unsigned char*)stackData;
				numberOfBitsAllocated = BITSTREAM_STACK_ALLOCATION_SIZE << 3;
			}
			else data = (unsigned char*)malloc(lengthInBytes);

			memcpy(data, _data, lengthInBytes);

		}
		else data = 0;
	}
	else data = (unsigned char*)_data;
}

BitStream::~BitStream()
{
	if (copyData && numberOfBitsAllocated > BITSTREAM_STACK_ALLOCATION_SIZE << 3) free(data);
}

void BitStream::SetNumberOfBitsAllocated(const unsigned int lengthInBits)
{
	numberOfBitsAllocated = lengthInBits;
}

void BitStream::Reset(void)
{
	numberOfBitsUsed = 0;
	readOffset = 0;
}

void BitStream::WriteBool(const bool input)
{
	AddBitsAndReallocate(1);

	if (input)
	{
		int numberOfBitsMod8 = numberOfBitsUsed % 8;

		if (numberOfBitsMod8 == 0)
			data[numberOfBitsUsed >> 3] = 0x80;
		else
			data[numberOfBitsUsed >> 3] |= 0x80 >> (numberOfBitsMod8);
	}
	else {

		if ((numberOfBitsUsed % 8) == 0) data[numberOfBitsUsed >> 3] = 0;
	}

	numberOfBitsUsed++;
}

void BitStream::WriteFloat(const float input)
{
	unsigned int intval = *((unsigned int*)(&input));
	static unsigned char uint32w[4]{};

	uint32w[B32_3] = (intval >> 24) & (0x000000ff);
	uint32w[B32_2] = (intval >> 16) & (0x000000ff);
	uint32w[B32_1] = (intval >> 8) & (0x000000ff);
	uint32w[B32_0] = (intval) & (0x000000ff);

	WriteBits(uint32w, sizeof(intval) * 8, true);
}

void BitStream::WriteString(const char* input, const int numberOfBytes)
{
	if ((numberOfBitsUsed & 7) == 0)
	{
		AddBitsAndReallocate(BYTES_TO_BITS(numberOfBytes));
		memcpy(data + BITS_TO_BYTES(numberOfBitsUsed), input, numberOfBytes);
		numberOfBitsUsed += BYTES_TO_BITS(numberOfBytes);
	}
	else WriteBits((unsigned char*)input, numberOfBytes * 8, true);
}

void BitStream::WriteBitstream(const BitStream* bitStream)
{
	WriteBits(bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), false);
}

bool BitStream::ReadBool(bool& output)
{
	if (readOffset + 1 > numberOfBitsUsed) return false;

	if (data[readOffset >> 3] & (0x80 >> (readOffset++ % 8)))
		output = true;
	else
		output = false;

	return true;
}

bool BitStream::ReadFloat(float& output)
{
	unsigned int val = 0;

	if (Read(val) == false) return false;
	output = *((float*)(&val));
	return true;
}

bool BitStream::ReadString(char* output, const int numberOfBytes)
{
	return ReadBits((unsigned char*)output, numberOfBytes * 8);
}


bool BitStream::ReadCompressedFloat(float& output)
{
	return Read(output);
}

bool BitStream::ReadCompressed(double& output)
{
	return Read(output);
}

void BitStream::ResetReadPointer(void)
{
	readOffset = 0;
}

void BitStream::ResetWritePointer(void)
{
	numberOfBitsUsed = 0;
}

bool BitStream::ReadBit(void)
{
#pragma warning( disable : 4800 )
	return (bool)(data[readOffset >> 3] & (0x80 >> (readOffset++ % 8)));
#pragma warning( default : 4800 )
}

void BitStream::WriteAlignedBytes(const unsigned char* input, const int numberOfBytesToWrite)
{
	AlignWriteToByteBoundary();
	AddBitsAndReallocate(numberOfBytesToWrite << 3);

	memcpy(data + (numberOfBitsUsed >> 3), input, numberOfBytesToWrite);
	numberOfBitsUsed += numberOfBytesToWrite << 3;
}

bool BitStream::ReadAlignedBytes(unsigned char* output, const int numberOfBytesToRead)
{
	if (numberOfBytesToRead <= 0) return false;

	AlignReadToByteBoundary();

	if (GetNumberOfUnreadBits() < (numberOfBytesToRead << 3)) return false;

	memcpy(output, data + (readOffset >> 3), numberOfBytesToRead);

	readOffset += numberOfBytesToRead << 3;

	return true;
}

void BitStream::AlignWriteToByteBoundary(void)
{
	if (numberOfBitsUsed) numberOfBitsUsed += 8 - ((numberOfBitsUsed - 1) % 8 + 1);
}

void BitStream::AlignReadToByteBoundary(void)
{
	if (readOffset) readOffset += 8 - ((readOffset - 1) % 8 + 1);
}

void BitStream::WriteBits(const unsigned char* input, int numberOfBitsToWrite, const bool rightAlignedBits)
{
	AddBitsAndReallocate(numberOfBitsToWrite);

	int offset = 0;
	unsigned char dataByte;
	int numberOfBitsUsedMod8;

	numberOfBitsUsedMod8 = numberOfBitsUsed % 8;

	while (numberOfBitsToWrite > 0)
	{
		dataByte = *(input + offset);

		if (numberOfBitsToWrite < 8 && rightAlignedBits) dataByte <<= 8 - numberOfBitsToWrite;

		if (numberOfBitsUsedMod8 == 0)
			*(data + (numberOfBitsUsed >> 3)) = dataByte;
		else
		{
			*(data + (numberOfBitsUsed >> 3)) |= dataByte >> (numberOfBitsUsedMod8);

			if (8 - (numberOfBitsUsedMod8) < 8 && 8 - (numberOfBitsUsedMod8) < numberOfBitsToWrite)
			{
				*(data + (numberOfBitsUsed >> 3) + 1) = (unsigned char)(dataByte << (8 - (numberOfBitsUsedMod8)));
			}
		}

		if (numberOfBitsToWrite >= 8)
			numberOfBitsUsed += 8;
		else
			numberOfBitsUsed += numberOfBitsToWrite;

		numberOfBitsToWrite -= 8;
		offset++;
	}
}

void BitStream::SetData(const unsigned char* input, const int numberOfBits)
{
	if (numberOfBits <= 0) return;

	AddBitsAndReallocate(numberOfBits);
	memcpy(data, input, BITS_TO_BYTES(numberOfBits));

	numberOfBitsUsed = numberOfBits;
}

void BitStream::WriteCompressed(const unsigned char* input, const int size, const bool unsignedData)
{
	int currentByte = (size >> 3) - 1;
	unsigned char byteMatch;

	if (unsignedData)
	{
		byteMatch = 0;

	}
	else byteMatch = 0xFF;

	while (currentByte > 0)
	{
		if (input[currentByte] == byteMatch)
		{
			bool b = true;
			WriteBool(b);
		}
		else
		{
			bool b = false;
			WriteBool(b);
			WriteBits(input, (currentByte + 1) << 3, true);
			return;
		}

		currentByte--;
	}

	if ((unsignedData && ((*(input + currentByte)) & 0xF0) == 0x00) || (unsignedData == false && ((*(input + currentByte)) & 0xF0) == 0xF0))
	{
		bool b = true;
		WriteBool(b);
		WriteBits(input + currentByte, 4, true);
	}
	else
	{
		bool b = false;
		WriteBool(b);
		WriteBits(input + currentByte, 8, true);
	}
}

bool BitStream::ReadBits(unsigned char* output, int numberOfBitsToRead, const bool alignBitsToRight)
{
	if (readOffset + numberOfBitsToRead > numberOfBitsUsed) return false;

	int readOffsetMod8;
	int offset = 0;

	memset(output, 0, BITS_TO_BYTES(numberOfBitsToRead));

	readOffsetMod8 = readOffset % 8;

	while (numberOfBitsToRead > 0)
	{
		*(output + offset) |= *(data + (readOffset >> 3)) << (readOffsetMod8);

		if (readOffsetMod8 > 0 && numberOfBitsToRead > 8 - (readOffsetMod8))
			*(output + offset) |= *(data + (readOffset >> 3) + 1) >> (8 - (readOffsetMod8));

		numberOfBitsToRead -= 8;

		if (numberOfBitsToRead < 0)
		{
			if (alignBitsToRight) *(output + offset) >>= -numberOfBitsToRead;

			readOffset += 8 + numberOfBitsToRead;
		}
		else readOffset += 8;

		offset++;
	}

	return true;
}

bool BitStream::ReadCompressed(unsigned char* output, const int size, const bool unsignedData)
{
	int currentByte = (size >> 3) - 1;
	unsigned char byteMatch, halfByteMatch;

	if (unsignedData)
	{
		byteMatch = 0;
		halfByteMatch = 0;
	}
	else
	{
		byteMatch = 0xFF;
		halfByteMatch = 0xF0;
	}

	while (currentByte > 0)
	{
		bool b;

		if (ReadBool(b) == false) return false;

		if (b)
		{
			output[currentByte] = byteMatch;
			currentByte--;
		}
		else
		{

			if (ReadBits(output, (currentByte + 1) << 3) == false) return false;

			return true;
		}
	}

	if (readOffset + 1 > numberOfBitsUsed) return false;

	bool b;

	if (ReadBool(b) == false) return false;

	if (b)
	{
		if (ReadBits(output + currentByte, 4) == false) return false;
		output[currentByte] |= halfByteMatch;
	}
	else {
		if (ReadBits(output + currentByte, 8) == false) return false;
	}

	return true;
}

void BitStream::AddBitsAndReallocate(const int numberOfBitsToWrite)
{
	if (numberOfBitsToWrite <= 0) return;

	int newNumberOfBitsAllocated = numberOfBitsToWrite + numberOfBitsUsed;

	if (numberOfBitsToWrite + numberOfBitsUsed > 0 && ((numberOfBitsAllocated - 1) >> 3) < ((newNumberOfBitsAllocated - 1) >> 3))
	{
		newNumberOfBitsAllocated = (numberOfBitsToWrite + numberOfBitsUsed) * 2;

		int amountToAllocate = BITS_TO_BYTES(newNumberOfBitsAllocated);
		if (data == (unsigned char*)stackData)
		{
			if (amountToAllocate > BITSTREAM_STACK_ALLOCATION_SIZE)
			{
				data = (unsigned char*)malloc(amountToAllocate);
				memcpy((void*)data, (void*)stackData, BITS_TO_BYTES(numberOfBitsAllocated));
			}
		}
		else
		{
			data = (unsigned char*)realloc(data, amountToAllocate);
		}
	}

	if (newNumberOfBitsAllocated > numberOfBitsAllocated) numberOfBitsAllocated = newNumberOfBitsAllocated;
}

void BitStream::IgnoreBits(const int numberOfBits)
{
	readOffset += numberOfBits;
}

void BitStream::SetWriteOffset(const int offset)
{
	numberOfBitsUsed = offset;
}

int BitStream::GetNumberOfBitsUsed(void) const
{
	return numberOfBitsUsed;
}

int BitStream::GetNumberOfBytesUsed(void) const
{
	return BITS_TO_BYTES(numberOfBitsUsed);
}

int BitStream::GetReadOffset(void) const
{
	return readOffset;
}

int BitStream::GetNumberOfUnreadBits(void) const
{
	return numberOfBitsUsed - readOffset;
}

unsigned char* BitStream::GetData(void) const
{
	return data;
}