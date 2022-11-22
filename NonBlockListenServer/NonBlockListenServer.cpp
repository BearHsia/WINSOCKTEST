#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include<stdlib.h> // for system pause

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1294"

int main()
{
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; // use local IP

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	printf("Create a listener.\n");
	// Create a SOCKET for the server to listen for client connections.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// make listening socket as non-blocking
	unsigned long ul = 1;
	iResult = ioctlsocket(ListenSocket, FIONBIO, (unsigned long *)&ul);

	printf("Bind a listener.\n");
	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	printf("Listen to a listener socket.\n");
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	while (true) {
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			//closesocket(ListenSocket);
			//WSACleanup();
			//return 1;
		}
		else {
			break;
		}
	}
	
	printf("Accept a client socket.\n");
	// No longer need server socket
	closesocket(ListenSocket); // assume we have only one client

	bool echo_once = false;

	while (true) {
		int recvResult;
		recvResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (recvResult > 0) {
			printf("Bytes received: %d\n", recvResult);
			recvbuf[recvResult] = '\0';
			printf("Recv: %s\n", recvbuf);
			echo_once = true;
		}
		else if (recvResult == 0) {
			printf("Connection closing...\n");
			closesocket(ClientSocket);
			break;
		}
		else {
			//printf("recv failed with error: %d\n", WSAGetLastError());
		}
		int sendResult;
		if (echo_once) {
			sendResult = send(ClientSocket, recvbuf, recvResult, 0);
			if (sendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
			}
			else {
				printf("Bytes sent: %d\n", sendResult);
			}
			echo_once = false;
		}
	}

	WSACleanup();
	printf("Close successfully\n");
	system("pause");
	return 0;
}

