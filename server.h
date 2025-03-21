#pragma once
#ifndef SERVER_H
#define SERVER_H

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

class Server {
private:
    struct addrinfo* result, * ptr, hints;
    SOCKET ListenSocket;
    SOCKET ClientSocket;

    int createSocket();
    int bindSocket();
    int listenSocket();
    int acceptConnection();
    int disconnectSocket();
    int receiveData();
    int readData(void* buf, int buflen);
    int readLong(long* value);
    int readFile();
    int sendACK();

public:
    Server();
    void startServer();
};

#endif // SERVER_H
