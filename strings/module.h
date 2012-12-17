#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>	
#include <string>
#include "DynArray.h"

class module
{
	MODULEENTRY32W moduleDetails;
public:
	bool contains(unsigned int address);

	module(MODULEENTRY32W details);
	~module(void);
	bool operator== (const module &other) const;
};
