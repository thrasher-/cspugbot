#pragma once

class cINIHandler
{
public:
	cINIHandler();
	bool OpenFile();
	bool FileExists(string fileName);
	void RetrieveModulePath();
	string StripFilePath(string filePath);

private:
	string m_fileName;
};

