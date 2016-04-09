/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_DESPAWN_PROJECTILE__H_
#define __PACKET_DESPAWN_PROJECTILE__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SDespawnProjectilePacket : public CPacketData
	{
		IMPLEMENT_PACKET(SDespawnProjectilePacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t type;
			uint8_t remove;
		};

		SPacket& packet;
	} TSDespawnProjectilePacket;
}

#pragma pack(pop)

#endif
