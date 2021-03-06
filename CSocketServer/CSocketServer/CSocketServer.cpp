// CSocketServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Packet.h"
#include "MClient.h"
#include "CSocketServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// The one and only application object

CWinApp theApp;

using namespace std;

CRITICAL_SECTION critSec;
HANDLE _hExit = CreateEvent(NULL, TRUE, FALSE, "EventConfirm");

int waitTime = 10 * CLOCKS_PER_SEC;

map<int, shared_ptr<MClient>> clientsContainer;

int clientsCount = 0;

void sendingPackets(SOCKET socket, MClient mclient) {
	CSocket ClientSocket;
	ClientSocket.Attach(socket);

	while (mclient.isContainingPackets()) {
		Packet savedPacket = mclient.getPacket();
		savedPacket.sendPacket(ClientSocket);
		mclient.setEvent();
	}
}

void eraseClient(SOCKET socket, int clientId) {
	CSocket sckt;
	sckt.Attach(socket);

	clientsContainer.erase(clientId);
	Packet sentPacket(SERVER, clientId, EXIT);
	sentPacket.sendPacket(sckt);

	sckt.Close();
}

void checkTime(SOCKET socket) {
	CSocket sock;
	sock.Attach(socket);
	while (1) {
		Sleep(waitTime);
		int start = clock();
		for (auto client : clientsContainer) {
			int  time = abs(start - client.second->checkTime()) / CLOCKS_PER_SEC;
			if (time >= 60) {
				eraseClient(sock, client.first);
			}
		}
	}
}

void sendConfirmationMessage(SOCKET sckt, int clientId) {
	CSocket socket;
	socket.Attach(sckt);

	Packet sentPacket(SERVER, clientId, OK);
	sentPacket.sendPacket(socket);
}

/*
 * SOCKET - The new type to be used in all
 * instances which refer to sockets.
 */
void processClientThread(SOCKET sckt) {
	bool exit = false;
	InitializeCriticalSection(&critSec);

	CSocket ClientSocket;
	ClientSocket.Attach(sckt);

	Packet recievedPacket;
	PacketTypes type = recievedPacket.recievePacket(ClientSocket);

	EnterCriticalSection(&critSec);
	cout << "Packet type: " << type << endl;
	while (!exit) {
		switch (type) {
		case REGISTER: {
			auto pClient = make_shared <MClient>(++clientsCount);
			clientsContainer[pClient->getClientId()] = pClient;

			cout << "Registered new client with " << clientsCount << " id!" << endl;

			Packet sentPacket(SERVER, pClient->getClientId(), REGISTER);
			sentPacket.sendPacket(ClientSocket);
			type = OK;
			break;
		}
		case SEND: {
			if (clientsContainer.find(recievedPacket.header.fromId) != clientsContainer.end()) {
				auto toClient = clientsContainer.find(recievedPacket.header.toId);
				if (toClient != clientsContainer.end()) {
					cout << "Sending data to " << recievedPacket.header.toId << endl;
					toClient->second->packetAddition(recievedPacket);

					sendConfirmationMessage(ClientSocket, recievedPacket.header.fromId);

					MClient temp = *toClient->second;
					thread sending(sendingPackets, ClientSocket.Detach(), temp);
					sending.detach();
				}
				else if (recievedPacket.header.toId == ALL_CLIENTS) {
					cout << "Sending data to all clients" << endl;
					for (auto client : clientsContainer) {
						if (client.first != recievedPacket.header.fromId) {
							client.second->packetAddition(recievedPacket);

							MClient temp = *client.second;
							thread sending(sendingPackets, ClientSocket.Detach(), temp);
							sending.detach();
						}
					}
					sendConfirmationMessage(ClientSocket, recievedPacket.header.fromId);
				}
				else {
					cout << "Reciever id (" << recievedPacket.header.toId << ") is not registered!" << endl;
				}
			}
			else {
				cout << "Sender id (" << recievedPacket.header.fromId << ") is not registered!" << endl;
			}
			type = OK;
			break;
		}
		case EXIT: {
			sendConfirmationMessage(ClientSocket, recievedPacket.header.fromId); 
			eraseClient(ClientSocket, recievedPacket.header.fromId);
			cout << "Client with " << clientsCount << " id left." << endl;
			exit = true;
			type = OK;
			break;
		}
		default: {
			break;

		}
		}
	}
	LeaveCriticalSection(&critSec);

	/*char *pBuf = new char[1025];
	int n = ClientSocket.Receive(pBuf, 500);
	pBuf[n] = NULL;
	cout << pBuf << endl;

	ClientSocket.Close();
	delete[] pBuf;*/
}

void startServer() {
	CSocket ServerSocket;
	ServerSocket.Create(8080);
	ServerSocket.Bind(8080);

	thread time(checkTime, ServerSocket.m_hSocket);
	time.detach();

	while (ServerSocket.Listen()) {
		CSocket ClientSocket;
		ServerSocket.Accept(ClientSocket);
		thread clientThread(processClientThread, ClientSocket.Detach());
		clientThread.detach();
	}
}

int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: code your application's behavior here.
			wprintf(L"Fatal Error: MFC initialization failed\n");
			nRetCode = 1;
		}
		else if (AfxSocketInit())
		{
			cout << "Server socket initialized" << endl;
			startServer();
			// TODO: code your application's behavior here.
		}
		else if (!AfxSocketInit()) {
			cout << "Socket intitialization error!" << endl;
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		wprintf(L"Fatal Error: GetModuleHandle failed\n");
		nRetCode = 1;
	}
	DeleteCriticalSection(&critSec);

	return nRetCode;
}
