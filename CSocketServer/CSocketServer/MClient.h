#pragma once
#include "pch.h"
#include "Packet.h"

class MClient
{
public:
	MClient(int);

	int getClientId();
	bool isContainingPackets();
	Packet getPacket();
	int checkTime();
	void packetAddition(Packet pckt);
	void setEvent();
	void waitForEvent();

	~MClient();

private:
	int _id;
	queue<Packet> _packets;
	CRITICAL_SECTION _critSec;
	HANDLE _hEvent = CreateEvent(NULL, TRUE, FALSE, "ReadyForPacket");
	int _time = 0;
	int _waitTime= 3000;
};
