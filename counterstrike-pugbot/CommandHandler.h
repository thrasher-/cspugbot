class cCommandHandler
{
public:
	cCommandHandler();

	void HandleIRCBuffer(vector <string> ircBuffer);
	void HandleIRCCommand(vector <string> ircBuffer);
	void HandleCSBuffer(vector <string> csBuffer);
	void HandleCSCommand(vector <string> csBuffer, int iPos);
	void RetrievePlayerAndSteamID(vector<string> csBuffer, int iPos);

private:
	string m_bombCarrier;
	string m_playerName;
	string m_steamID;

	USHORT m_usCounter;

	bool m_bLoggedIn;
};