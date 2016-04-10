#include "stdafx.h"

#include "Proxy.h"
#include "ProxyManager.h"
#include "CustomPacketHandler.h"

CProxyManager * g_pProxyManager = NULL;

CProxyManager::CProxyManager(boost::asio::io_service& service) :
	m_Service(service)
{

}

CProxyManager::~CProxyManager()
{
	Clear();
}

void CProxyManager::Clear()
{
	std::for_each(m_vProxyList.begin(), m_vProxyList.end(), [](CProxy* pProxy)
	{
		pProxy->Shutdown();
	});

	m_vProxyList.clear();
}

void CProxyManager::RemoveProxy(CProxy * pProxy)
{
	m_Service.post(m_Service.wrap([pProxy, this]()
	{
		std::remove_if(m_vProxyList.begin(), m_vProxyList.end(), [pProxy](CProxy* pProxy_)
		{
			return pProxy == pProxy_;
		});
	}));
}

void CProxyManager::AddProxy(CProxy * pProxy)
{
	m_Service.post(m_Service.wrap([pProxy, this]()
	{
		m_vProxyList.push_back(pProxy);
	}));
}

void CProxyManager::StartAccept()
{
	m_bAcceptProxy = true;
	{
		AcceptImpl();
	}
}

void CProxyManager::StopAccept()
{
	m_bAcceptProxy = false;
}

void CProxyManager::AcceptImpl()
{
	CProxy* pProxy					= new CProxy(m_Service, "7801");
	CCustomPacketHandler* pHandler	= new CCustomPacketHandler();

	boost::promise<void>* pCallback = new boost::promise<void>();
	
	pProxy->SetPacketHandler(pHandler);
	pProxy->AsyncWaitForClientConnection("79.110.94.211", "7801", pCallback);

	AddProxy(pProxy);

	boost::async([this, pCallback]
	{
		pCallback->get_future().wait();

		if (m_bAcceptProxy)
		{
			m_Service.post(boost::bind(&CProxyManager::AcceptImpl, this));
		}
	});
}


