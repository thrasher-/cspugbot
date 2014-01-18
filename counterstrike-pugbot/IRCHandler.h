class cIRCHandler : public cWorkerThread
{
public:
	cIRCHandler();
	~cIRCHandler();

	bool ConnectToHost();
	ULONG cIRCHandler::ResolveHost(string targetIP);
	void RecvData();
	void SendData(const char* szArgs, ...);
	void SendMessage(const char* szArgs, ...);

	void setIRCPort(USHORT usPort);
	void setIRCServer(string IRCServer);
	void setIRCChannel(string ircChannel);
	void setIRCNick(string ircNick);

	virtual DWORD Run( LPVOID /* arg */ );

	USHORT getIRCPort();
	string getIRCServer();
	string getIRCChannel();
	string getIRCNick();

	void Init();
	void TokeniseBuffer(string buffer);

private:
	SOCKET m_Socket;
	USHORT m_usPort;

	string m_ircServer;
	string m_ircNick;
	string m_ircChannel;

	bool m_bConnected;
};