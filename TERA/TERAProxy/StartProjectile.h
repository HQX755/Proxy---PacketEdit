/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef _PACKETS_H_
#define _PACKETS_H_

#pragma pack(push)
#pragma pack(1)

#include "Opcodes.h"
#include "Container.h"

namespace Packets
{
	typedef struct STargetPacket
	{
		STargetPacket(uint32_t target, uint32_t type, CPacketData &header, bool last)
		{
			packet.target_id = target;
			packet.target_type = type;
			packet.size_now = static_cast<uint16_t>(header.GetContainerSize());
			packet.size_next = last ? 0 : packet.size_now + sizeof(STargetPacket);
		}

		STargetPacket() {
		}

		struct SPacket
		{
			uint16_t size_now;
			uint16_t size_next;
			uint32_t target_id;
			uint32_t target_type;
		};

		SPacket packet;
	} TCTargetPacket;

	typedef struct SProjectilePacket : public CPacketData
	{
		SProjectilePacket(

			uint16_t targetCount,
			uint32_t projType,
			uint32_t projId,
			float fx,
			float fy,
			float fz) :

			CPacketData(sizeof(SPacket), sizeof(STargetPacket) * targetCount + sizeof(SPacket) + 1),
			packet(*GetContainerData<SPacket*>())
		{
			packet.size = sizeof(STargetPacket) * targetCount + sizeof(SPacket);
			packet.packet_size = sizeof(SPacket);
			packet.header = C_HIT_USER_PROJECTILE;
			packet.proj_type = projType;
			packet.proj_id = projId;
			packet.target_count = targetCount;
			packet.x = fx;
			packet.y = fy;
			packet.z = fz;
			packet.unk = 0;
		}

		SProjectilePacket& operator=(const SProjectilePacket& other)
		{
			memcpy(&this->packet, &other.packet, sizeof(SPacket)); return *this;
		}

		struct SPacket
		{
			uint16_t size;
			uint16_t header;
			uint16_t target_count;
			uint16_t packet_size;
			uint32_t proj_id;
			uint32_t proj_type;
			uint8_t unk;
			float x;
			float y;
			float z;
		};

		SPacket &packet;
	} TCProjectilePacket;
}

#pragma pack(pop)

#endif
