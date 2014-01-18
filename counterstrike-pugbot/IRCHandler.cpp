#include "Includes.h"
#include "Main.h"
#include "Externs.h"

cIRCHandler::cIRCHandler() 
{ 
	// class constructor, assign variables
	m_Socket = SOCKET_ERROR;
	m_bConnected = false;
}

cIRCHandler::~cIRCHandler() 
{
	// class desconstructor
	if (m_Socket)
		closesocket(m_Socket);
}

void cIRCHandler::setIRCChannel(string ircChannel) {
	// set IRC Channel
	m_ircChannel = ircChannel.c_str();
}

void cIRCHandler::setIRCNick(string ircNick) {
	// set IRC Nickname
	m_ircNick = ircNick.c_str();
}

void cIRCHandler::setIRCServer(string IRCServer) {
	// set IRC Server
	m_ircServer = IRCServer.c_str();
}

void cIRCHandler::setIRCPort(USHORT usPort) {
	// set IRC Port
	m_usPort = usPort;
}

USHORT cIRCHandler::getIRCPort() {
	// retrieve IRC Port
	return m_usPort;
}

string cIRCHandler::getIRCChannel() {
	// retrieve IRC Channel
	return m_ircChannel.c_str();
}

string cIRCHandler::getIRCServer() {
	// retrieve IRC Server
	return m_ircServer.c_str();
}

string cIRCHandler::getIRCNick() {
	// retrieve IRC Nick
	return m_ircNick.c_str();
}

DWORD cIRCHandler::Run(LPVOID) { 
	// let the magic begin
	Init();
	return 0;
}

void cIRCHandler::Init()
{
	// validate member variables
	if (m_ircServer.empty() || m_ircChannel.empty() || m_ircNick.empty() || m_usPort < 0 || m_usPort > 65355) {
		g_MainCtrl.DebugConsole.PrintText("Error - Invalid IRC server settings.");
		return;
	}

	g_MainCtrl.DebugConsole.PrintText("Attempting connection to IRC server %s:%i", m_ircServer.c_str(), m_usPort);

	// connect to IRC server
	while (1)
	{
		if (ConnectToHost()) {
			g_MainCtrl.DebugConsole.PrintText("Successfully connected to %s:%i.", m_ircServer.c_str(), m_usPort);
			SendData("NICK %s", getIRCNick().c_str());
			SendData("USER %s 0 0 0", getIRCNick().c_str());

			while (m_bConnected) {
				RecvData();
			}
		}

		Sleep(1000 * 10);
	}
}

void cIRCHandler::TokeniseBuffer(string buffer)
{
	// variables for tokenising
	vector<string> ircBuffer;
	string tempString;

	for (string::size_type i = 0; i < buffer.length(); i++) 
	{
		// store single character in a char variable
		char szByte = buffer[i];

		// we have reached end of buffer
		if (i == buffer.length()-1) 
		{
			// add string to vector
			tempString += szByte; 
			ircBuffer.push_back(tempString);
		}

		if (szByte == ' ')
		{
			// found empty char, add string to vector
			ircBuffer.push_back(tempString); 
			tempString = "";	
		}
		else 
		{
			// no empty char, store byte.
			tempString += szByte;
		}
	}
	
	// Handle IRC Buffer
	g_MainCtrl.CommandHandler.HandleIRCBuffer(ircBuffer);
}

void cIRCHandler::RecvData()
{
	char szBuffer[2] = {0};
	string recvBuffer;

	while (1)
	{
		// recv one byte at a time
		int iBytes = recv(m_Socket, szBuffer, 1, 0);

		// check for socket error
		if (iBytes == SOCKET_ERROR) 
			return;

		recvBuffer += szBuffer;

		// if we find a carrage return, it means we have recieved a full line. Time for processing!
		if (recvBuffer.find("\r\n") != string::npos) {
			string::size_type i = recvBuffer.find("\r\n");
			recvBuffer = recvBuffer.substr(0, i);
			TokeniseBuffer(recvBuffer);
			recvBuffer = "";
		}
	}
}

void cIRCHandler::SendMessage(const char* szArgs, ...)
{
	// send message to IRC server
	if (!m_bConnected)
		return;

	char szBuffer[1024] = {0};
	char szFormat[512] = {0};

	// compile args and store in buffer
	vsprintf(szFormat, szArgs, (char*)&szArgs + _INTSIZEOF(szArgs));
	_snprintf_s(szBuffer, sizeof(szBuffer), 1024, "PRIVMSG %s :%s", m_ircChannel.c_str(), szFormat);

	g_MainCtrl.DebugConsole.PrintText("Sending IRC Message: %s\n", szBuffer);

	strncat(szBuffer, "\r\n", 2);
	send(m_Socket, szBuffer, strlen(szBuffer), 0);
}

void cIRCHandler::SendData(const char* szArgs, ...)
{
	// create buffer and format args
	char szBuffer[1024] = {0};

	vsprintf(szBuffer, szArgs, (char*)&szArgs + _INTSIZEOF(szArgs));

	g_MainCtrl.DebugConsole.PrintText("Sending IRC Buffer: %s", szBuffer);

	// send data to IRC server
	strncat(szBuffer, "\r\n", 2);
	send(m_Socket, szBuffer, strlen(szBuffer), 0);
}

bool cIRCHandler::ConnectToHost()
{
	// fill SOCKADDR_IN struct with server info
	SOCKADDR_IN sockAddr_in;

	sockAddr_in.sin_family = AF_INET;
	sockAddr_in.sin_port = htons(m_usPort);
	sockAddr_in.sin_addr.s_addr = ResolveHost(m_ircServer);

	// intialise TCP socket
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// check if socket is valid
	if (m_Socket == INVALID_SOCKET) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to intialise m_Socket - Error %d", WSAGetLastError());
		return false;
	}

	// check if connect was successfull
	if (connect(m_Socket, (LPSOCKADDR)&sockAddr_in, sizeof(sockAddr_in)) == SOCKET_ERROR) {
		g_MainCtrl.DebugConsole.PrintText("Error - Unable to connect to IRC server - Error %d", WSAGetLastError());
		return false;
	}

	m_bConnected = true;

	return true;
}

ULONG cIRCHandler::ResolveHost(string targetIP)
{
	// resolve IP address
	ULONG ulIP = inet_addr(targetIP.c_str());

	if (ulIP == INADDR_NONE) {
		hostent *pHostent = gethostbyname(targetIP.c_str());

		if (pHostent == 0) 
			return INADDR_NONE;

		ulIP = *((ULONG * )pHostent->h_addr_list[0]);
	}

	return ulIP;
}



