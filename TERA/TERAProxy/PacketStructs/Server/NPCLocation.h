/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_NPC_LOCATION__H_
#define __PACKET_NPC_LOCATION__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SNPCLocationPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SNPCLocationPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t type;
			float x;
			float y;
			float z;
			uint16_t dir;
			uint16_t spd;
			float targetX;
			float targetY;
			float targetZ;
			uint8_t move_type;
			uint32_t move_obj;
		};

		SPacket &packet;
	} TSNPCLocationPacket;
}

#pragma pack(pop)

#endif