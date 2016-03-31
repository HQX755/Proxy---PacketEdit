#include "stdafx.h"
#include "Controller.h"

#include "Packets.h"

CController::CController()
{

}

CController::~CController()
{

}

void CController::Move(float x, float y, float z, float dir)
{
	m_Object.x		= x;
	m_Object.y		= y;
	m_Object.z		= z;
	m_Object.dir	= dir;
}

CNpc::CNpc()
{
}

CNpc::CNpc(uint8_t *pData, uint32_t size)
{

}

CNpc::~CNpc()
{

}

CPlayer::CPlayer()
{
}

CPlayer::CPlayer(uint8_t *pData, uint32_t size)
{
	Packets::SSpawnPlayerPacket p(pData, size);

	m_Object.dir	= p.packet.dir / 180;
	m_Object.id		= p.packet.id;
	m_Object.type	= p.packet.type;
	m_Object.x		= p.packet.x;
	m_Object.y		= p.packet.y;
	m_Object.z		= p.packet.z;
	
	m_Player.state = p.packet.state;
}

CPlayer::~CPlayer()
{

}

