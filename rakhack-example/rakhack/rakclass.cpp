#include "../../rakhack/headers/rakclass.hpp"

static std::uintptr_t OffsetNakedReceive[] = { 0x4FF91, 0x53341, 0xFF, 0xFF };
static std::uintptr_t OffsetPacketReceive[][3] = { { 0x311A1, 0x3CDA0, 0x372F0 }, { 0x34551, 0x40150, 0x3A6A0 }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF } };

static std::uintptr_t OffsetSendPacket[] = { 0x388E0, 0x3BC90, 0xFF, 0xFF };
static std::uintptr_t OffsetSendRPC[] = { 0x3A560, 0x3E570, 0xFF, 0xFF };

static std::uintptr_t OffsetOnSendNaked[][2]  = { {0x4FFC0, 0x50046}, {0x53370, 0x533F6}, {0xFF, 0xFF}, {0xFF, 0xFF} };
static std::uintptr_t OffsetOnSendPacket[][2] = { {0x37490, 0x3751A}, {0x3A840, 0x3A8CA}, {0xFF, 0xFF}, {0xFF, 0xFF} };

using NakedPacketReceive = void(__stdcall*)(unsigned int, unsigned short, unsigned char*, unsigned int, void*);
NakedPacketReceive m_NakedPacket = nullptr;

using PacketReceive = Packet*(__fastcall*)(void*, void*);
PacketReceive m_ReceivePacket = nullptr;

void* m_SendNakedPacket = nullptr;
void* m_SendPacket = nullptr;

static uintptr_t SampBase;
static unsigned int sampVersion;

static void* RakPeer;
static PlayerID player;

void __stdcall HookProcessNetworkPacket(unsigned int binaryAddress, unsigned short serverPort, unsigned char* data, unsigned int length, void* this_)
{
    player.binaryAddress = binaryAddress;
    player.port = serverPort;

    RakPeer = this_;

    if (!RakClass::onReceiveSystemPacket(binaryAddress, serverPort, data, length))
        return;

    return m_NakedPacket(binaryAddress, serverPort, (unsigned char*)data, length, this_);
}

using receive_ignore_t = Packet*(__thiscall*)(void*);
receive_ignore_t ReceiveIgnoreRPC = nullptr;

using handle_rpc_t = bool(__thiscall*)(void*, char*, unsigned int, PlayerID);
handle_rpc_t HandleRPCPacket = nullptr;

Packet* __fastcall HookReceivePacket(void* EDX, void* rakPeer)
{
    ReceiveIgnoreRPC = std::bit_cast<receive_ignore_t>((SampBase + OffsetPacketReceive[sampVersion][1]));
    HandleRPCPacket = std::bit_cast<handle_rpc_t>((SampBase + OffsetPacketReceive[sampVersion][2]));

    Packet* packet = ReceiveIgnoreRPC(EDX);

    if (packet)
    {
        if (!RakClass::onReceivePacket(packet->data, packet->length))
            return NULL;

        if (packet->data[0] == 20)
            HandleRPCPacket(EDX, (char*)packet->data, packet->length, packet->playerId);
    }

    return packet;
}

void __declspec(naked) HookSocketSend(void)
{
    static int socket = 0;
    static unsigned char* date = nullptr;
    static int len = 0;
    static int binaryAddress = 0;
    static int port = 0;

    __asm {
        mov eax, [esp + 0x04]
        mov socket, eax
        mov eax, [esp + 0x08]
        mov date, eax
        mov eax, [esp + 0x0C]
        mov len, eax
        mov eax, [esp + 0x10]
        mov binaryAddress, eax
        mov eax, [esp + 0x14]
        mov port, eax
        pushad
    }

    static unsigned int m_SendNakedPacketExit = (SampBase + OffsetOnSendNaked[sampVersion][1]);

    if (RakClass::onSendSystemPacket(socket, date, len, binaryAddress, port))
    {
        __asm {
            popad
            jmp m_SendNakedPacket
        }
    }
    else {
        __asm {
            popad
            jmp m_SendNakedPacketExit
        }
    }
}

void __declspec(naked) HookPacketSend(void)
{
    static unsigned char* date = nullptr;
    static unsigned int len = 0;
    static unsigned int priority = 0;
    static unsigned int reliability = 0;
    static unsigned int orderingChannel = 0;
    static unsigned int binaryAddress = 0;
    static unsigned int port = 0;
    static unsigned int broadcast = 0;
    static unsigned int connectMode = 0;

    __asm {
        mov eax, [esp + 0x04]
        mov date, eax
        mov eax, [esp + 0x08]
        mov len, eax
        mov eax, [esp + 0x0C]
        mov priority, eax
        mov eax, [esp + 0x10]
        mov reliability, eax
        mov eax, [esp + 0x14]
        mov orderingChannel, eax
        mov eax, [esp + 0x18]
        mov binaryAddress, eax
        mov eax, [esp + 0x1C]
        mov port, eax
        mov eax, [esp + 0x20]
        mov broadcast, eax
        mov eax, [esp + 0x24]
        mov connectMode, eax
        pushad
    }

    static unsigned int m_SendPacketExit = (SampBase + OffsetOnSendPacket[sampVersion][1]);

    if(RakClass::onSendPacket(date, len, priority, reliability, (char)orderingChannel, binaryAddress, port, broadcast, connectMode))
    {
        __asm {
            popad
            jmp m_SendPacket
        }

    } else {
        __asm {
            popad
            jmp m_SendPacketExit
        }
    }
}

void RakClass::SetupHooks()
{
    SampBase = reinterpret_cast<uintptr_t>(GetModuleHandleA("samp.dll"));
    std::uintptr_t hexVersion = reinterpret_cast<IMAGE_NT_HEADERS*>(SampBase + reinterpret_cast<IMAGE_DOS_HEADER*>(SampBase)->e_lfanew)->OptionalHeader.AddressOfEntryPoint;

    switch (hexVersion)
    {
        case 0x31DF13: sampVersion = 0; break;
        case 0xCC4D0:  sampVersion = 1; break;
        case 0xCBCB0:  sampVersion = 2; break;
        case 0xFDB60:  sampVersion = 3; break;
    }

    m_NakedPacket = reinterpret_cast<NakedPacketReceive>(RakClass::SetCallHook(SampBase + OffsetNakedReceive[sampVersion], &HookProcessNetworkPacket));
    m_ReceivePacket = reinterpret_cast<PacketReceive>(RakClass::SetCallHook(SampBase + OffsetPacketReceive[sampVersion][0], &HookReceivePacket));

    m_SendNakedPacket = RakClass::SetJmpHook(SampBase + OffsetOnSendNaked[sampVersion][0], 8, &HookSocketSend);
    m_SendPacket = RakClass::SetJmpHook(SampBase + OffsetOnSendPacket[sampVersion][0], 9, &HookPacketSend);
}

using packet_send_t = int(__thiscall*)(void*, RakNet::BitStream*, unsigned int, unsigned int, char, PlayerID, bool);
using rpc_send_t = bool(__thiscall*)(void*, int*, char*, unsigned int, unsigned int, unsigned int, char, PlayerID, bool, bool, NetworkID, RakNet::BitStream*);

void RakClass::SendPacket(RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability, char orderingChannel)
{
    packet_send_t packetSend = nullptr;
    packetSend = std::bit_cast<packet_send_t>((SampBase + OffsetSendPacket[sampVersion]));
    packetSend(RakPeer, bitStream, priority, reliability, orderingChannel, player, false);
}

void RakClass::SendRPC(int* rpcID, RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability)
{
    rpc_send_t rpcSend = nullptr;
    rpcSend = std::bit_cast<rpc_send_t>((SampBase + OffsetSendRPC[sampVersion]));
    rpcSend(RakPeer, rpcID, std::bit_cast<char*>(bitStream->GetData()), bitStream->GetNumberOfBitsUsed(), priority, reliability, 0, player, false, false, UNASSIGNED_NETWORK_ID, NULL);
}