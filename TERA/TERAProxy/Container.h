#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#include <vector>
#include <algorithm>
#include <assert.h>

class CPacketData
{
private:
	std::vector<uint32_t> m_vSizes;
	std::vector<uint8_t> m_vAllocations;

public:
	template<class T = uint8_t*>
	T GetContainerData()
	{
		return reinterpret_cast<T>(m_vAllocations.data());
	}

	size_t GetContainerSize()
	{
		return m_vAllocations.size();
	}

	size_t GetOffset(size_t offset)
	{
		if (m_vSizes.size() < offset)
		{
			return 0;
		}

		if (offset == 0)
		{
			return 0;
		}

		return m_vSizes[offset - 1];
	}

	size_t GetSizeAt(size_t offset)
	{
		if (m_vSizes.size() < offset)
		{
			return 0;
		}

		if (offset == 0)
		{
			return m_vSizes[0];
		}

		return m_vSizes[offset] - m_vSizes[offset - 1];
	}

public:
	CPacketData() 
	{
	}

	template<class Object>
	CPacketData(Object& obj)
	{
		Insert(obj);
	}

	CPacketData(size_t size, size_t reserved = 0)
	{
		m_vSizes.push_back(size);
		{
			if (!reserved)
			{
				m_vAllocations.reserve(size);
			}
			else
			{
				m_vAllocations.reserve(reserved);
			}
		}
	}

	CPacketData(uint8_t* pData, size_t size)
	{
		m_vSizes.push_back(size);
		{
			m_vAllocations.insert(m_vAllocations.begin(), pData, pData + size);
		}
	}

	~CPacketData() 
	{
	}

	template<class T>
	void operator+=(T& other)
	{
		Insert(other);
	}

	template<class T = uint8_t>
	T* operator[](size_t offset)
	{
		if (m_vAllocations.empty())
		{
			return nullptr;
		}

		return reinterpret_cast<T*>(&m_vAllocations[0] + GetOffset(offset));
	}

	template<class T>
	void Insert(T& val) 
	{
		assert(m_vAllocations.capacity() > m_vAllocations.size() + sizeof(T));

		if (!m_vSizes.empty())
		{
			m_vSizes.push_back(m_vSizes[m_vSizes.size() - 1] + sizeof(T));
		}
		else
		{
			m_vSizes.push_back(sizeof(T));
		}

		m_vAllocations.insert(m_vAllocations.end(), reinterpret_cast<uint8_t*>(&val),
			reinterpret_cast<uint8_t*>(&val) + sizeof(T));
	}

	void Insert(unsigned char* begin, unsigned char *end)
	{
		if (begin == end)
		{
			return;
		}

		assert(m_vAllocations.capacity() > m_vAllocations.size() + (end - begin));

		if (!m_vSizes.empty())
		{
			m_vSizes.push_back(m_vSizes[m_vSizes.size() - 1] + (end - begin));
		}
		else
		{
			m_vSizes.push_back(end - begin);
		}

		m_vAllocations.insert(m_vAllocations.end(), begin, end);
	}

	template<class T>
	void Get(size_t offset, T& ref) 
	{
		if (m_vSizes.size() <= offset)
		{
			return;
		}

		if (offset == 0)
		{
			ref = *reinterpret_cast<T*>(&m_vAllocations[0]);
		}
		else
		{
			ref = *reinterpret_cast<T*>(&m_vAllocations[0] + m_vSizes[offset - 1]);
		}
	}

	void Remove(size_t offset) 
	{
		if (m_vSizes.size() < offset)
		{
			return;
		}

		if (offset == 0)
		{
			m_vAllocations.erase(m_vAllocations.begin(), m_vAllocations.begin() + m_vSizes[0]);
		}
		else
		{
			m_vAllocations.erase(m_vAllocations.begin() + m_vSizes[offset - 1],
				m_vAllocations.begin() + m_vSizes[offset - 1] + (m_vSizes[offset] - m_vSizes[offset - 1]));
		}

		if (m_vSizes.size() != offset + 1)
		{
			std::for_each(m_vSizes.begin() + offset, m_vSizes.end(), [&](size_t& size)
			{
				size -= m_vSizes[offset];
			});
		}

		m_vSizes.erase(m_vSizes.begin() + offset);
	}

	template<class Pred, class T>
	void Get(Pred& p, T& ref)
	{
		boost::function<Pred> function(p);

		unsigned int dist = 0;
		for (auto it = m_vAllocations.begin(); ;)
		{
			while (dist != m_vSizes.size() - 1)
			{
				if (GetSizeAt(dist) != sizeof(T))
				{
					it += GetSizeAt(dist++);
				}
				else
				{
					break;
				}
			}

			if (it == m_vAllocations.end())
			{
				break;
			}

			if (function(it))
			{
				ref = *reinterpret_cast<T*>(&*it);
				break;
			}

			it += GetSizeAt(dist);
		}
	}

	template<class Pred, class T, class... Args>
	void Get(Pred& p, T& ref, Args&&... args)
	{
		boost::function<Pred> function(p);

		unsigned int dist = 0;
		for (auto it = m_vAllocations.begin(); ;)
		{
			while (dist < m_vSizes.size())
			{
				if (GetSizeAt(dist) != sizeof(T))
				{
					it += GetSizeAt(dist++);
				}
				else
				{
					break;
				}
			}

			if (it == m_vAllocations.end())
			{
				break;
			}

			if (function(*reinterpret_cast<T*>(&*it), args...))
			{
				ref = *reinterpret_cast<T*>(&*it);
				break;
			}

			it += GetSizeAt(dist);
		}
	}

	template<class Pred, class Range, size_t N = 0>
	void GetAny(Pred& p, Range& r)
	{
		boost::function<Pred> function(p);

		unsigned int dist = 0;
		for (auto it = m_vAllocations.begin() + N; ;)
		{
			while (dist != m_vSizes.size() - 1)
			{
				if (GetSizeAt(dist) != sizeof(Range::value_type))
				{
					it += GetSizeAt(dist++);
				}
				else
				{
					break;
				}
			}

			if (it == m_vAllocations.end())
			{
				break;
			}

			if (function(*reinterpret_cast<Range::value_type*>(&*it)))
			{
				r.push_back(*reinterpret_cast<Range::value_type*>(&*it));
			}

			it += GetSizeAt(dist);
		}
	}

	template<class Pred, class Range, class... Args>
	void GetAny(Pred& p, Range& r, Args&&... args)
	{
		boost::function<Pred> function(p);

		unsigned int dist = 0;
		for (auto it = m_vAllocations.begin(); it != m_vAllocations.end(); )
		{
			while (dist != m_vSizes.size() - 1)
			{
				if (GetSizeAt(dist) != sizeof(Range::value_type))
				{
					it += GetSizeAt(dist++);
				}
				else
				{
					break;
				}
			}

			if (it == m_vAllocations.end())
			{
				break;
			}

			if (function(*reinterpret_cast<Range::value_type*>(&*it), args...))
			{
				r.push_back(*reinterpret_cast<Range::value_type*>(&*it));
			}

			it += GetSizeAt(dist);
		}
	}

	template<class T, class Iter = std::vector<unsigned char>::iterator>
	static T* Find(const T& data, size_t size, Iter begin, Iter end)
	{
		for (auto it = begin; it != end; ++it)
		{
			if (memcmp(&data, &*it, size) == 0)
			{
				return reinterpret_cast<T*>(&*it);
			}
		}

		return nullptr;
	}

	template<class T>
	T* Find(const T& data, size_t size)
	{
		for (auto it = m_vAllocations.begin(); it != m_vAllocations.end(); ++it)
		{
			if (memcmp(&data, &*it, size) == 0)
			{
				return reinterpret_cast<T*>(&*it);
			}
		}

		return nullptr;
	}
};

#endif