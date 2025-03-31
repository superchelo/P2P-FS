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
		char ac[80];
		if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) {
			std::cerr << "Error " << WSAGetLastError() <<
				" when getting local host name.\n";
			return 1;
		}
		std::cout << "\nHost name is " << ac << ".\n";

		struct hostent* phe = gethostbyname(ac);
		if (phe == 0) {
			std::cerr << "Yow! Bad host lookup.\n";
			return 1;
		}
		std::cout << "Server Local IP Address(es):\n";
		for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			std::cout << "Address " << i << ": " << inet_ntoa(addr) << "\n";
		}
		std::cout << std::endl;
		Server server;
		server.startServer();
	}
	
	return 0;
}