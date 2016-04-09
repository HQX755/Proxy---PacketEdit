/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_DESPAWN_USER__H_
#define __PACKET_DESPAWN_USER__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SDespawnPlayerPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SDespawnPlayerPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t type;
		};

		SPacket &packet;
	} TSDespawnPlayerPacket;
}

#pragma pack(pop)

#endif
