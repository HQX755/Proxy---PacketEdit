#include "stdafx.h"

#include "Emulator.h"

#include "Proxy.h"
#include "ProxyManager.h"

CProxy* proxy;

int main(int argc, char **argv)
{
	boost::asio::io_service service;

	g_pProxyManager = new CProxyManager(service);
	g_pProxyManager->StartAccept();

	while (true)
	{
		service.run();
	}
}

