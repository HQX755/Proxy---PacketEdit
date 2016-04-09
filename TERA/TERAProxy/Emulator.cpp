#include "stdafx.h"
#include "Emulator.h"

bool CEmulator::ConnectToSL()
{
	DebugPrint("[Emulator] Trying to connect to Serverlist.\n");

	boost::asio::ip::tcp::resolver res(m_Service);
	boost::asio::ip::tcp::resolver::query query(m_SLIp, m_SLPort);

	boost::asio::ip::tcp::resolver::iterator it = res.resolve(query, m_LastError);

	if (m_LastError)
	{
		DebugPrint("[Emulator] Error on resolving ip: %s\n", m_LastError.message().c_str());
		return false;
	}

	boost::asio::connect(m_Server, it, m_LastError);

	if (!m_LastError)
	{
		DebugPrint("[Emulator] Connected to serverlist.\n");
		SL_Read();
	}
	else
	{
		DebugPrint("[Emulator] Couldn't connect to serverlist. %s\n", m_LastError.message().c_str());
		return false;
	}

	return true;
}

CEmulator::CEmulator(boost::asio::io_service & Service, const char * szIp, const char * szPort, const char * szIp_SL, const char * szPort_SL) :
	m_Acceptor(Service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), atoi(szPort))),
	m_Service(Service),
	m_Socket(Service),
	m_Server(Service),
	m_SLIp(szIp_SL),
	m_SLPort(szPort_SL),
	m_Port(szPort),
	m_Ip(szIp),
	m_vInputData(4096),
	m_vClientData(1024)
{
}

CEmulator::~CEmulator()
{
	//m_Socket.cancel();
	//m_Server.cancel();

	m_Socket.close();
	m_Server.close();
}

void CEmulator::RunEmulator()
{
	Listen();
}

boost::asio::ip::tcp::socket & CEmulator::GetSock()
{
	return m_Socket;
}

void CEmulator::CLIENT_OnClose()
{
	if (m_Server.is_open()) m_Server.cancel();
	if (m_Socket.is_open()) m_Socket.cancel();
}

bool CEmulator::CLIENT_OnConnect()
{
	if (!ConnectToSL())
	{
		CLIENT_OnClose();

		return false;
	}

	return true;
}

void CEmulator::SERVER_Send(uint8_t * pData, uint32_t size)
{
	if (!m_Server.is_open())
	{
		return;
	}

	DebugPrint("[Emulator -> ServerList] Sent new packet. %d\n", size);

	m_Server.send(boost::asio::buffer(pData, size));
}

void CEmulator::SOCKET_Send(uint8_t * pData, uint32_t size)
{
	if (!m_Socket.is_open())
	{
		return;
	}

	DebugPrint("[Emulator -> Client] Sent new packet. %d\n", size);

	m_Socket.send(boost::asio::buffer(pData, size));
}

void CEmulator::HandleRead(const boost::system::error_code & ec, size_t trans)
{
	if (!ec)
	{
		DebugPrint("[Emulator <- Client] Received new packet. S:%d\n", trans);

		if (m_Server.is_open())
		{
			m_Server.send(boost::asio::buffer(m_vClientData.data(), trans), 0, m_LastError);

			if(m_LastError)
			{
				DebugPrint("[Emulator -> ServerList] Error when sending data. %s\n", ec.message().c_str());
			}
			else
			{
				DebugPrint("[Emulator -> ServerList] Send data\n");
			}
		}

		CL_Read();
	}
	else
	{
		DebugPrint("[Emulator <- Client] Failed when reading. %s\n", ec.message().c_str());

		CLIENT_OnClose();
		m_Server.close();
	}
}

void CEmulator::SL_HandleRead(const boost::system::error_code & ec, size_t trans)
{
	if (!ec)
	{
		DebugPrint("[Emulator <- ServerList] Received new packet. S:%d\n", trans);

		if (m_Socket.is_open())
		{
			m_Socket.send(boost::asio::buffer(m_vInputData.data(), trans), 0, m_LastError);

			if(m_LastError)
			{
				DebugPrint("[Emulator -> Client] Error when sending data. %s\n", m_LastError.message().c_str());
			}
			else
			{
				DebugPrint("[Emulator -> Client] Sent data.\n");
			}
		}

		SL_Read();
	}
	else
	{
		DebugPrint("[Emulator <- ServerList] Failed when reading. %s\n", ec.message().c_str());

		CLIENT_OnClose();
		m_Socket.close();

		Listen();
	}
}

void CEmulator::CL_Read()
{
	m_Socket.async_read_some(boost::asio::buffer(m_vClientData.data(), m_vClientData.capacity()), boost::bind(&CEmulator::HandleRead,
		this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void CEmulator::SL_Read()
{
	m_Server.async_read_some(boost::asio::buffer(m_vInputData.data(), m_vInputData.capacity()), boost::bind(&CEmulator::SL_HandleRead,
		this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void CEmulator::Listen()
{
	DebugPrint("[Emulator] Listening on port %s.\n", m_Port.c_str());

	m_Acceptor.async_accept(m_Socket, boost::bind(&CEmulator::HandleAccept, this, boost::asio::placeholders::error));
}

void CEmulator::HandleAccept(const boost::system::error_code & ec)
{
	if (ec)
	{
		DebugPrint("[Emulator] Couldn't connect. EC: %s\n", ec.message().c_str());

		CLIENT_OnClose();
		m_Socket.close();

		Listen();
	}
	else
	{
		DebugPrint("[Emulator] New connection from: %s\n", m_Socket.local_endpoint().address().to_string().c_str());

		if (CLIENT_OnConnect())
		{
			CL_Read();
		}
	}
}