/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKETS_SPAWN_NPC__H_
#define __PACKETS_SPAWN_NPC__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SSpawnNPCPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SSpawnNPCPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t unk_0;
			uint16_t party;
			uint32_t id;
			uint32_t type;
			uint32_t unk_1;
			uint32_t unk_2;
			float x;
			float y;
			float z;
			uint16_t dir;
			uint32_t unk_3;
			uint32_t npc_id;
			uint16_t type_id;
		};

		SPacket& packet;
	} TSSpawnNPCPacket;
}

#pragma pack(pop)

#endif