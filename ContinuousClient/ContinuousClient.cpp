#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define DEFAULT_PORT "1294"
#define DEFAULT_BUFLEN 512

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <iphlpapi.h>
#include <stdio.h>
#include<stdlib.h> // for system pause

#pragma comment(lib, "Ws2_32.lib") //make VS happy

int main(int argc, char *argv[]) {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendbuf[DEFAULT_BUFLEN];
	char recvbuf[DEFAULT_BUFLEN];
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	// Validate the parameters
	if (argc != 2) {
		printf("usage: %s server-name\n", argv[0]);
		system("pause");
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		system("pause");
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		system("pause");
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	int timeout = 3000; //3s
	int ret = setsockopt(ConnectSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	int loopcount = 0;
	while (true) {
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);
			recvbuf[iResult] = '\0';
			//printf("Recv: %s\n", recvbuf);
			for (int i = 0; i < iResult; i++) {
				printf("%c", recvbuf[i]);
			}
			printf("\n");
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
			break;
		}
		else {
			int errnumber = WSAGetLastError();
			if (errnumber == 10060) {
				printf("TIMEOUT\n");
			}
			else {
				printf("recv failed with error: %d\n", errnumber);
				closesocket(ConnectSocket);
				WSACleanup();
				system("pause");
				return 1;
			}
		}


		char user_input[50];
		printf("send this time?(y\\n\\b)");
		scanf_s("%s", user_input, 50);
		if (user_input[0]=='y') {
			loopcount++;
			sprintf_s(sendbuf,"this is a test.%3d", loopcount);
			// Send an initial buffer
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes Sent: %ld\n", iResult);
		}
		else if (user_input[0] == 'n') {
			//do nothing
		}
		else if (user_input[0] == 'b') {
			break;
		}
	}
	

	// shutdown the connection for sending since no more data will be sent
	// the client can still use the ConnectSocket for receiving data
	iResult = shutdown(ConnectSocket, SD_SEND); //this will trigger the server to close
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		system("pause");
		return 1;
	}

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	system("pause");
	return 0;
}