Очередной RakNet хук для сампа, но с небольшими отличиями.

1) Не тянет за собой rakclient interface
2) Использует урезанный class битстрима
3) Не тянет огромные либы для хуков

Функционал:

1) Отправка пакета (SendPacket(RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability, char orderingChannel))
2) Отправка RPC (SendRPC(int* id, RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability))

Events:

1) bool onSendPacket(unsigned char* data, unsigned int numberOfBitsToSend, unsigned int priority, unsigned int reliability, char orderingChannel, unsigned int binaryAddress, unsigned short port, bool broadcast, unsigned int connectMode);
2) bool onSendSystemPacket(unsigned int socket, unsigned char* data, unsigned int length, unsigned int binaryAddress, unsigned short port);
3) bool onReceiveSystemPacket(unsigned int binaryAddress, unsigned short port, unsigned char* data, unsigned int length);
4) bool onReceivePacket(unsigned char* data, unsigned int length);

*Если вы хотите использовать этот ракнет хук для аризоны, чтобы не ломался connectFix - закомментируйте строку "m_SendNakedPacket = RakClass::SetJmpHook(SampBase + OffsetOnSendNaked[sampVersion][0], 8, &HookSocketSend)", которая находится в rakclass.cpp

TODOs:
1) Добавить возможность эмуляции получения пакета/rpc
2) Выполнить небольшой рефакторинг
3) Добавить поддержку R4 и DL-R1
