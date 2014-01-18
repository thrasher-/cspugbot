#include "Includes.h"
#include "Main.h"
#include "Externs.h"

// initial class constructor, set core variables
cScoreHandler::cScoreHandler() { 
	m_playerStats.clear();
	m_usCTScore = 0;
	m_usTScore = 0;
	m_usFirstCTScore = 0;
	m_usFirstTScore = 0;
	m_bFirstHalf = false;
	m_bSecondHalf = false;
	m_bMatchStarted = false;
}
cScoreHandler::~cScoreHandler() { /* unsued class deconstructor */ }

// get playerStats vector
vector<playerInfo> cScoreHandler::getPlayerStats() {
	return m_playerStats;
}

// get CT Score
USHORT cScoreHandler::getCTScore() {
	return m_usCTScore;
}

// get T score
USHORT cScoreHandler::getTScore() {
	return m_usTScore;
}

// get first half T score
USHORT cScoreHandler::getTFirstScore() {
	return m_usFirstTScore;
}

// get first half CT score
USHORT cScoreHandler::getCTFirstScore() {
	return m_usFirstTScore;
}

// check to see if the game is at first half
bool cScoreHandler::isFirstHalf() {
	return m_bFirstHalf;
}

// set second half (true/false)
void cScoreHandler::setSecondHalf(bool bStatus) {
	m_bSecondHalf = bStatus;
}

// check to see if the game is at second half
bool cScoreHandler::isSecondHalf() {
	return m_bSecondHalf;
}

// set CT score
void cScoreHandler::setCTScore(USHORT score) {
	m_usCTScore = score;
}

// set T score
void cScoreHandler::setTScore(USHORT score) {
	m_usTScore = score;
}

// Check to see if match has started
bool cScoreHandler::getMatchStatus() {
	return m_bMatchStarted;
}

// set match status (true/false)
void cScoreHandler::setMatchStatus(bool bStatus) {
	m_bMatchStarted = bStatus;
}

// reset core variables
void cScoreHandler::Reset() {
	m_playerStats.clear();
	m_usCTScore = 0;
	m_usTScore = 0;
	m_bFirstHalf = false;
	m_bSecondHalf = false;
	m_bMatchStarted = false;
}

// create player and add to playerStats vector
void cScoreHandler::CreatePlayer(string player)
{
	playerInfo pInfo;
	pInfo.playerName = player;
	pInfo.usKills = 0;
	pInfo.usDeaths = 0;
	pInfo.bAdmin = false;

	m_playerStats.push_back(pInfo);
}

// check score and perform logic based upon it
void cScoreHandler::CheckScore()
{
	USHORT usSum = getCTScore() + getTScore();
	// check to see if game is first half and has reached MR 15
	if (m_bFirstHalf = true && usSum == 15) 
	{
		// it has, lets print info the channel
		m_usFirstCTScore = getCTScore();
		m_usFirstTScore = getTScore();

		Reset();

		setTScore(m_usFirstCTScore);
		setCTScore(m_usFirstTScore);
		
		// notify players in IRC & in game
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say **************************************************", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Half time reached! Score: CTs ( %i ) Ts ( %i ) ", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(),
			m_usFirstCTScore, m_usFirstTScore);
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Type !lo3 when ready to continue second half! ", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say **************************************************", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.IRCHandler.SendMessage("  Half time reached! Score: CTs ( %i ) Ts ( %i )  ", m_usFirstCTScore, m_usFirstTScore);
		m_bSecondHalf = true;
	}
	
	if (m_bSecondHalf)
	{
		// check to see if we have reached max score.
		if (getCTScore()  == 16 || getTScore() == 16) {
			// we have, notify players in IRC & in game
			g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say **************************************************", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
			g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say  Max score reached! Good Game! Score: CTs ( %i ) Ts ( %i ) ",  g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(), 
				getCTScore(), getTScore());
			g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say **************************************************", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
			g_MainCtrl.IRCHandler.SendMessage("  Max score reached! Good Game! Score: CTs ( %i ) Ts ( %i ) ", getCTScore(), getTScore());
			
			// reset core variables and end pug
			Reset();
			g_MainCtrl.PugSession.EndPug();
		}
	}
}

void cScoreHandler::GiveAdminPrivileges(string playerName)
{
	// Give admin privileges
	if (!HavePlayer(playerName)) 
		CreatePlayer(playerName);

	for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
		if (m_playerStats[i].playerName == playerName) {
			m_playerStats[i].bAdmin = true;
		}
	}
}

void cScoreHandler::UpdatePlayerName(string playerName, string newName)
{
	// update player name if player changes nickname
	if (!HavePlayer(playerName)) 
		CreatePlayer(playerName);

	for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
		if (m_playerStats[i].playerName == playerName) {
			m_playerStats[i].playerName = newName;
		}
	}
}


void cScoreHandler::GiveBombPoints(string player)
{
	// give player +3 points for successfull bomb plant
	if (!HavePlayer(player)) 
		CreatePlayer(player);

	for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
		if (m_playerStats[i].playerName == player) {
			m_playerStats[i].usKills += 3;
		}
	}
}

void cScoreHandler::HandleKD(string killer, string victim) 
{
	// Handle kill, death event
	if (!HavePlayer(killer))
		CreatePlayer(killer);

	if (!HavePlayer(victim)) 
		CreatePlayer(victim);

	GiveKillPoints(killer);
	GiveDeathPoints(victim);
}

void cScoreHandler::GiveKillPoints(string player) 
{
	// give player +1 points
	if (HavePlayer(player)) {
		for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
			if (m_playerStats[i].playerName == player) {
				m_playerStats[i].usKills += 1;
			}
		}
	}
}

void cScoreHandler::GiveDeathPoints(string player) 
{
	// give player +1 points
	if (HavePlayer(player)) {
		for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
			if (m_playerStats[i].playerName == player) {
				m_playerStats[i].usDeaths += 1;
			}
		}
	}
}

playerInfo cScoreHandler::getIndividualPlayer(string player)
{
	// enumerate player vector and return playerInfo structure
	if (!HavePlayer(player)) 
		CreatePlayer(player);

	for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
		if (m_playerStats[i].playerName == player)
			return m_playerStats[i];
	}
	return m_playerStats[0];
}

bool cScoreHandler::HavePlayer(string player) 
{
	// check to see if supplied player has been stored in the player stats vector
	for (vector<playerInfo>::size_type i = 0; i < m_playerStats.size(); i++) {
		if (m_playerStats[i].playerName == player)
			return true;
	}
	return false;
}

