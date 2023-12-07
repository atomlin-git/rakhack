Очередной RakNet хук для сампа, но с небольшими отличиями.

- Не тянет за собой rakclient interface
- Использует урезанный class битстрима
- Не тянет огромные либы для хуков

Функционал:

- Установить, и инициализировать хуки -> SetupHooks()

- Отправка пакета (SendPacket(RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability, char orderingChannel))
- Отправка RPC (SendRPC(int* id, RakNet::BitStream* bitStream, unsigned int priority, unsigned int reliability))

Events:

- bool onSendPacket(unsigned char* data, unsigned int numberOfBitsToSend, unsigned int priority, unsigned int reliability, char orderingChannel, unsigned int binaryAddress, unsigned short port, bool broadcast, unsigned int connectMode);
- bool onSendSystemPacket(unsigned int socket, unsigned char* data, unsigned int length, unsigned int binaryAddress, unsigned short port);
- bool onReceiveSystemPacket(unsigned int binaryAddress, unsigned short port, unsigned char* data, unsigned int length);
- bool onReceivePacket(unsigned char* data, unsigned int length);

TODOs:
- Добавить возможность эмуляции получения пакета/rpc
- Выполнить небольшой рефакторинг
- Добавить поддержку R4 и DL-R1

*Если вы хотите использовать этот ракнет хук для аризоны, то чтобы не ломался connectFix - закомментируйте строку "m_SendNakedPacket = RakClass::SetJmpHook(SampBase + OffsetOnSendNaked[sampVersion][0], 8, &HookSocketSend)", которая находится в rakclass.cpp
