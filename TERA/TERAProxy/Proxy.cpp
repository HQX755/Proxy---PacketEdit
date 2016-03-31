#include "stdafx.h"
#include "Proxy.h"

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

inline void CProxy::SERVER_SendPacket(uint8_t * pData, uint32_t size)
{
	DebugPrint("[Proxy -> Server] Sent new packet: %d\n", size);

	boost::asio::write(m_Server, boost::asio::buffer(pData, size), m_LastError);

	if (m_LastError)
	{
		DebugPrint("[Proxy -> Server] Error when sending packet. %s\n", m_LastError.message().c_str());

		//Shutdown after failure.
		Shutdown();
	}
}

inline void CProxy::CLIENT_SendPacket(uint8_t * pData, uint32_t size)
{
	DebugPrint("[Proxy -> Client] Sent new packet: %d\n", size);

	boost::asio::write(m_Client, boost::asio::buffer(pData, size), m_LastError);

	if (m_LastError)
	{
		DebugPrint("[Proxy -> Client] Error when sending packet. %s\n", m_LastError.message().c_str());

		//Shutdown after failure.
		Shutdown();
	}
}

inline void CProxy::ParseServerPacket(uint8_t * pData, uint32_t size)
{
	uint16_t wHeader = _WORD(pData, 2);

	switch (wHeader)
	{
	case S_SPAWN_NPC: ///< Sighting of an npc.
		m_ClientManager.AddNPC(pData, size);

		break;

	case S_SPAWN_ME: ///< Happens after login.
		m_ClientManager.SpawnMainController(pData, size);

		break;

	case S_DESPAWN_NPC: ///< When a npc vanishes / gets out of range.
		m_ClientManager.RemoveNPC(pData, size);

		break;

	case S_SPAWN_USER: ///< Sighting of a player.
		m_ClientManager.AddPC(pData, size);

		break;

	case S_DESPAWN_USER: ///< When a player vanishes / gets out of range.
		m_ClientManager.RemovePC(pData, size);

		break;

	case S_USER_LOCATION:
	case S_USER_LOCATION_IN_ACTION:
		m_ClientManager.OnPCMove(pData, size);

		break;

	case S_NPC_LOCATION:
		m_ClientManager.OnNPCMove(pData, size);

		break;
	}

	m_CryptManager.ServerEncrypt(pData, size);
	{
		CLIENT_SendPacket(pData, size);
	}
}

inline void CProxy::ParseClientPacket(uint8_t * pData, uint32_t size)
{
	uint16_t wHeader = _WORD(pData, 2);

	switch (wHeader)
	{
	case C_PLAYER_LOCATION:
		m_ClientManager.OnMainControllerMove(pData, size);

		break;
	}

	m_CryptManager.ClientEncrypt(pData, size);
	{
		SERVER_SendPacket(pData, size);
	}
}

inline void CProxy::SERVER_CallbackRead(const boost::system::error_code & ec, uint32_t trans)
{
	if (ec)
	{
		DebugPrint("[Client <- Proxy <- Server] Error on read. %s\n", ec.message().c_str());

		//Shutdown after failure.
		Shutdown();

		return;
	}
	else
	{
		// First two packets will have the key mat.
		if (!m_CryptManager.HasServerKey())
		{
			m_CryptManager.SVSetKeyData(m_vServerData.data());

			DebugPrint("[Proxy] Applied Server Key.\n");

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

		if (bShutdown)
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

		if (bShutdown)
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

		// Setup key
		m_CryptManager.SVSetKeyData(m_vServerData.data());

		// Send key
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

		// Make the crypto key update. Decrypt and analyze the data.
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

		//Shutdown after failure.
		Shutdown();

		return;
	}
	else
	{
		if (!m_CryptManager.HasClientKey())
		{
			m_CryptManager.CLSetKeyData(m_vClientData.data());

			DebugPrint("[Proxy] Applied Client key.\n");

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

						//Shutdown after failure.
						Shutdown();

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
}

CProxy::CProxy(boost::asio::io_service & service, const char * szPort) :
	m_Acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), atoi(szPort))),
	m_Service(service),
	m_Server(service),
	m_Client(service),
	m_bShutdown(false),
	m_vServerData(UINT16_MAX),
	m_vClientData(UINT16_MAX),
	m_ServerRecvThread(NULL),
	m_ClientRecvThread(NULL)
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
