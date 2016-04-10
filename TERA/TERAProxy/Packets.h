/*/
/*												
/*		TODO: Expand the packet structures.
/*												
/*/

#ifndef __PACKETS__H_
#define __PACKETS__H_

#pragma pack(push)
#pragma pack(1)

#include "PacketStructs\Server\CreatureChangeHP.h"
#include "PacketStructs\Server\DespawnNPC.h"
#include "PacketStructs\Server\DespawnProjectile.h"
#include "PacketStructs\Server\DespawnUser.h"
#include "PacketStructs\Server\PlayerStatsUpdate.h"
#include "PacketStructs\Server\SpawnMe.h"
#include "PacketStructs\Server\SpawnNPC.h"
#include "PacketStructs\Server\SpawnUser.h"
#include "PacketStructs\Server\SpawnUserProjectile.h"
#include "PacketStructs\Server\NPCLocation.h"
#include "PacketStructs\Server\EndUserProjectile.h"
#include "PacketStructs\Client\Teleport.h"

#include "PacketStructs\Client\CanLockOnTarget.h"
#include "PacketStructs\Client\Move.h"
#include "PacketStructs\Client\StartProjectile.h"
#include "PacketStructs\Client\StartSkill.h"
#include "PacketStructs\Client\Chat.h"

#define PACKET(x, y, z) x z = *reinterpret_cast<x*>(y)
#define PACKET_REF(x, y, z) x& z = *reinterpret_cast<x*>(y)

#pragma pack(pop)

#endif