#include "../include/rakclass.hpp"

static std::uintptr_t OffsetNakedReceive[] = { 0x4FF91, 0x53341, 0xFF, 0xFF };
static std::uintptr_t OffsetPacketReceive[][3] = { { 0x311A1, 0x3CDA0, 0x372F0 }, { 0x34551, 0x40150, 0x3A6A0 }, { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF } };

static std::uintptr_t OffsetSendPacket[] = { 0x387D0, 0x3BB80, 0xFF, 0xFF };

static std::uintptr_t OffsetOnSendNaked[][2]  = { {0x4FFC0, 0x50046}, {0x53370, 0x533F6}, {0xFF, 0xFF}, {0xFF, 0xFF} };
static std::uintptr_t OffsetOnSendPacket[][2] = { {0x388E0, 0x389D4}, {0x3BC90, 0x3BD84}, {0xFF, 0xFF}, {0xFF, 0xFF} };

void* tr_send_naked_packet = nullptr;
void* tr_send_packet = nullptr;

static void* RakPeer = nullptr;

static unsigned int binaryAddress  = 0;
static unsigned short port = 0;

static unsigned int SampBase = 0;
static unsigned char sampVersion = 0;

naked_receive_t tr_naked_packet = nullptr;
packet_receive_t tr_receive_packet = nullptr;

void __stdcall HookProcessNetworkPacket(unsigned int binaryAddress, unsigned short serverPort, unsigned char* data, unsigned int length, void* rakPeer)
{
    binaryAddress = binaryAddress;
    port = serverPort;

    RakPeer = rakPeer;

    if (!RakClass::onReceiveSystemPacket(binaryAddress, serverPort, data, length)) return;
    return tr_naked_packet(binaryAddress, serverPort, (unsigned char*)data, length, rakPeer);
}

using receive_ignore_t = Packet*(__thiscall*)(void*);
receive_ignore_t ReceiveIgnoreRPC = nullptr;

using handle_rpc_t = bool(__thiscall*)(void*, char*, unsigned int, unsigned int, unsigned short);
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
            HandleRPCPacket(EDX, (char*)packet->data, packet->length, packet->binaryAddress, packet->port);
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

    __asm 
    {
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

    static unsigned int tr_return_naked = (SampBase + OffsetOnSendNaked[sampVersion][1]);

    if (RakClass::onSendSystemPacket(socket, date, len, binaryAddress, port))
        __asm jmp tr_send_naked_packet
        else 
        __asm jmp tr_return_naked

    __asm popad
}

void __declspec(naked) HookPacketSend(void)
{
    static RakNet::BitStream* bitStream = nullptr;
    static unsigned int priority = 0;
    static unsigned int reliability = 0;
    static unsigned int orderingChannel = 0;
    static unsigned int binaryAddress = 0;
    static unsigned int port = 0;
    static unsigned int broadcast = 0;

    __asm 
    {
        mov eax, [esp + 0x04]
        mov bitStream, eax
        mov eax, [esp + 0x08]
        mov priority, eax
        mov eax, [esp + 0x0C]
        mov reliability, eax
        mov eax, [esp + 0x10]
        mov orderingChannel, eax
        mov eax, [esp + 0x14]
        mov binaryAddress, eax
        mov eax, [esp + 0x18]
        mov port, eax
        mov eax, [esp + 0x1C]
        mov broadcast, eax
        pushad
    }

    static unsigned int tr_return_packet = (SampBase + OffsetOnSendPacket[sampVersion][1]);

    if(RakClass::onSendPacket(bitStream, priority, reliability, (char)orderingChannel, binaryAddress, port, broadcast, 0))
        __asm jmp tr_send_packet
        else
        __asm jmp tr_return_packet

    __asm popad
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

    tr_naked_packet = reinterpret_cast<naked_receive_t>(RakClass::SetCallHook(SampBase + OffsetNakedReceive[sampVersion], &HookProcessNetworkPacket));
    tr_receive_packet = reinterpret_cast<packet_receive_t>(RakClass::SetCallHook(SampBase + OffsetPacketReceive[sampVersion][0], &HookReceivePacket));

    tr_send_naked_packet = RakClass::SetJmpHook(SampBase + OffsetOnSendNaked[sampVersion][0], 8, &HookSocketSend);
    tr_send_packet = RakClass::SetJmpHook(SampBase + OffsetOnSendPacket[sampVersion][0], 9, &HookPacketSend);
}

void RakClass::SendPacket(RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability, char orderingChannel)
{
    if(!RakPeer) return;
    packet_send_t packetSend = nullptr;
    packetSend = std::bit_cast<packet_send_t>((SampBase + OffsetSendPacket[sampVersion]));
    packetSend(RakPeer, bitStream->GetData(), bitStream->GetNumberOfBytesUsed(), priority, reliability, orderingChannel, binaryAddress, port, false);
}

void RakClass::SendRPC(int rpcID, RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability)
{
    if(!RakPeer) return;
    RakNet::BitStream rpcBitStream;
    rpcBitStream.Write((unsigned char)20);
    rpcBitStream.Write((unsigned char)rpcID);
    rpcBitStream.WriteCompressed((unsigned int)bitStream->GetNumberOfBitsUsed());
    rpcBitStream.WriteBits((const unsigned char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), false);
    RakClass::SendPacket(&rpcBitStream, priority, reliability, 0);
}

void RakClass::EmulateRPC(int rpcID, RakNet::BitStream* bitStream)
{
    if(!RakPeer) return;
    RakNet::BitStream rpcBitStream;
    rpcBitStream.Write((unsigned char)20);
    rpcBitStream.Write((unsigned char)rpcID);
    rpcBitStream.WriteCompressed((unsigned int)bitStream->GetNumberOfBitsUsed());
    rpcBitStream.WriteBits((const unsigned char*)bitStream->GetData(), bitStream->GetNumberOfBitsUsed(), false);

    HandleRPCPacket = std::bit_cast<handle_rpc_t>((SampBase + OffsetPacketReceive[sampVersion][2]));
    HandleRPCPacket(RakPeer, (char*)rpcBitStream.GetData(), rpcBitStream.GetNumberOfBytesUsed(), binaryAddress, port);
}