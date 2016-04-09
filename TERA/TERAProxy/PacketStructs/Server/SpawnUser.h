/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKETS_SPAWN_USER__H_
#define __PACKETS_SPAWN_USER__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SSpawnPlayerPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SSpawnPlayerPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint8_t unk[14];
			float x;
			float y;
			float z;
			uint16_t dir;
			uint8_t unk_1[5];
			uint32_t id;
			uint32_t type;
			uint8_t unk_2[15];
			uint8_t state;
			uint8_t unk_3[15];
			uint8_t dead;
		};

		SPacket &packet;
	} TSSpawnPlayerPacket;
}

#pragma pack(pop)

#endif