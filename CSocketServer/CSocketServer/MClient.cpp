#include "pch.h"
#include "MClient.h"


MClient::MClient(int id)
{
	InitializeCriticalSection(&_critSec);
	EnterCriticalSection(&_critSec);

	_id = id;
	_time = clock();

	LeaveCriticalSection(&_critSec);
}

int MClient::getClientId()
{
	return _id;
}

void MClient::packetAddition(Packet pckt)
{
	EnterCriticalSection(&_critSec);

	_packets.push(pckt);
	_time = clock();

	LeaveCriticalSection(&_critSec);
}

bool MClient::isContainingPackets()
{
	return !_packets.empty();
}

Packet MClient::getPacket()
{
	EnterCriticalSection(&_critSec);

	Packet temp = _packets.front();
	_packets.pop();
	_time = clock();

	LeaveCriticalSection(&_critSec);

	return temp;
}

int MClient::checkTime()
{
	return _time;
}

void MClient::setEvent()
{
	SetEvent(_hEvent);
}

void MClient::waitForEvent()
{
	WaitForSingleObject(_hEvent, _waitTime);
}

MClient::~MClient()
{
	DeleteCriticalSection(&_critSec);
}
