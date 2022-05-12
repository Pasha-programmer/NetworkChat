#include <stdarg.h>
#include <iostream>
#include <WinSock2.h>
#include "ClientM.h"
#include <string>
#include <csignal>
#include <Windows.h>

#define _WONSOCK_DEPRECATED_NO_WARNINGS

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib")

using namespace std;

SOCKET Connection;
int iResult;
string name;

enum Packet {
	P_ChatMessage,
	P_Test,
	P_SetName
};

bool ProcessPacket(Packet packetType) {
	switch (packetType) {
	case P_ChatMessage: {
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		std::cout << msg << std::endl;
		delete[] msg;
		break;
	}
	case P_Test: {
		std::cout << "Test packet\n";
		break;
	}
	default: {
		std::cout << "Unrecongnized packet: " << packetType << std::endl;
		break;
	}
	}
	return true;
}
void ClientHandler() {
	Packet packetType;
	while (true) {
		iResult = recv(Connection, (char*)&packetType, sizeof(Packet), NULL);
		if (iResult == 0){
			printf("Connection closed\n");
			break;
		}
		else if(iResult < 0){
			printf("recv failed: " + WSAGetLastError());
			break;
		}

		if (!ProcessPacket(packetType)) {
			break;
		}
	}
	closesocket(Connection);
}

void ctrl_c(int sig) {
	std::cerr << "Ctrl-C caught" << std::endl;
	signal(sig, ctrl_c); // re-installs handler
}

void getLocalSetting() {
	setlocale(LC_ALL, "Russian");
}

int main(int args, char* argv[]) {
	getLocalSetting();

	void (*old)(int);
	old = signal(SIGINT, ctrl_c); // installs handler
	//SetConsoleMode(CTRL_C_EVENT, ENABLE_PROCESSED_INPUT);
	SetConsoleCtrlHandler(NULL, true);

	/*cout << "IP-adress: ";
	string ip;
	getline(cin, ip);*/

	//WSAStartUp
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	iResult = WSAStartup(DllVersion, &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed :" + iResult << std::endl;
		exit(1);
	}

	SOCKADDR_IN addr;
	int sizeAddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");//inet_addr((char*)&ip); 
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	//процесс1 привязки имени к клиенту
	cout << "Your name: ";
	getline(cin, name);
	int name_size = name.size();

	//connect to server
	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (Connection == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
		std::cout << "Error: failed connect to server.\n";
		return 1;
	}

	std::cout << "Connected to server.\n";
	
	//thread for eventer listen server
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

	std::string msg;

	//процесс2 привязки имени к клиенту
	Packet packetType = P_SetName;
	send(Connection, (char*)&packetType, sizeof(Packet), NULL);//send packet
	send(Connection, (char*)&name_size, sizeof(int), NULL);//send size message
	send(Connection, name.c_str(), name_size, NULL);//send message

	while(true) {	
		getline(cin, msg);
		int msg_size = msg.size();
		if (cin.fail() || cin.eof() || msg_size <= 0) {
			if(signal(CTRL_C_EVENT, ctrl_c) == SIG_ERR)
				cin.clear();
				continue;
			continue;
		}
		Packet packetType = P_ChatMessage;
		send(Connection, (char*)&packetType, sizeof(Packet), NULL);//send packet
		send(Connection, (char*)&msg_size, sizeof(int), NULL);//send size message
		send(Connection, msg.c_str(), msg_size, NULL);//send message
		Sleep(10);
	}

	signal(SIGINT, old); // restore initial handler
	SetConsoleCtrlHandler(NULL, false);

	// cleanup
	closesocket(Connection);
	WSACleanup();

	system("pause");
	return 0;
}



