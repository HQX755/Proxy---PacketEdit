#include "Commands.h"
#include "stdafx.h"

#include "Proxy.h"

void CCommandManager::Teleport(float x, float y, float z, float dir)
{
	if (!m_Proxy)
	{
		return;
	}

	m_Proxy->LockClientRecv();
	{
		Packets::TCTeleportPacket packet;

		packet.packet.header = I_TELEPORT;
		packet.packet.size = packet.GetContainerSize();
		packet.packet.x = m_Proxy->GetClient()->GetMainController()->GetObjectRef().x + x;
		packet.packet.y = m_Proxy->GetClient()->GetMainController()->GetObjectRef().y + y;
		packet.packet.z = m_Proxy->GetClient()->GetMainController()->GetObjectRef().z + z;
		packet.packet.dir = TO_U16_DIR(m_Proxy->GetClient()->GetMainController()->GetObjectRef().dir + dir);

		m_Proxy->CLIENT_SendPacket(packet.GetContainerData(), packet.GetContainerSize(), 1);
	}
	m_Proxy->UnlockClientRecv();
}

void CCommandManager::TeleportTo(float x, float y, float z, float dir)
{
	if (!m_Proxy)
	{
		return;
	}

	m_Proxy->LockClientRecv();
	{
		Packets::TCTeleportPacket packet;

		packet.packet.header = I_TELEPORT;
		packet.packet.size = packet.GetContainerSize();
		packet.packet.x = x;
		packet.packet.y = y;
		packet.packet.z = z;
		packet.packet.dir = TO_U16_DIR(dir);

		m_Proxy->CLIENT_SendPacket(packet.GetContainerData(), packet.GetContainerSize(), 1);
	}
	m_Proxy->UnlockClientRecv();
}
