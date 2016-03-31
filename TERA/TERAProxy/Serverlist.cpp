#include "stdafx.h"
#include "Emulator.h"

CServerList::CServerList(boost::asio::io_service & service, unsigned short port) :
	m_Service(service),
	m_Acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	m_ActiveSockets = 0;
}

CServerList::~CServerList()
{
	DebugPrint("[ServerList] Shutdown.\n");
}

bool CServerList::ReadXMLServerList(const char * szFile)
{
	std::ifstream input(szFile);

	if (!input.is_open())
	{
		return false;
	}

	m_ServerList << input.rdbuf();

	input.close();

	return true;
}

void CServerList::Run()
{
	boost::asio::ip::tcp::socket *socket = new boost::asio::ip::tcp::socket(m_Service);
	{
		m_Acceptor.async_accept(*socket, boost::bind(&CServerList::HandleAccept, this, boost::asio::placeholders::error, socket));
	}
}

void CServerList::Stop()
{
	m_bDeleteLater = true;
	{
		if (m_ActiveSockets == 0)
		{
			delete this;
		}
	}
}

void CServerList::HandleAccept(const boost::system::error_code & ec, boost::asio::ip::tcp::socket * socket)
{
	if (m_bDeleteLater)
	{
		return;
	}

	if (!ec)
	{
		DebugPrint("[ServerList] New connection from: %s\n", socket->local_endpoint().address().to_string().c_str());

		m_ActiveSockets++;

		boost::shared_ptr<boost::thread>(new boost::thread([this, socket]
		{
			int32_t timeout = 5000;

			if (setsockopt(socket->native(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) < 0)
			{
				DebugPrint("[ServerList] Failed to set recv timeout. %d\n", WSAGetLastError());

				delete socket;
				return;
			}

			std::vector<char> v(512);
			boost::system::error_code error;

			boost::asio::read(*socket, boost::asio::buffer(v.data(), v.capacity()), boost::asio::transfer_at_least(1), error);
			
			if (error)
			{
				DebugPrint("[ServerList] Failed when reading: %s\n", error.message().c_str());
			}
			else if (strncmp(v.data(), "GET", 3) == 0 && socket->is_open())
			{
				socket->send(boost::asio::buffer(m_ServerList.rdbuf()->str().c_str(), m_ServerList.rdbuf()->str().size()), 0, error);

				if (error)
				{
					DebugPrint("[ServerList] Failed when sending server list: %s\n", error.message().c_str());
				}
				else
				{
					DebugPrint("[ServerList] Sent serverlist.\n");
				}
			}
			else
			{
				DebugPrint("[ServerList] Recvd invalid data.\n");
			}

			delete socket;

			m_ActiveSockets--;
			{
				if (m_bDeleteLater && m_ActiveSockets == 0)
				{
					delete this;
				}
			}

		}));
	}
	else
	{
		DebugPrint("[ServerList] Failed to accept socket.\n");
	}

	if (!m_bDeleteLater)
	{
		Run();
	}
}
