#include "stdafx.h"
#include "ClientData.h"

CClientManager::CClientManager()
{
}

CClientManager::~CClientManager()
{
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
	Packets::TSDespawnNPCPacket::SPacket p = *(Packets::TSDespawnNPCPacket::SPacket*)pData;
	{
		auto it = std::find_if(m_vNpcList.begin(), m_vNpcList.end(), [&](const CNpc *pNpc)
		{
			return pNpc->GetObjectRef().id == p.npc_id;
		});

		if (it != m_vNpcList.end())
		{
			delete *it;
			m_vNpcList.erase(it);
		}
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
}

void CClientManager::RemovePC(uint8_t * pData, uint32_t size)
{
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

void CClientManager::AddProjectile(uint8_t * pData, uint32_t size)
{
}

void CClientManager::RemoveProjectile(uint8_t * pData, uint32_t size)
{
}

void CClientManager::SpawnMainController(uint8_t * pData, uint32_t size)
{
	Packets::SSpawnMePacket::SPacket p = *(Packets::SSpawnMePacket::SPacket*)pData;
	{
		if (m_Player)
		{
			delete m_Player;
		}

		m_Player = new CPlayer();
		{
			m_Player->SetId(p.id);
			m_Player->SetType(p.typeId);
		}
	}
}

void CClientManager::OnMainControllerMove(uint8_t * pData, uint32_t size)
{
	if (m_Player)
	{
		Packets::TSMovePacket::SPacket p = *(Packets::TSMovePacket::SPacket*)pData;
		{
			m_Player->Move(p.x, p.y, p.z, p.dir);
		}
	}
}

void CClientManager::OnPCMove(uint8_t * pData, uint32_t size)
{
}

void CClientManager::OnNPCMove(uint8_t * pData, uint32_t size)
{
}

void CClientManager::MoveTo(float x, float y, float z, float dir)
{
	if (m_Player)
	{
		m_Player->Move(x, y, z, dir);
	}
}
