class cDebugConsole
{
public:
	cDebugConsole();
	void SetupLogFile();
	void PrintText(const char* szArgs, ...);
	void WriteLogFile(string dataToWrite);
	string getLogFile();

private:
	string m_logFile;
};