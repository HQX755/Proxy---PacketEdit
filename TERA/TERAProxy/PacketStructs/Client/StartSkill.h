/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_START_SKILL__H_
#define __PACKET_START_SKILL__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SStartSkillPacket : public CPacketData
	{
		IMPLEMENT_PACKET(SStartSkillPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint16_t skill_id;
			uint16_t skill_stage;
			float x;
			float y;
			float z;
			uint16_t dir;
			uint8_t unk[22];
		};

		SPacket &packet;
	} TCStartSkillPacket;
}

#pragma pack(pop)

#endif
