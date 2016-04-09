/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKETS_SPAWN_ME__H_
#define __PACKETS_SPAWN_ME__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SSpawnMePacket : public CPacketData
	{
		IMPLEMENT_PACKET(SSpawnMePacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t typeId;
			float x;
			float y;
			float z;
			uint16_t dir;
			uint16_t state;
		};

		SPacket &packet;
	} TSSpawnMePacket;
}

#pragma pack(pop)

#endif
