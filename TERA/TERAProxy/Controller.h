#ifndef __CONTROLLER__H_
#define __CONTROLLER__H_

#include <cstdint>

#define TO_FLOAT_DIR(x) static_cast<float>		(x / 180)
#define TO_U16_DIR(x)	static_cast<uint16_t>	(x * 180)

struct OBJECT
{
	uint32_t id;
	uint32_t type;
	float x;
	float y;
	float z;
	float dir;
	int32_t hp;
	int32_t mp;
	bool dead;
};

struct PLAYER
{
	const char *name;
	const char *guild;
	unsigned char state;
};

class CController
{
protected:
	OBJECT m_Object;

public:
	CController();
	~CController();

	void Move(float x, float y, float z, float dir);

	inline void SetX(float x) 
	{
		m_Object.x = x;
	}

	inline void SetY(float y) 
	{
		m_Object.y = y;
	}

	inline void SetZ(float z) 
	{
		m_Object.z = z;
	}

	inline void SetDir(float dir) 
	{
		m_Object.dir = dir;
	}

	inline void SetId(uint32_t id) 
	{
		m_Object.id = id;
	}

	inline void SetType(uint32_t type) 
	{
		m_Object.type = type;
	}

	inline void SetHp(int32_t hp)
	{
		m_Object.hp = hp;

		if (hp == 0)
		{
			m_Object.dead = true;
		}
	}

	inline void SetMp(int32_t mp)
	{
		m_Object.mp = mp;
	}

	inline void SetDead(bool bDead)
	{
		m_Object.dead = bDead;
	}

public:
	inline const OBJECT& GetObjectRef() const
	{
		return m_Object;
	}
};

class CNpc : public CController
{
public:
	CNpc();
	CNpc(uint8_t *pData, uint32_t size);
	~CNpc();
};

class CPlayer : public CController
{
private:
	PLAYER m_Player;

public:
	CPlayer();
	CPlayer(uint8_t *pData, uint32_t size);
	~CPlayer();

	inline const char* GetPlayerName()
	{
		return m_Player.name;
	}

	inline const char* GetPlayerGuild()
	{
		return m_Player.guild;
	}

	inline void SetPlayerName(const char *szName)
	{
		m_Player.name = szName;
	}

	inline void SetPlayerGuild(const char *szGuild)
	{
		m_Player.guild = szGuild;
	}

	inline bool IsEnemy()
	{
		return m_Player.state >= 8;
	}
};

class CProjectile : public CController
{
private:
	CController * m_Owner;
	CController * m_Target;

	float m_fDestX;
	float m_fDestY;
	float m_fDestZ;

public:
	CProjectile(CController * pOwner)
	{
		m_Owner = pOwner;
	}

	inline CController *GetOwner()
	{
		return m_Owner;
	}

	inline CController *GetTarget()
	{
		return m_Target;
	}

	inline float GetDestinationX()
	{
		return m_fDestX;
	}

	inline float GetDestinationY()
	{
		return m_fDestY;
	}

	inline float GetDestinationZ()
	{
		return m_fDestZ;
	}

	inline void SetDestinationX(float x)
	{
		m_fDestX = x;
	}

	inline void SetDestinationY(float y)
	{
		m_fDestY = y;
	}

	inline void SetDestinationZ(float z)
	{
		m_fDestZ = z;
	}

	inline void SetTarget(CController * pTarget)
	{
		m_Target = pTarget;
	}

	inline void SetOwner(CController * pOwner)
	{
		m_Owner = pOwner;
	}
};

#endif