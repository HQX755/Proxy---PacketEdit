/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_SPAWN_USER_PROJECTILE__H_
#define __PACKET_SPAWN_USER_PROJECTILE__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SSpawnUserProjectilePacket : public CPacketData
	{
		IMPLEMENT_PACKET(SSpawnUserProjectilePacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t owner_id;
			uint32_t owner_type;
			uint32_t owner_model;
			uint32_t owner_party;
			uint32_t id;
			uint32_t type;
			uint32_t skillId;
			float srcX;
			float srcY;
			float srcZ;
			float dstX;
			float dstY;
			float dstZ;
			uint16_t unk;
			uint16_t spd;
		};

		SPacket& packet;
	} TSSpawnUserProjectilePacket;
}

#pragma pack(pop)

#endif
