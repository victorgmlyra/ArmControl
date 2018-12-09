#pragma once
#ifndef server_h					   // Guarda de inclusão (evita problemas de compilação caso o usuário inclua a biblioteca mais de uma vez)
#define server_h

#include <iostream>
#include <string>
#include <WS2tcpip.h>					// Includes WinSock
#pragma comment (lib, "ws2_32.lib")		// Add "ws_32.lib" library

class Server
{
public:
	Server(std::string ip, int port);		// Constructor, recieve ip adress and port number		
	int initServer(int joint[]);			// Initializes server

private:
	std::string ip;												// ip adress
	int port;													// Port number
	bool string_equal(const char* arg0, const char* arg1);		// Function to compare char*
};

#endif

