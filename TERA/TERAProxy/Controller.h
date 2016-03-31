#ifndef __CONTROLLER__H_
#define __CONTROLLER__H_

#include <cstdint>

struct OBJECT
{
	uint32_t id;
	uint32_t type;
	float x;
	float y;
	float z;
	float dir;
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

public:
	CProjectile(CController * pOwner)
	{
		m_Owner = pOwner;
	}

	inline CController *GetOwner()
	{
		return m_Owner;
	}
};

#endif