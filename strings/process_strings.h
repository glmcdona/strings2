#pragma once
#include "windows.h"
#include "module.h"
#include <tlhelp32.h>
#include "string_parser.h"
#include <Psapi.h>
#include <vector>
#include "basics.h"
#pragma comment(lib, "Psapi")
using namespace std;

class process_strings
{
	vector<module> m_modules;
	string_parser* m_parser;

	void _generate_module_list(HANDLE hSnapshot);
	bool _process_all_memory(HANDLE ph, char* process_name);
public:
	process_strings(string_parser* parser);
	bool dump_process(DWORD pid);
	bool dump_system();
	~process_strings(void);
};
