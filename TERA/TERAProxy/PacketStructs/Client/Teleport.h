/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_TELEPORT__H_
#define __PACKET_TELEPORT__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct STeleportPacket : public CPacketData
	{
		IMPLEMENT_PACKET(STeleportPacket, packet)
		{
			uint16_t size	= sizeof(SPacket);
			uint16_t header = I_TELEPORT;
			float x;
			float y;
			float z;
			uint16_t dir;
			//TODO
		};

		SPacket &packet;
	} TCTeleportPacket;
}

#pragma pack(pop)

#endif
