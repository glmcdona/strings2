#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>	
#include <string>
#include <locale>
#include <codecvt>

using namespace std;

class module
{
	MODULEENTRY32W m_module_details;
public:
	bool contains(PVOID64 address);
	string get_filepath();
	string get_filename();

	module(MODULEENTRY32W details);
	~module(void);
	bool operator== (const module &other) const;
};
