#include "StdAfx.h"
#include "process_strings.h"

bool IsWin64(HANDLE process)
{
    BOOL ret_val;
	if( IsWow64Process(process, &ret_val) )
	{
		return ret_val;
	}
	PrintLastError((LPTSTR) "IsWow64Process");
	return false;
}

bool process_strings::dump_system()
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

bool process_strings::dump_process(DWORD pid)
{
	// Open the process
	HANDLE ph = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);
	if( ph != NULL )
	{
		// Assign the process name
		TCHAR* process_name_w = new TCHAR[0x100];
		process_name_w[0] = 0;
		GetModuleBaseName(ph, 0, process_name_w, 0x100 );
		char* process_name = new char[0x100];
		process_name[0] = 0;

		// Convert from wchar to char filename
		wcstombs( process_name, process_name_w, 0x100 );
		
		// Generate the module list
		HANDLE hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
		if ( hSnapshot != INVALID_HANDLE_VALUE )
		{
			this->_generate_module_list(hSnapshot);
			CloseHandle(hSnapshot);
			
			// Walk through the process heaps, extracting the strings
			bool result = this->_process_all_memory(ph, process_name);

			free(process_name);
			return result;
		}else{
			fprintf(stderr,"Failed gather module information for process 0x%x (%i). ", pid, pid);
			PrintLastError((LPTSTR) "dump_process");
		}

		free(process_name);
		return false;
	}else{
		fprintf(stderr,"Failed open process 0x%x (%i). ", pid, pid);
		PrintLastError((LPTSTR) "dump_process");
	}
}

process_strings::process_strings(string_parser* parser)
{
	this->m_parser = parser;
}


bool process_strings::_process_all_memory(HANDLE ph, string process_name)
{
	// Set the max address of the target process. Assume it is a 64 bit process.
	__int64 max_address = 0;
	max_address = 0x7ffffffffff;

    // Walk the process heaps
    __int64 address = 0;
    MEMORY_BASIC_INFORMATION mbi;
	
    while (address < max_address)
    {
        // Load this region information
        __int64 block_size = VirtualQueryEx(ph, (LPCVOID) address, (_MEMORY_BASIC_INFORMATION*) &mbi, sizeof(_MEMORY_BASIC_INFORMATION64));
		__int64 new_address = (__int64)mbi.BaseAddress + (__int64)mbi.RegionSize + (__int64)1;
		if( new_address <= address )
			break;
		address = new_address;

		if( mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) )
		{
			// Process this region

			// Read in the region
			unsigned char* buffer = new unsigned char[mbi.RegionSize];
			if( buffer != NULL )
			{
				__int64 num_read = 0;
				bool result = ReadProcessMemory(ph, (LPCVOID) mbi.BaseAddress, buffer, mbi.RegionSize,(SIZE_T*) &num_read);

				//fprintf(stderr,"Current address: %016llX\n",mbi.BaseAddress);
				if( num_read > 0 )
				{
					if( num_read != (unsigned int) mbi.RegionSize )
						fprintf(stderr,"Failed read full region from address 0x%016llX: %s. Only %i of expected %i bytes were read.\n", mbi.BaseAddress, strerror(errno), num_read, mbi.RegionSize);

					// Print the strings from this region
					std::stringstream stream;
					stream << "0x" << std::hex << mbi.BaseAddress;
					m_parser->parse_block( buffer, num_read, process_name, stream.str() );
				}else if( !result ){
					fprintf(stderr,"Failed to read from address 0x%016llX. ", mbi.BaseAddress);
					PrintLastError((LPTSTR) "ReadProcessMemory");
				}

				// Cleanup
				free(buffer);
			}else{
				fprintf(stderr,"Failed to allocate space of %x for reading in a region.", mbi.RegionSize);
			}
		}
    }

	return true;
}

void process_strings::_generate_module_list(HANDLE hSnapshot)
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

process_strings::~process_strings(void)
{
}
