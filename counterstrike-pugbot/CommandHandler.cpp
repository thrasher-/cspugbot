#include "Includes.h"
#include "Main.h"
#include "Externs.h"

cCommandHandler::cCommandHandler() {
	m_bLoggedIn = false;
	m_usCounter = 0;
}

// Handle IRC Events
void cCommandHandler::HandleIRCBuffer(vector <string> ircBuffer)
{
	if (ircBuffer[1].compare("001") == 0) {
		// we have joined the IRC server, lets join the channel
		g_MainCtrl.IRCHandler.SendData("JOIN %s", g_MainCtrl.IRCHandler.getIRCChannel().c_str());
	}
	else if (ircBuffer[0].compare("PING") == 0) {
		// Ping? PONG!
		ircBuffer[0].replace(1,1,"O");
		g_MainCtrl.IRCHandler.SendData("%s %s", ircBuffer[0].c_str(), ircBuffer[1].c_str());
	}
	else if (ircBuffer[1].compare("NICK") == 0) {
		// somebody changed there nickname, check to see if it's anyone in the PUG
		if (g_MainCtrl.PugSession.PugStarted()) 
		{
			string oldName = ircBuffer[0].substr(1, ircBuffer[0].find_first_of("!")-1);
			string nickName = ircBuffer[2].substr(1, ircBuffer[2].length());
			g_MainCtrl.PugSession.UpdatePlayerList(oldName, nickName);
		}
	}
	else if (ircBuffer[1].compare("PART") == 0) {
		// somebody left the channel, check to see if it's anyone in the PUG
		string nickName = ircBuffer[1].substr(1, ircBuffer[1].length());
		if (g_MainCtrl.PugSession.PugStarted()) {
			g_MainCtrl.PugSession.ErasePlayerFromList(nickName);
		}
	}
	else if (ircBuffer[0].compare("QUIT") == 0) {
		// somebody quit the IRC server, check to see if it's anyone in the PUG
		string speakerName = ircBuffer[0].substr(1, ircBuffer[0].find_first_of("!")-1);
		if (g_MainCtrl.PugSession.PugStarted()) {
			g_MainCtrl.PugSession.ErasePlayerFromList(speakerName);
		}
	}
	else if (ircBuffer[1].compare("PRIVMSG") == 0) {
		// Handle IRC commands
		if (ircBuffer.size() > 2) {
			HandleIRCCommand(ircBuffer);
		}
	}
}

// Handle IRC Commands
void cCommandHandler::HandleIRCCommand(vector<string> ircBuffer)
{
	ircBuffer[3] = ircBuffer[3].substr(1, ircBuffer[3].length());
	string speakerName = ircBuffer[0].substr(1, ircBuffer[0].find_first_of("!")-1);

	if (ircBuffer[3].compare("!pug") == 0) {
		// start PUG, can take map name as param
		string mapName = "de_dust2";

		if (ircBuffer.size() > 4) {
			mapName = ircBuffer[4];
		}
		g_MainCtrl.PugSession.StartPug(mapName, speakerName);
	}
	else if (ircBuffer[3].compare("!say") == 0) {
		// send message to CS server
		string messageToSend;

		for (vector<string>::size_type i = 3+1; i < ircBuffer.size(); i++) {
			messageToSend += ircBuffer[i] += " ";
		}

		g_MainCtrl.IRCHandler.SendMessage("Sent message to CS server.");
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say [IRC]: %s", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(),
			messageToSend.c_str());
	}
	else if (ircBuffer[3].compare("!rcon") == 0) {
		// send RCON command to CS server.
		string messageToSend;

		for (vector<string>::size_type i = 3+1; i < ircBuffer.size(); i++) {
			messageToSend += ircBuffer[i] += " ";
		}

		g_MainCtrl.IRCHandler.SendMessage("Sent RCON command to CS server.");
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s %s", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(),
			messageToSend.c_str());
	}
	else if (ircBuffer[3].compare("!join") == 0) {
		// join player from pug
		g_MainCtrl.PugSession.JoinPug(speakerName);
	}
	else if (ircBuffer[3].compare("!leave") == 0) {
		// remove player from pug
		g_MainCtrl.PugSession.LeavePug(speakerName);
	}
	else if (ircBuffer[3].compare("!endpug") == 0) {
		// end pug
		g_MainCtrl.IRCHandler.SendMessage("%s has ended the pug.", speakerName.c_str());
	}	
	else if (ircBuffer[3].compare("!status") == 0) {
		// show player status
		g_MainCtrl.PugSession.ShowPlayers();
	}
}

// Handle CS Buffer
void cCommandHandler::HandleCSBuffer(vector <string> csBuffer) 
{
	csBuffer[0] = csBuffer[0].substr(4, csBuffer[0].length());

	// we recieved challenge string, lets parse it
	if (csBuffer[0].compare("challenge") == 0 && csBuffer.size() > 2) {
		string challengeString = csBuffer[2];
		g_MainCtrl.CSHandler.setRconChallenge(challengeString);
		m_usCounter++;
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s serverinfo", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str()); 
		return;

	}
	// check to see if we received bad rcon from CS server
	if (m_usCounter == 1) 
	{
		if (csBuffer[0].find("Bad") != string::npos && csBuffer[1].compare("rcon_password.") == 0) {
			m_usCounter = 2;
			g_MainCtrl.DebugConsole.PrintText("Error - Invalid RCON Password, can not continue.");
			g_MainCtrl.CSHandler.setRetry(false);
			return;
		}
		// rcon password was good! 
		m_usCounter = 0;
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say PUG bot has successfully connected.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());

		// Setup logging
		g_MainCtrl.DebugConsole.PrintText("Setting up logging..");

		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s mp_logmessages 1", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s log on", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s logaddress_delall", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s logaddress_add %s 5454", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(),
		g_MainCtrl.CSHandler.convertHostToIP(g_MainCtrl.CSHandler.getHostname()).c_str());

		// start logging server
		g_MainCtrl.CSHandler.LogMessages();
	}

	for (vector<string>::size_type i = 0; i < csBuffer.size(); i++) 
	{
		if (csBuffer[i].find("disconnected") != string::npos) 
		{
			// notify channel that player has disconnected
			RetrievePlayerAndSteamID(csBuffer, i);

			// check to see if the player who disconnected was the PUG Admin
			if (g_MainCtrl.ScoreHandler.getMatchStatus()) {
				if (g_MainCtrl.ScoreHandler.getIndividualPlayer(m_playerName).bAdmin) {
					g_MainCtrl.PugSession.setAdminPasswrd("temp");
					g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say PUG Admin disconnected, set new master password to: temp. Use \"!login temp\" to continue the match.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
					m_bLoggedIn = false;
				}
			}
			g_MainCtrl.IRCHandler.SendMessage("%s%s %s has disconnected.%s", CRED, m_playerName.c_str(), m_steamID.c_str(), CEND);
		}
		else if (csBuffer[i].find("connected") != string::npos)
		{
			// notity channel that player has connected
			RetrievePlayerAndSteamID(csBuffer, i);
	
			g_MainCtrl.IRCHandler.SendMessage("%s%s %s has connected.", CGREEN, m_playerName.c_str(), m_steamID.c_str(), CEND);
		}

		else if (csBuffer[i].find("say") != string::npos) {
			// handle in game CS command
			HandleCSCommand(csBuffer, i);
		}

		if (g_MainCtrl.ScoreHandler.getMatchStatus())
		{
			if (csBuffer[i].find("killed") != string::npos)
			{
				// format strings & report to channel who killed who.
				string killer, victim, weapon, first, second;

				for (vector<string>::size_type k = 5; (int)k < i; k++) {
					killer += csBuffer[k]; killer += " ";
				}

				// organise colours for display
				if (killer.find("TERRORIST") != string::npos) {
					first = CRED;
					second = CBLUE;
				}
				else {
					first = CBLUE;
					second = CRED;
				}
					
				killer = killer.substr(1, killer.find_first_of("><")-1);

				for (vector<string>::size_type k = i+1; (int)k < csBuffer.size()-2; k++) {
					victim += csBuffer[k]; victim += " ";
				}

				victim = victim.substr(1, victim.find_first_of("><")-1);
				weapon = csBuffer[csBuffer.size()-1].substr(1, csBuffer[csBuffer.size()-1].length()-2);				
				g_MainCtrl.ScoreHandler.HandleKD(killer, victim);

				// notify channel

				g_MainCtrl.IRCHandler.SendMessage("%s%s%s [%i/%i] killed %s%s%s [%i/%i] with weapon %s%s.%s", first.c_str(), killer.c_str(), CEND, g_MainCtrl.ScoreHandler.getIndividualPlayer(killer).usKills,
					g_MainCtrl.ScoreHandler.getIndividualPlayer(killer).usDeaths, second.c_str(), victim.c_str(), CEND,  g_MainCtrl.ScoreHandler.getIndividualPlayer(victim).usKills,
					g_MainCtrl.ScoreHandler.getIndividualPlayer(victim).usDeaths, CORANGE, weapon.c_str(), CEND);
			}
			else if (csBuffer[i].find("changed") != string::npos && csBuffer[i+1].find("name") != string::npos) 
			{
				// user changed name, update player details
				RetrievePlayerAndSteamID(csBuffer, i);

				string newName;

				for (vector<string>::size_type j = i+3; j < csBuffer.size(); j++) {
					newName += csBuffer[j] += " ";
				}

				newName = newName.substr(1, newName.length() - 3);
				g_MainCtrl.ScoreHandler.UpdatePlayerName(m_playerName, newName);
			}
			else if (csBuffer[i].find("Terrorists_Win") != string::npos || csBuffer[i].find("Target_Bombed") != string::npos) 
			{
				// report that the terrorists team won the round
				g_MainCtrl.IRCHandler.SendMessage("**************************************************");
			
				if (csBuffer[i].find("Target_Bombed") != string::npos)	{	
					g_MainCtrl.IRCHandler.SendMessage("  %sTarget successfully bombed.%s  ", CRED, CEND);
					g_MainCtrl.ScoreHandler.GiveBombPoints(m_bombCarrier);
				}

				g_MainCtrl.ScoreHandler.setTScore(g_MainCtrl.ScoreHandler.getTScore()+1);
				g_MainCtrl.IRCHandler.SendMessage("  %sTerrorists Win! Score: CTs ( %i ) Ts ( %i ).%s  ", CRED, g_MainCtrl.ScoreHandler.getCTScore(), g_MainCtrl.ScoreHandler.getTScore(), CEND);	
				g_MainCtrl.IRCHandler.SendMessage("**************************************************");
				Sleep(1000);
				g_MainCtrl.ScoreHandler.CheckScore();
			}
			else if (csBuffer[i].find("CTs_Win") != string::npos || csBuffer[i].find("Bomb_Defused") != string::npos) 
			{
				// report that the counter-terrorists won the round
				g_MainCtrl.IRCHandler.SendMessage("**************************************************");
			
				if (csBuffer[i].find("Bomb_Defused") != string::npos)
					g_MainCtrl.IRCHandler.SendMessage("  %sBomb successfully defused.%s  ", CBLUE, CEND);

				g_MainCtrl.ScoreHandler.setCTScore(g_MainCtrl.ScoreHandler.getCTScore()+1);	
				g_MainCtrl.IRCHandler.SendMessage("  %sCounter-Terrorists Win! Score: CTs ( %i ) Ts ( %i ).%s  ", CBLUE, g_MainCtrl.ScoreHandler.getCTScore(), 
					g_MainCtrl.ScoreHandler.getTScore(), CEND);
				g_MainCtrl.IRCHandler.SendMessage("**************************************************");
				Sleep(1000);
				g_MainCtrl.ScoreHandler.CheckScore();
			}
			else if (csBuffer[i].find("_Bomb") != string::npos) 
			{
				// Handle bomb related events. Drop/Spawn/Defused/Plant/Pickup
				RetrievePlayerAndSteamID(csBuffer, i);

				if (csBuffer[i].find("Defused_The_Bomb") != string::npos) {
					g_MainCtrl.IRCHandler.SendMessage("%s%s has defused the bomb.%s", CBLUE, m_playerName.c_str(), CEND);
					g_MainCtrl.ScoreHandler.GiveBombPoints(m_playerName);
				}
		
				else if (csBuffer[i].find("Spawned_With_The_Bomb") != string::npos)
					g_MainCtrl.IRCHandler.SendMessage("%s%s has spawned with the bomb.%s", CRED, m_playerName.c_str(), CEND);
				else if (csBuffer[i].find("Dropped_The_Bomb") != string::npos) 
					g_MainCtrl.IRCHandler.SendMessage("%s%s has dropped the bomb.%s", CRED, m_playerName.c_str(), CEND);
				else if (csBuffer[i].find("Got_The_Bomb") != string::npos) 
					g_MainCtrl.IRCHandler.SendMessage("%s%s has picked up the bomb.%s", CRED, m_playerName.c_str(), CEND);
				else if (csBuffer[i].find("Planted_The_Bomb") != string::npos) {
					g_MainCtrl.IRCHandler.SendMessage("%s%s has planted the bomb.%s", CRED, m_playerName.c_str(), CEND);
					m_bombCarrier = m_playerName;
				}
			}
		}
	}
	 m_playerName.clear();
}

void cCommandHandler::HandleCSCommand(vector <string> csBuffer, int iPos)
{
	// obtain player from CS buffer.
	string playerName;

	for (vector<string>::size_type k = 5; (int)k < iPos; k++) {
		playerName += csBuffer[k]; playerName += " ";
	}

	playerName = playerName.substr(1, playerName.find_first_of("><")-1);

	if (csBuffer[iPos+1].find("!login") != string::npos && !m_bLoggedIn) 
	{
		// player is attempting to login
		if (csBuffer.size() > (size_t)iPos+2) 
		{
			// obtain password param
			string password = csBuffer[iPos+2].substr(0, csBuffer[iPos+2].length()-1);

			// test password matches admin password
			if (password == g_MainCtrl.PugSession.getAdminPassword()) {
				// admin logged in successfully
				g_MainCtrl.ScoreHandler.GiveAdminPrivileges(playerName);
				m_bLoggedIn = true;
				g_MainCtrl.IRCHandler.SendMessage("[IN-GAME] Admin <%s> logged in successfully.", playerName.c_str());
				g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Logged in successfully.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
			}
		}
	}

	if (!g_MainCtrl.ScoreHandler.getIndividualPlayer(playerName).bAdmin && !m_bLoggedIn) {
		// player hasn't logged in, can't issue game commands.
		return;
	}

	if (csBuffer[iPos+1].find("!lo3") != string::npos) {
		// commence live on 3, but first check to see if the match has already started.
		if (g_MainCtrl.ScoreHandler.getMatchStatus()) {
			return;
		}

		g_MainCtrl.IRCHandler.SendMessage("**************************************************");
		g_MainCtrl.IRCHandler.SendMessage("Live on 3! Match has started. Good luck & Have Fun!");
		g_MainCtrl.IRCHandler.SendMessage("**************************************************");

		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Live on 3", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s sv_restart 1", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		Sleep(1000);
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s sv_restart 1", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		Sleep(1000);
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s sv_restart 3", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		Sleep(3000);

		g_MainCtrl.ScoreHandler.setMatchStatus(true);
	}
	else if (csBuffer[iPos+1].find("!restart") != string::npos) {
		// simple restart command
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s sv_restart 1", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
	}
	else if (csBuffer[iPos+1].find("!cancel") != string::npos) {
		// cancel half

		if (g_MainCtrl.ScoreHandler.isSecondHalf())
		{
			// reset scores
			g_MainCtrl.ScoreHandler.Reset();

			g_MainCtrl.ScoreHandler.setTScore(g_MainCtrl.ScoreHandler.getCTFirstScore());
			g_MainCtrl.ScoreHandler.setCTScore(g_MainCtrl.ScoreHandler.getTFirstScore());

			// notify players & IRC channel
			g_MainCtrl.ScoreHandler.setSecondHalf(true);
			g_MainCtrl.IRCHandler.SendMessage("**************************************************");
			g_MainCtrl.IRCHandler.SendMessage("                Match half cancelled.             ");
			g_MainCtrl.IRCHandler.SendMessage("**************************************************");
			g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Match half cancelled, please type !lo3 to commence match.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
			return;
		}
		// notify players & IRC channel
		g_MainCtrl.IRCHandler.SendMessage("**************************************************");
		g_MainCtrl.IRCHandler.SendMessage("                Match half cancelled.             ");
		g_MainCtrl.IRCHandler.SendMessage("**************************************************");
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Match half cancelled, please type !lo3 to commence match.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
		g_MainCtrl.ScoreHandler.Reset();
	}
	else if (csBuffer[iPos+1].find("!request") != string::npos) {
		// request for additional players
		g_MainCtrl.IRCHandler.SendData("Need additional player! Connect string: connect %s:%i; password %s", g_MainCtrl.CSHandler.getCSServer().c_str(), g_MainCtrl.CSHandler.getCSServerPort(),
			g_MainCtrl.PugSession.getServerPassword().c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Requesting for more players.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
	}
	else if (csBuffer[iPos+1].find("!irc") != string::npos) {
		// send IRC message from CS
		string messageToSend;

		for (vector<string>::size_type i = iPos+2; i < csBuffer.size(); i++) {
			messageToSend += csBuffer[i] += " ";
		}

		messageToSend = messageToSend.substr(0, messageToSend.length()-2);
		g_MainCtrl.IRCHandler.SendMessage("[IN-GAME]: %s", messageToSend.c_str());
		g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Sent message to IRC channel.", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str());
	}
	else if (csBuffer[iPos+1].find("!map") != string::npos) {
		// change map
		string mapName; vector<string>::size_type j; j = iPos+2;
		if (csBuffer.size() > j) {
			mapName = csBuffer[j].substr(0, csBuffer[j].length()-1);;
			g_MainCtrl.IRCHandler.SendData("Changed map to: %s", mapName.c_str());
			g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s say Changing level to %s", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(),
				mapName.c_str());
			Sleep(1000);
			g_MainCtrl.CSHandler.SendData("ÿÿÿÿrcon %s %s changelevel %s", g_MainCtrl.CSHandler.getRCONChallanege().c_str(), g_MainCtrl.CSHandler.getRCONPassword().c_str(),
				mapName.c_str());			
		}
	}
}

void cCommandHandler::RetrievePlayerAndSteamID(vector<string> csBuffer, int iPos)
{
	string playerName, steamID;
	//obtain player from buffer

	for (vector<string>::size_type k = 5; (int)k < iPos; k++) {
		playerName += csBuffer[k]; playerName += " ";
	}

	playerName = playerName.substr(1, playerName.length());
	steamID = playerName.substr(playerName.find_first_of(">")+1, playerName.length()-1);
	steamID = steamID.substr(0, steamID.find_last_of(">")-1);

	// bot support
	if (steamID.find("BOT") != string::npos) {
		steamID = steamID.substr(0, steamID.find_last_of(">")+1);
	}

	// when member has disconnect and was in a team, fixes extra character bug
	if (steamID.compare(steamID.length()-1, 1, "S") == 0 || steamID.compare(steamID.length()-1, 1, "C") == 0) {
		steamID = steamID.substr(0, steamID.find_last_of(">")+1);
	}

	// some string manipulation :)
	while (true) 
	{
		string::size_type k = playerName.find_last_of("<");
		if (playerName.find("<") == string::npos)
			break;
		
		playerName = playerName.substr(0, k); 
	}

	m_playerName = playerName;
	m_steamID = steamID;
}