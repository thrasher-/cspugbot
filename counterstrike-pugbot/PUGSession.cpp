#include "Includes.h"
#include "Main.h"
#include "Externs.h"

// random generator function:
ptrdiff_t myrandom (ptrdiff_t i) { return rand()%i;}

// pointer object to it:
ptrdiff_t (*p_myrandom)(ptrdiff_t) = myrandom;

cPUGSession::cPUGSession() { 
	// default constructor
	MAX_PLAYERS = 10;
	counter = 0;
	m_mapName = "de_dust2";
	m_pugAdminPassword = "master"; // used for testing without initiating a pug.
	m_bPugStarted = false;
	m_bPugActive = false;
}
cPUGSession::~cPUGSession() { /* unused deconstructor */ }

void cPUGSession::StartPug(string mapName, string pugAdmin) {
	if (m_bPugStarted) {
		// check to see if PUG has already been started
		g_MainCtrl.IRCHandler.SendMessage("A PUG has already started.");
		return;
	}

	// if not, lets initialise settings
	counter++;
	m_mapName = mapName;
	m_pugAdmin = pugAdmin;
	m_pugPlayers.push_back(pugAdmin);
	m_bPugStarted = true;
	m_bPugActive = false;
	m_password = GenerateRandomString(5);
	m_pugAdminPassword = "admin"+GenerateRandomString(2);

	g_MainCtrl.IRCHandler.SendMessage("%s has created a pug on map: %s. Type !join to participate.", pugAdmin.c_str(), mapName.c_str());
	g_MainCtrl.IRCHandler.SendData("TOPIC %s :%s has created a pug on map: %s. Type !join to participate.", g_MainCtrl.IRCHandler.getIRCChannel().c_str(), pugAdmin.c_str(), mapName.c_str());
}

// return pug password
string cPUGSession::getServerPassword() {
	return m_password;
}

// return admin password
string cPUGSession::getAdminPassword() {
	return m_pugAdminPassword;
}

// set admin password
void cPUGSession::setAdminPasswrd(string password) {
	m_password = password;
}

string cPUGSession::GenerateRandomString(int iLength)
{
	// generate random string 
	string randString;
	srand(GetTickCount());

	for (int i = 0; i < iLength; i++)
		randString += char((rand()%26)+97);

	return randString;
}

void cPUGSession::UpdatePlayerList(string oldNick, string newNick) 
{
	// update players nickname from old to new
	for (vector<string>::size_type i = 0; i < m_pugPlayers.size(); i++) {
		if (m_pugPlayers[i] == oldNick) {
			m_pugPlayers[i] = newNick;
		}
	}
}

bool cPUGSession::HasPlayerJoined(string playerName)
{
	// check to see if specified player is in the pug vector
	for (vector<string>::size_type i = 0; i < m_pugPlayers.size(); i++) {
		if (m_pugPlayers[i] == playerName) {
			return true;
		}
	}
	return false;
}

void cPUGSession::JoinPug(string playerName)
{
	if (!m_bPugStarted) {
		// check to see if pug hasn't started
		g_MainCtrl.IRCHandler.SendMessage("A PUG has not been started. Please type !pug to create a PUG.");
		return;
	}

	if (m_bPugStarted && counter != MAX_PLAYERS)
	{
		// While pug has not started and player counter has not reached it's limit, process clients
		 if (!m_bPugActive) // && !HasPlayerJoined(playerName)) 
		 {
			 // add player to vector
			 counter++;
			 g_MainCtrl.IRCHandler.SendMessage("%s has joined the pug! [%i/%i].", playerName.c_str(), counter, MAX_PLAYERS);
			 m_pugPlayers.push_back(playerName);

			 if (counter == MAX_PLAYERS) 
			 {
				m_bPugActive = true;
				g_MainCtrl.IRCHandler.SendData("MODE %s :+m\r\n", g_MainCtrl.IRCHandler.getIRCChannel().c_str());
				g_MainCtrl.IRCHandler.SendData("TOPIC %s :PUG has been filled. Server details will be private messaged to players.", g_MainCtrl.IRCHandler.getIRCChannel().c_str());
				g_MainCtrl.IRCHandler.SendMessage("PUG has been filled. Server details will be private messaged to players.");

				//randomise players
				random_shuffle(m_pugPlayers.begin(), m_pugPlayers.end());
				random_shuffle(m_pugPlayers.begin(), m_pugPlayers.end(), p_myrandom);

				string CT, T;

				for (vector<string>::size_type i = 0; i < m_pugPlayers.size(); i++) {
					if (i < 5) 
						CT += m_pugPlayers[i] + " ";
					else
						T += m_pugPlayers[i] + " ";
				}

				// display teams
				g_MainCtrl.IRCHandler.SendMessage("Teams generated - Terrorists: %sVS Counter-Terrorists: %s", T.c_str(), CT.c_str());

				// set password
				g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s sv_password %s", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(), m_password.c_str());

				// send server details to players
				for (vector<string>::size_type i = 0; i < m_pugPlayers.size(); i++) {
					if (m_pugPlayers[i] == m_pugAdmin) {
						g_MainCtrl.IRCHandler.SendData("PRIVMSG %s :Server details: connect %s:%i; password %s. Admin password: %s.\r\n", m_pugPlayers[i].c_str(),
							g_MainCtrl.CSHandler.getCSServer().c_str(), g_MainCtrl.CSHandler.getCSServerPort(), m_password.c_str(), m_pugAdminPassword.c_str());
					}
					else {
						g_MainCtrl.IRCHandler.SendData("PRIVMSG %s :Server details: connect %s:%i; password %s.\r\n", m_pugPlayers[i].c_str(),
							g_MainCtrl.CSHandler.getCSServer().c_str(), g_MainCtrl.CSHandler.getCSServerPort(), m_password.c_str());
					}
				}		
			 }
			 return;
		 }
		 if (m_bPugActive)
		 {
			 //pug is active and is not accepting join requests, notify user
			 g_MainCtrl.IRCHandler.SendMessageA("A PUG is currently in process, please wait until it completes and try again.");
			 return;
		 }
		 if (HasPlayerJoined(playerName))
		 {
			 //check to see if user has already joined the pug
			 g_MainCtrl.IRCHandler.SendMessage("You (%s) are already in the PUG.", playerName.c_str());
			 return;
		 }
	}
}

void cPUGSession::EndPug()
{
	// pug has ended, reset settings
	g_MainCtrl.IRCHandler.SendData("MODE %s :-m\r\n", g_MainCtrl.IRCHandler.getIRCChannel().c_str());
	g_MainCtrl.IRCHandler.SendMessage("The PUG has ended. Please type !pug to create a PUG.");
	g_MainCtrl.IRCHandler.SendData("TOPIC %s :A PUG has not been started. Please type !pug to create a PUG.", 
		g_MainCtrl.IRCHandler.getIRCChannel().c_str());

	counter = 0;
	m_mapName = "de_dust2";
	m_bPugStarted = false;
	m_bPugActive = false;
	m_pugAdmin.clear();
	m_pugPlayers.clear();
}

void cPUGSession::ErasePlayerFromList(string playerName)
{
	// enumerate player list and check to see if player is in pug list
	for (vector<string>::size_type i = 0; i < m_pugPlayers.size(); i++) {
		if (playerName == m_pugPlayers[i]) {
			// player has been found, erase player from pug vector
			m_pugPlayers.erase(m_pugPlayers.begin() + i);
		}
	}
	// if pug size is 0, all players have left, end pug.
	if (m_pugPlayers.size() == 0) {
		EndPug();
	}
}

void cPUGSession::LeavePug(string playerName) 
{
	if (!m_bPugStarted) {
		// check to see if pug has started
		g_MainCtrl.IRCHandler.SendMessage("A PUG hasn't been initated. Please type !pug to create a PUG.");
		return;
	}

	if (HasPlayerJoined(playerName)) {
		// check to see if player has joined the pug, if so remove player from pug vector and notify channel
		counter--;
		g_MainCtrl.IRCHandler.SendMessage("%s has left the pug [%i/%i].", playerName.c_str(), counter, MAX_PLAYERS);

		// if player was pug admin and there is still people wanting to pug, reset pug admin and notify channel
		if (playerName == m_pugAdmin && counter != 0) {
			m_pugAdmin = m_pugPlayers[0];
			g_MainCtrl.IRCHandler.SendMessage("PUG Admin has left. %s has been assigned the new admin.", m_pugAdmin.c_str());
		}
		
		// no players left, end pug
		if (counter == 0)
			EndPug();

		return;
	}
}

bool cPUGSession::PugStarted() {
	// return status of pug
	return m_bPugStarted;
};

void cPUGSession::ShowPlayers()
{
	if (!m_bPugStarted) {
		// check to see if pug has started
		g_MainCtrl.IRCHandler.SendMessage("A PUG hasn't been initated. Please type !pug to create a PUG.");
		return;
	}

	string playerList;
	
	// enumerate player list and build player string
	for (vector<string>::size_type i = 0; i < m_pugPlayers.size(); i++) {
		playerList += m_pugPlayers[i] + " ";
	}

	// notify channel players participating in player list
	g_MainCtrl.IRCHandler.SendMessage("Players: %s", playerList.c_str());
}


