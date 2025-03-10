#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>


#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512


class Client {
	private:
		struct addrinfo *result, *ptr, hints;
		SOCKET ConnectSocket;
		int recvbuflen;

		int createSocket() {
			
			std::string userInput;
			std::cout << "Enter Server Address:\n";
			std::cin >> userInput;
			const char* ipadd = userInput.c_str();

			// Resolve the server address and port
			int iResult = getaddrinfo(ipadd, DEFAULT_PORT, &hints, &result);
			if (iResult != 0) {
				printf("getaddrinfo failed: %d\n", iResult);
				WSACleanup();
				return 1;
			}

			ptr = result;

			// Create a SOCKET for connecting to server
			ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

			if (ConnectSocket == INVALID_SOCKET) {
				printf("Error at socket(): %ld\n", WSAGetLastError());
				freeaddrinfo(result);
				WSACleanup();
				return 1;
			}
			return 0;
		}
		int conSocket() {

			// Connect to server.
			int iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(ConnectSocket);
				ConnectSocket = INVALID_SOCKET;
				freeaddrinfo(result);
				printf("Unable to connect to server!\n");
				WSACleanup();
				return 1;
			}
			return 0;
		}
		int disconnectSocket() {
			// shutdown the send half of the connection since no more data will be sent
			int iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			// cleanup
			closesocket(ConnectSocket);
			WSACleanup();
			
			printf("Socket Disconnected\n");
			return 0;
		}
		int sendData() {

			std::string input;
			std::cout << "Enter a Message: ";
			std::cin >> input;
			const char* sendbuf = input.c_str();

			// Send an initial buffer
			int iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}

			printf("Bytes Sent: %ld\n", iResult);

			// shutdown the connection for sending since no more data will be sent
			// the client can still use the ConnectSocket for receiving data
			iResult = shutdown(ConnectSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			return 0;
		}
		int receiveData() {
			char recvbuf[DEFAULT_BUFLEN];
			int iResult = 0;
			// Receive data until the server closes the connection
			do {
				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				if (iResult > 0)
					printf("Bytes sent: %d\n", iResult);
				else if (iResult == 0) {
					printf("Connection closed\n");
					return 0;
				}
				else {
					printf("recv failed: %d\n", WSAGetLastError());
					return 1;
				}
			} while (iResult > 0);
			return 0;
		}
		int sendFile() {
			//C:\Users\simon\Desktop\New Text Document (3).txt
			FILE* File;
			char* Buffer;
			unsigned long Size;
			File = fopen("C:\\Users\\simon\\Desktop\\New Text Document (3).txt", "rb");
			if (!File)
			{
				printf("Error while readaing the file\n");
				return 1;
			}

			fclose(File);
			return 0;
		}
		public:
			Client() :result(NULL), ptr(NULL), ConnectSocket(INVALID_SOCKET), recvbuflen(DEFAULT_BUFLEN) {
				ZeroMemory(&hints, sizeof(hints));
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
			}
			void startClient() {
				this->createSocket();
				this->conSocket();
				this->sendData();
				this->receiveData();
				this->disconnectSocket();
			}
};

class Server {
	private:
		struct addrinfo* result, * ptr, hints;
		SOCKET ListenSocket;
		SOCKET ClientSocket;
	
		int createSocket() {

			// Resolve the local address and port to be used by the server
			int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
			if (iResult != 0) {
				printf("getaddrinfo failed: %d\n", iResult);
				WSACleanup();
				return 1;
			}

			ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

			if (ListenSocket == INVALID_SOCKET) {
				printf("Error at socket(): %ld\n", WSAGetLastError());
				freeaddrinfo(result);
				WSACleanup();
				return 1;
			}
			printf("created socket\n");
			return 0;
		}
		int bindSocket() {
			// Setup the TCP listening socket
			int iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				printf("bind failed with error: %d\n", WSAGetLastError());
				freeaddrinfo(result);
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}
			freeaddrinfo(result);
			printf("binded socket\n");

			return 0;
		}
		int listenSocket() {
			if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
				printf("Listen failed with error: %ld\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}
			printf("listening...\n");

			return 0;
		}
		int acceptConnection() {
			

			// Accept a client socket
			ClientSocket = accept(ListenSocket, NULL, NULL);
			if (ClientSocket == INVALID_SOCKET) {
				printf("accept failed: %d\n", WSAGetLastError());
				closesocket(ListenSocket);
				WSACleanup();
				return 1;
			}
			printf("accepted connection..\n");

			//REMOVE LATER
			// No longer need server socket
			closesocket(ListenSocket);

			return 0;
		}
		int disconnectSocket() {
			int iResult = shutdown(ClientSocket, SD_SEND);
			if (iResult == SOCKET_ERROR) {
				printf("shutdown failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			// cleanup
			closesocket(ClientSocket);
			WSACleanup();

			return 0;
		}
		int receiveData() {
			char recvbuf[DEFAULT_BUFLEN];
			int iResult, iSendResult;
			int recvbuflen = DEFAULT_BUFLEN;
			
			// Receive until the peer shuts down the connection
			do {

				iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
				if (iResult > 0) {
					printf("Bytes received: %d\n", iResult);

					// Echo the buffer back to the sender
					iSendResult = send(ClientSocket, recvbuf, iResult, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed: %d\n", WSAGetLastError());
						closesocket(ClientSocket);
						WSACleanup();
						return 1;
					}
					printf("Bytes sent: %d\n", iSendResult);
				}
				else if (iResult == 0)
					printf("Connection closing...\n");
				else {
					printf("recv failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}

			} while (iResult > 0);
		}
	public:
		Server() :result(NULL), ptr(NULL), ListenSocket(INVALID_SOCKET), ClientSocket(INVALID_SOCKET) {
			ZeroMemory(&hints, sizeof(hints));
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			hints.ai_flags = AI_PASSIVE;
		}
		void startServer() {
			this->createSocket();
			this->bindSocket();
			this->listenSocket();
			this->acceptConnection();
			this->receiveData();
			this->disconnectSocket();
		}
};


int main(int argc, char* argv[]) {
	
	WSADATA wsaData;

	int iResult{};

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout <<"WSAStartup failed:" << iResult << "\n";
		return 1;
	}

	std::string userInput;
	std::cout << "For Server enter 0; Client 1\n";
	std::cin >> userInput;
	if (userInput == "1") {
		Client client;
		client.startClient();
	}
	else {
		Server server;
		server.startServer();
	}
	
	return 0;
}