#include "MemoryLeaksDumper.h"
#include <iostream>

#if defined(_MSC_VER) && defined(_DEBUG) && defined(DEBUG_MEMORY_LEAKS)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace unitexcpp
{

	MemoryLeaksDumper::MemoryLeaksDumper(void)
	{
		std::cout << "creating memory leaks dumper" << std::endl;
#if defined(_MSC_VER) && defined(DEBUG_MEMORY_LEAKS)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode( _CRT_ERROR | _CRT_WARN, _CRTDBG_MODE_DEBUG );
#endif
	}


	MemoryLeaksDumper::~MemoryLeaksDumper(void)
	{
		std::cout << "destroying leaks dumper" << std::endl;
#if defined(_MSC_VER) && defined(DEBUG_MEMORY_LEAKS)
		_CrtDumpMemoryLeaks();
#endif
	}

}
