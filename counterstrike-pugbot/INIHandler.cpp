#include "Includes.h"
#include "Main.h"
#include "Externs.h"

cINIHandler::cINIHandler(void) {
	m_fileName = "Settings.ini";
	RetrieveModulePath();
}

string cINIHandler::StripFilePath(string filePath)
{
	// strip file path
	string::size_type i = filePath.find_last_of("\\")+1;
	return (filePath = filePath.substr(0,  i));
}

bool cINIHandler::FileExists(string fileName)
{
	// check to see if file name exists
	if (GetFileAttributes(fileName.c_str()) == INVALID_FILE_ATTRIBUTES) {
		return false;
	}
	return true;
}

void cINIHandler::RetrieveModulePath()
{
	// retrieve module file
	char szModuleFile[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModuleFile, MAX_PATH);

	string modulePath = StripFilePath(szModuleFile);
	modulePath += m_fileName;
	m_fileName = modulePath;
}

bool cINIHandler::OpenFile()
{
	if (!FileExists(m_fileName)) {
		return false;
	}

	g_MainCtrl.DebugConsole.PrintText("Successfully opened settings.ini file");

	string ircServer, ircNickname, ircChannel;
	string csServer, rconPassword;
	USHORT usIRCPort, usCSPort;

	//Retrieving IRC server variables from settings
	GetPrivateProfileString ("IRCSERVER", "ircServer", NULL, (LPSTR)ircServer.c_str(), 50, m_fileName.c_str());
	usIRCPort = GetPrivateProfileInt ("IRCSERVER", "ircPort", 0, m_fileName.c_str());
	GetPrivateProfileString ("IRCSERVER", "ircNickname", NULL, (LPSTR)ircNickname.c_str(), 20, m_fileName.c_str());
	GetPrivateProfileString ("IRCSERVER", "ircChannel", NULL, (LPSTR)ircChannel.c_str(), 20, m_fileName.c_str());

	//Retrieving Counterstrike variables from settings
	GetPrivateProfileString ("CounterStrike", "csServer", NULL, (LPSTR)csServer.c_str(), 50, m_fileName.c_str());
	usCSPort = GetPrivateProfileInt ("CounterStrike", "csPort", 0, m_fileName.c_str());
	GetPrivateProfileString ("CounterStrike", "rconPassword", NULL, (LPSTR)rconPassword.c_str(), 50 ,m_fileName.c_str());

	//Set IRC settings.
	g_MainCtrl.IRCHandler.setIRCServer(ircServer.c_str());
	g_MainCtrl.IRCHandler.setIRCChannel(ircChannel.c_str());
	g_MainCtrl.IRCHandler.setIRCPort(usIRCPort);
	g_MainCtrl.IRCHandler.setIRCNick(ircNickname.c_str());

	//set CS settings.
	g_MainCtrl.CSHandler.setCSServer(csServer.c_str());
	g_MainCtrl.CSHandler.setCSRcon(rconPassword.c_str());
	g_MainCtrl.CSHandler.setCSPort(usCSPort);

	g_MainCtrl.DebugConsole.PrintText("[Settings.ini]:\n\nIRC Settings:\n\tServer: %s:%i.\n\tNickname: %s.\n\tChannel: %s.\n\nCS Settings:\n\tServer %s:%i.\n\tRCON Password: %s.\n", ircServer.c_str(),
		usIRCPort, ircNickname.c_str(), ircChannel.c_str(), csServer.c_str(), usCSPort, rconPassword.c_str());
	return true;
}
