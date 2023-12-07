#include <Windows.h>
#include <iostream>

#include "../../rakhack/headers/rakclass.hpp"

bool RakClass::onSendPacket(unsigned char* data, unsigned int numberOfBitsToSend, unsigned int priority, unsigned int reliability, char orderingChannel, unsigned int binaryAddress, unsigned short port, bool broadcast, unsigned int connectMode)
{
    printf("\nsendPacket: %d id\n", data[0]);

    return true;
}

bool RakClass::onSendSystemPacket(unsigned int socket, unsigned char* data, unsigned int length, unsigned int binaryAddress, unsigned short port)
{
    printf("\nsendSystem: %d id\n", data[0]);

    return true;
}

bool RakClass::onReceiveSystemPacket(unsigned int binaryAddress, unsigned short port, unsigned char* data, unsigned int length)
{
    printf("\nreceiveSystemPacket: %d id\n", data[0]);

    return true;
}

bool RakClass::onReceivePacket(unsigned char* data, unsigned int length)
{
    printf("\nreceivePacket: %d id\n", data[0]);

    return true;
}


BOOL APIENTRY DllMain ( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            RakClass::SetupHooks();

            break;
        }
    }
    return TRUE;
}

