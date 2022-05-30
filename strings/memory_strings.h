#pragma once
#include "windows.h"
#include "module.h"
#include <tlhelp32.h>
#include "string_parser.h"
#include <Psapi.h>
#include <vector>
#include "basics.h"
#include <string>
#include <sstream>

#pragma comment(lib, "Psapi")
using namespace std;

struct MBI_BASIC_INFO
{
	unsigned __int64 base;
	unsigned __int64 end;
	DWORD protect;
	bool valid;
	bool executable;
	unsigned __int64 size;
};

class memory_strings
{
	vector<module> m_modules;
	string_parser* m_parser;

	void _generate_module_list(HANDLE hSnapshot);
	bool _process_all_memory(HANDLE ph, string process_name);
	MBI_BASIC_INFO _get_mbi_info(unsigned __int64 address, HANDLE ph);
public:
	memory_strings(string_parser* parser);
	bool dump_process(DWORD pid);
	bool dump_system();

	~memory_strings(void);
};
