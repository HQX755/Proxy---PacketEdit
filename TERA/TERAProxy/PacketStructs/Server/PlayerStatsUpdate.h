/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_PLAYER_STATS_UPDATE__H_
#define __PACKET_PLAYER_STATS_UPDATE__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	typedef struct SPlayerStatsUpdatePacket : public CPacketData
	{
		IMPLEMENT_PACKET(SPlayerStatsUpdatePacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint8_t unk[12];
			int32_t hp;
			int32_t mp;
			int32_t power_factor_base;
			int32_t defense_factor_base;
			int32_t impact_factor_base;
			int32_t balance_factor_factor_base;
			int16_t attack_speed_base;
			int16_t run_speed_base;
			int16_t speed_mod;
			float crit_rate_base;
			float crit_resistance_base;
			float crit_factor_base;
			int32_t base_attack_min;
			int32_t base_attack_max;
			int32_t base_defense;
			int32_t base_impact;
			int32_t base_balance;
			float weak_resist;
			float damage_resist;
			float stun_resist;

			int32_t power_factor_add;
			int32_t defense_factor_add;
			int32_t impact_factor_add;
			int32_t balance_factor_add;
			int32_t run_speed_add;
			int32_t attack_speed_add;
			float crit_rate_add;
			float crit_resistance_add;
			float crit_factor_add;

			int32_t attack_min_add;
			int32_t attack_max_add;
			int32_t defense_add;
			int16_t impact_add;
			int16_t balance_add;

			float weak_resist_add;
			float damage_resist_add;
			float stun_resist_add;

			int32_t run_speed_combat;
			
			int16_t level;
			int16_t stance;
			int16_t unk_1;
			int8_t unk_2;
			int32_t hp_add;
			int32_t mp_add;
			int32_t stamina;
			int32_t stamina_max;
			int32_t unk_3;
			int32_t unk_4;
			int32_t unk_5;
			int32_t il_min;
			int32_t il_max;
			int64_t unk_6;
			int32_t unk_7;
			int32_t unk_8;
		};

		SPacket &packet;
	} TSPlayerStatsUpdatePacket;
}

#pragma pack(pop)

#endif
