#include "Includes.h"
#include "Main.h"
#include "Externs.h"

cDebugConsole::cDebugConsole() 
{
	// generate log file, allocate console and set console title.
	SetupLogFile();
	SetConsoleTitle("PugBot: Awww yeah");
}

string cDebugConsole::getLogFile() 
{ 
	// get function, returns path of log file
	return m_logFile; 
}

void cDebugConsole::PrintText(const char* szArgs, ...) 
{
	char szFormat[512] = {0};
	char szBuffer[1024] = {0};

	// handle args
	vsprintf(szFormat, szArgs, (char*)&szArgs + _INTSIZEOF(szArgs));

	// Get system time
	SYSTEMTIME sysTime; 
	GetLocalTime(&sysTime);

	// format buffer 
	_snprintf_s(szBuffer, sizeof(szBuffer), 1024, "%d-%d-%d %d:%d:%d - %s\n", 
		sysTime.wDay, sysTime.wMonth, sysTime.wYear, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, szFormat);
	cout << szBuffer;

	WriteLogFile(szBuffer);
}

void cDebugConsole::WriteLogFile(string dataToWrite) 
{
	// open log file and write buffer to it.
	ofstream logFile;
	logFile.open(m_logFile, ios::app);

	if (logFile.is_open()) {
		// write to log file
		logFile << dataToWrite;
		logFile.close();
	}
}

void cDebugConsole::SetupLogFile() 
{
	// retrieve module file
	char szModuleFile[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModuleFile, MAX_PATH);

	// strip exe and append txt
	string logFile = szModuleFile;
	logFile = logFile.substr(0, logFile.length()-3);
	logFile += "txt";

	m_logFile = logFile;
}