#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <stdint.h>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 1024
#define MAX_BUFFER 1024



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
		int sendData(void* buf, int buflen) {

			const char* sendbuf = (const char *)buf;
			
			while (buflen > 0) {
				int iResult = send(ConnectSocket, sendbuf, buflen, 0);
				if (iResult == SOCKET_ERROR) {
					printf("send failed: %d\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
				sendbuf += iResult;
				buflen -= iResult;
				//printf("Bytes Sent: %ld\n", iResult);
				if (receiveACK() == 1) {
					closesocket(ConnectSocket);
					WSACleanup();
					return 1;
				}
			}

			return 0;
		}
		int receiveData() {
			char recvbuf[MAX_BUFFER];
			int iResult = 0;
			// Receive data until the server closes the connection
			
				iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
				if (iResult > 0){
					//printf("Bytes Received: %d\n", iResult);
				}else if (iResult == 0) {
					printf("Connection closed\n");
					return 0;
				}
				else {
					printf("recv failed: %d\n", WSAGetLastError());
					return 1;
				}
			 //while (iResult > 0);
			return 0;
		}
		int receiveACK() {
			char recvbuf[1];
			int iResult = 0;
			// Receive data until the server closes the connection

			iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
			if (iResult > 0) {
				//printf("ACK Received: %d\n", iResult);
			}else if (iResult == 0) {
				printf("Connection closed\n");
				return 0;
			}
			else {
				printf("recv failed: %d\n", WSAGetLastError());
				return 1;
			}
			//while (iResult > 0);
			return 0;
		}
		
		int sendLong(long value) {
			//std::cout << "sending long\n";
			value = htonl(value);
			if (sendData(&value, sizeof(value)) == 1) {
				printf("sending long failed\n");
				return 1;
			}

			return 0;
		}
		int sendFile() {
			
			std::string filePath;
			std::cout << "Enter File path (dont use spaces): ";
			std::cin >> filePath;

			std::string fileName;
			std::cout << "Enter File name (dont use spaces): ";
			std::cin >> fileName;

			//filll buffer with file name
			char* sendbuf = new char[MAX_BUFFER];
			for (int i = 0; i < fileName.size(); i++) {
				sendbuf[i] = fileName[i];
			}
			//  send file name length
			sendbuf[fileName.size() + 1] = '\0';
			if (sendLong(fileName.size() + 1) == 1) {
				return 1;
			}
			// send file name
			if (sendData(sendbuf, fileName.size()+1) == 1) {
				return 1;
			}
			uint64_t fileSize = getFileSize(filePath);
			//std::cout << "file size is: " << fileSize << "\n";
			if (sendData(&fileSize, 8) == 1) {
				printf("failed to send file sizez\n");
				return 1;
			}
			std::fstream fp(filePath, std::ios::in | std::ios::binary);

			char* fileBuffer = new char[MAX_BUFFER];
			while (fp.read(fileBuffer, MAX_BUFFER) || fp.gcount() > 0) {
				size_t bytesRead = fp.gcount();
				if (sendData(fileBuffer, bytesRead) == 1) {
					return 1;
				}
			}
			std::cout << "finished sending file\n";
			fp.close();
			delete[] fileBuffer;
			delete[] sendbuf;
			return 0;
		}
		u_int64 getFileSize(std::string filePath) {
			std::filesystem::path p{ filePath };
			u_int64 size = std::filesystem::file_size(p);
			return size;
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
				this->sendFile();
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
					//iSendResult = send(ClientSocket, recvbuf, iResult, 0);
					int iSendResult{};
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
		int readData(void* buf, int buflen) {
			char* pBuf = (char*)buf;

			while (buflen > 0) {
				int iResult = recv(ClientSocket, pBuf, buflen, 0);
				
				if (iResult < 0) {
					printf("receive failed: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				//std::cout << "Received " << buflen << " Bytes\n";
				if (sendACK() == 1) {
					return 1;
				}
				pBuf += iResult;
				buflen -= iResult;
			}
			return 0;

		}
		int readLong(long* value) {
			if (readData(value, sizeof(long))) {
				printf("reading long failed\n");
				return 1;
			}
			*value = ntohl(*value);
			return 0;
		}
		int readFile() {
			//time_t start = time(NULL);
			long fileNameSize{};
			char* receivebuf = new char[DEFAULT_BUFLEN];
			uint64_t fileSize{};

			// receive length of file name
			if (readLong(&fileNameSize) == 1) {
				return 1;
			}
			// receive file name
			//std::cout << "File name size is: " << fileNameSize << "\n";
			if (readData(receivebuf, fileNameSize) == 1) {
				return 1;
			}
			if (readData(&fileSize, 8) == 1) {
				printf("failed reading filesize\n");
				return 1;
			}
			
			// create file name string
			std::string fileName(receivebuf, fileNameSize - 1);
			//std::cout << "File name: " << fileName << "\n";
			
			std::fstream fp(fileName.c_str(), std::ios::out | std::ios::binary);
			if (!fp) {
				std::cerr << "Error opening file!" << std::endl;
				return 1;
			}
			while (fileSize > 0) {

				uint64_t bufSize = std::min<uint64_t>(fileSize, (uint64_t)MAX_BUFFER);
				
				if (readData(receivebuf, bufSize) == 1) {
					return 1;
				}
				fileSize -= bufSize;
				fp.write(receivebuf, bufSize);
			}
			std::cout << "File Download Complete\n";
			//time_t taken = time(NULL) - start;
			//std::cout << "time taken: " << taken << std::endl;

			fp.close();
			delete[] receivebuf;
			return 0;
		}
		int sendACK() {
			char buf[1];
			buf[0] = 1;
			int iResult = send(ClientSocket, buf, 1, 0);
			if (iResult < 0) {
				printf("sending ACK failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			return 0;
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
			this->readFile();
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