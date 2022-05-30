#include "StdAfx.h"
#include "memory_strings.h"

bool IsWin64(HANDLE process)
{
    BOOL ret_val;
	if( IsWow64Process(process, &ret_val) )
	{
		return ret_val;
	}
	PrintLastError((LPTSTR) L"IsWow64Process");
	return false;
}

bool memory_strings::dump_system()
{
	// Enumerate processes, and process the strings from each one
	HANDLE h_snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	
	if( h_snapshot != INVALID_HANDLE_VALUE )
	{
		// Handle the first i_process
		PROCESSENTRY32 tmp_p;
		tmp_p.dwSize = sizeof(PROCESSENTRY32);
		int result;
		if( (result = Process32First(h_snapshot, &tmp_p)) )
		{
			if( result == TRUE )
				dump_process(tmp_p.th32ProcessID);

			while( (result = Process32Next(h_snapshot, &tmp_p)) )
			{
				if( result == TRUE )
					dump_process(tmp_p.th32ProcessID);
			}
		}

		// Cleanup the handle
		CloseHandle( h_snapshot );
		return true;
	}
	return false;
}

bool memory_strings::dump_process(DWORD pid)
{
	// Open the process
	HANDLE ph = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
	if( ph != NULL )
	{
		// Assign the process name
		char process_name[0x100] = { 0 };
		GetModuleBaseNameA(ph, 0, process_name, 0x100 );
		
		// Generate memory region list
		HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if ( hSnapshot != INVALID_HANDLE_VALUE )
		{
			this->_generate_module_list(hSnapshot);
			CloseHandle(hSnapshot);
			
			// Walk through the process regions, extracting the strings
			bool result = this->_process_all_memory(ph, process_name);

			return result;
		}else{
			fprintf(stderr,"Failed gather module information for process 0x%x (%i). ", pid, pid);
			PrintLastError( (LPTSTR) L"dump_process");
		}
	}else{
		fprintf(stderr,"Failed open process 0x%x (%i). ", pid, pid);
		PrintLastError((LPTSTR) L"dump_process");
	}
	return false;
}

memory_strings::memory_strings(string_parser* parser)
{
	this->m_parser = parser;
}


MBI_BASIC_INFO memory_strings::_get_mbi_info(unsigned __int64 address, HANDLE ph)
{
	_MEMORY_BASIC_INFORMATION64 mbi;
	MBI_BASIC_INFO result;
	result.base = 0;
	result.end = 0;
	result.protect = 0;
	result.valid = false;
	result.executable = false;

	// Load this heap information
	__int64 blockSize = VirtualQueryEx(ph, (LPCVOID)address, (PMEMORY_BASIC_INFORMATION)&mbi, sizeof(_MEMORY_BASIC_INFORMATION64));

	if (blockSize == sizeof(_MEMORY_BASIC_INFORMATION64))
	{
		result.base = mbi.BaseAddress;
		result.end = mbi.BaseAddress + mbi.RegionSize;
		result.protect = mbi.Protect;
		result.valid = mbi.State != MEM_FREE && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD));
		result.executable = (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) > 0;
		result.size = mbi.RegionSize;
	}
	else if (blockSize == sizeof(_MEMORY_BASIC_INFORMATION32))
	{
		_MEMORY_BASIC_INFORMATION32* mbi32 = (_MEMORY_BASIC_INFORMATION32*)&mbi;

		result.base = mbi32->BaseAddress;
		result.end = (long long) mbi32->BaseAddress + (long long)mbi32->RegionSize;
		result.protect = mbi32->Protect;
		result.valid = mbi32->State != MEM_FREE && !(mbi32->Protect & (PAGE_NOACCESS | PAGE_GUARD));
		result.executable = (mbi32->Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) > 0;
		result.size = mbi.RegionSize;
	}

	return result;
}

bool memory_strings::_process_all_memory(HANDLE ph, string process_name)
{
	// Set the max address of the target process. Assume it is a 64 bit process.
	unsigned __int64 max_address = 0xffffffffffffffff; // Not a problem for 32bit targets

    // Walk the process heaps
    unsigned __int64 address = 0;
	
    while (address < max_address)
    {
        // Load this region information
		MBI_BASIC_INFO mbi_info = _get_mbi_info(address, ph);

		if (mbi_info.end + 1 <= address)
			break;
		address = mbi_info.end + 1;
		
		if(mbi_info.valid && mbi_info.size > 0)
		{
			// Process this region

			// Read in the region
			unsigned char* buffer = new unsigned char[mbi_info.size];
			if( buffer != NULL )
			{
				unsigned __int64 num_read = 0;
				bool result = ReadProcessMemory(ph, (LPCVOID) mbi_info.base, buffer, mbi_info.size,(SIZE_T*) &num_read);

				//fprintf(stderr,"Current address: %016llX\n",mbi.BaseAddress);
				if( num_read > 0 )
				{
					if( num_read != mbi_info.size)
						fprintf(stderr,"Failed read full region from address 0x%llx: %s. Only %lld of expected %lld bytes were read.\n", mbi_info.base, strerror(errno), num_read, mbi_info.size);

					// Load the module name if applicable
					string module_name_short = "region";
					string module_name_long = "region";

					for (int i = 0; i < m_modules.size(); i++)
					{
						if (m_modules[i].contains((PVOID64) mbi_info.base))
						{
							module_name_short = m_modules[i].get_filename();
							module_name_long = m_modules[i].get_filepath();
						}
					}

					// Print the strings from this region
					std::stringstream long_name;
					long_name << process_name << ":" << module_name_long << "@0x" << std::hex << mbi_info.base;
					std::stringstream short_name;
					short_name << process_name << ":" << module_name_short << "@0x" << std::hex << mbi_info.base;
					m_parser->parse_block( buffer, num_read, short_name.str(), long_name.str(), mbi_info.base );
				}

				// Cleanup
				delete[] buffer;
			}else{
				fprintf(stderr,"Failed to allocate space of %lld for reading in a region.", mbi_info.size);
			}
		}
    }

	return true;
}

void memory_strings::_generate_module_list(HANDLE hSnapshot)
{
	MODULEENTRY32 tmp_m;
	tmp_m.dwSize = sizeof(MODULEENTRY32);
	if( Module32First(hSnapshot, &tmp_m) )
	{
		// Add this i_module to our array
		tmp_m.dwSize = sizeof(MODULEENTRY32);
		m_modules.push_back(module(tmp_m));

		while(Module32Next(hSnapshot,&tmp_m))
		{
			// Add this i_module to our array
			m_modules.push_back(module(tmp_m));
			tmp_m.dwSize = sizeof(MODULEENTRY32);
		}
	}
}

memory_strings::~memory_strings(void)
{
}
