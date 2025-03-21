#pragma once
#ifndef CLIENT_H
#define CLIENT_H

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
    struct addrinfo* result, * ptr, hints;
    SOCKET ConnectSocket;
    int recvbuflen;

    int createSocket();
    int conSocket();
    int disconnectSocket();
    int sendData(void* buf, int buflen);
    int receiveData();
    int receiveACK();
    int sendLong(long value);
    int sendFile();
    uint64_t getFileSize(std::string filePath);

public:
    Client();
    void startClient();
};

#endif // CLIENT_H
