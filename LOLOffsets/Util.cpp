#include "stdafx.h"
#include <vector>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;


CUtil* CUtil::inst = 0;

CUtil::CUtil()
{
	currentPath = std::string();
	lolBaseAddress = 0;
}

CUtil::~CUtil()
{
	delete CUtil::inst;
}

void CUtil::GetOffsets() {

	HMODULE hModule = NULL;
	while (!hModule)
	{
		hModule = GetModuleHandleA(CUtil::Instance()->GetLOLExeName()); // get dll handle
		Sleep(100);
	}

	// ObjectManager
	std::string pattern = "8B 44 24 18 83 C4 14 0F B7 D0 85 D2";
	GetOffset("ObjectManager", hModule, pattern, std::string((pattern.length() + 1) / 3, 'x'), 0x18);

	// LocalPlayer
	pattern = "8B 46 04 5E C2 04 00 8B 46 08 5E C2 04 00 8B 44 24 08 8B 44 86 04 5E C2 04 00";
	GetOffset("LocalPlayer", hModule, pattern, std::string((pattern.length() + 1) / 3, 'x'), -0x38);

	// CastSpell
	pattern = "8D 64 24 00 8A 0E 8D 76 01 80 C1 5F 8A D9 8A C1 D0 EB 02 C0 32 D8 02 C9 80 E3 55 32 D9 8A D3 8A C3 D0 EA 02 C0 32 D0 02 DB 80 E2 55 32 D3 C0 CA 03 80 EA 03 C0 C2 03 80 C2";
	GetOffset("CastSpell", hModule, pattern, std::string((pattern.length() + 1) / 3, 'x'), -0x9C);

	// GameState
	pattern = "C7 80 C8 02 00 00 00 00 00 00 C7 80 B4 02 00 00 00 00 00 00 C7 80 B8 02";
	GetOffset("GameState", hModule, pattern, std::string((pattern.length() + 1) / 3, 'x'), 0x52);

	// CRenderer
	pattern = "6A 04 8B 4E 14 8D 86 44 02 00 00 50 6A 00 8B 11 FF 92 90 00 00 00 8B 86 B8 03 02 00 89 86 DC 02 02 00 83";
	GetOffset("CRenderer", hModule, pattern, std::string((pattern.length() + 1) / 3, 'x'), -0x4);

	// SetSpellCD
	pattern = "8B C8 8B 10 8B 42 04 FF D0 D8 44 24 0C F3 0F 10 4C 24";
	GetOffset("SetSpellCD", hModule, pattern, std::string((pattern.length() + 1) / 3, 'x'), -0x20);
}


void CUtil::GetOffset(std::string name, HMODULE hModule, std::string pattern, std::string mask, DWORD offset) {

	pattern.erase(std::remove(pattern.begin(), pattern.end(), ' '), pattern.end());
	unsigned char byte_array[2048];
	HexStrToByteArray(pattern.c_str(), byte_array, 2048);
	DWORD objManager = this->FindPattern((DWORD)hModule, this->GetModuleInfo(hModule).SizeOfImage, byte_array, (char*)mask.c_str());
	objManager = (*((DWORD*)(objManager + offset))) - (DWORD)this->GetLOLBaseAddress();
	std::stringstream buffer;
	buffer << name << ": 0x" << std::hex << objManager << std::endl;
	this->AddLog((char*)buffer.str().c_str());
	//memset(&byte_array[0], 0, sizeof(byte_array));
}

void CUtil::AddLog(char* string)
{
	USES_CONVERSION;
	HANDLE filehandle;
	DWORD dwReadBytes;
	char buffer[2048];
	filehandle = CreateFile(CA2W((GetCurrentPath() + "\Log.txt").c_str()), GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	SetFilePointer(filehandle, 0, 0, FILE_END);
	sprintf_s(buffer, 1024, "%s\r\n", string);
	printf_s("%s\r\n", string);
	WriteFile(filehandle, buffer, strlen(buffer), &dwReadBytes, 0);
	CloseHandle(filehandle);
}


void CUtil::PrintByteArray(unsigned char *byte_array, int byte_array_size)
{
	int i = 0;
	printf("0x");
	for (; i < byte_array_size; i++)
	{
		printf("%02x", byte_array[i]);
	}
	printf("\n");
}

int CUtil::HexStrToByteArray(const char *hex_str, unsigned char *byte_array, int byte_array_max)
{
	int hex_str_len = strlen(hex_str);
	int i = 0, j = 0;

	int byte_array_size = (hex_str_len + 1) / 2;

	if (byte_array_size > byte_array_max)
	{
		return -1;
	}

	if (hex_str_len % 2 == 1)
	{
		if (sscanf(&(hex_str[0]), "%1hhx", &(byte_array[0])) != 1)
		{
			return -1;
		}

		i = j = 1;
	}

	for (; i < hex_str_len; i += 2, j++)
	{
		if (sscanf(&(hex_str[i]), "%2hhx", &(byte_array[j])) != 1)
		{
			return -1;
		}
	}

	return byte_array_size;
}

void CUtil::FAllocConsole() {

	AllocConsole();
	FILE* f1;
	FILE* f2;
	FILE* f3;
	freopen_s(&f1, "CONOUT$", "wb", stdout);
	freopen_s(&f2, "CONOUT$", "wb", stderr);
	freopen_s(&f3, "CONIN$", "rb", stdin);
}

std::string CUtil::GetCurrentPath()
{
	if (currentPath.length() == 0) {
		USES_CONVERSION;
		LPTSTR  lpCurrentPath = new TCHAR[_MAX_PATH];
		GetModuleFileName((HINSTANCE)&__ImageBase, lpCurrentPath, _MAX_PATH);
		std::string currPath = W2A(lpCurrentPath);
		currPath.replace(currPath.end() - 14, currPath.end(), "\0");
		currentPath = currPath;
	}
	return currentPath;
}

unsigned int CUtil::GetLOLBaseAddress() {
	if (this->lolBaseAddress == 0) {
		this->lolBaseAddress = (unsigned)GetModuleHandleA("League of Legends.exe");
	}
	return this->lolBaseAddress;
}

_MODULEINFO CUtil::GetModuleInfo(HMODULE hModule) {
	_MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(_MODULEINFO));
	return modInfo;
}


bool CUtil::Mask(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
		if (*szMask == 'x' && *pData != *bMask)
			return false;
	return (*szMask) == NULL;
}

DWORD CUtil::FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask)
{
	for (DWORD i = 0; i < dwLen; i++)
		if (this->Mask((BYTE*)(dwAddress + i), bMask, szMask))
			return (DWORD)(dwAddress + i);
	return 0;
}


CUtil* CUtil::Instance()
{
	if (inst == 0)
		inst = new CUtil();
	return inst;
}


