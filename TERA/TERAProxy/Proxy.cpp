#include "stdafx.h"
#include "Proxy.h"

CProxy::CProxy(boost::asio::io_service & service, const char * szPort) :
	m_Acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), atoi(szPort))),
	m_Service(service),
	m_Server(service),
	m_Client(service),
	m_bShutdown(false),
	m_vServerData(UINT16_MAX),
	m_vClientData(UINT16_MAX),
	m_ServerRecvThread(NULL),
	m_ClientRecvThread(NULL),
	m_CommandManager(this),
	m_PacketHandler(new CPacketHandler)
{
	m_CryptManager.Initialize();
}

void CProxy::AsyncWaitForClientConnection(const char * szServer, const char * szServerPort)
{
	m_strServerIp = szServer;
	m_strServerPort = szServerPort;

	m_Acceptor.async_accept(m_Client, boost::bind(&CProxy::ConnectToServer, this, boost::asio::placeholders::error));
}

void CProxy::Shutdown()
{
	if (m_bShutdown)
	{
		return;
	}

	m_bShutdown = true;

	DebugPrint("[Proxy] Shutting down.\n");

	if (m_Client.is_open())
	{
		m_Client.cancel(m_LastError);

		if (m_LastError)
		{
			DebugPrint("[Proxy] Error when canceling client socket. %s\n", m_LastError.message().c_str());
		}

		m_Client.close(m_LastError);

		if (m_LastError)
		{
			DebugPrint("[Proxy] Error when closing client socket. %s\n", m_LastError.message().c_str());
		}
	}

	if (m_Server.is_open())
	{
		m_Server.cancel(m_LastError);

		if (m_LastError)
		{
			DebugPrint("[Proxy] Error when canceling server socket. %s\n", m_LastError.message().c_str());
		}

		m_Server.close(m_LastError);

		if (m_LastError)
		{
			DebugPrint("[Proxy] Error when closing server socket. %s\n", m_LastError.message().c_str());
		}
	}

	delete this;
}

void CProxy::SetPacketHandler(CPacketHandler* pHandler)
{
	if (m_PacketHandler)
	{
		delete m_PacketHandler;
	}

	m_PacketHandler = pHandler;
}

void CProxy::ConnectToServer(const boost::system::error_code & ec)
{
	if (ec)
	{
		DebugPrint("[Proxy] Failed accepting client. %s\n", ec.message().c_str());
		Shutdown();
	}
	else
	{
		boost::asio::ip::tcp::resolver res(m_Service);
		boost::asio::ip::tcp::resolver::query query(m_strServerIp, m_strServerPort);

		boost::asio::ip::tcp::resolver::iterator it = res.resolve(query, m_LastError);

		if (m_LastError)
		{
			DebugPrint("[Proxy] Error on resolving server ip. %s\n", m_LastError.message().c_str());
			Shutdown();
		}
		else
		{
			boost::asio::connect(m_Server, it, m_LastError);

			if (m_LastError)
			{
				DebugPrint("[Proxy] Error when connecting to server. %s\n", m_LastError.message().c_str());
				Shutdown();
			}
			else
			{
				DebugPrint("[Proxy] Connected to server.\n");
				WaitForHandshake();
			}
		}
	}
}

void CProxy::WaitForHandshake()
{
	size_t s = boost::asio::read(m_Server, boost::asio::buffer(m_vServerData.data(), 4), m_LastError);

	if (m_LastError)
	{
		DebugPrint("[Proxy] Error on Server handshake. %s\n", m_LastError.message().c_str());
		Shutdown();
	}
	else
	{
		m_Client.send(boost::asio::buffer(m_vServerData.data(), s), 0, m_LastError);

		if (m_LastError)
		{
			DebugPrint("[Proxy] Error on sending Server handshake. %s\n", m_LastError.message().c_str());
			Shutdown();
		}
		else
		{
			DebugPrint("[Proxy] Handshake completed.\n");

			CLIENT_StartRecv();
			SERVER_StartRecv();
		}
	}
}

inline bool CProxy::SERVER_SendPacket(uint8_t * pData, uint32_t size)
{
	DebugPrint("[Proxy -> Server] Sent new packet: %d\n", size);

	boost::asio::write(m_Server, boost::asio::buffer(pData, size), m_LastError);

	if (m_LastError)
	{
		DebugPrint("[Proxy -> Server] Error when sending packet. %s\n", m_LastError.message().c_str());

		// Shutdown after failure.
		Shutdown();

		return false;
	}

	return true;
}

bool CProxy::SERVER_SendPacket(uint8_t * pData, uint32_t size, int32_t count)
{
	if (count < 1)
	{
		return false;
	}
	
	if (count == 1)
	{
		m_CryptManager.ClientEncrypt(pData, size, false);
		{
			return SERVER_SendPacket(pData, size);
		}
	}
	else
	{
		for (int32_t i = 0; i < count; ++i)
		{
			m_CryptManager.ClientEncrypt(pData, size, m_vClientData.data(), false);
			{
				if (!SERVER_SendPacket(m_vClientData.data(), size))
				{
					return false;
				}
			}
		}
	}

	return true;
}

inline bool CProxy::CLIENT_SendPacket(uint8_t * pData, uint32_t size)
{
	DebugPrint("[Proxy -> Client] Sent new packet: %d\n", size);

	boost::asio::write(m_Client, boost::asio::buffer(pData, size), m_LastError);

	if (m_LastError)
	{
		DebugPrint("[Proxy -> Client] Error when sending packet. %s\n", m_LastError.message().c_str());
		Shutdown();

		return false;
	}

	return true;
}

bool CProxy::CLIENT_SendPacket(uint8_t * pData, uint32_t size, int32_t count)
{
	if (count < 1)
	{
		return false;
	}

	if (count == 1)
	{
		m_CryptManager.ServerEncrypt(pData, size, false);
		{
			return CLIENT_SendPacket(pData, size);
		}
	}
	else
	{
		uint8_t *pSaved = CopyHeap(pData, size);

		for (int32_t i = 0; i < count; ++i)
		{
			m_CryptManager.ServerEncrypt(pSaved, size, pData, false);
			{
				if (!CLIENT_SendPacket(pData, size))
				{
					break;
				}
			}
		}

		delete[] pSaved;
	}

	return true;
}

void CProxy::ParseServerPacket(uint8_t * pData, uint32_t size)
{ 
	boost::mutex::scoped_lock lock(m_ServerRecvMutex);

	uint16_t wHeader = _WORD(pData, 2);

	uint8_t* pSendData = pData;
	uint32_t iSendSize = size;
	int32_t iSendCount = 1;

	switch (wHeader)
	{
	case S_SPAWN_NPC: ///< Sighting of an npc.

		if ((iSendCount = m_PacketHandler->OnRecvNPCSpawn(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.AddNPC(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.AddNPC(pData, size);
		}

		break;

	case S_SPAWN_ME: ///< Happens after login.


		if ((iSendCount = m_PacketHandler->OnRecvSpawnMe(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.OnMainControllerSpawn(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.OnMainControllerSpawn(pData, size);
		}

		break;

	case S_PLAYER_STAT_UPDATE:

		if ((iSendCount = m_PacketHandler->OnRecvPlayerStatsUpdate(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.OnPlayerStatsUpdate(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.OnPlayerStatsUpdate(pData, size);
		}

		break;

	case S_DESPAWN_NPC: ///< When a npc vanishes / gets out of range.
		
		if ((iSendCount = m_PacketHandler->OnRecvNPCDespawn(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.RemoveNPC(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.RemoveNPC(pData, size);
		}

		break;

	case S_SPAWN_USER: ///< Sighting of a player.
		
		if ((iSendCount = m_PacketHandler->OnRecvUserSpawn(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.AddPC(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.AddPC(pData, size);
		}

		break;

	case S_DESPAWN_USER: ///< When a player vanishes / gets out of range.
		
		if ((iSendCount = m_PacketHandler->OnRecvUserDespawn(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.RemovePC(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.RemovePC(pData, size);
		}

		break;

	case S_START_USER_PROJECTILE: ///< User Projectile fired.
		
		if ((iSendCount = m_PacketHandler->OnRecvUserProjectile(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.AddUserProjectile(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.AddUserProjectile(pData, size);
		}

		break;

	case S_DESPAWN_PROJECTILE: ///< Projectile hit / vanish.
		
		if ((iSendCount = m_PacketHandler->OnRecvDespawnProjectile(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.RemoveProjectile(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.RemoveProjectile(pData, size);
		}
		
		break;

	case S_END_USER_PROJECTILE: ///< User projectile hit / vanish.

		if ((iSendCount = m_PacketHandler->OnRecvDespawnUserProjectile(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.RemoveUserProjectile(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.RemoveUserProjectile(pData, size);
		}

		break;

	case S_USER_LOCATION: ///< Player move.
	case S_USER_LOCATION_IN_ACTION: ///< Player move while using skill.

		if ((iSendCount = m_PacketHandler->OnRecvUserLocation(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.OnPCMove(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.OnPCMove(pData, size);
		}

		break;

	case S_NPC_LOCATION: ///< Npc move.
		
		if ((iSendCount = m_PacketHandler->OnRecvNPCLocation(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.OnNPCMove(pSendData, iSendSize);
		}

		if (iSendCount == 0)
		{
			m_ClientManager.OnNPCMove(pData, size);
		}

		break;

	case S_EACH_SKILL_RESULT: ///< Skill result.

		iSendCount = m_PacketHandler->OnRecvEachSkillResult(this, pData, size, &pSendData, &iSendSize);

		break;

	case S_BOSS_GAGE_INFO: ///< .

		iSendCount = m_PacketHandler->OnRecvBossGageInfo(this, pData, size, &pSendData, &iSendSize);

		break;

	case S_BOSS_GAGE_STACK_INFO: ///< .

		iSendCount = m_PacketHandler->OnRecvBossGageStackInfo(this, pData, size, &pSendData, &iSendSize);

		break;

	case S_HIT_COMBO: ///< Hit count.

		iSendCount = m_PacketHandler->OnRecvHitCombo(this, pData, size, &pSendData, &iSendSize);

		break;

	case S_CREATURE_CHANGE_HP: ///< NPC HP.

		if ((iSendCount = m_PacketHandler->OnRecvCreatureHP(this, pData, size, &pSendData, &iSendSize)) > 0)
		{
			m_ClientManager.OnNPCChangeHP(pData, size);
		}

		break;
	}

	CLIENT_SendPacket(pSendData, iSendSize, iSendCount);

	if (pSendData != pData)
	{
		delete[] pSendData;
	}
}

void CProxy::ParseClientPacket(uint8_t * pData, uint32_t size)
{
	boost::mutex::scoped_lock lock(m_ClientRecvMutex);

	uint16_t wHeader = _WORD(pData, 2);

	uint8_t* pSendData = pData;
	uint32_t iSendSize = size;
	int32_t iSendCount = 1;

	switch (wHeader)
	{
	case C_PLAYER_LOCATION:
		m_ClientManager.OnMainControllerMove(pData, size);

		break;

	case C_HIT_USER_PROJECTILE:
		iSendCount = m_PacketHandler->OnSendHitProjectile(this, pData, size, &pSendData, &iSendSize);

		break;

	case C_CHAT:
		iSendCount = m_PacketHandler->OnSendChat(this, pData, size, &pSendData, &iSendSize);

		break;
	}

	SERVER_SendPacket(pSendData, iSendSize, iSendCount);

	if (pSendData != pData)
	{
		delete[] pSendData;
	}
}

inline void CProxy::SERVER_CallbackRead(const boost::system::error_code & ec, uint32_t trans)
{
	if (ec)
	{
		DebugPrint("[Client <- Proxy <- Server] Error on read. %s\n", ec.message().c_str());
		Shutdown();

		return;
	}
	else
	{
		// First two packets will have the key mat.
		if (!m_CryptManager.HasServerKey())
		{
			// Apply key.
			m_CryptManager.SVSetKeyData(m_vServerData.data());

			DebugPrint("[Proxy] Applied Server Key.\n");

			// Send key to client.
			CLIENT_SendPacket(m_vServerData.data(), trans);
		}
		else
		{
			uint8_t bHeader[4];

			// Save encrypted header.
			_DWORD(bHeader, 0) = _DWORD(m_vServerData.data(), 0);

			// Decrypt and restore after, to find out size of packet.
			m_CryptManager.DoEncrypt(m_vServerData.data(), trans);
			m_CryptManager.DoRestoreEncrypt();
			{
				// First two bytes define the packet size.
				uint16_t packetSize = _WORD(m_vServerData.data(), 0);

				// Restore encrypted header.
				_DWORD(m_vServerData.data(), 0) = _DWORD(bHeader, 0);

				// Need to wait till the full packet has been read.
				while (trans < packetSize)
				{
					trans += boost::asio::read(m_Server, boost::asio::buffer(m_vServerData.data() + trans, packetSize - trans), m_LastError);

					if (m_LastError)
					{
						DebugPrint("[Client <- Proxy <- Server] Error on read. %s\n", ec.message().c_str());

						// Shutdown after failure.
						Shutdown();

						return;
					}
				}
			}

			// Make the crypto key update. Decrypt and analyze the data.
			m_CryptManager.DoEncrypt(m_vServerData.data(), trans);
			{
				DebugPrint("[Client <- Proxy <- Server] New packet: %d\t%s\n", trans, GetHeaderByKey(_WORD(m_vServerData.data(), 2)));

				// Parse data now.
				ParseServerPacket(m_vServerData.data(), trans);
			}
		}
	}

	SERVER_Read();
}

inline bool CProxy::SERVER_CheckRecvError(bool bShutdown)
{
	if (m_LastServerRecvError)
	{
		DebugPrint("[Proxy] Error on recv. %s\n", m_LastServerRecvError.message().c_str());

		if (bShutdown && !m_bShutdown)
		{
			// Shutdown after failure.
			m_Service.post(boost::bind(&CProxy::Shutdown, this));
		}

		return true;
	}

	return false;
}

inline bool CProxy::CLIENT_CheckRecvError(bool bShutdown)
{
	if (m_LastClientRecvError)
	{
		DebugPrint("[Proxy] Error on client recv. %s\n", m_LastClientRecvError.message().c_str());

		if (bShutdown && !m_bShutdown)
		{
			// Shutdown after failure.
			m_Service.post(boost::bind(&CProxy::Shutdown, this));
		}

		return true;
	}

	return false;
}

inline void CProxy::CLIENT_ThreadRecv()
{
	size_t trans = 0;

	while (!m_CryptManager.HasClientKey())
	{
		boost::asio::read(m_Client, boost::asio::buffer(m_vClientData.data(), 128), m_LastClientRecvError);

		if (CLIENT_CheckRecvError())
		{
			return;
		}

		// Setup key
		m_CryptManager.CLSetKeyData(m_vClientData.data());

		// Send key
		SERVER_SendPacket(m_vClientData.data(), 128);
	}

	while (true)
	{
		trans = boost::asio::read(m_Client, boost::asio::buffer(m_vClientData.data(), 4), m_LastClientRecvError);

		if (CLIENT_CheckRecvError())
		{
			return;
		}

		uint8_t bHeader[4];

		// Save encrypted header.
		_DWORD(bHeader, 0) = _DWORD(m_vClientData.data(), 0);

		m_CryptManager.DoDecrypt(m_vClientData.data(), trans);
		m_CryptManager.DoRestoreDecrypt();
		{
			uint16_t packetSize = _WORD(m_vClientData.data(), 0);

			// Restore encrypted header.
			_DWORD(m_vClientData.data(), 0) = _DWORD(bHeader, 0);

			while (trans < packetSize)
			{
				trans += boost::asio::read(m_Client, boost::asio::buffer(m_vClientData.data() + trans, packetSize - trans), m_LastClientRecvError);

				if (CLIENT_CheckRecvError())
				{
					return;
				}
			}
		}

		m_CryptManager.DoDecrypt(m_vClientData.data(), trans);
		{
			DebugPrint("[Client -> Proxy -> Server] New packet. %d\t%s\n", trans, GetHeaderByKey(_WORD(m_vClientData.data(), 2)));

			// Analyze the packet and send it to server.
			ParseClientPacket(m_vClientData.data(), trans);
		}
	}
}

inline void CProxy::SERVER_ThreadRecv()
{
	size_t trans = 0;

	while (!m_CryptManager.HasServerKey())
	{
		boost::asio::read(m_Server, boost::asio::buffer(m_vServerData.data(), 128), m_LastServerRecvError);

		if (SERVER_CheckRecvError())
		{
			return;
		}

		// Setup key.
		m_CryptManager.SVSetKeyData(m_vServerData.data());

		// Send key.
		CLIENT_SendPacket(m_vServerData.data(), 128);
	}

	while (true)
	{
		trans = boost::asio::read(m_Server, boost::asio::buffer(m_vServerData.data(), 4), m_LastServerRecvError);

		if (SERVER_CheckRecvError())
		{
			return;
		}

		uint8_t bHeader[4];

		// Save encrypted header.
		_DWORD(bHeader, 0) = _DWORD(m_vServerData.data(), 0);

		// Decrypt and restore after, to find out size of packet.
		m_CryptManager.DoEncrypt(m_vServerData.data(), trans);
		m_CryptManager.DoRestoreEncrypt();
		{
			// First two bytes define the packet size.
			uint16_t packetSize = _WORD(m_vServerData.data(), 0);

			// Restore encrypted header.
			_DWORD(m_vServerData.data(), 0) = _DWORD(bHeader, 0);

			// Need to wait till the full packet has been read.
			while (trans < packetSize)
			{
				trans += boost::asio::read(m_Server, boost::asio::buffer(m_vServerData.data() + trans, packetSize - trans), m_LastServerRecvError);

				if (SERVER_CheckRecvError())
				{
					return;
				}
			}
		}

		// Make the crypto key update. Decrypt and parse the data.
		m_CryptManager.DoEncrypt(m_vServerData.data(), trans);
		{
			DebugPrint("[Client <- Proxy <- Server] New packet: %d\t%s\n", trans, GetHeaderByKey(_WORD(m_vServerData.data(), 2)));

			// Parse data now.
			ParseServerPacket(m_vServerData.data(), trans);
		}
	}
}

inline void CProxy::SERVER_StartRecv()
{
	if (m_Server.is_open() && !m_ServerRecvThread)
	{
		m_ServerRecvThread = new boost::thread(boost::bind(&CProxy::SERVER_ThreadRecv, this));
	}
	else
	{
		DebugPrint("[Proxy] Error when starting server recv thread.\n");
	}
}

inline void CProxy::CLIENT_StartRecv()
{
	if (m_Client.is_open() && !m_ClientRecvThread)
	{
		m_ClientRecvThread = new boost::thread(boost::bind(&CProxy::CLIENT_ThreadRecv, this));
	}
	else
	{
		DebugPrint("[Proxy] Error when starting client recv thread.\n");
	}
}

inline void CProxy::SERVER_Read()
{
	if (!m_CryptManager.HasServerKey())
	{
		boost::asio::async_read(m_Server, boost::asio::buffer(m_vServerData.data(), 128),
			boost::bind(&CProxy::SERVER_CallbackRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		boost::asio::async_read(m_Server, boost::asio::buffer(m_vServerData.data(), 4),
			boost::bind(&CProxy::SERVER_CallbackRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
}

inline void CProxy::CLIENT_CallbackRead(const boost::system::error_code & ec, uint32_t trans)
{
	if (ec)
	{
		DebugPrint("[Server <- Proxy <- Client] Error when reading data. %s\n", ec.message().c_str());

		// Shutdown after failure.
		Shutdown();

		return;
	}
	else
	{
		if (!m_CryptManager.HasClientKey())
		{
			// Apply key.
			m_CryptManager.CLSetKeyData(m_vClientData.data());

			DebugPrint("[Proxy] Applied Client key.\n");

			// Send key to server.
			SERVER_SendPacket(m_vClientData.data(), trans);
		}
		else
		{
			uint8_t bHeader[4];

			// Save encrypted header.
			_DWORD(bHeader, 0) = _DWORD(m_vClientData.data(), 0);

			m_CryptManager.DoDecrypt(m_vClientData.data(), trans);
			m_CryptManager.DoRestoreDecrypt();
			{
				uint16_t packetSize = _WORD(m_vClientData.data(), 0);

				// Restore encrypted header.
				_DWORD(m_vClientData.data(), 0) = _DWORD(bHeader, 0);

				while (trans < packetSize)
				{
					trans += boost::asio::read(m_Client, boost::asio::buffer(m_vClientData.data() + trans, packetSize - trans), m_LastError);

					if (m_LastError)
					{
						DebugPrint("[Client <- Proxy <- Server] Error on read. %s\n", m_LastError.message().c_str());

						// Shutdown after failure.
						Shutdown();

						return;
					}
				}
			}

			m_CryptManager.DoDecrypt(m_vClientData.data(), trans);
			{
				DebugPrint("[Client -> Proxy -> Server] New packet. %d\t%s\n", trans, GetHeaderByKey(_WORD(m_vClientData.data(), 2)));

				// Parse the packet and send it to server.
				ParseClientPacket(m_vClientData.data(), trans);
			}
		}
	}

	CLIENT_Read();
}

inline void CProxy::CLIENT_Read()
{
	if (!m_CryptManager.HasClientKey())
	{
		boost::asio::async_read(m_Client, boost::asio::buffer(m_vClientData.data(), 128),
			boost::bind(&CProxy::CLIENT_CallbackRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		boost::asio::async_read(m_Client, boost::asio::buffer(m_vClientData.data(), 4),
			boost::bind(&CProxy::CLIENT_CallbackRead, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
}

void CProxy::Lock()
{
	m_ServerRecvMutex.lock();
}

void CProxy::LockClientRecv()
{
	m_ClientRecvMutex.lock();
}

/*
*	Unlock client mutex;
*/

void CProxy::Unlock()
{
	m_ServerRecvMutex.unlock();
}

void CProxy::UnlockClientRecv()
{
	m_ClientRecvMutex.unlock();
}


/*
*	Return pointer to client;
*/

CClientManager * CProxy::GetClient()
{
	return &m_ClientManager;
}

/*
*	Returns pointer to command manager.
*/

CCommandManager * CProxy::GetCommandManager()
{
	return &m_CommandManager;
}

CProxy::~CProxy()
{
	if (!m_bShutdown)
	{
		m_bShutdown = true;

		DebugPrint("[Proxy] Shutting down.\n");

		if (m_Client.is_open())
		{
			m_Client.cancel(m_LastError);

			if (m_LastError)
			{
				DebugPrint("[Proxy] Error when canceling client socket. %s\n", m_LastError.message().c_str());
			}

			m_Client.close(m_LastError);

			if (m_LastError)
			{
				DebugPrint("[Proxy] Error when closing client socket. %s\n", m_LastError.message().c_str());
			}
		}

		if (m_Server.is_open())
		{
			m_Server.cancel(m_LastError);

			if (m_LastError)
			{
				DebugPrint("[Proxy] Error when canceling server socket. %s\n", m_LastError.message().c_str());
			}

			m_Server.close(m_LastError);

			if (m_LastError)
			{
				DebugPrint("[Proxy] Error when closing server socket. %s\n", m_LastError.message().c_str());
			}
		}
	}

	if (m_ServerRecvThread)
	{
		m_ServerRecvThread->interrupt();
		delete m_ServerRecvThread;
		m_ServerRecvThread = NULL;
	}

	if (m_ClientRecvThread)
	{
		m_ClientRecvThread->interrupt();
		delete m_ClientRecvThread;
		m_ClientRecvThread = NULL;
	}

	if (m_PacketHandler)
	{
		delete m_PacketHandler;
		m_PacketHandler = NULL;
	}
}