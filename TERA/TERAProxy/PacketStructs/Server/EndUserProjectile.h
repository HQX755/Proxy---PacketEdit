/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_END_USER_PROJECTILE__H_
#define __PACKET_END_USER_PROJECTILE__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SEndUserProjectile : public CPacketData
	{
		IMPLEMENT_PACKET(SEndUserProjectile, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t type;
			uint32_t owner_id;
			uint32_t owner_type;
		};

		SPacket& packet;
	} TSEndUserProjectile;
}

#pragma pack(pop)

#endif
