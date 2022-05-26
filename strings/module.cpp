#include "StdAfx.h"
#include "module.h"

bool module::contains(unsigned int address)
{
	// Check if this module contains the specified address
	return (byte*) address >= m_module_details.modBaseAddr && (byte*) address < m_module_details.modBaseAddr + m_module_details.modBaseSize;
}

bool module::operator== (const module &other) const
{
	return this->m_module_details.hModule == other.m_module_details.hModule;
}

module::module(MODULEENTRY32W details)
{
	m_module_details = details;
}

module::~module(void)
{
}
