#ifndef __CRYPT__H_
#define __CRYPT__H_

#pragma warning(disable: 4018)

#include "stdafx.h"

#include <Windows.h>
#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <algorithm>
#include <stdio.h>

#include <boost/atomic.hpp>
#include <boost/random.hpp>

#define arr_size(x) sizeof(x)/sizeof(*x)

class CSha
{
private:
	uint32_t Computed;
	uint32_t Corrupted;
	uint32_t LengthLow;
	uint32_t LengthHigh;
	uint32_t MsgIndex;
	uint32_t MessageBlock[64];
	uint32_t MessageDigest[5];

public:
	CSha() : Computed(0), Corrupted(0), LengthLow(0), MsgIndex(0), LengthHigh(0)
	{
		memset(&MessageBlock,  0, 64 * sizeof(uint32_t));
		memset(&MessageDigest, 0, 5  * sizeof(uint32_t));

		MessageDigest[0] = 0x67452301;
		MessageDigest[1] = 0xEFCDAB89;
		MessageDigest[2] = 0x98BADCFE;
		MessageDigest[3] = 0x10325476;
		MessageDigest[4] = 0xC3D2E1F0;
	};

	uint32_t CircularShift(uint32_t bits, uint32_t word)
	{
		return ((((word) << bits) & 0xFFFFFFFF) | ((word) >> (32 - bits)));
	}

	void ProcessMessageBlock()
	{
		uint32_t k[] =
		{
			0x5A827999,
			0x6ED9EBA1,
			0x8F1BBCDC,
			0xCA62C1D6
		};

		uint32_t t;
		uint32_t temp;
		uint32_t w[80];

		uint32_t a, b, c, d, e;

		for (t = 0; t < 16; t++)
		{
			w[t]  = ((uint32_t)MessageBlock[t * 4])		<< 24;
			w[t] |= ((uint32_t)MessageBlock[t * 4 + 1]) << 16;
			w[t] |= ((uint32_t)MessageBlock[t * 4 + 2]) << 8;
			w[t] |= MessageBlock[t * 4 + 3];
		}

		for (t = 16; t < 80; t++)
		{
			w[t] = w[t - 3] ^ w[t - 8] ^ w[t - 14] ^ w[t - 16];
		}

		a = MessageDigest[0];
		b = MessageDigest[1];
		c = MessageDigest[2];
		d = MessageDigest[3];
		e = MessageDigest[4];

		for (t = 0; t < 20; t++)
		{
			temp = CircularShift(5, a) + ((b & c) | ((~b) & d)) + e + w[t] + k[0];
			temp &= 0xFFFFFFFF;
			e = d;
			d = c;
			c = CircularShift(30, b);
			b = a;
			a = temp;
		}

		for (t = 20; t < 40; t++)
		{
			temp = CircularShift(5, a) + (b ^ c ^ d) + e + w[t] + k[1];
			temp &= 0xFFFFFFFF;
			e = d;
			d = c;
			c = CircularShift(30, b);
			b = a;
			a = temp;
		}

		for (t = 40; t < 60; t++)
		{
			temp = CircularShift(5, a) + ((b & c) | (b & d) | (c & d)) + e + w[t] + k[2];
			temp &= 0xFFFFFFFF;
			e = d;
			d = c;
			c = CircularShift(30, b);
			b = a;
			a = temp;
		}

		for (t = 60; t < 80; t++)
		{
			temp = CircularShift(5, a) + (b ^ c ^ d) + e + w[t] + k[3];
			temp &= 0xFFFFFFFF;
			e = d;
			d = c;
			c = CircularShift(30, b);
			b = a;
			a = temp;
		}

		MessageDigest[0] = (uint32_t)((MessageDigest[0] + a) & 0xFFFFFFFF);
		MessageDigest[1] = (uint32_t)((MessageDigest[1] + b) & 0xFFFFFFFF);
		MessageDigest[2] = (uint32_t)((MessageDigest[2] + c) & 0xFFFFFFFF);
		MessageDigest[3] = (uint32_t)((MessageDigest[3] + d) & 0xFFFFFFFF);
		MessageDigest[4] = (uint32_t)((MessageDigest[4] + e) & 0xFFFFFFFF);

		MsgIndex = 0;
	}

	void PadMessage()
	{
		if (MsgIndex > 55)
		{
			MessageBlock[MsgIndex++] = 0x80;
			while (MsgIndex < 64)
			{
				MessageBlock[MsgIndex++] = 0;
			}

			ProcessMessageBlock();

			while (MsgIndex < 56)
			{
				MessageBlock[MsgIndex++] = 0;
			}
		}
		else
		{
			MessageBlock[MsgIndex++] = 0x80;
			while (MsgIndex < 56)
			{
				MessageBlock[MsgIndex++] = 0;
			}
		}

		MessageBlock[56] = (uint8_t)(LengthHigh >> 24);
		MessageBlock[57] = (uint8_t)(LengthHigh >> 16);
		MessageBlock[58] = (uint8_t)(LengthHigh >> 8);
		MessageBlock[59] = (uint8_t)(LengthHigh);
		MessageBlock[60] = (uint8_t)(LengthLow >> 24);
		MessageBlock[61] = (uint8_t)(LengthLow >> 16);
		MessageBlock[62] = (uint8_t)(LengthLow >> 8);
		MessageBlock[63] = (uint8_t)(LengthLow);

		ProcessMessageBlock();
	}


	uint32_t Result()
	{
		if (Corrupted != 0)
		{
			return 0;
		}

		if (Computed == 0)
		{
			PadMessage();
			Computed = 1;
		}

		return 1;
	}

	void Input(uint8_t _1[], uint32_t size)
	{
		if (size == 0)
		{
			return;
		}

		if (Computed > 0 || Corrupted > 0)
		{
			Corrupted = 1;
			return;
		}

		uint32_t counter = 0;
		while (counter < size && Corrupted == 0)
		{
			MessageBlock[MsgIndex++] = _1[counter];

			LengthLow += 8;
			LengthLow &= 0xFFFFFFFF;

			if (LengthLow == 0)
			{
				LengthHigh++;
				LengthHigh &= 0xFFFFFFFF;

				if (LengthHigh == 0)
				{
					Corrupted = 1;
				}
			}

			if (MsgIndex == 64)
			{
				ProcessMessageBlock();
			}

			counter++;
		}
	}

	uint32_t *GetMessageDigest()
	{
		return this->MessageDigest;
	}

	uint32_t* Digest(uint8_t _1[], uint32_t size)
	{
		Input(_1, size);
		Result();

		return GetMessageDigest();
	}
};

class CKey
{
public:
	uint32_t Buffer[60];
	uint32_t m_KEY;
	uint32_t Pos1, Pos2, MaxPos;
	uint32_t Sum;
	uint32_t Size;

	CKey()
	{
	}

	CKey(uint32_t size, uint32_t mPos) : m_KEY(0), Pos1(0), Pos2(0), Sum(0), MaxPos(0), Size(0)
	{
		Size = size;
		Pos2 = MaxPos = mPos;

		memset(Buffer, 0, 60 * 4);
	}

	~CKey()
	{
	}
};

class CCrypt
{
private:
	int32_t m_ChangeLen;
	int32_t m_ChangeData;
	int32_t m_LastInput;
	int32_t m_LastChangeLen;
	int32_t m_LastChangeData;

	CKey m_KEY[3];
	CKey m_LastKEY[3];

public:
	inline CKey * GetKeyMat()
	{
		return m_KEY;
	}

	inline CKey * GetLastKeyMat() 
	{
		return m_LastKEY;
	}

	inline int32_t GetChangeLen()
	{
		return m_ChangeLen;
	}

	inline int32_t GetChangeData()
	{
		return m_ChangeData;
	}

	inline int32_t GetLastInput()
	{
		return m_LastInput;
	}

	inline int32_t GetLastChangeLen()
	{
		return m_LastChangeLen;
	}

	inline int32_t GetLastChangeData()
	{
		return m_LastChangeData;
	}

public:
	CCrypt() : m_ChangeLen(0), m_ChangeData(0), m_LastChangeLen(0), m_LastChangeData(0), m_LastInput(0)
	{
		memset(m_KEY, 0, sizeof(CKey) * 3);

		m_KEY[0] = CKey(55, 31);
		m_KEY[1] = CKey(57, 50);
		m_KEY[2] = CKey(58, 39);

		m_LastKEY[0] = m_KEY[0];
		m_LastKEY[1] = m_KEY[1];
		m_LastKEY[2] = m_KEY[2];
	}

	~CCrypt()
	{
	}

	inline void operator=(CCrypt& other)
	{
		memcpy(this, &other, sizeof(CCrypt));
	}

	inline void SaveData()
	{
		m_LastChangeLen	= m_ChangeLen;
		m_LastChangeData= m_ChangeData;

		m_LastKEY[0] = m_KEY[0];
		m_LastKEY[1] = m_KEY[1];
		m_LastKEY[2] = m_KEY[2];
	}

	inline void RestoreData()
	{
		m_ChangeLen	= m_LastChangeLen;
		m_ChangeData= m_LastChangeData;

		m_KEY[0] = m_LastKEY[0];
		m_KEY[1] = m_LastKEY[1];
		m_KEY[2] = m_LastKEY[2];
	}

	static inline void FillKey(uint8_t result[680], uint8_t _1[])
	{
		for (int i = 0; i < 680; i++)
		{
			result[i] = _1[i % 128];
		}

		result[0] = 128;
	}

	inline void GenerateKey(uint8_t _1[], uint32_t size)
	{
		uint8_t result[680];

		FillKey(result, _1);

		for (int i = 0; i < 680; i += 20)
		{
			CSha SHA;
			uint32_t *sha = SHA.Digest(result, 680);

			for (int j = 0; j < 5; j++)
			{
				memcpy(result + i + j * 4, sha + j, 4);
			}
		}

		for (int i = 0; i < 220; i += 4)
		{
			m_KEY[0].Buffer[i / 4] = *(uint32_t*)(result + i);
		}

		for (int i = 0; i < 228; i += 4)
		{
			m_KEY[1].Buffer[i / 4] = *(uint32_t*)(result + 220 + i);
		}

		for (int i = 0; i < 232; i += 4)
		{
			m_KEY[2].Buffer[i / 4] = *(uint32_t*)(result + 448 + i);
		}

		memcpy(m_LastKEY, m_KEY, sizeof(CKey) * 3);
	}

	inline void ApplyCryptor(uint8_t *_1, uint32_t size, bool save = true)
	{
		if (save)
		{
			SaveData();
		}

		uint32_t real_size = size;
		uint32_t s = size;
		uint32_t pre = (s < m_ChangeLen) ? s : m_ChangeLen;

		if (pre != 0)
		{
			for (int j = 0; j < pre; j++)
			{
				*(uint8_t*)(_1 + j) ^= (uint8_t)(m_ChangeData >> (8 * (4 - m_ChangeLen + j)));
			}

			m_ChangeLen -= pre;
			real_size	-= pre;
		}

		for (int i = pre; i < s - 3; i += 4)
		{
			uint32_t result = m_KEY[0].m_KEY & m_KEY[1].m_KEY | m_KEY[2].m_KEY & (m_KEY[0].m_KEY | m_KEY[1].m_KEY);

			for (int j = 0; j < 3; j++)
			{
				CKey *k = &m_KEY[j];

				if (result == k->m_KEY)
				{
					uint32_t t1, t2, t3;

					t1 = k->Buffer[k->Pos1];
					t2 = k->Buffer[k->Pos2];
					t3 = (t1 <= t2) ? t1 : t2;

					k->Sum		= t1 + t2;
					k->m_KEY	= (t3 > k->Sum) ? 1 : 0;
					k->Pos1		= (k->Pos1 + 1) % k->Size;
					k->Pos2		= (k->Pos2 + 1) % k->Size;
				}

				*(uint8_t*)(_1 + i)		^= (uint8_t)(k->Sum);
				*(uint8_t*)(_1 + i + 1) ^= (uint8_t)(k->Sum >> 8);
				*(uint8_t*)(_1 + i + 2) ^= (uint8_t)(k->Sum >> 16);
				*(uint8_t*)(_1 + i + 3) ^= (uint8_t)(k->Sum >> 24);
			}
		}

		uint32_t remain = real_size & 3;

		if (remain != 0)
		{
			uint32_t result = m_KEY[0].m_KEY & m_KEY[1].m_KEY | m_KEY[2].m_KEY & (m_KEY[0].m_KEY | m_KEY[1].m_KEY);

			m_ChangeData = 0;

			for (int j = 0; j < 3; j++)
			{
				CKey *k = &m_KEY[j];

				if (result == k->m_KEY)
				{
					uint32_t t1, t2, t3;

					t1 = k->Buffer[k->Pos1];
					t2 = k->Buffer[k->Pos2];
					t3 = (t1 <= t2) ? t1 : t2;

					k->Sum		= t1 + t2;
					k->m_KEY	= (t3 > k->Sum) ? 1 : 0;
					k->Pos1		= (k->Pos1 + 1) % k->Size;
					k->Pos2		= (k->Pos2 + 1) % k->Size;
				}

				m_ChangeData ^= (int32_t)k->Sum;
			}

			for (int j = 0; j < remain; j++)
			{
				*(uint8_t*)(_1 + real_size + pre - remain + j) ^= (uint8_t)(m_ChangeData >> (j * 8));
			}

			m_ChangeLen = 4 - remain;
		}
	}

	static inline void XorKey(uint8_t result[128], uint8_t _1[], uint8_t _2[], uint32_t size1, uint32_t size2)
	{
		for (int i = 0; i < size1; i++)
		{
			result[i] = (uint8_t)(_1[i] ^ _2[i]);
		}
	}

	static inline void ShiftKey(uint8_t result[128], uint8_t _1[], uint32_t n, uint32_t size, bool dir = true)
	{
		for (int i = 0; i < size; i++)
		{
			if (dir)
			{
				result[(i + n) % size] = _1[i];
			}
			else
			{
				result[i] = _1[(i + n) % size];
			}
		}
	}

	inline void RandomKey(uint8_t result[128])
	{
		boost::mt19937 rng;
		boost::uniform_int<> twobytes(0x00, 0xFF);
		boost::variate_generator< boost::mt19937, boost::uniform_int<> >
			rnd(rng, twobytes);

		for (int i = 0; i < arr_size(result); i++)
		{
			result[i] = ~rnd();
		}
	}
};

class CryptManager
{
protected:
	uint8_t SV_KEY_1[128];
	uint8_t SV_KEY_2[128];
	uint8_t CL_KEY_1[128];
	uint8_t CL_KEY_2[128];

	uint8_t DECRYPT_KEY[128];
	uint8_t ENCRYPT_KEY[128];

	CCrypt *Encrypt;
	CCrypt *EncryptLayer;

	CCrypt *Decrypt;
	CCrypt *DecryptLayer;

	uint32_t m_ClientKeyCount;
	uint32_t m_ServerKeyCount;

public:
	CryptManager()
	{
	}

	~CryptManager()
	{
	}

	void Initialize()
	{
		m_ClientKeyCount = 0;
		m_ServerKeyCount = 0;

		memset(SV_KEY_1,	0, 128);
		memset(SV_KEY_2,	0, 128);
		memset(CL_KEY_1,	0, 128);
		memset(CL_KEY_2,	0, 128);
		memset(DECRYPT_KEY, 0, 128);
		memset(ENCRYPT_KEY, 0, 128);

		Encrypt = new CCrypt();
		Decrypt = new CCrypt();

		EncryptLayer = new CCrypt();
		DecryptLayer = new CCrypt();
	}

	inline void SVSetKeyData(void *k)
	{
		m_ServerKeyCount++;

		if (m_ServerKeyCount == 1) 
		{
			memcpy(reinterpret_cast<void*>(SV_KEY_1), reinterpret_cast<const void*>(k), 128);
		}
		else if (m_ServerKeyCount == 2) 
		{
			memcpy(reinterpret_cast<void*>(SV_KEY_2), reinterpret_cast<const void*>(k), 128);

			if (m_ClientKeyCount == 2)
			{
				ApplyData();
			}
		};
	}

	inline void CLSetKeyData(void *k)
	{
		m_ClientKeyCount++;

		if (m_ClientKeyCount == 1) 
		{
			memcpy(reinterpret_cast<void*>(CL_KEY_1), reinterpret_cast<const void*>(k), 128);
		}
		else if(m_ClientKeyCount == 2)
		{
			memcpy(reinterpret_cast<void*>(CL_KEY_2), reinterpret_cast<const void*>(k), 128);

			if (m_ServerKeyCount == 2)
			{
				ApplyData();
			}
		};
	}

	void ApplyData()
	{
		uint8_t tmp0[128];
		uint8_t tmp1[128];
		uint8_t tmp2[128];

		CCrypt::ShiftKey(tmp0, SV_KEY_1, 31, 128);
		memcpy(tmp1, tmp0, 128);

		CCrypt::XorKey(tmp0, tmp1, CL_KEY_1, 128, 128);
		memcpy(tmp2, tmp0, 128);

		CCrypt::ShiftKey(tmp0, CL_KEY_2, 17, 128, false);
		memcpy(tmp1, tmp0, 128);

		CCrypt::XorKey(tmp0, tmp1, tmp2, 128, 128);
		memcpy(DECRYPT_KEY, tmp0, 128);

		Decrypt->GenerateKey(DECRYPT_KEY, 128);

		CCrypt::ShiftKey(tmp0, SV_KEY_2, 79, 128);
		memcpy(tmp1, tmp0, 128);

		Decrypt->ApplyCryptor(tmp1, 128, true);

		memcpy(ENCRYPT_KEY, tmp1, 128);

		Encrypt->GenerateKey(ENCRYPT_KEY, 128);
		
		*EncryptLayer = *Encrypt;
		*DecryptLayer = *Decrypt;
	}

	inline void ClientEncrypt(uint8_t data[], uint32_t size)
	{
		DecryptLayer->ApplyCryptor(data, size);
	}

	inline void ServerEncrypt(uint8_t data[], uint32_t size, bool save = false)
	{
		EncryptLayer->ApplyCryptor(data, size);
	}

	inline void DoEncrypt(uint8_t data[], uint32_t size)
	{
		Encrypt->ApplyCryptor(data, size);
	}

	inline void DoDecrypt(uint8_t data[], uint32_t size)
	{
		Decrypt->ApplyCryptor(data, size);
	}

	inline void DoRestoreEncrypt()
	{
		Encrypt->RestoreData();
	}

	inline void DoRestoreDecrypt()
	{
		Decrypt->RestoreData();
	}

	inline void DoResetEncryptLayer()
	{
		EncryptLayer->RestoreData();
	}

	inline void DoResetDecryptLayer()
	{
		DecryptLayer->RestoreData();
	}

	inline bool HasServerKey()
	{
		return m_ServerKeyCount > 1;
	}

	inline bool HasClientKey()
	{
		return m_ClientKeyCount > 1;
	}
};

#endif