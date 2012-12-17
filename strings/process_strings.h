#pragma once
#include "windows.h"
#include "module.h"
#include <tlhelp32.h>
#include "string_parser.h"
#include <Psapi.h>
#include "basics.h"
#pragma comment(lib, "Psapi")

class process_strings
{
	DynArray<module*> modules;
	string_parser* parser;

	void generateModuleList(HANDLE hSnapshot);
	bool processAllHeaps(HANDLE ph, char* process_name);
public:
	process_strings(string_parser* parser);
	bool dump_process(DWORD pid);
	bool dump_system();
	~process_strings(void);
};
