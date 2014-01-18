class cWorkerThread
{
public:
	cWorkerThread();
	cWorkerThread(LPTHREAD_START_ROUTINE lpExternalRoutine);
	~cWorkerThread();

	DWORD Start(void *arg);
	DWORD Stop(bool bForceKill);
	DWORD GetExitCode();
	
	void Attach( LPTHREAD_START_ROUTINE lpThreadFunc );
	void Detach();

	virtual DWORD Run( LPVOID /* arg */ );

protected:

	static DWORD WINAPI EntryPoint( LPVOID pArg) 
	{
		cWorkerThread *pParent = reinterpret_cast<cWorkerThread*>(pArg);
		pParent->ThreadCtor();
		pParent->Run( pParent->m_ThreadCtx.m_pUserData );
		pParent->ThreadDtor();

		return STILL_ACTIVE;
	}



	virtual void ThreadCtor() { }
	virtual void ThreadDtor() { }

private:

	class cThreadContext
	{
	public:
		cThreadContext() {
			memset(this, 0, sizeof(this));
		}

		HANDLE m_hThread;
		DWORD m_dwTID;
		LPVOID m_pUserData;
		LPVOID m_pParent;
		DWORD m_dwExitCode;
	};


protected:
	cThreadContext m_ThreadCtx;
	LPTHREAD_START_ROUTINE m_pThreadFunc;
};