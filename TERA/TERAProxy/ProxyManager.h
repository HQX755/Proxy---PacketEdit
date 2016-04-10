#ifndef __PROXY_MANAGER__H_
#define __PROXY_MANAGER__H_

#include <vector>
#include <boost/asio.hpp>

class CProxy;
class CProxyManager
{
private:
	// Container for proxy.
	std::vector<CProxy*> m_vProxyList;

	// TheService.
	boost::asio::io_service& m_Service;

	// Run acceptor or not.
	bool m_bAcceptProxy;

public:
	CProxyManager(boost::asio::io_service& service);
	~CProxyManager();

	/*
	*	Clear all proxies and shutdown.
	*/
	void Clear();

	/*
	*	Add a proxy to list.
	*/
	void AddProxy(CProxy* pProxy);

	/*
	*	Remove a proxy without shutdown.
	*/
	void RemoveProxy(CProxy* pProxy);

	/*
	*	Wait for clients.
	*/
	void StartAccept();

	/*
	*	Stop accepting clients.
	*/
	void StopAccept();

private:
	/*
	*	On accept.
	*/
	void AcceptImpl();
};

extern CProxyManager * g_pProxyManager;

#endif
