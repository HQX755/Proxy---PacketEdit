#ifndef __CLIENT_DATA__H_
#define __CLIENT_DATA__H_

#pragma once

#include "stdafx.h"
#include "Controller.h"
#include "Packets.h"

#include <vector>

class CClientManager
{
private:
	std::vector<CNpc*>			m_vNpcList;
	std::vector<CPlayer*>		m_vPlayerList;
	std::vector<CProjectile*>	m_vProjectileList;

	CPlayer * m_Player; ///< Me

public:
	CClientManager();
	~CClientManager();

	void AddNPC(uint8_t *pData, uint32_t size);
	void RemoveNPC(uint8_t *pData, uint32_t size);

	CNpc* GetNpcById(uint32_t id);

	void AddPC(uint8_t *pData, uint32_t size);
	void RemovePC(uint8_t *pData, uint32_t size);

	CPlayer* GetPlayerById(uint32_t id);
	CPlayer* GetPlayerByName(const char* szName);

	void AddProjectile(uint8_t *pData, uint32_t size);
	void RemoveProjectile(uint8_t *pData, uint32_t size);

	void SpawnMainController(uint8_t *pData, uint32_t size);
	void OnMainControllerMove(uint8_t *pData, uint32_t size);
	void OnPCMove(uint8_t *pData, uint32_t size);
	void OnNPCMove(uint8_t *pData, uint32_t size);

	void MoveTo(float x, float y, float z, float dir);
};

#endif