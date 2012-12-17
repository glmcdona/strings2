#include "StdAfx.h"
#include "process_strings.h"

bool IsWin64(HANDLE process)
{
    BOOL retVal;
	if( IsWow64Process(process, &retVal) )
	{
		return retVal;
	}
	PrintLastError(L"IsWow64Process");
	return false;
}

bool process_strings::dump_system()
{
	// Enumerate processes, and process the strings from each one
	HANDLE hSnapShot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	
	if( hSnapShot != INVALID_HANDLE_VALUE )
	{
		// Handle the first i_process
		PROCESSENTRY32 tmpProcess;
		tmpProcess.dwSize = sizeof(PROCESSENTRY32);
		int result;
		if( (result = Process32First(hSnapShot, &tmpProcess)) )
		{
			if( result == TRUE )
				dump_process(tmpProcess.th32ProcessID);

			while( (result = Process32Next(hSnapShot, &tmpProcess)) )
			{
				if( result == TRUE )
					dump_process(tmpProcess.th32ProcessID);
			}
		}

		// Cleanup the handle
		CloseHandle( hSnapShot );
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
			this->generateModuleList(hSnapshot);
			CloseHandle(hSnapshot);
			
			// Walk through the process heaps, extracting the strings
			bool result = this->processAllHeaps(ph, process_name);

			free(process_name);
			return result;
		}else{
			fprintf(stderr,"Failed gather module information for process 0x%x (%i). ", pid, pid);
			PrintLastError(L"dump_process");
		}

		free(process_name);
		return false;
	}else{
		fprintf(stderr,"Failed open process 0x%x (%i). ", pid, pid);
		PrintLastError(L"dump_process");
	}
}

process_strings::process_strings(string_parser* parser)
{
	this->parser = parser;
}


bool process_strings::processAllHeaps(HANDLE ph, char* process_name)
{
	// Set the max address of the target process. Assume it is a 64 bit process.
	__int64 maxAddress = 0;
	maxAddress = 0x7ffffffffff;

    // Walk the process heaps
    __int64 address = 0;
    MEMORY_BASIC_INFORMATION mbi;
	
    while (address < maxAddress)
    {
        // Load this heap information
        __int64 blockSize = VirtualQueryEx(ph, (LPCVOID) address, (_MEMORY_BASIC_INFORMATION*) &mbi, sizeof(_MEMORY_BASIC_INFORMATION64));
		__int64 newAddress = (__int64)mbi.BaseAddress + (__int64)mbi.RegionSize + (__int64)1;
		if( newAddress <= address )
			break;
		address = newAddress;

		if( mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) )
		{
			// Process this heap

			// Read in the heap
			unsigned char* buffer = new unsigned char[mbi.RegionSize];
			if( buffer != NULL )
			{
				__int64 numRead = 0;
				bool result = ReadProcessMemory(ph, (LPCVOID) mbi.BaseAddress, buffer, mbi.RegionSize,(SIZE_T*) &numRead);

				//fprintf(stderr,"Current address: %016llX\n",mbi.BaseAddress);
				if( numRead > 0 )
				{
					if( numRead != (unsigned int) mbi.RegionSize )
						fprintf(stderr,"Failed read full heap from address 0x%016llX: %s. Only %i of expected %i bytes were read.\n", mbi.BaseAddress, strerror(errno), numRead, mbi.RegionSize);

					// Print the strings from this heap
					parser->parse_block( buffer, numRead, process_name);
				}else if( !result ){
					fprintf(stderr,"Failed to read from address 0x%016llX. ", mbi.BaseAddress);
					PrintLastError(L"ReadProcessMemory");
				}

				// Cleanup
				free(buffer);
			}else{
				fprintf(stderr,"Failed to allocate space of %x for reading in a heap.", mbi.RegionSize);
			}
		}
    }

	return true;
}

void process_strings::generateModuleList(HANDLE hSnapshot)
{
	MODULEENTRY32 tmpModule;
	tmpModule.dwSize = sizeof(MODULEENTRY32);
	if( Module32First(hSnapshot, &tmpModule) )
	{
		// Add this i_module to our array
		tmpModule.dwSize = sizeof(MODULEENTRY32);
		modules.Add(new module(tmpModule));

		while(Module32Next(hSnapshot,&tmpModule))
		{
			// Add this i_module to our array
			modules.Add(new module(tmpModule));
			tmpModule.dwSize = sizeof(MODULEENTRY32);
		}
	}
}

process_strings::~process_strings(void)
{
}
