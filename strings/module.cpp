#include "StdAfx.h"
#include "module.h"

bool module::contains(unsigned int address)
{
	// Check if this module contains the specified address
	return (byte*) address >= moduleDetails.modBaseAddr && (byte*) address < moduleDetails.modBaseAddr + moduleDetails.modBaseSize;
}

bool module::operator== (const module &other) const
{
	return this->moduleDetails.hModule == other.moduleDetails.hModule;
}

module::module(MODULEENTRY32W details)
{
	moduleDetails = details;
}

module::~module(void)
{
}
