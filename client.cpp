#include "Client.h"

Client::Client() : result(NULL), ptr(NULL), ConnectSocket(INVALID_SOCKET), recvbuflen(DEFAULT_BUFLEN) {
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
}

int Client::createSocket() {
    std::string userInput;
    std::cout << "Enter Server Address:\n";
    std::cin >> userInput;
    const char* ipadd = userInput.c_str();

    int iResult = getaddrinfo(ipadd, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    ptr = result;
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    return 0;
}

int Client::conSocket() {
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

int Client::disconnectSocket() {
    int iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ConnectSocket);
    WSACleanup();

    printf("Socket Disconnected\n");
    return 0;
}

int Client::sendData(void* buf, int buflen) {
    const char* sendbuf = (const char*)buf;

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

        if (receiveACK() == 1) {
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
    }
    return 0;
}

int Client::receiveData() {
    char recvbuf[MAX_BUFFER];
    int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

    if (iResult > 0) {
        // Data received
    }
    else if (iResult == 0) {
        printf("Connection closed\n");
        return 0;
    }
    else {
        printf("recv failed: %d\n", WSAGetLastError());
        return 1;
    }
    return 0;
}

int Client::receiveACK() {
    char recvbuf[1];
    int iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

    if (iResult > 0) {
        // ACK received
    }
    else if (iResult == 0) {
        printf("Connection closed\n");
        return 0;
    }
    else {
        printf("recv failed: %d\n", WSAGetLastError());
        return 1;
    }
    return 0;
}

int Client::sendLong(long value) {
    value = htonl(value);
    if (sendData(&value, sizeof(value)) == 1) {
        printf("sending long failed\n");
        return 1;
    }
    return 0;
}

int Client::sendFile() {
    std::string filePath;
    std::cout << "Enter File path (dont use spaces): ";
    std::cin >> filePath;

    std::string fileName;
    std::cout << "Enter File name (dont use spaces): ";
    std::cin >> fileName;

    char* sendbuf = new char[MAX_BUFFER];
    for (size_t i = 0; i < fileName.size(); i++) {
        sendbuf[i] = fileName[i];
    }

    sendbuf[fileName.size() + 1] = '\0';
    if (sendLong(fileName.size() + 1) == 1) {
        delete[] sendbuf;
        return 1;
    }

    if (sendData(sendbuf, fileName.size() + 1) == 1) {
        delete[] sendbuf;
        return 1;
    }

    uint64_t fileSize = getFileSize(filePath);
    if (sendData(&fileSize, 8) == 1) {
        printf("failed to send file size\n");
        delete[] sendbuf;
        return 1;
    }

    std::fstream fp(filePath, std::ios::in | std::ios::binary);
    char* fileBuffer = new char[MAX_BUFFER];

    while (fp.read(fileBuffer, MAX_BUFFER) || fp.gcount() > 0) {
        size_t bytesRead = fp.gcount();
        if (sendData(fileBuffer, bytesRead) == 1) {
            delete[] fileBuffer;
            delete[] sendbuf;
            return 1;
        }
    }

    std::cout << "finished sending file\n";
    fp.close();
    delete[] fileBuffer;
    delete[] sendbuf;
    return 0;
}

uint64_t Client::getFileSize(std::string filePath) {
    std::filesystem::path p{ filePath };
    return std::filesystem::file_size(p);
}

void Client::startClient() {
    createSocket();
    conSocket();
    sendFile();
    disconnectSocket();
}
