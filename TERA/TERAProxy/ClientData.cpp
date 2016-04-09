#include "stdafx.h"
#include "ClientData.h"

CClientManager::CClientManager()
{
	m_Player = NULL;
}

CClientManager::~CClientManager()
{
	if (m_Player)
	{
		delete m_Player;
		m_Player = NULL;
	}

	ClearNPCList();
	ClearPlayerList();
	ClearProjectileList();
}

void CClientManager::ClearNPCList()
{
	std::for_each(m_vNpcList.begin(), m_vNpcList.end(), [](CNpc *pNpc)
	{
		delete pNpc;
	});
}

void CClientManager::ClearPlayerList()
{
	std::for_each(m_vPlayerList.begin(), m_vPlayerList.end(), [](CPlayer *pPlayer)
	{
		delete pPlayer;
	});
}

void CClientManager::ClearProjectileList()
{
	std::for_each(m_vProjectileList.begin(), m_vProjectileList.end(), [](CProjectile *pProjectile)
	{
		delete pProjectile;
	});
}

void CClientManager::AddNPC(uint8_t * pData, uint32_t size)
{
	CNpc *pNpc = new CNpc(pData, size);
	{
		m_vNpcList.push_back(pNpc);
	}
}

void CClientManager::RemoveNPC(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSDespawnNPCPacket::SPacket, pData, packet);

	auto it = std::find_if(m_vNpcList.begin(), m_vNpcList.end(), [&](CNpc *pNpc)
	{
		return pNpc->GetObjectRef().id == packet.id;
	});

	if (it != m_vNpcList.end())
	{
		delete *it; m_vNpcList.erase(it);
	}
}

CNpc * CClientManager::GetNpcById(uint32_t id)
{
	auto it = std::find_if(m_vNpcList.begin(), m_vNpcList.end(), [&](CNpc *pNpc)
	{
		return pNpc->GetObjectRef().id == id;
	});

	if (it != m_vNpcList.end())
	{
		return *it;
	}

	return NULL;
}

void CClientManager::AddPC(uint8_t * pData, uint32_t size)
{
	CPlayer * pPlayer = new CPlayer(pData, size);
	{
		m_vPlayerList.push_back(pPlayer);
	}
}

void CClientManager::RemovePC(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSDespawnPlayerPacket::SPacket, pData, packet);
	
	auto it = std::find_if(m_vPlayerList.begin(), m_vPlayerList.end(), [&](CPlayer *pPlayer)
	{
		return pPlayer->GetObjectRef().id == packet.id;
	});

	if (it != m_vPlayerList.end())
	{
		delete *it; m_vPlayerList.erase(it);
	}
}

CPlayer * CClientManager::GetPlayerById(uint32_t id)
{
	auto it = std::find_if(m_vPlayerList.begin(), m_vPlayerList.end(), [&](CPlayer *pPlayer)
	{
		return pPlayer->GetObjectRef().id == id;
	});

	if (it != m_vPlayerList.end())
	{
		return *it;
	}

	return NULL;
}

CPlayer * CClientManager::GetPlayerByName(const char * szName)
{
	auto it = std::find_if(m_vPlayerList.begin(), m_vPlayerList.end(), [&](CPlayer *pPlayer)
	{
		return strcmp(pPlayer->GetPlayerName(), szName) == 0;
	});

	if (it != m_vPlayerList.end())
	{
		return *it;
	}

	return NULL;
}

void CClientManager::AddUserProjectile(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSSpawnUserProjectilePacket::SPacket, pData, packet);
	{
		CPlayer *pPlayer = GetPlayerById(packet.owner_id);

		if (!pPlayer)
		{
			if (m_Player->GetObjectRef().id == packet.owner_id)
			{
				pPlayer = m_Player;
			}
		}

		if (pPlayer)
		{
			CProjectile *pProjectile = new CProjectile(pPlayer);

			pProjectile->SetX(packet.srcX);
			pProjectile->SetY(packet.srcY);
			pProjectile->SetZ(packet.srcZ);
			pProjectile->SetId(packet.id);
			pProjectile->SetType(packet.type);
			pProjectile->SetDestinationX(packet.dstX);
			pProjectile->SetDestinationY(packet.dstY);
			pProjectile->SetDestinationZ(packet.dstZ);

			m_vProjectileList.push_back(pProjectile);
		}
	}
}

void CClientManager::RemoveUserProjectile(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSEndUserProjectile::SPacket, pData, packet);

	auto it = std::find_if(m_vProjectileList.begin(), m_vProjectileList.end(), [&](CProjectile* pProjectile)
	{
		return pProjectile->GetObjectRef().id == packet.id;
	});

	if (it != m_vProjectileList.end())
	{
		delete *it; m_vProjectileList.erase(it);
	}
}

void CClientManager::RemoveProjectile(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSDespawnProjectilePacket::SPacket, pData, packet);
	
	auto it = std::find_if(m_vProjectileList.begin(), m_vProjectileList.end(), [&](CProjectile* pProjectile)
	{
		return pProjectile->GetObjectRef().id == packet.id;
	});

	if (it != m_vProjectileList.end())
	{
		delete *it; m_vProjectileList.erase(it);
	}
}

CProjectile * CClientManager::GetProjectileById(uint32_t id)
{
	auto it = std::find_if(m_vProjectileList.begin(), m_vProjectileList.end(), [&](CProjectile* pProjectile)
	{
		return pProjectile->GetObjectRef().id == id;
	});

	if (it != m_vProjectileList.end())
	{
		return *it;
	}

	return NULL;
}

std::vector<CProjectile*> CClientManager::GetProjectilesByOwner(uint32_t ownerId)
{
	std::vector<CProjectile*> v;

	std::for_each(m_vProjectileList.begin(), m_vProjectileList.end(), [&](CProjectile* pProjectile)
	{
		if (pProjectile->GetOwner())
		{
			if (pProjectile->GetOwner()->GetObjectRef().id == ownerId)
			{
				v.push_back(pProjectile);
			}
		}
	});

	return v;
}

void CClientManager::OnMainControllerSpawn(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSSpawnMePacket::SPacket, pData, packet);

	if (m_Player)
	{
		delete m_Player;
	}

	m_Player = new CPlayer();
	{
		m_Player->SetId(packet.id);
		m_Player->SetType(packet.typeId);
		m_Player->SetX(packet.x);
		m_Player->SetY(packet.y);
		m_Player->SetZ(packet.z);
		m_Player->SetDir(packet.dir);
		m_Player->SetDead(packet.state == 0);
	}
}

void CClientManager::OnMainControllerMove(uint8_t * pData, uint32_t size)
{
	if (m_Player)
	{
		PACKET(Packets::TCMovePacket::SPacket, pData, packet);
		{
			m_Player->Move(packet.x, packet.y, packet.z, packet.dir);
		}
	}
}

void CClientManager::OnPCMove(uint8_t * pData, uint32_t size)
{
}

void CClientManager::OnNPCMove(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSNPCLocationPacket::SPacket, pData, packet);
	{
		CNpc *pNpc = GetNpcById(packet.id);

		if (pNpc)
		{
			pNpc->Move(packet.x, packet.y, packet.z, packet.dir);
		}
	}
}

void CClientManager::OnNPCRotate(uint8_t * pData, uint32_t size)
{
}

void CClientManager::OnNPCChangeHP(uint8_t * pData, uint32_t size)
{
	PACKET(Packets::TSCreatureHPPacket::SPacket, pData, packet);
	{
		CNpc *pNpc = GetNpcById(packet.id);

		if (pNpc)
		{
			pNpc->SetHp(packet.hp);
			pNpc->SetMp(packet.mp);
		}
	}
}

void CClientManager::MoveTo(float x, float y, float z, float dir)
{
	if (m_Player)
	{
		m_Player->Move(x, y, z, dir);
	}
}
