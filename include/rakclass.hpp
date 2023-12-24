#pragma once

#include <Windows.h>
#include <iostream>
#include <cstdint>
#include <bit>

#include "bitstream.hpp"

#pragma pack(push, 1)
struct PlayerID
{
	unsigned int binaryAddress;
	unsigned short port;
};

struct Packet
{
	unsigned short playerIndex;

	PlayerID playerId;

	unsigned int length;
	unsigned int bitSize;

	unsigned char* data;

	bool deleteData;
};

struct NetworkID
{
	static bool peerToPeerMode;

	PlayerID playerId;

	unsigned short localSystemId;
};

const NetworkID UNASSIGNED_NETWORK_ID = { 0xFFFFFFFF, 0xFFFF, 65535 };
#pragma pack(pop)

class RakClass
{
	public:
		static void SetupHooks();

		static bool onSendPacket(RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability, char orderingChannel, unsigned int binaryAddress, unsigned short port, bool broadcast, unsigned int connectMode);
		static bool onSendSystemPacket(unsigned int socket, unsigned char* data, unsigned int length, unsigned int binaryAddress, unsigned short port);

		static bool onReceiveSystemPacket(unsigned int binaryAddress, unsigned short port, unsigned char* data, unsigned int length);
		static bool onReceivePacket(unsigned char* data, unsigned int length);

		static void SendPacket(RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability, char orderingChannel);
		static void SendRPC(int rpcID, RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability);

        static void EmulateRPC(int rpcID, RakNet::BitStream* bitStream);

	private:
        static void* SetCallHook(uintptr_t HookAddress, void* DetourFunction)
        {
            uintptr_t OriginalFunction = *reinterpret_cast<uintptr_t*>(HookAddress + 1) + HookAddress + 5;
            DWORD oldProt;
            VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uintptr_t), PAGE_READWRITE, &oldProt);
            *reinterpret_cast<uintptr_t*>(HookAddress + 1) = reinterpret_cast<uintptr_t>(DetourFunction) - HookAddress - 5;
            VirtualProtect(reinterpret_cast<void*>(HookAddress + 1), sizeof(uintptr_t), oldProt, &oldProt);

            return reinterpret_cast<void*>(OriginalFunction);
        }

        static void* SetJmpHook(uintptr_t HookAddress, size_t HookSize, void* DetourFunction)
        {
            void* Trampoline = VirtualAlloc(0, HookSize + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

            if (Trampoline)
            {
                uintptr_t TrampolineJmpBack = reinterpret_cast<uintptr_t>(Trampoline) + HookSize;
                memcpy(Trampoline, reinterpret_cast<void*>(HookAddress), HookSize);

                DWORD oldProt;
                VirtualProtect(reinterpret_cast<void*>(HookAddress), HookSize, PAGE_READWRITE, &oldProt);
                memset(reinterpret_cast<void*>(HookAddress), 0x90, HookSize);
                *reinterpret_cast<unsigned char*>(HookAddress) = 0xE9;
                *reinterpret_cast<uintptr_t*>(HookAddress + 1) = reinterpret_cast<uintptr_t>(DetourFunction) - HookAddress - 5;
                VirtualProtect(reinterpret_cast<void*>(HookAddress), HookSize, oldProt, &oldProt);

                *reinterpret_cast<unsigned char*>(TrampolineJmpBack) = 0xE9;
                *reinterpret_cast<uintptr_t*>(TrampolineJmpBack + 1) = (HookAddress + HookSize) - TrampolineJmpBack - 5;

                return Trampoline;
            }

            return nullptr;
        }
};