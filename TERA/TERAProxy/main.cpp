#include "stdafx.h"
#include "Emulator.h"
#include "Proxy.h"

int main(int argc, char **argv)
{
	boost::asio::io_service service_0;

	CProxy *proxy = new CProxy(service_0, "7801");
	proxy->AsyncWaitForClientConnection("79.110.94.211", "7801");

	while (true)
	{
		service_0.run();
	}
}

