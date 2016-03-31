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
			packet.target_id	= target;
			packet.target_type	= type;
			packet.size_now		= static_cast<uint16_t>(header.GetContainerSize());
			packet.size_next	= last ? 0 : packet.size_now + sizeof(STargetPacket);
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
			packet.size				= sizeof(STargetPacket) * targetCount + sizeof(SPacket);
			packet.packet_size		= sizeof(SPacket);
			packet.header			= C_HIT_USER_PROJECTILE;
			packet.proj_type		= projType;
			packet.proj_id			= projId;
			packet.target_count		= targetCount;
			packet.x				= fx;
			packet.y				= fy;
			packet.z				= fz;
			packet.unk				= 0;
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

	typedef struct SSpawnPlayerPacket : public CPacketData
	{
		SSpawnPlayerPacket(

			uint8_t *pData,
			uint32_t size) :

			CPacketData(pData, size),
			packet(*this->GetContainerData<SPacket*>())
		{
		}

		struct SPacket
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
		};

		SPacket &packet;
	} TSSpawnPlayerPacket;

	typedef struct SMovePacket : public CPacketData
	{
		SMovePacket() :
			CPacketData(sizeof(SPacket)),
			packet(*this->GetContainerData<SPacket*>())
		{
		}

		struct SPacket
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			float x;
			float y;
			float z;
			uint16_t dir;
		};

		SPacket &packet;
	} TSMovePacket;

	typedef struct SSpawnMePacket : public CPacketData
	{
		SSpawnMePacket(
			
			uint8_t *pData,
			uint32_t size) :

			CPacketData(pData, size),
			packet(*GetContainerData<SPacket*>())
		{
		}

		struct SPacket
		{
			uint16_t size;
			uint16_t header;
			uint32_t id;
			uint32_t typeId;
		};

		SPacket &packet;
	} TSSpawnMePacket;

	typedef struct SPlayerStatsUpdatePacket : public CPacketData
	{
		SPlayerStatsUpdatePacket(
			
			uint8_t *pData, 
			uint32_t size) :

			CPacketData(pData, size),
			packet(*GetContainerData<SPacket*>())
		{
		}

		struct SPacket
		{
			uint16_t size;
			uint16_t header;
			uint8_t unk[102];
			uint16_t move_speed;
		};

		SPacket &packet;
	} TSPlayerStatsUpdatePacket;

	typedef struct CStartSkillPacket : public CPacketData
	{
		struct SPacket
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
	} TSCanLockTargetPacket;

	typedef struct SDespawnNPCPacket : public CPacketData
	{
		SDespawnNPCPacket(
			
			uint8_t *pData, 
			uint32_t size) :

			CPacketData(pData, size),
			packet(*GetContainerData<SPacket*>())
		{
		}

		struct SPacket
		{
			uint16_t size;
			uint16_t header;
			uint32_t npc_id;
			uint32_t npc_type;
		};

		SPacket &packet;
	} TSDespawnNPCPacket;
}

#pragma pack(pop)

#endif