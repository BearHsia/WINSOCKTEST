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

	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	printf("Accept a client socket.\n");
	// No longer need server socket
	closesocket(ListenSocket); // assume we have only one client
	
	fd_set clients_set;
	fd_set fdRead;
	fd_set fdWrte;
	fd_set fdExpt;
	FD_ZERO(&clients_set);
	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrte);
	FD_ZERO(&fdExpt);
	FD_SET(ClientSocket, &clients_set);

	struct timeval TimeOut = { 0,0 }; //make select as non-blocking
	bool echo_once = false;
	int echo_leng = 0;
	int loop_count = 0;
	while (true) {
		//printf("loop count: %d\n", loop_count);
		loop_count++;
		fdRead = clients_set;
		fdWrte = clients_set;
		fdExpt = clients_set;
		if (select(0, &fdRead, &fdWrte, &fdExpt, &TimeOut) == SOCKET_ERROR) {
			printf("select failed with error: %d\n", WSAGetLastError());
			for (int i = 0; i < clients_set.fd_count; i++) {
				closesocket(clients_set.fd_array[i]);
			}
			WSACleanup();
			system("pause");
			return 1;
		}
		else {
			for (int i = 0; i < clients_set.fd_count; i++) {
				if (FD_ISSET(clients_set.fd_array[i], &fdRead)) {
					int recvResult;
					recvResult = recv(clients_set.fd_array[i], recvbuf, recvbuflen, 0);
					if (recvResult > 0) {
						printf("Bytes received: %d\n", recvResult);
						recvbuf[recvResult] = '\0';
						printf("Recv: %s\n", recvbuf);
						echo_once = true;
						echo_leng = recvResult;
					}
					else if (recvResult == 0) {
						printf("Connection closing...\n");
						closesocket(clients_set.fd_array[i]);
						FD_CLR(clients_set.fd_array[i], &clients_set);
					}
				}
				if (FD_ISSET(clients_set.fd_array[i], &fdWrte) && echo_once) {
					// Echo the buffer back to the sender
					int sendResult;
					sendResult = send(clients_set.fd_array[i], recvbuf, echo_leng, 0);
					if (sendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						closesocket(clients_set.fd_array[i]);
						FD_CLR(clients_set.fd_array[i], &clients_set);
					}
					printf("Bytes sent: %d\n", sendResult);
					echo_once = false;
				}
				if (FD_ISSET(clients_set.fd_array[i], &fdExpt)) {
					printf("Socket error: %d\n", WSAGetLastError());
					closesocket(clients_set.fd_array[i]);
					FD_CLR(clients_set.fd_array[i], &clients_set);
				}
			}
		}
		if (clients_set.fd_count == 0) break;
	}
	
	WSACleanup();
	printf("Close successfully\n");
	system("pause");
	return 0;
}

