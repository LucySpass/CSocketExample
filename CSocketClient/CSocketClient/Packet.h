#pragma once
#include "pch.h"

enum RecieverId {
	SERVER,
	ALL_CLIENTS = -1
};

enum PacketTypes {
	OK,
	REGISTER,
	SEND,
	EXIT
};

struct Header {
	int fromId;
	int toId;
	PacketTypes packetType;
	int packetSize;
};

class Packet
{
public:
	Packet(int from = SERVER, int to = 0, PacketTypes type = OK, const string&data = "");

	Header header;
	string text;

	void sendPacket(CSocket& socket);
	PacketTypes recievePacket(CSocket&socket);

	~Packet();
};
