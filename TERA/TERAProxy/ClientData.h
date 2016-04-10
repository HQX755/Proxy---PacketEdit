#ifndef __CLIENT_DATA__H_
#define __CLIENT_DATA__H_

#pragma once

#include "stdafx.h"
#include "Controller.h"
#include "Packets.h"

#include <vector>
#include <boost/function.hpp>

template<typename T>
static inline bool __TRUE(T*t)
{
	return true;
}

class CClientManager
{
private:
	std::vector<CNpc*>			m_vNpcList;
	std::vector<CPlayer*>		m_vPlayerList;
	std::vector<CProjectile*>	m_vProjectileList;

	CPlayer * m_Player; ///< Me

public:
	inline CPlayer* GetMainController() {
		return m_Player;
	}

public:
	CClientManager();
	~CClientManager();

	void ClearNPCList();
	void ClearPlayerList();
	void ClearProjectileList();

	void AddNPC(uint8_t *pData, uint32_t size);
	void RemoveNPC(uint8_t *pData, uint32_t size);

	CNpc* GetNpcById(uint32_t id);

	void AddPC(uint8_t *pData, uint32_t size);
	void RemovePC(uint8_t *pData, uint32_t size);

	CPlayer* GetPlayerById(uint32_t id);
	CPlayer* GetPlayerByName(const char* szName);

	void AddUserProjectile(uint8_t *pData, uint32_t size);
	void RemoveUserProjectile(uint8_t *pData, uint32_t size);
	void RemoveProjectile(uint8_t *pData, uint32_t size);

	CProjectile* GetProjectileById(uint32_t id);
	std::vector<CProjectile*> GetProjectilesByOwner(uint32_t ownerId);

	void OnMainControllerMove(uint8_t *pData, uint32_t size);
	void OnMainControllerSpawn(uint8_t *pData, uint32_t size);
	void OnPlayerStatsUpdate(uint8_t *pData, uint32_t size);

	void OnPCMove(uint8_t *pData, uint32_t size);
	void OnNPCMove(uint8_t *pData, uint32_t size);
	void OnNPCRotate(uint8_t *pData, uint32_t size);
	void OnNPCChangeHP(uint8_t *pData, uint32_t size);

	void MoveTo(float x, float y, float z, float dir);

	template<class Pred = bool(CPlayer*), class... Args>
	std::vector<CPlayer*> GetAnyPlayers(Pred& pred, Args&&... args)
	{
		std::vector<CPlayer*> v;
		
		for (auto it = m_vPlayerList.begin(); it != m_vPlayerList.end(); ++it)
		{
			if (pred(args..., *it))
			{
				v.push_back(*it);
			}
		}

		return v;
	}

	template<class Pred, class... Args>
	std::vector<CNpc*> GetAnyNpc(Pred& pred, Args&&... args)
	{
		std::vector<CNpc*> v;

		for (auto it = m_vNpcList.begin(); it != m_vNpcList.end(); ++it)
		{
			if (pred(args..., *it))
			{
				v.push_back(*it);
			}
		}

		return v;
	}

	template<class Pred, class... Args>
	std::vector<CProjectile*> GetAnyProjectiles(Pred& pred, Args&&... args)
	{
		std::vector<CProjectile*> v;

		for (auto it = m_vProjectileList.begin(); it != m_vProjectileList.end(); ++it)
		{
			if (pred(args..., *it))
			{
				v.push_back(*it);
			}
		}

		return v;
	}
};

static inline bool In2DRange(float x, float y, float f2DRange, const CController* pController)
{
	float fDeltaX = pController->GetObjectRef().x - x;
	float fDeltaY = pController->GetObjectRef().y - y;

	if (sqrt(fDeltaX * fDeltaX + fDeltaY * fDeltaY) < f2DRange)
	{
		return true;
	}

	return false;
};

static inline bool In3DRange(float x, float y, float z, float f3DRange, const CController* pController)
{
	float fDeltaX = pController->GetObjectRef().x - x;
	float fDeltaY = pController->GetObjectRef().y - y;
	float fDeltaZ = pController->GetObjectRef().z - z;

	if (sqrt(fDeltaX * fDeltaX + fDeltaY * fDeltaY + fDeltaZ * fDeltaZ) < f3DRange)
	{
		return true;
	}

	return false;
};

static inline bool IsDead(const CController* pController)
{
	return pController->GetObjectRef().dead;
}

#endif