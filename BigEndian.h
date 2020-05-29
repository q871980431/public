#ifndef __BIGENDIAN_H__
#define __BIGENDIAN_H__

#include <string>
namespace endian{
	const static int tmp = 0x4321;
	constexpr bool IsBigEndian(){return 0x21 != *(unsigned char *)&tmp;}
	const static bool IS_BIG_ENDIAN = IsBigEndian();

	template<size_t N>
	inline void reverse(char *l, const char *r)
	{
		*l = *r;
		return reverse<N - 1>(l+1, r-1);
	}

	template<>
	inline void reverse<1>(char *l, const char *r)
	{
		*l = *r;
	}

	template<int32_t BUFF_SIZE = 16>
	class BigEndian
	{
	public:
		BigEndian() :m_size(0) {};
		BigEndian(std::string &strContent) {
			int32_t iLen = strContent.length();
			if (iLen > BUFF_SIZE)
				iLen = BUFF_SIZE;
			memcpy(m_buff, strContent.c_str(), iLen);
			m_size = iLen;
		}

		template<typename T>
		void Append(T val) { InnerAppend<sizeof(T)>(&val); };
		template<typename T>
		void Pick(int32_t iOffSet, T &val) { InnerPick<sizeof(T)>(iOffSet, &val); };
		template<typename ...Args>
		void Append(Args... args){std::initializer_list<int32_t> {(Append(args), 0)...};}

		inline const char *GetBuff() { return m_buff; }
		inline int32_t Size() { return m_size; };
		inline std::string ToString() { return m_size == 0 ? "" : std::string(m_buff, m_size); };
	private:
		template<size_t N>
		void InnerAppend(void *data)
		{
			if (m_size + N <= BUFF_SIZE)
			{
				if (IS_BIG_ENDIAN)
					memcpy(m_buff + m_size, data, N);
				else
					reverse<N>(m_buff + m_size, (const char*)data+(N-1));
				m_size += N;
			}
		}
		template<size_t N>
		void InnerPick(int32_t iOffSet, void *content)
		{
			if (iOffSet + N <= m_size)
			{
				if (IS_BIG_ENDIAN)
					memcpy(content, m_buff + iOffSet, N);
				else
					reverse<N>((char*)content, m_buff + iOffSet + N - 1);
			}
		}
	protected:
	private:
		char m_buff[BUFF_SIZE];
		int32_t m_size;
	};
}
#endif
