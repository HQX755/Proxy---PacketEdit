/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_CREATURE_CHANGE_HP__H_
#define __PACKET_CREATURE_CHANGE_HP__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SCreatureHPPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SCreatureHPPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			int32_t hp;
			int32_t mp;
			uint32_t count;
			uint32_t type_change;
			uint32_t id;
			uint32_t type;
			uint32_t attacker_id;
			uint32_t attacker_type;
		};

		SPacket& packet;
	} TSCreatureHPPacket;
}

#pragma pack(pop)

#endif
