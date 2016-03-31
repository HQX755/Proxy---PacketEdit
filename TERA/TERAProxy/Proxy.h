#ifndef _PROXY_H_
#define _PROXY_H_

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include "stdafx.h"

#include "Opcodes.h"
#include "OpcodeStrings.h"

#include "Utils.h"
#include "Crypt.h"
#include "ClientData.h"

class CProxy : boost::noncopyable
{
private:
	boost::asio::io_service &m_Service;	///<  TheService.

	boost::asio::ip::tcp::socket m_Server;	///< Socket connected with server.
	boost::asio::ip::tcp::socket m_Client;	///< Socket connected with client.

	boost::system::error_code m_LastError;				///< Last system error.
	boost::system::error_code m_LastServerRecvError;	///< Last system error on recv thread.
	boost::system::error_code m_LastClientRecvError;	///< Last system error on send thread.

	std::vector<uint8_t> m_vServerData;	///< Data read from server.
	std::vector<uint8_t> m_vClientData; ///< Data read from client.

	boost::asio::ip::tcp::acceptor m_Acceptor; ///< One time acceptor for client connection.

	boost::thread* m_ServerRecvThread;
	boost::thread* m_ClientRecvThread;

	std::string m_strServerIp;		///< Ip of server.
	std::string m_strServerPort;	///< Port of server.

	boost::atomics::atomic_bool m_bShutdown;

	CryptManager	m_CryptManager;	///< Crypto class.
	CClientManager	m_ClientManager; ///< Holds data that the client also does.

public:
	/*
	*	szPort = port to listen on for inc. client connections.
	*/
	CProxy(boost::asio::io_service &service, const char * szPort);

	/*
	*	Wait for client connection. Will connect to server on new connection.
	*/
	void AsyncWaitForClientConnection(const char * szServer, const char * szServerPort);

	/*
	*	Shutdown everything (On delete).
	*/
	void Shutdown();

private:
	~CProxy();

	/*
	*	Client accept callback.
	*/
	void ConnectToServer(const boost::system::error_code &ec);

	/*
	*	After connection with server established.
	*/
	void WaitForHandshake();

	/*
	*	Sends a packet to the server.
	*/
	void SERVER_SendPacket(uint8_t *pData, uint32_t size);

	/*
	*	Sends a packet to the client.
	*/
	void CLIENT_SendPacket(uint8_t *pData, uint32_t size);

	/*
	*	Handles incomming server packets and sends them to the client.
	*/
	void ParseServerPacket(uint8_t *pData, uint32_t size);

	/*
	*	Handles incomming client packets and sends them to the server.
	*/
	void ParseClientPacket(uint8_t *pData, uint32_t size);

	/*
	*	Returns true if any error happened on server recv thread.
	*/
	inline bool SERVER_CheckRecvError(bool bShutdown = true);

	/*
	*	Returns true if any error happened on client recv thread.
	*/
	inline bool CLIENT_CheckRecvError(bool bShutdown = true);

	/*
	*	Loop for client recv.
	*/
	void CLIENT_ThreadRecv();

	/*
	*	Loop for server recv.
	*/
	void SERVER_ThreadRecv();

	/*
	*	Start new server recv thread.
	*/
	void SERVER_StartRecv();

	/*
	*	Start new client recv thread.
	*/
	void CLIENT_StartRecv();

	/*
	*	Called after new data was read from server. (ASYNC)
	*/
	void SERVER_CallbackRead(const boost::system::error_code &ec, uint32_t trans);

	/*
	*	Start server read. (ASYNC)
	*/
	void SERVER_Read();

	/*
	*	Called after new data was read from client. (ASYNC)
	*/
	void CLIENT_CallbackRead(const boost::system::error_code &ec, uint32_t trans);

	/*
	*	Start client read. (ASYNC)
	*/
	void CLIENT_Read();
};

#endif