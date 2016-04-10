#ifndef __TEST_HANDLER__H_
#define __TEST_HANDLER__H_

#include "Proxy.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

class CCustomPacketHandler : public CPacketHandler
{
private:
	int		m_iHitCount;
	int		m_iRunSpeed;
	float	m_fHitRange;

	bool m_bEnableHitCounter;
	bool m_bEnableBossGage;
	bool m_bEnableChangeHP;
	bool m_bEnableSkillResult;
	bool m_bEnableShowHP;
	bool m_bEnableActionStage;
	bool m_bEnableGhostMode;

public:
	CCustomPacketHandler()
	{
		m_iHitCount = 100;
		m_fHitRange = 1000.0f;
		m_iRunSpeed = 200;

		m_bEnableHitCounter = m_bEnableBossGage = m_bEnableChangeHP = m_bEnableSkillResult = m_bEnableShowHP = m_bEnableActionStage = m_bEnableGhostMode = false;
	}

	PACKET_HANDLER(OnSendUserProjectile) override
	{
		return 1;
	}

	PACKET_HANDLER(OnRecvHitCombo) override
	{
		return m_bEnableHitCounter ? 1 : 0;
	}

	PACKET_HANDLER(OnRecvBossGageInfo) override
	{
		return m_bEnableBossGage ? 1 : 0;
	}

	PACKET_HANDLER(OnRecvCreatureHP) override
	{
		return m_bEnableChangeHP ? 1 : 0;
	}

	PACKET_HANDLER(OnSendChat) override
	{
		using namespace Packets;

		PACKET(CChatPacket<128>::SPacket, data, packet);
		{
			std::wstring str(packet.text + wcslen(L"<FONT>"), packet.text + wcslen(packet.text) - wcslen(L"</FONT>"));
			std::vector<std::wstring> v;

			if (wcsncmp(str.c_str(), L"HITCOUNT", wcslen(L"HITCOUNT")) == 0)
			{
				boost::split(v, str, boost::is_any_of(L" "));

				if (v.size() > 1)
				{
					m_iHitCount = boost::lexical_cast<int>(v[1]);
				}

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"HITRANGE", wcslen(L"HITRANGE")) == 0)
			{
				boost::split(v, str, boost::is_any_of(L" "));

				if (v.size() > 1)
				{
					m_fHitRange = boost::lexical_cast<float>(v[1]);
				}

				return 0;
			}
			if (wcsncmp(str.c_str(), L"RUNSPEED", wcslen(L"RUNSPEED")) == 0)
			{
				boost::split(v, str, boost::is_any_of(L" "));

				if (v.size() > 1)
				{
					m_iRunSpeed = boost::lexical_cast<int>(v[1]);
				}

				return 0;
			}
			if (wcsncmp(str.c_str(), L"TELEPORT", wcslen(L"TELEPORT")) == 0)
			{
				boost::split(v, str, boost::is_any_of(L" "));

				if (v.size() > 3)
				{
					float x = boost::lexical_cast<float>(v[1]);
					float y = boost::lexical_cast<float>(v[2]);
					float z = boost::lexical_cast<float>(v[3]);
					proxy->GetCommandManager()->Teleport(x, y, z, 0);
				}

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"HITCOUNTER", wcslen(L"HITCOUNTER")) == 0)
			{
				m_bEnableHitCounter = m_bEnableHitCounter ? false : true;

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"BOSSGAGE", wcslen(L"BOSSGAGE")) == 0)
			{
				m_bEnableBossGage = m_bEnableBossGage ? false : true;

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"CHANGEHP", wcslen(L"CHANGEHP")) == 0)
			{
				m_bEnableChangeHP = m_bEnableChangeHP ? false : true;

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"SKILLRESULT", wcslen(L"SKILLRESULT")) == 0)
			{
				m_bEnableSkillResult = m_bEnableSkillResult ? false : true;

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"SHOWHP", wcslen(L"SHOWHP")) == 0)
			{
				m_bEnableShowHP = m_bEnableShowHP ? false : true;

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"ACTION", wcslen(L"ACTION")) == 0)
			{
				m_bEnableActionStage = m_bEnableActionStage ? false : true;

				return 0;
			}
			else if (wcsncmp(str.c_str(), L"GHOSTMODE", wcslen(L"GHOSTMODE")) == 0)
			{
				m_bEnableGhostMode = m_bEnableGhostMode ? false : true;

				return 0;
			}
		}

		return 1;
	}

	PACKET_HANDLER(OnRecvPlayerStatsUpdate) override
	{
		using namespace Packets;

		PACKET_REF(TSPlayerStatsUpdatePacket::SPacket, data, packet);

		packet.run_speed_base	= 120;
		packet.run_speed_add	= m_iRunSpeed;

		return 1;
	}

	PACKET_HANDLER(OnSendPlayerLocation) override
	{
		if (m_bEnableGhostMode)
		{
			_DWORD(data, 32) = 1;
		}

		return 1;
	}

	PACKET_HANDLER(OnRecvActionStage) override
	{
		return m_bEnableActionStage ? 1 : 0;
	}

	PACKET_HANDLER(OnRecvActionStageEnd) override
	{
		return m_bEnableActionStage ? 1 : 0;
	}

	PACKET_HANDLER(OnRecvShowHP) override
	{
		return m_bEnableShowHP ? 1 : 0;
	}

	PACKET_HANDLER(OnSendHitProjectile) override
	{
		using namespace Packets;

		PACKET(TCProjectilePacket::SPacket, data, packet);

		proxy->Lock();

		CProjectile *pProjectile = proxy->GetClient()->GetProjectileById(packet.proj_id);

		if (!pProjectile)
		{
			proxy->Unlock();
			return 0;
		}

		if (pProjectile->GetOwner() != proxy->GetClient()->GetMainController())
		{
			proxy->Unlock();
			return 0;
		}

		if (packet.target_count > 0)
		{
			PACKET(TCTargetPacket::SPacket, data + sizeof(TCProjectilePacket::SPacket), target);

			TCProjectilePacket projectile(MAX_USER_PROJECTILE_HIT_COUNT);

			projectile.packet.proj_id = packet.proj_id;
			projectile.packet.proj_type = packet.proj_type;
			projectile.packet.x = packet.x;
			projectile.packet.y = packet.y;
			projectile.packet.z = packet.z;

			for (size_t i = 0; i < MAX_USER_PROJECTILE_HIT_COUNT; ++i)
			{
				projectile += TCTargetPacket(target.target_id, target.target_type, projectile, i == MAX_USER_PROJECTILE_HIT_COUNT - 1).packet;
			}

			*result = CopyHeap(projectile.GetContainerData(), projectile.GetContainerSize());
			*result_size = projectile.packet.size;
		}
		else
		{
			TCProjectilePacket projectile(MAX_USER_PROJECTILE_HIT_COUNT);

			projectile.packet.proj_id = packet.proj_id;
			projectile.packet.proj_type = packet.proj_type;
			projectile.packet.x = packet.x;
			projectile.packet.y = packet.y;
			projectile.packet.z = packet.z;

			std::vector<CNpc*> vList = proxy->GetClient()->GetAnyNpc(In2DRange,
				proxy->GetClient()->GetMainController()->GetObjectRef().x, proxy->GetClient()->GetMainController()->GetObjectRef().y, m_fHitRange);

			std::remove_if(vList.begin(), vList.end(), [](CNpc* pNpc)
			{
				return pNpc->GetObjectRef().dead;
			});

			if (vList.empty())
			{
				proxy->Unlock();
				return 0;
			}

			size_t index = 0;

			for (size_t i = 0; i < MAX_USER_PROJECTILE_HIT_COUNT; ++i, index = index + 1 >= vList.size() ? 0 : index + 1)
			{
				projectile += TCTargetPacket(vList[index]->GetObjectRef().id, vList[index]->GetObjectRef().type, projectile, i == MAX_USER_PROJECTILE_HIT_COUNT - 1).packet;
			}

			*result = CopyHeap(projectile.GetContainerData(), projectile.GetContainerSize());
			*result_size = projectile.packet.size;
		}

		proxy->Unlock();

		return m_iHitCount;
	}

	PACKET_HANDLER(OnRecvEachSkillResult) override
	{
		return m_bEnableSkillResult ? 1 : 0;
	}
};


#endif