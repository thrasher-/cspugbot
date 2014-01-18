#include "Includes.h"
#include "Main.h"
#include "Externs.h"

cMainControl g_MainCtrl;

int main()
{
	// Initalise debug console.
	g_MainCtrl.DebugConsole.PrintText("PugBot: Starting up..");
	g_MainCtrl.DebugConsole.PrintText("Debug file: %s", g_MainCtrl.DebugConsole.getLogFile().c_str());

	// Create mutex so we are the only pugbot process running
	if (WaitForSingleObject(CreateMutex(NULL, FALSE, "PugBot"), 30000) == WAIT_TIMEOUT)  
		ExitProcess(EXIT_FAILURE);

	WSADATA wsaData;
	
	// Initialise Winsock
	if (WSAStartup(MAKEWORD(2,0), &wsaData)) {
		g_MainCtrl.DebugConsole.PrintText("FATAL ERROR - Unable to start Winsock. %d", WSAGetLastError());
		return 0;
	}

	// Read data from ini file.
	if (!g_MainCtrl.INIHandler.OpenFile()) {
		g_MainCtrl.DebugConsole.PrintText("FATAL ERROR - Unable to find Settings.ini file.");
		return 0;
	}

	// Start IRC Thread
	 g_MainCtrl.IRCHandler.Start(NULL);

	// Start CS Thread
	g_MainCtrl.CSHandler.Start(NULL);

	_getch();

	return 0;
}
