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
	PACKET(Packets::SSpawnNPCPacket::SPacket, pData, packet);

	m_Object.dir	= TO_FLOAT_DIR(packet.dir);
	m_Object.id		= packet.id;
	m_Object.type	= packet.type;
	m_Object.x		= packet.x;
	m_Object.y		= packet.y;
	m_Object.z		= packet.z;
	m_Object.dead	= false;
}

CNpc::~CNpc()
{

}

CPlayer::CPlayer()
{
}

CPlayer::CPlayer(uint8_t *pData, uint32_t size)
{
	PACKET(Packets::SSpawnPlayerPacket::SPacket, pData, packet);

	m_Object.dir	= TO_FLOAT_DIR(packet.dir);
	m_Object.id		= packet.id;
	m_Object.type	= packet.type;
	m_Object.x		= packet.x;
	m_Object.y		= packet.y;
	m_Object.z		= packet.z;
	m_Object.dead	= packet.dead == 0 ? false : true;
	
	m_Player.state = packet.state;
}

CPlayer::~CPlayer()
{

}

