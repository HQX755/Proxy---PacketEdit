#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <cstdint>

#include <sstream>
#include <fstream>
#include <iostream>
#include <ostream>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "SocketUtils.h"
#include "Utils.h"

class CEmulator
{
private:
	boost::asio::ip::tcp::socket m_Socket; ///< socket connected to launcher
	boost::asio::ip::tcp::socket m_Server; ///< socket connected to server list

	boost::asio::ip::tcp::acceptor m_Acceptor; ///< TheListener for inc. client connections.

	boost::system::error_code m_LastError; // Last error.

	boost::asio::io_service &m_Service; ///< TheService.

	std::vector<uint8_t> m_vInputData;	///< Data container for serverlist. 
	std::vector<uint8_t> m_vClientData; ///< Data container for client.

	std::string m_Ip;	///< Localhost anyways.
	std::string m_Port; ///< Port emulator is listening on.

	std::string m_SLIp;		///< Serverlist ip.
	std::string m_SLPort;	///< Serverlist port.

private:
	/*
	*	Connect to serverlist.
	*/
	bool ConnectToSL();

	/*
	*	Canceling async operations and closing all sockets.
	*/
	void CLIENT_OnClose();

	/*
	*	When a client connects then need to establish the connection to serverlist.
	*/
	bool CLIENT_OnConnect();

public:
	/*
	*	szIp		= localhost
	*	szPort		= listening port for launcher
	*	szIp_SL		= serverlist ip
	*	szPort_SL	= serverlist port
	*/
	CEmulator(boost::asio::io_service &Service, const char *szIp, const char* szPort, const char *szIp_SL, const char *szPort_SL);
	~CEmulator();

	/*
	*	Start emu.
	*/
	void RunEmulator();

	/*
	*	Returns socket data of client.
	*/
	boost::asio::ip::tcp::socket &GetSock();

private:
	/*
	*	Send to serverlist.
	*/
	void SERVER_Send(uint8_t *pData, uint32_t size);
	
	/*
	*	Send to client.
	*/
	void SOCKET_Send(uint8_t *pData, uint32_t size);

	/*
	*	Async callback for client.
	*/
	void HandleRead(const boost::system::error_code &ec, size_t trans);

	/*
	*	Async callback for server.
	*/
	void SL_HandleRead(const boost::system::error_code &ec, size_t trans);

	/*
	*	Async read data recvd by client.
	*/
	void CL_Read();

	/*
	*	Async read data recvd by serverlist.
	*/
	void SL_Read();

	/*
	*	Listen for inc. client connections.
	*/
	void Listen();

	/*
	*	Callback for inc. client connections.
	*/
	void HandleAccept(const boost::system::error_code &ec);
};

#include <boost/atomic.hpp>

class CServerList
{
private:
	// Service this should be run from.
	boost::asio::io_service& m_Service;

	// Our listener.
	boost::asio::ip::tcp::acceptor m_Acceptor;

	// Where the XML server list is stored.
	std::stringstream m_ServerList;

	// Currently running threads.
	boost::atomic<uint32_t> m_ActiveSockets;

	// Delete after threads finished.
	boost::atomic<bool> m_bDeleteLater;

public:
	CServerList(boost::asio::io_service & service, unsigned short port);

	/*
	*	Read serverlist.
	*/
	bool ReadXMLServerList(const char *szFile);

	/*
	*	Start accepting sockets and act like the official serverlist.
	*/
	void Run();

	/*
	*	Stops serverlist after all sockets closed.
	*/
	void Stop();

private:
	~CServerList();

	/*
	*	Handle incoming connections. Creates a new thread for each socket. (Lifetime: ~5s)
	*/
	void HandleAccept(const boost::system::error_code & ec, boost::asio::ip::tcp::socket * socket);
};

#endif