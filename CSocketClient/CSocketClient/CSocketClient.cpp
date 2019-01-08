// CSocketClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "Packet.h"
#include "MClient.h"
#include "CSocketClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object

CWinApp theApp;

using namespace std;
CRITICAL_SECTION cs;

int getUserInput(string menu) {
	int input = 0;
	do {
		cout << menu << endl;
		cout << "Your choice is: ";
		cin >> input;
		cout << endl;
	} while (input < 0 || input>2);

	return input;
}

void startClient() {
	InitializeCriticalSection(&cs);
	bool exit = false;

	CSocket ClientSocket;
	ClientSocket.Create();
	if (ClientSocket.Connect("192.168.0.105", 8080)) {
		cout << "Client connected successfully" << endl;
	}
	else {
		cout << "Client connection error!" << endl;
	}

	EnterCriticalSection(&cs);

	/*
	char charMessage[50];
	cin >> charMessage;
	CString buffer = charMessage;

	cout << "Sending message: '" << charMessage << "' ..." << endl;

	// CString buffer = " Hello World";
	ClientSocket.Send(buffer, 500);
	ClientSocket.Close();
	*/

	Packet sentPacket(0, SERVER, REGISTER);
	sentPacket.sendPacket(ClientSocket);

	Packet recievedPacket;
	PacketTypes packetType = recievedPacket.recievePacket(ClientSocket);
	MClient mclient(recievedPacket.header.toId);

	LeaveCriticalSection(&cs);

	if (packetType == REGISTER) {
		while (!exit) {
			cout << "Connected successfully with id: " << recievedPacket.header.toId << endl;

			EnterCriticalSection(&cs);
			int userInput = getUserInput("*** MENU *** \n 1) Send message \n 2) Disconnect \n");

			switch (userInput) {
			case 1: {
				int ui = getUserInput("Send to who? \n 1) All clients \n 2) Specific client \n");
				cout << "Enter message:" << endl;
				string mssg;
				cin >> mssg;

				cout << "got message" << endl;

				switch (ui) {
				case 1: {
					Packet sentToAllPacket(mclient.getClientId(), ALL_CLIENTS, SEND, mssg);
					sentToAllPacket.sendPacket(ClientSocket);
					cout << "sent message?" << endl;
					Packet answerPacket;
					PacketTypes type = answerPacket.recievePacket(ClientSocket);
					if (type == OK) {
						cout << "Messages were sent!" << endl;
					}
					break;
				}
				case 2: {
					int cl;
					cout << "Input client id: ";
					cin >> cl;
					cout << endl;

					Packet sentToClientPacket(mclient.getClientId(), cl, SEND, mssg);
					sentToClientPacket.sendPacket(ClientSocket);

					Packet answerPacket;
					PacketTypes type = answerPacket.recievePacket(ClientSocket);
					if (type == OK) {
						cout << "Message was sent!" << endl;
					}
					break;
				}
				default: {
					break;
				}
				}
				break;
			}
			case 2: {
				Packet deletionPakcet(mclient.getClientId(), SERVER, SEND);
				deletionPakcet.sendPacket(ClientSocket);

				Packet answerPacket;
				PacketTypes type = answerPacket.recievePacket(ClientSocket);
				if (type == OK) {
					cout << "Client disconnected!" << endl;
					exit = true;
				}
				break;
			}
			default: {
				break;
			}
			}
			LeaveCriticalSection(&cs);
		}
	}

	DeleteCriticalSection(&cs);
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
			cout << "Client socket initialized" << endl;
			startClient();
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

	return nRetCode;
}
