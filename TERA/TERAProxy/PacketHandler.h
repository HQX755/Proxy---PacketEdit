#ifndef __PACKET_HANDLER__H_
#define __PACKET_HANDLER__H_

#pragma once

class CProxy;

/*
*	Returns amount of times to send packet.
*/
#define PACKET_HANDLER(x) virtual inline int32_t x(CProxy *proxy, uint8_t* data, uint32_t size, uint8_t** result, uint32_t* result_size)

class CPacketHandler
{
public:
	PACKET_HANDLER(OnRecvUserSpawn)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvUserDespawn)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvUserLocation)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvNPCSpawn)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvNPCDespawn)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvNPCLocation)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvSpawnMe)
	{
		return 1;
	}

	PACKET_HANDLER(OnSendUserProjectile)
	{
		return 1;
	}

	PACKET_HANDLER(OnSendHitProjectile)
	{
		return 1;
	}

	PACKET_HANDLER(OnSendUserLocation)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvEachSkillResult)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvBossGageInfo)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvBossGageStackInfo)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvHitCombo)
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvCreatureHP)
	{
		return 1;
	}

	PACKET_HANDLER(OnSendChat)
	{
		return 1;
	}
};

#endif