#include "stdafx.h"
#include "Emulator.h"
#include "Proxy.h"
#include "CustomPacketHandler.h"

CProxy* proxy;

int main(int argc, char **argv)
{
	boost::asio::io_service service_0;

	proxy = new CProxy(service_0, "7801");

	CCustomPacketHandler *pHandler = new CCustomPacketHandler();

	proxy->SetPacketHandler(pHandler);
	proxy->AsyncWaitForClientConnection("79.110.94.211", "7801");

	while (true)
	{
		service_0.run();
	}
}

