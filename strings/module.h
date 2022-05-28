#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>	
#include <string>

class module
{
	MODULEENTRY32W m_module_details;
public:
	bool contains(unsigned int address);

	module(MODULEENTRY32W details);
	~module(void);
	bool operator== (const module &other) const;
};
