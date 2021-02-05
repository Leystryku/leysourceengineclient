
#include <Shlwapi.h>
#include <string>

#include "../deps/osw/Steamworks.h"
#include "../deps/osw/ISteamUser017.h"
#include "steam.h"


std::wstring Steam::GetSteamInstallFolder()
{
	wchar_t installFolder[MAX_PATH] = { 0 };
	unsigned long bufferLength = sizeof(installFolder);
	unsigned long type = REG_SZ;

	if (SHGetValue(HKEY_CURRENT_USER, TEXT("Software\\Valve\\Steam"), TEXT("SteamPath"), &type, installFolder, &bufferLength) != ERROR_SUCCESS)
	{
		MessageBox(0, L"Could not find Steam Install directory in:\n HKEY_CURRENT_USER\\Software\\Valve\\Steam\\SteamPath)\n", L"leysourceengineclient - GetSteamInstallFolder", MB_OK);
	}

	return installFolder;
}

ISteamUser017* Steam::GetSteamUser()
{
	return this->steamUser;
}

ISteamUser017* temporaryHack = 0;

int Steam::Initiate()
{
	SetDllDirectory(GetSteamInstallFolder().c_str());

	HMODULE steam = LoadLibrary(L"steam");
	HMODULE steamapi = LoadLibrary(L"steam_api");
	HMODULE steamclientdll = LoadLibrary(L"steamclient");

	if (!steamclientdll)
	{
		return 1;
	}

	CreateInterfaceFn fnApiInterface = (CreateInterfaceFn)GetProcAddress(steamclientdll, "CreateInterface");

	if (!fnApiInterface)
	{
		return 2;
	}

	if (!(this->steamClient = (ISteamClient017*)fnApiInterface(STEAMCLIENT_INTERFACE_VERSION_017, NULL)))
	{
		return 3;
	}

	if (!(this->steamPipeHandle = this->steamClient->CreateSteamPipe()))
	{
		return 4;
	}

	if (!(this->steamUserHandle = this->steamClient->ConnectToGlobalUser(this->steamPipeHandle)))
	{
		return 5;
	}

	if (!(this->clientEngine = (IClientEngine*)fnApiInterface(CLIENTENGINE_INTERFACE_VERSION, NULL)))
	{
		return 6;
	}

	if (!(this->steamUser = (ISteamUser017*)this->steamClient->GetISteamUser(this->steamUserHandle, this->steamPipeHandle, STEAMUSER_INTERFACE_VERSION_017)))
	{
		return 7;
	}

	temporaryHack = this->steamUser;

	return 0;
}