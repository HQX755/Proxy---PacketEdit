/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_DESPAWN_NPC__H_
#define __PACKET_DESPAWN_NPC__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SDespawnNPCPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SDespawnNPCPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t type;
		};

		SPacket &packet;
	} TSDespawnNPCPacket;
}

#pragma pack(pop)

#endif