#include <iostream>
#include <string>
#include <winsock2.h>
#include <Windows.h>
#include <vector>
#include <conio.h>
#include <fstream>
#include <algorithm>

using namespace std;

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

// IRC colours
#define CBLUE "\x03\x31\x32"
#define CGREEN "\x03\x33"
#define CORANGE "\x03\x37"
#define CRED "\x03\x34"
#define CEND "\x03"

#include "DebugConsole.h"
#include "WorkerThread.h"
#include "CSHandler.h"
#include "ScoreHandler.h"
#include "CommandHandler.h"
#include "IRCHandler.h"
#include "INIHandler.h"
#include "PUGSession.h"
