class cCSHandler : public cWorkerThread
{
public:
	cCSHandler();
	~cCSHandler();

	bool ConnectToHost();
	bool LogMessages();
	void RecvData(SOCKET socket);
	void SendData(const char* szArgs, ...);

	void setCSPort(USHORT usPort);
	void setCSServer(string csServer);
	void setCSRcon (string csRcon);
	void setRconChallenge(string rconChallenge);
	void setRetry(bool bRetry);

	USHORT getCSServerPort();
	string getCSServer();
	string getRCONPassword();
	string getRCONChallanege();
	string getHostname();
	string convertHostToIP(string host);

	virtual DWORD Run( LPVOID /* arg */ );

	void Init();
	void TokeniseBuffer(string buffer);

private:
	SOCKET m_Socket, m_srvSocket;
	USHORT m_usPort;
	string m_csServer;
	string m_rconPassword, m_rconChallenge;
	bool m_bConnected;
	bool m_bRetry;
};