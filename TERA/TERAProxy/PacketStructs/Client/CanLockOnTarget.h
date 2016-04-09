/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_CAN_LOCK_ON_TARGET__H_
#define __PACKET_CAN_LOCK_ON_TARGET__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SCanLockTargetPacket : public CPacketData
	{
		struct SPacket
		{
			uint16_t size;
			uint16_t header;
			uint32_t target_id;
			uint32_t target_type;
			uint16_t skill_id;
			uint16_t skill_stage;
		};

		SPacket &packet;
	} TCCanLockTargetPacket;
}

#pragma pack(pop)

#endif
