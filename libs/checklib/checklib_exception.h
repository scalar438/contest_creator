#pragma once
#include <exception>

namespace checlib
{
	class exception_base : public std::exception
	{
	public:
		exception(const char * const s)
			: std::exception(s)
		{
		}
	};
};
