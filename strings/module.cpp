#include "StdAfx.h"
#include "module.h"

bool module::contains(PVOID64 address)
{
	// Check if this module contains the specified address
	return (BYTE*) address >= m_module_details.modBaseAddr && (BYTE*) address < m_module_details.modBaseAddr + m_module_details.modBaseSize;
}

string module::get_filepath()
{
	wstring ws = m_module_details.szExePath;

	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.to_bytes(ws);
}

string module::get_filename()
{
	wstring ws = m_module_details.szModule;

	using convert_type = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.to_bytes(ws);
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
