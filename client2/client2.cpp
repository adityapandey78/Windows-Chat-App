#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<thread>
#include<stdio.h>
#include<string>
using namespace std;
#pragma comment(lib,"ws2_32.lib")
/*
Steps for the client code
1.) initialise winsock
2.) create socket
3.) connect to the server
4.) send/recv
5.) close the socket
*/
bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}
void SendMsg(SOCKET s) {
	cout << "Enter your chat name:" << endl;
	string name;
	getline(cin, name);
	string message;
	while (1) {
		getline(cin, message);
		string msg = name + ":" + message;
		int bytesent = send(s, msg.c_str(), msg.length(), 0);
		if (bytesent == SOCKET_ERROR) {
			cout << "Error sending message.." << endl;
			break;
		}
		if (message == "quit") {
			cout << "Stopping the application.." << endl;
			break;
		}
	}
	closesocket(s);
	WSACleanup();
}
void ReceiveMessage(SOCKET s) {
	char buffer[4096];
	int recvlength;
	string msg = "";
	while (1) {
		recvlength = recv(s, buffer, sizeof(buffer), 0);
		if (recvlength <= 0) {
			cout << "Disconnected from server" << endl;
			break;
		}
		else {
			msg = string(buffer, recvlength);
			cout << msg << endl;
		}

	}
	closesocket(s);
	WSACleanup();
}
int main() {

	if (!Initialize()) {
		cout << "Initialization is failed" << endl;
		return 1;
	}
	//2.) Created socket
	SOCKET s;
	s = socket(AF_INET, SOCK_STREAM, 0); //same as server 
	if (s == INVALID_SOCKET) {
		cout << "invalid socket created" << endl;
		return 1;
	}

	// We will set up the adddresss
	int port = 12345;
	string serveraddress = "127.0.0.1";
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));

	//3.) connecting to the server
	if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
		cout << "Not able to connect to server" << endl;
		closesocket(s);
		WSACleanup();
		return 1;
	}
	cout << "Successfully connected to server.." << endl;
	/**[Now we will create two threads, one to send the data and other to recv, both will run parallely]**/

	thread senderthread(SendMsg, s);
	thread receiverthread(ReceiveMessage, s);
	senderthread.join();
	receiverthread.join();
	/*1.thread::join():
•	The join() function blocks the calling thread (in this case, the main thread) until		the thread on which it is called (senderthread or receiverthread) finishes			execution.
•	This ensures that the main thread does not terminate prematurely while the other		threads are still running.*/
	return 0;

}