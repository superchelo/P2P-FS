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
#include <thread>


#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 1024
#define MAX_BUFFER 1024

class Server {
private:
    struct addrinfo* result, * ptr, hints;
    SOCKET ListenSocket;
    //SOCKET ClientSocket;
    std::vector<std::thread> threads;

    int createSocket();
    int bindSocket();
    int listenSocket();
    int acceptConnection();
    int disconnectSocket(SOCKET* ClientSocket);
    int receiveData(SOCKET* ClientSocket);
    int readData(void* buf, int buflen, SOCKET* ClientSocket);
    int readLong(long* value, SOCKET* ClientSocket);
    int readFile(SOCKET* ClientSocket);
    int sendACK(SOCKET* ClientSocket);
    void joinThreads();
    void downloadThread(SOCKET* ClientSocket);
public:
    Server();
    void startServer();
};

#endif // SERVER_H
