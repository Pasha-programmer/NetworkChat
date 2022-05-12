#include <string>
#include <WinSock2.h>
#pragma once
using namespace std;

class ClientConnection{
public: SOCKET Socket;
		string Name;
		/*ClientConnection(SOCKET socket, string name) {
			Socket = socket;
			Name = name;
		}*/
};

