class cPUGSession
{
public:
	cPUGSession();
	~cPUGSession();

	void StartPug(string mapName, string pugAdmin);
	void JoinPug(string playerName);
	void EndPug();
	void LeavePug(string playerName);
	void ErasePlayerFromList(string playerName);
	void UpdatePlayerList(string oldNick, string newNick);
	void ShowPlayers();
	void setAdminPasswrd(string password);

	bool HasPlayerJoined(string playerName);
	bool PugStarted();
	
	string getServerPassword();
	string getAdminPassword();
	string GenerateRandomString(int iLength);

private:
	bool m_bPugStarted;
	bool m_bPugActive;


	vector<string> m_pugPlayers;
	int counter;
	int MAX_PLAYERS;
	string m_mapName;
	string m_password;
	string m_pugAdmin;
	string m_pugAdminPassword;
};