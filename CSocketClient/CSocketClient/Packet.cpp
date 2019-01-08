#include "pch.h"
#include "Packet.h"


Packet::Packet(int from, int to, PacketTypes type, const string&data) {
	header.fromId = from;
	header.toId = to;
	header.packetType = type;
	header.packetSize = data.length();
	text = data;
}

void Packet::sendPacket(CSocket& socket) {
	socket.Send(&header, sizeof(Header));
	if (header.packetSize != 0) {
		socket.Send(text.c_str(), header.packetSize + 1);
	}
}

PacketTypes Packet::recievePacket(CSocket&socket) {
	socket.Receive(&header, sizeof(Header));
	if (header.packetSize != 0) {
		char *buff = new char[header.packetSize + 1];
		socket.Receive(buff, header.packetSize + 1);
		text = buff;
		delete[]buff;
	}
	return header.packetType;
}

Packet::~Packet()
{
}
