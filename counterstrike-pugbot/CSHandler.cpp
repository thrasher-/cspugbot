#include "Includes.h"
#include "Main.h"
#include "Externs.h"

cCSHandler::cCSHandler() {
	// set socket to SOCKET_ERROR on start up.
	m_Socket = SOCKET_ERROR;
	m_bConnected = false;
	m_bRetry = true;
}
cCSHandler::~cCSHandler() {
	// if m_Socket is active, close socket on class destruction
	if (m_Socket)
		closesocket(m_Socket);
}

void cCSHandler::setCSServer(string csServer) { 
	// set CS Server
	m_csServer = csServer;
}

void cCSHandler::setCSPort(USHORT usPort) {  
	// set CS Server Port
	m_usPort = usPort;
}

void cCSHandler::setCSRcon(string csRcon) { 
	// set CS RCON password
	m_rconPassword = csRcon;
}

void cCSHandler::setRconChallenge(string csRconChallenge) {
	// set CS RCON challenge
	m_rconChallenge = csRconChallenge;
}

string cCSHandler::getCSServer() {
	// retrieve CS Server
	return m_csServer;
}

string cCSHandler::getRCONPassword() {
	// retrieve rcon password
	return m_rconPassword;
}

string cCSHandler::getRCONChallanege() {
	// retrieve rcon challenge
	return m_rconChallenge;
}

USHORT cCSHandler::getCSServerPort() {
	// retrieve CS Server port
	return m_usPort;
}

DWORD cCSHandler::Run(LPVOID) { 
	// let the magic begin
	Init();
	return 0;
}

string cCSHandler::convertHostToIP(string host) {
	// create in_addr and fill it with supplied hostname.
	in_addr inaddr;
	inaddr.S_un.S_addr = inet_addr(host.c_str());
	
	HOSTENT *pHostent = gethostbyname(host.c_str());
	// gethostbyname

	if (pHostent == 0)
		return 0;

	if (sizeof(inaddr) != pHostent->h_length)
		return NULL;

	// convert IP to string and return result
	inaddr.S_un.S_addr = *((unsigned long*) pHostent->h_addr);
	return (strdup(inet_ntoa(inaddr)));
}

string cCSHandler::getHostname() {
	char szHostname[64] = {0};

	// use gethostname to retrieve hostname of machine
	if (gethostname(szHostname, 64) == SOCKET_ERROR) 
		return "localhost";

	return szHostname;
}

bool cCSHandler::LogMessages()
{
	// fill sockaddr_in struct with server info
	SOCKADDR_IN sockAddr_in;

	// bind to port 5454
	sockAddr_in.sin_family = AF_INET;
	sockAddr_in.sin_port = htons(5454);
	sockAddr_in.sin_addr.s_addr = g_MainCtrl.IRCHandler.ResolveHost(getHostname());

	g_MainCtrl.DebugConsole.PrintText("Hostname: %s. LocalIP: %s", getHostname().c_str(), convertHostToIP(getHostname()).c_str());

	m_srvSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (m_srvSocket == INVALID_SOCKET) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to intialise m_srvSocket - Error %d", WSAGetLastError());
		return false;
	}

	if (bind(m_srvSocket, (LPSOCKADDR)&sockAddr_in, sizeof(sockAddr_in)) == SOCKET_ERROR) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to bind. - Error %d", WSAGetLastError());
		return false;
	}

	while (m_bConnected) {
		g_MainCtrl.CSHandler.RecvData(m_srvSocket);
	}

	return true;
}

bool cCSHandler::ConnectToHost() 
{
	// fill SOCKADDR_IN struct with server info
	SOCKADDR_IN sockAddr_in;

	sockAddr_in.sin_family = AF_INET;
	sockAddr_in.sin_port = htons(m_usPort);
	sockAddr_in.sin_addr.s_addr = g_MainCtrl.IRCHandler.ResolveHost(m_csServer.c_str());

	// intialise UDP Socket
	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);

	// check if socket is valid
	if (m_Socket == INVALID_SOCKET) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to intialise m_Socket - Error %d", WSAGetLastError());
		return false;
	}

	// check if connect was successfull
	if (connect(m_Socket, (LPSOCKADDR)&sockAddr_in, sizeof(sockAddr_in)) == SOCKET_ERROR) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to connect to host - Error %d", WSAGetLastError());
		return false;
	}

	return true;
}

void cCSHandler::RecvData(SOCKET socket) {
	char szBuffer[1024] = {0};
	string recvBuffer;

	// recv one byte at a time
	int iBytes = recv(socket, szBuffer, sizeof(szBuffer), 0);

	if (m_bConnected == false && iBytes <= 0) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to communicate with CS server, expected buffer - Error %d", WSAGetLastError());
		return;
	}

	m_bConnected = true;

	// check for socket error
	if (iBytes == SOCKET_ERROR) {
		m_bConnected = false;
		return;
	}

	recvBuffer += szBuffer;

	// if we find a \r\n, time for processing!
	if (recvBuffer.find("\n") != string::npos) {
		string::size_type i = recvBuffer.find("\n");
		recvBuffer = recvBuffer.substr(0, i);
		TokeniseBuffer(recvBuffer);
		recvBuffer = "";
	}
}

void cCSHandler::setRetry(bool bRetry) {
	m_bRetry = bRetry;
}

void cCSHandler::SendData(const char* szArgs, ...) 
{
	char szBuffer[1024] = {0};

	vsprintf(szBuffer, szArgs, (char*)&szArgs + _INTSIZEOF(szArgs));

	g_MainCtrl.DebugConsole.PrintText("Sending CS Buffer: %s", szBuffer);

	// send data to CS server
	strncat(szBuffer, "\r\n", 2);
	send(m_Socket, szBuffer, strlen(szBuffer), 0);
}

void cCSHandler::Init() {
	// validate member variables
	if (m_csServer.empty() || m_rconPassword.empty() || m_usPort < 0 || m_usPort > 65355) {
		g_MainCtrl.DebugConsole.PrintText("Error - Invalid CS server settings.");
		return;
	}

	while (m_bRetry)
	{
		g_MainCtrl.DebugConsole.PrintText("Attempting connection to CS server %s:%i", m_csServer.c_str(), m_usPort);

		// connect to CS server
		if (ConnectToHost())
		{
			// challange RCON
			SendData("ÿÿÿÿchallenge rcon");
			RecvData(m_Socket);

			while (m_bConnected) 
			{
				RecvData(m_Socket);
			}
		}
	}
}

void cCSHandler::TokeniseBuffer(string buffer) { 
// variables for tokenising
	vector<string> CSBuffer;
	string tempString;

	g_MainCtrl.DebugConsole.PrintText("CS Buffer Recieved: %s", buffer.c_str());

	for (string::size_type i = 0; i < buffer.length(); i++) 
	{
		// store single character in a char variable
		char szByte = buffer[i];

		// we have reached end of buffer
		if (i == buffer.length()-1) 
		{
			// add string to vector
			tempString += szByte; 
			CSBuffer.push_back(tempString);
		}

		if (szByte == ' ')
		{
			// found empty char, add string to vector
			CSBuffer.push_back(tempString); 
			tempString = "";	
		}
		else 
		{
			// no empty char, store byte.
			tempString += szByte;
		}
	}
	
	// Handle CS Buffer
	g_MainCtrl.CommandHandler.HandleCSBuffer(CSBuffer);
}
