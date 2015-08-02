#pragma once

class CUtil
{
public:
	static CUtil* Instance();
	void GetOffset(std::string name, HMODULE hModule, std::string pattern, std::string mask, DWORD offset);
	void AddLog(char* string);
	void PrintByteArray(unsigned char * byte_array, int byte_array_size);
	int HexStrToByteArray(const char * hex_str, unsigned char * byte_array, int byte_array_max);
	void FAllocConsole();
	std::string GetCurrentPath();
	unsigned int GetLOLBaseAddress();
	_MODULEINFO GetModuleInfo(HMODULE hModule);
	bool Mask(const BYTE * pData, const BYTE * bMask, const char * szMask);
	DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE * bMask, char * szMask);
	std::string currentPath;
	inline LPCSTR GetLOLExeName(void) { return "League of Legends.exe"; }
	void GetOffsets();

protected:
	CUtil();
private:
	unsigned int lolBaseAddress;
	static CUtil *inst;
	~CUtil();
	
};


