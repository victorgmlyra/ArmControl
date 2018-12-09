#include "server.h"

Server::Server(std::string ip, int port)
{
	this->ip = ip;
	this->port = port;
}

int Server::initServer(int joint[])
{
	using namespace std;
	// Initailize winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	if (WSAStartup(ver, &wsData) != 0) {
		cerr << "Error initalizing winsock!" << endl;
		return 1;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET) {
		cerr << "Can't create a socket!" << endl;
	}

	// Bind an ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(this->port);
	//hint.sin_addr.S_un.S_addr = INADDR_ANY;
	InetPtonA(AF_INET, this->ip.c_str(), &hint.sin_addr.S_un.S_addr);
	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell winsock the socket is for listening
	listen(listening, SOMAXCONN);  // Number of maximum conections

	// Wait for a communication
	sockaddr_in client;
	int clientSize = sizeof(client);

	cout << "Waiting for connection..." << endl;
	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
	if (clientSocket == INVALID_SOCKET) {
		cerr << "Can't create socket!" << endl;
		getchar();
		return 1;
	}

	char host[NI_MAXHOST];			// Client's remote name
	char service[NI_MAXSERV];		// Service (i.e. port) the client is conected on

	ZeroMemory(host, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0) {
		cout << host << " connected on port " << service << endl;
	}
	else {
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " << ntohs(client.sin_port) << endl;
	}

	// Close listening socket
	closesocket(listening);

	// While loop:
	char buf[4096];
	string data;

	while (true) {
		ZeroMemory(buf, sizeof(buf));
		fill(data.begin(), data.end(), 0);

		// Wait for client to send data
		int bytesRecieved = recv(clientSocket, buf, sizeof(buf), 0);

		if (bytesRecieved == SOCKET_ERROR) {
			cerr << "Error in recieving data." << endl;
			return 1;
		}
		if (bytesRecieved == 0) {
			cout << "Client disconnected." << endl;
			break;
		}
		//cout << buf << endl;			// Print recieved message

		if (string_equal(buf, "<CONNECTION_OPEN>")) {				// Connection open
			data = "CONNECTION_OPEN;;";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_NUM_ARMS>")) {					// Number of arms
			data = "GET_NUM_ARMS;1;";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_ARM_CODNAME;1>")) {				// Arm codiname
			data = "GET_ARM_CODNAME;1;NS_00101";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_ARM_AXES;1>")) {				// Arm axes
			data = "GET_ARM_AXES;1;111111----";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_ARM_AUX;1>")) {					// Arm aux
			data = "GET_ARM_AUX;1;----------";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_ARM_BASE;1>")) {				// Arm base
			data = "GET_ARM_BASE;1;0;0;0;0;0;0";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_TOOL_RMT;1>")) {				// Arm tool RTM
			data = "GET_TOOL_RMT;1;False";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_ARM_ALL_FRAMES;1>")) {			// Arm all frames
			data = "GET_ARM_ALL_FRAMES;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_AUX_BASE;1>")) {				// Aux base
			data = "GET_AUX_BASE;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_ALL_JNT>")) {					// Get all arm joints
			data = "GET_ALL_JNT;" + to_string(joint[0]) + ";" + to_string(joint[1]) + ";" + to_string(joint[2]) + ";" + to_string(joint[3]) + ";" + to_string(joint[4]) + ";" + to_string(joint[5]) + ";0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
		if (string_equal(buf, "<GET_IR_TYPES>")) {
			data = "GET_IR_TYPES;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0";
			send(clientSocket, data.c_str(), data.size(), 0);
		}
	}

	// Close the socket
	closesocket(clientSocket);

	// Shutdown winsock
	WSACleanup();
	return 0;
}

bool Server::string_equal(const char* arg0, const char* arg1)
{
	using namespace std;
	string var0 = (string)arg0;
	string var1 = (string)arg1;
	if (var0 == var1) {
		return true;
	}
	else {
		return false;
	}
}