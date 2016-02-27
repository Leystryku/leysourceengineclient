
#pragma pack( push, 1 )		

//-----------------------------------------------------------------------------
// Purpose: encapsulates an appID/modID pair
//-----------------------------------------------------------------------------
class CGameID
{
public:

	CGameID()
	{
		m_gameID.m_nType = k_EGameIDTypeApp;
		m_gameID.m_nAppID = 0x0;
		m_gameID.m_nModID = 0;
	}

	explicit CGameID(unsigned long long ulGameID)
	{
		m_ulGameID = ulGameID;
	}

	explicit CGameID(int nAppID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
	}

	explicit CGameID(unsigned int nAppID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
	}

	CGameID(unsigned int nAppID, unsigned int nModID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nModID = nModID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;
	}

	// Hidden functions used only by Steam
	explicit CGameID(const char *pchGameID);
	const char *Render() const;					// render this Game ID to string
	static const char *Render(unsigned long long ulGameID);		// static method to render a uint64 representation of a Game ID to a string

	// must include checksum_crc.h first to get this functionality
#if defined( CHECKSUM_CRC_H )
	CGameID(uint32 nAppID, const char *pchModPath)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;

		char rgchModDir[MAX_PATH];
		//Q_FileBase(pchModPath, rgchModDir, sizeof(rgchModDir));
		strncpy(rgchModDir, pchModPath, MAX_PATH);

		CRC32_t crc32;
		CRC32_Init(&crc32);
		CRC32_ProcessBuffer(&crc32, rgchModDir, Q_strlen(rgchModDir));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

	CGameID(const char *pchExePath, const char *pchAppName)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeShortcut;

		CRC32_t crc32;
		CRC32_Init(&crc32);
		CRC32_ProcessBuffer(&crc32, (void*)pchExePath, Q_strlen(pchExePath));
		CRC32_ProcessBuffer(&crc32, (void*)pchAppName, Q_strlen(pchAppName));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

#if defined( VSTFILEID_H )

	CGameID(VstFileID vstFileID)
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeP2P;

		CRC32_t crc32;
		CRC32_Init(&crc32);
		const char *pchFileId = vstFileID.Render();
		CRC32_ProcessBuffer(&crc32, pchFileId, Q_strlen(pchFileId));
		CRC32_Final(&crc32);

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | (0x80000000);
	}

#endif /* VSTFILEID_H */

#endif /* CHECKSUM_CRC_H */


	unsigned long long ToUint64() const
	{
		return m_ulGameID;
	}

	unsigned long long *GetUint64Ptr()
	{
		return &m_ulGameID;
	}

	bool IsMod() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeGameMod);
	}

	bool IsShortcut() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeShortcut);
	}

	bool IsP2PFile() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeP2P);
	}

	bool IsSteamApp() const
	{
		return (m_gameID.m_nType == k_EGameIDTypeApp);
	}

	unsigned int ModID() const
	{
		return m_gameID.m_nModID;
	}

	unsigned int AppID() const
	{
		return m_gameID.m_nAppID;
	}

	bool operator == (const CGameID &rhs) const
	{
		return m_ulGameID == rhs.m_ulGameID;
	}

	bool operator != (const CGameID &rhs) const
	{
		return !(*this == rhs);
	}

	bool operator < (const CGameID &rhs) const
	{
		return (m_ulGameID < rhs.m_ulGameID);
	}

	bool IsValid() const
	{
		// each type has it's own invalid fixed point:
		switch (m_gameID.m_nType)
		{
		case k_EGameIDTypeApp:
			return m_gameID.m_nAppID != 0x0;
			break;
		case k_EGameIDTypeGameMod:
			return m_gameID.m_nAppID != 0x0 && m_gameID.m_nModID & 0x80000000;
			break;
		case k_EGameIDTypeShortcut:
			return (m_gameID.m_nModID & 0x80000000) != 0;
			break;
		case k_EGameIDTypeP2P:
			return m_gameID.m_nAppID == 0x0 && m_gameID.m_nModID & 0x80000000;
			break;
		default:
#if defined(Assert)
			Assert(false);
#endif
			return false;
		}

	}

	void Reset()
	{
		m_ulGameID = 0;
	}



private:

	enum EGameIDType
	{
		k_EGameIDTypeApp = 0,
		k_EGameIDTypeGameMod = 1,
		k_EGameIDTypeShortcut = 2,
		k_EGameIDTypeP2P = 3,
	};

	struct GameID_t
	{
#ifdef VALVE_BIG_ENDIAN
		unsigned int m_nModID : 32;
		unsigned int m_nType : 8;
		unsigned int m_nAppID : 24;
#else
		unsigned int m_nAppID : 24;
		unsigned int m_nType : 8;
		unsigned int m_nModID : 32;
#endif
	};

	union
	{
		unsigned long long m_ulGameID;
		GameID_t m_gameID;
	};
};

#pragma pack( pop )

class IClientUtils
{
public:

	virtual const char *GetInstallPath() = 0;
	virtual const char *GetUserBaseFolderInstallImage() = 0;
	virtual const char *GetManagedContentRoot() = 0;

	// return the number of seconds since the user 
	virtual uint32 GetSecondsSinceAppActive() = 0;
	virtual uint32 GetSecondsSinceComputerActive() = 0;
	virtual void SetComputerActive() = 0;

	// the universe this client is connecting to
	virtual EUniverse GetConnectedUniverse() = 0;

	// server time - in PST, number of seconds since January 1, 1970 (i.e unix time)
	virtual uint32 GetServerRealTime() = 0;

	// returns the 2 digit ISO 3166-1-alpha-2 format country code this client is running in (as looked up via an IP-to-location database)
	// e.g "US" or "UK".
	virtual const char *GetIPCountry() = 0;

	// returns true if the image exists, and valid sizes were filled out
	virtual bool GetImageSize(int32 iImage, uint32 *pnWidth, uint32 *pnHeight) = 0;

	// returns true if the image exists, and the buffer was successfully filled out
	// results are returned in RGBA format
	// the destination buffer size should be 4 * height * width * sizeof(char)
	virtual bool GetImageRGBA(int32 iImage, uint8 *pubDest, int32 nDestBufferSize) = 0;

	// returns the IP of the reporting server for valve - currently only used in Source engine games
	virtual bool GetCSERIPPort(uint32 *unIP, uint16 *usPort) = 0;

	virtual uint32 GetNumRunningApps() = 0;

	// return the amount of battery power left in the current system in % [0..100], 255 for being on AC power
	virtual uint8 GetCurrentBatteryPower() = 0;

	virtual void SetOfflineMode(bool bOffline) = 0;
	virtual bool GetOfflineMode() = 0;

	virtual int SetAppIDForCurrentPipe(int nAppID, bool bTrackProcess) = 0;
	virtual int GetAppID() = 0;

	virtual void SetAPIDebuggingActive(bool bActive, bool bVerbose) = 0;

	// API asynchronous call results
	// can be used directly, but more commonly used via the callback dispatch API (see steam_api.h)
	virtual bool IsAPICallCompleted(int hSteamAPICall, bool *pbFailed) = 0;
	virtual int GetAPICallFailureReason(int hSteamAPICall) = 0;
	virtual bool GetAPICallResult(int hSteamAPICall, void *pCallback, int32 cubCallback, int32 iCallbackExpected, bool *pbFailed) = 0;

	virtual bool SignalAppsToShutDown() = 0;
	virtual bool TerminateAllAppsMultiStep(uint32 uUnk) = 0;

	virtual int GetCellID() = 0;

	virtual bool BIsGlobalInstance() = 0;

	// Asynchronous call to check if file is signed, result is returned in CheckFileSignature_t
	virtual int CheckFileSignature(const char *szFileName) = 0;

	virtual uint64 GetBuildID() = 0;

	virtual void SetCurrentUIMode(int eUIMode) = 0;
	virtual void SetLauncherType(int eLauncherType) = 0;
	virtual int GetLauncherType() = 0;

	virtual bool ShowGamepadTextInput(int eInputMode, int eInputLineMode, const char *szText, uint32 uMaxLength, const char * szUnk) = 0;
	virtual uint32 GetEnteredGamepadTextLength() = 0;
	virtual bool GetEnteredGamepadTextInput(char *pchValue, uint32 cchValueMax) = 0;
	virtual void GamepadTextInputClosed(HSteamPipe hSteamPipe, bool, const char *) = 0;

	virtual void SetSpew(int eSpewGroup, int32 iSpewLevel, int32 iLogLevel) = 0;

	virtual bool BDownloadsDisabled() = 0;

	virtual void SetFocusedWindow(int eWindowType, CGameID gameID, uint64 ulUnk) = 0;
	virtual const char *GetSteamUILanguage() = 0;

	virtual uint64 CheckSteamReachable() = 0;
	virtual void SetLastGameLaunchMethod(int eGameLaunchMethod) = 0;
	virtual bool IsSteamOS() = 0;
	virtual void SetVideoAdapterInfo(int32, int32, int32, int32, int32) = 0;
	virtual void SetControllerOVerrideMode(int eWindowType, CGameID gameID, const char * szUnk) = 0;
	virtual void SetOverlayWindowFocusForPipe(bool, bool, CGameID gameID) = 0;
	virtual CGameID GetGameOverlayUIInstanceFocusGameID(bool * pbUnk) = 0;

	virtual bool SetControllerConfigFileForAppID(int unAppID, const char * pszControllerConfigFile) = 0;
	virtual bool GetControllerConfigFileForAppID(int unAppID, const char * pszControllerConfigFile, uint32 cubControllerConfigFile) = 0;

	virtual bool IsSteamRunningInVR() = 0;
};


class ISteamUser012
{
public:
	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual HSteamUser GetHSteamUser() = 0;

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn() = 0;

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID() = 0;


	// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
	// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
	//
	// Parameters:
	// void *pAuthBlob - a pointer to empty memory that will be filled in with the authentication token.
	// int cbMaxAuthBlob - the number of bytes of allocated memory in pBlob. Should be at least 2048 bytes.
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// int nGameID - the ID of the current game.
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	// bool bSecure - whether or not the client thinks that the game server is reporting itself as secure (i.e. VAC is running)
	//
	// return value - returns the number of bytes written to pBlob. If the return is 0, then the buffer passed in was too small, and the call has failed
	// The contents of pBlob should then be sent to the game server, for it to use to complete the authentication process.
	virtual int InitiateGameConnection(void *pBlob, int cbMaxBlob, CSteamID steamID,  uint32 unIPServer, uint16 usPortServer, bool bSecure) = 0;

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection(uint32 unIPServer, uint16 usPortServer) = 0;

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo = "" ) = 0;

	// legacy authentication support - need to be called if the game server rejects the user with a 'bad ticket' error
	virtual bool GetUserDataFolder(char *pchBuffer, int cubBuffer) = 0;

	// Starts voice recording. Once started, use GetCompressedVoice() to get the data
	virtual void StartVoiceRecording() = 0;

	// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
	// a little bit after this function is called. GetCompressedVoice() should continue to be called until it returns
	// k_eVoiceResultNotRecording
	virtual void StopVoiceRecording() = 0;

	// Gets the latest voice data. It should be called as often as possible once recording has started.
	// nBytesWritten is set to the number of bytes written to pDestBuffer. 
	virtual int GetCompressedVoice(void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten) = 0;

	// Decompresses a chunk of data produced by GetCompressedVoice(). nBytesWritten is set to the 
	// number of bytes written to pDestBuffer. The output format of the data is 16-bit signed at 
	// 11025 samples per second.
	virtual int DecompressVoice(void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten) = 0;

	// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
	// pcbTicket retrieves the length of the actual ticket.
	virtual int GetAuthSessionTicket(void *pTicket, int cbMaxTicket, uint32 *pcbTicket) = 0;

	// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual int BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, CSteamID steamID) = 0;

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession(CSteamID steamID) = 0;

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket(int hAuthTicket) = 0;

	// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual int UserHasLicenseForApp(CSteamID steamID, short appID) = 0;
};