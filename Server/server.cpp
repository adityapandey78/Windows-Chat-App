#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<tchar.h>
#include<thread>
#include<vector>
using namespace std;

//linking the winsock library
#pragma comment(lib, "ws2_32.lib")


/*
 1.) Initialise winsock lib
 2.) Create the socket
 3.) create IP and Port
 4.) Bind the I/P and Port with the socket we create
 5.) We start listening of the socket
 6.) Accept the connection --This will be the blocking call, Tip: explore the non blocking call as well
 7.) read the data from the socket and send
 8.) close the socket
*/

//1.) initialing the winsocklib

bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}
void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients) {
	cout << "client connected.." << endl;

	//send recv new clients

	char buffer[4096]; //it will store the message
	while (1) {
		
		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
		
		if (bytesrecvd <= 0) {
			cout << "Client Disconnected..." << endl;
			break;
		}
		string message(buffer, bytesrecvd);
		cout << "Message from the client received: " << message << endl;


		for (auto client : clients) {
			//server broadcast to everyone in network except the sender
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}

	}
	//in-case of connction closure, we will remove that connection from the vector as well
	auto it = find(clients.begin(), clients.end(), clientSocket);

	if (it != clients.end()) {
		clients.erase(it);
	}
	closesocket(clientSocket);
}
int main() {
	if (!Initialize()) {
		cout << "winsock intialisationis failed" << endl;
	}

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	/* Socket involves 3 parts
	=> AF_INET :: is for IPV4 addressing
	=> SOCK_STREAM :: This is TCP type Socket
	=> 0 :: this para is for Protocol we kept as 0 so that service provider will decide the type of protocol or stream */

	if (listenSocket == INVALID_SOCKET) {
		cout << "Socket Creating Failed" << endl;
		return 1;
	}

	//2.) Creating the address structure
	int portNum = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(portNum);

	// Setting up the address structure
	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "setting address structure failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// 4.) bind the listen socket and serveraddrees structure
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << " bind field failed!" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// 5.) listen:> listen function takes the input to be listened and max no of liistens it can handle we have used here SOMAXCONN
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "Listen Failed";
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	};
	cout << "Server has started listening on port" << portNum << endl;
	vector<SOCKET>clients;
	while (1) {
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		//[**The server wwill accept connection from the client if it always remains on accept, so in order to achieve that we weill first start the connection and then move it to a thread**]//

		if (clientSocket == INVALID_SOCKET) {
			cout << "Invalid Client Socket" << endl;
		}
		clients.push_back(clientSocket); //storing all the clients' socket in the vector 

		//here we will move the connection to the thread
		thread tl(InteractWithClient, clientSocket, std::ref(clients));
		tl.detach();
	}

	closesocket(listenSocket);
	WSACleanup();


	cout << "hello form the Server!" << endl;
	WSACleanup(); // 8.) Closing the socket and finalization is done
	return 0;

}