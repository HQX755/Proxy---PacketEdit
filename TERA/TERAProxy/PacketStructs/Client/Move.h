/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_MOVE__H_
#define __PACKET_MOVE__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SMovePacket : public CPacketData
	{
		IMPLEMENT_PACKET(SMovePacket, packet)
		{
			uint16_t size;
			uint16_t header;
			float x;
			float y;
			float z;
			uint16_t dir;
			uint32_t type;
			//TODO
		};

		SPacket &packet;
	} TCMovePacket;
}

#pragma pack(pop)

#endif
