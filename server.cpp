#include "server.h"

Server::Server() : result(NULL), ptr(NULL), ListenSocket(INVALID_SOCKET), ClientSocket(INVALID_SOCKET) {
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
}

int Server::createSocket() {
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
    printf("Created socket\n");
    return 0;
}

int Server::bindSocket() {
    int iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    printf("Binded socket\n");
    return 0;
}

int Server::listenSocket() {
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %ld\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Listening...\n");
    return 0;
}

int Server::acceptConnection() {
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Accepted connection..\n");
    closesocket(ListenSocket);
    return 0;
}

int Server::disconnectSocket() {
    int iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}

int Server::receiveData() {
    char recvbuf[DEFAULT_BUFLEN];
    int iResult = recv(ClientSocket, recvbuf, DEFAULT_BUFLEN, 0);
    if (iResult > 0) {
        printf("Bytes received: %d\n", iResult);
    }
    else if (iResult == 0) {
        printf("Connection closing...\n");
    }
    else {
        printf("recv failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

int Server::readData(void* buf, int buflen) {
    char* pBuf = (char*)buf;
    while (buflen > 0) {
        int iResult = recv(ClientSocket, pBuf, buflen, 0);
        if (iResult < 0) {
            printf("receive failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        if (sendACK() == 1) {
            return 1;
        }
        pBuf += iResult;
        buflen -= iResult;
    }
    return 0;
}

int Server::readLong(long* value) {
    if (readData(value, sizeof(long))) {
        printf("Reading long failed\n");
        return 1;
    }
    *value = ntohl(*value);
    return 0;
}

int Server::readFile() {
    long fileNameSize{};
    char* receivebuf = new char[DEFAULT_BUFLEN];
    uint64_t fileSize{};

    if (readLong(&fileNameSize) == 1) {
        return 1;
    }
    if (readData(receivebuf, fileNameSize) == 1) {
        return 1;
    }
    if (readData(&fileSize, 8) == 1) {
        printf("Failed reading filesize\n");
        return 1;
    }

    std::string fileName(receivebuf, fileNameSize - 1);
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

    fp.close();
    delete[] receivebuf;
    return 0;
}

int Server::sendACK() {
    char buf[1] = { 1 };
    int iResult = send(ClientSocket, buf, 1, 0);
    if (iResult < 0) {
        printf("Sending ACK failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

void Server::startServer() {
    this->createSocket();
    this->bindSocket();
    this->listenSocket();
    this->acceptConnection();
    this->readFile();
    this->disconnectSocket();
}
