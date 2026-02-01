#pragma once

#include <stdlib.h>

namespace Util
{
	class SystemAPI
	{
	public:
#ifdef _MSC_VER
		static void Sleep(int milliSec)
		{
			_sleep(milliSec);
		}
#else
		static void Sleep(int milliSec)
		{
			sleep((milliSec/1000));
		}
#endif
	};
	
}