#pragma once

struct playerInfo
{
	USHORT usKills;
	USHORT usDeaths;
	bool bAdmin;
	string playerName;
};

class cScoreHandler
{
public:
	cScoreHandler();
	~cScoreHandler();

	bool getMatchStatus();
	bool HavePlayer(string player);

	vector<playerInfo> getPlayerStats();
	playerInfo getIndividualPlayer(string player);

	USHORT getCTScore();
	USHORT getTScore();

	void setTScore(USHORT score);
	void setCTScore(USHORT score);
	void setSecondHalf(bool bStatus);
	void setMatchStatus(bool bStatus);
	void Reset();

	USHORT getTFirstScore();
	USHORT getCTFirstScore();
	bool isFirstHalf();
	bool isSecondHalf();

	void GiveAdminPrivileges(string playerName);
	void GiveKillPoints(string player);
	void GiveDeathPoints(string player);
	void GiveBombPoints(string player);
	void HandleKD(string killer, string victim);
	void CreatePlayer(string playerName);
	void UpdatePlayerName(string playerName, string newName);
	void CheckScore();

private:
	bool m_bMatchStarted;
	bool m_bFirstHalf;
	bool m_bSecondHalf;

	USHORT m_usCTScore;
	USHORT m_usTScore;

	USHORT m_usFirstCTScore;
	USHORT m_usFirstTScore;

	vector<playerInfo> m_playerStats;
};