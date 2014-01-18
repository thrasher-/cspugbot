#include "Includes.h"
#include "Main.h"
#include "Externs.h"

/* http://www.codeproject.com/KB/threads/genericthreadclass.aspx
   Credits go to Arun N Kumar for thread class.
   Will rewrite soon, hopefully C++ 0x comes out soon with std::thread! ;)
*/

cWorkerThread::cWorkerThread() {
	m_pThreadFunc = cWorkerThread::EntryPoint;
}

cWorkerThread::cWorkerThread(LPTHREAD_START_ROUTINE lpExternalRoutine) {
	Attach(lpExternalRoutine);
}

cWorkerThread::~cWorkerThread() {
	if (m_ThreadCtx.m_hThread)
		Stop(true);
}

DWORD cWorkerThread::Start(void *arg = NULL) {
	m_ThreadCtx.m_pUserData = arg;
	m_ThreadCtx.m_pParent = CreateThread(NULL, 0, m_pThreadFunc, this, 0, &m_ThreadCtx.m_dwTID);
	m_ThreadCtx.m_dwExitCode = (DWORD)-1;

	return GetLastError();
}


DWORD cWorkerThread::Run( LPVOID /* arg */ ) {
	return m_ThreadCtx.m_dwExitCode; 
}

DWORD cWorkerThread::Stop(bool bForceKill = false) {
	if (m_ThreadCtx.m_hThread) 
	{
		GetExitCodeThread(m_ThreadCtx.m_hThread, &m_ThreadCtx.m_dwExitCode);

		if (m_ThreadCtx.m_dwExitCode == STILL_ACTIVE && bForceKill)
			TerminateThread(m_ThreadCtx.m_hThread, DWORD(-1));

		m_ThreadCtx.m_hThread = NULL;	
	}

	return m_ThreadCtx.m_dwExitCode;
}

DWORD cWorkerThread::GetExitCode() {
	if (m_ThreadCtx.m_pParent)
		GetExitCodeThread(m_ThreadCtx.m_hThread, (LPDWORD)&m_ThreadCtx.m_dwExitCode);

	return m_ThreadCtx.m_dwExitCode;
}

void cWorkerThread::Attach(LPTHREAD_START_ROUTINE lpThreadFunc) {
	m_pThreadFunc = lpThreadFunc;
}

void cWorkerThread::Detach(void) {
	m_pThreadFunc = cWorkerThread::EntryPoint;
}



