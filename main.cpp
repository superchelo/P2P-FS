#include "client.h"
#include "server.h"

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