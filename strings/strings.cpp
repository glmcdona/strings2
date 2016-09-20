// strings.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string_parser.h"
#include "windows.h"
#include <sys/types.h>
#include "dirent.h"
#include <errno.h>
#include <vector>
#include <string>
#include <iostream>
#include "Shlwapi.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include "process_strings.h"

using namespace std;


BOOL Is64BitWindows()
{
	#if defined(_WIN64)
		return TRUE;  // 64-bit programs run only on Win64
	#elif defined(_WIN32)
		// 32-bit programs run on both 32-bit and 64-bit Windows
		// so must sniff
		BOOL f64 = FALSE;
		return IsWow64Process(GetCurrentProcess(), &f64) && f64;
	#else
		return FALSE; // Win64 does not support Win16
	#endif
}

bool isElevated(HANDLE h_Process)
{
	HANDLE h_Token;
	TOKEN_ELEVATION t_TokenElevation;
    TOKEN_ELEVATION_TYPE e_ElevationType;
	DWORD dw_TokenLength;
	
	if( OpenProcessToken(h_Process, TOKEN_READ | TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES , &h_Token) )
	{
		if(GetTokenInformation(h_Token,TokenElevation,&t_TokenElevation,sizeof(t_TokenElevation),&dw_TokenLength))
		{
			if(t_TokenElevation.TokenIsElevated != 0)
			{
				if(GetTokenInformation(h_Token,TokenElevationType,&e_ElevationType,sizeof(e_ElevationType),&dw_TokenLength))
				{
					if(e_ElevationType == TokenElevationTypeFull || e_ElevationType == TokenElevationTypeDefault)
					{
						return true;
					}
				}
			}
		}
	}

    return false;
}

void processFolder( char* dir_name, WCHAR* filter, bool recursively, string_parser* parser )
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir (dir_name);
	if (dir != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			// Convert the path to wchar format
			wchar_t* result = new wchar_t[ent->d_namlen + 1];

			if( result != NULL )
			{
				for( int i = 0; i < ent->d_namlen; i++ )
					result[i] = ent->d_name[i];
				result[ent->d_namlen] = 0;

				if( (ent->d_type & DT_DIR) )
				{
					// Process this subdirectory if recursive flag is on
					if( recursively && wcscmp(result, L".") != 0 && wcscmp(result, L"..") != 0  )
					{
						// Build the directory path
						char* directory = new char[MAX_PATH+1];
						sprintf_s(directory, MAX_PATH+1, "%s/%s", dir_name, ent->d_name);

						processFolder( directory, filter, recursively, parser );

						// Cleanup
						delete[] directory;
					}
				}else{
					// Check if this filename is a match to the specified pattern
					if( PathMatchSpec( result, filter ) )
					{
						// Process this file
						int length = wcslen(result) + strlen(dir_name) + 1;
						char* filename = new char[length + 1];
						filename[length] = 0;
						sprintf_s( filename, length+1, "%s/%S", dir_name, result );

						// Processes the specified file for strings
						FILE* fh = fopen( filename, "rb" );
						if( fh != NULL )
						{
							parser->parse_stream(fh, filename);
							fclose(fh);
						}else{
							// Error
							fprintf(stderr, "Error opening file %s: %s.\n", filename, strerror(errno));
						}

						delete[] filename;
					}
				}
				delete[] result;
			}
			else
			{
				fprintf(stderr, "Failed to allocate memory block of size %i for filename: %s.\n", ent->d_namlen + 1, strerror(errno));
			}
		}
		closedir (dir);
	}else{
		fprintf(stderr, "Unable to open directory %s: %s.\n", dir_name, strerror(errno));
	}
}


bool getMaximumPrivileges(HANDLE h_Process)
{
	HANDLE h_Token;
	DWORD dw_TokenLength;
	if( OpenProcessToken(h_Process, TOKEN_READ | TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES , &h_Token) )
	{
		// Read the old token privileges
		TOKEN_PRIVILEGES* privilages = new TOKEN_PRIVILEGES[100];
		if( GetTokenInformation(h_Token, TokenPrivileges, privilages,sizeof(TOKEN_PRIVILEGES)*100,&dw_TokenLength) )
		{
			// Enable all privileges
			for( int i = 0; i < privilages->PrivilegeCount; i++ )
			{
				privilages->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
			}
			
			// Adjust the privilges
			if(AdjustTokenPrivileges( h_Token, false, privilages, sizeof(TOKEN_PRIVILEGES)*100, NULL, NULL  ))
			{
				delete[] privilages;
				return true;
			}
		}
		delete[] privilages;
	}
	return false;
}

int _tmain(int argc, _TCHAR* argv[])
{
	//char buf[1000000];
	//setvbuf(stdout, buf, _IOFBF, sizeof(buf));

	// Process the flags	
	WCHAR* filter = NULL;
	bool flagHelp = false;
	bool flagHeader = true;
	bool flagFile = false;
	bool flagFilePath = false;
	bool flagPrintType = false;
	bool flagAsmOnly = false;
	bool flagRawOnly = false;
	bool flagAsciiOnly = false;
	bool flagUnicodeOnly = false;
	bool pipedInput = !_isatty( _fileno( stdin ) );
	bool flagPidDump = false;
	bool flagSystemDump = false;
	bool flagRecursive = false;
	bool flagEscape = false;
	int minCharacters = 4;

	if( argc <= 1 && !pipedInput )
		flagHelp = true;
	for( int i = 1; i < argc; i++ )
	{
		if( lstrcmp(argv[i],L"--help") == 0 || lstrcmp(argv[i],L"-help") == 0 || lstrcmp(argv[i],L"-h") == 0 || lstrcmp(argv[i],L"--h") == 0)
			flagHelp = true;
		else if( lstrcmp(argv[i],L"-f") == 0 )
			flagFile = true;
		//else if( lstrcmp(argv[i],L"-ff") == 0 )
		//	flagFilePath = true;
		else if( lstrcmp(argv[i],L"-t") == 0 )
			flagPrintType = true;
		else if( lstrcmp(argv[i],L"-r") == 0 )
			flagRecursive = true;
		else if( lstrcmp(argv[i],L"-nh") == 0 )
			flagHeader = false;
		else if( lstrcmp(argv[i],L"-asm") == 0 )
			flagAsmOnly = true;
		else if( lstrcmp(argv[i],L"-raw") == 0 )
			flagRawOnly = true;
		else if( lstrcmp(argv[i],L"-pid") == 0 )
			flagPidDump = true;
		else if( lstrcmp(argv[i],L"-system") == 0 )
			flagSystemDump = true;
		else if( lstrcmp(argv[i],L"-a") == 0 )
			flagAsciiOnly = true;
		else if( lstrcmp(argv[i],L"-u") == 0 )
			flagUnicodeOnly = true;
		else if( lstrcmp(argv[i],L"-e") == 0 )
			flagEscape = true;
		else if( lstrcmp(argv[i],L"-l") == 0 )
		{
			if(  i + 1 < argc )
			{
				// Try to parse the number of characters
				int result = _wtoi(argv[i+1]);
				if( result >= 3 )
				{
					minCharacters = result;
				}else{
					fprintf(stderr,"Failed to parse -l argument. The string size must be 3 or larger:\n\teg. 'strings2 *.exe -l 6'\n");
					exit(0);
				}
				i++;
			}else{
				fprintf(stderr,"Failed to parse -l argument. It must be preceeded by a number:\n\teg. 'strings2 *.exe -l 6'\n");
				exit(0);
			}
		}else{
			// This is an unassigned argument
			if( filter == NULL )
			{
				filter = argv[i];
			}
			else
			{
				// This argument is an error, we already found our filter.
				fprintf(stderr,"Failed to parse argument number %i, '%S'. Try 'strings2 --help' for usage instructions.\n", i, argv[i]);
				exit(0);
			}
		}
	}

	// Fill out the options structure based on the flags
	STRING_OPTIONS options;
	options.printUniqueGlobal = false;
	options.printUniqueLocal = false;

	options.printAsciiOnly = false;
	options.printUnicodeOnly = false;
	options.printNormal = false;
	options.printASM = false;
	options.escapeNewLines = flagEscape;
	if( flagAsmOnly )
		options.printASM = true;
	if( flagRawOnly )
		options.printNormal = true;
	if( !flagAsmOnly && !flagRawOnly )
	{
		options.printASM = true;
		options.printNormal = true;
	}
	
	if( flagAsciiOnly && flagUnicodeOnly )
	{
		fprintf(stderr,"Warning. Default conditions extract both unicode and ascii strings. There is no need to use both '-a' and '-u' flags at the same time.\n");
	}else{
		if( flagAsciiOnly )
			options.printAsciiOnly = true;
		if( flagUnicodeOnly )
			options.printUnicodeOnly = true;
	}
	


	options.printType = flagPrintType;
	options.printFile = flagFile;
	options.minCharacters = minCharacters;

	// Print copyright header
	if( flagHeader )
	{
		printf("Strings2 v1.3\n");
		printf("  Copyright © 2016, Geoff McDonald\n");
		printf("  http://www.split-code.com/\n\n");
	}

	if( flagHelp )
	{
		// Print help page
		printf("Strings2 is an improved version of the Sysinternals strings tool that extracts all unicode/ascii strings from binary data. On top of the classical strings approach, this improved version is able to dump strings from process address spaces and also reconstructs assembly local variable assignment ascii/unicode strings.\n\n");
		printf("Example Usage:\n");
		printf("\tstrings2 malware.exe\n");
		printf("\tstrings2 *.exe > strings.txt\n");
		printf("\tstrings2 *.exe -nh -f -t -asm > strings.txt\n");
		printf("\tstrings2 -pid 419 > process_strings.txt\n");
		printf("\tstrings2 -pid 0x1a3 > process_strings.txt\n");
		printf("\tstrings2 -system > all_process_strings.txt\n");
		printf("\tcat abcd.exe | strings2 > out.txt\n\n");
		//printf("strings2 -process *notepad*");
		printf("Flags:\n");
		printf(" -f\n\tPrints the filename/processname before each string.\n");
		printf(" -r\n\tRecursively process subdirectories.\n");
		//printf(" -ff\n\tPrints the path\\filename before each string.\n");
		printf(" -t\n\tPrints the type before each string. Unicode,\n\tascii, or assembly unicode/ascii stack push.\n");
		printf(" -asm\n\tOnly prints the extracted ascii/unicode\n\tassembly stack push-hidden strings.\n");
		printf(" -raw\n\tOnly prints the regular ascii/unicode strings.\n");
		printf(" -a\n\tPrints only ascii strings.\n");
		printf(" -u\n\tPrints only unicode strings.\n");
		printf(" -l [numchars]\n\tMinimum number of characters that is\n\ta valid string. Default is 4.\n");
		printf(" -nh\n\tNo header is printed in the output.\n");
		//printf(" -process\n\tUses the filename filter as a process-name filter instead. The matching processes will have all their strings dumped.\n");
		printf(" -pid\n\tThe strings from the process address space for the\n\tspecified PID will be dumped. Use a '0x' prefix to\n\tspecify a hex PID.\n");
		printf(" -system\n\tDumps strings from all accessible processes on the\n\tsystem. This takes awhile.\n");
		printf(" -e\n\tEscape \\r and \\n characters.\n");
	}else{
		// Create the string parser object
		string_parser* parser = new string_parser(options);

		if (flagPidDump || flagSystemDump)
		{
			// Warn if running in 32 bit mode on a 64 bit OS
			if( Is64BitWindows() && sizeof(void*) == 4 )
			{
				fprintf(stderr, "WARNING: To properly dump address spaces of 64-bit processes the 64-bit version of strings2 should be used. Currently strings2 has been detected as running as a 32bit process under a 64bit operating system.\n\n");
			}

			// Elevate strings2 to the maximum privilges
			getMaximumPrivileges( GetCurrentProcess() );

			// Create a process string dump class
			process_strings* process = new process_strings(parser);

			if( flagPidDump )
			{
				// Extract all strings from the specified process
				if( filter != NULL )
				{
					// Check the prefix
					bool isHex = false;
					wchar_t* prefix = new wchar_t[3];
					memcpy(prefix, filter, 4);
					prefix[2] = 0;

					if( wcscmp(prefix, L"0x") == 0 )
					{
						filter = &filter[2];
						isHex = true;
					}
					delete[] prefix;
					
					// Extract the pid from the string
					unsigned int PID;
					if( (isHex && swscanf(filter, L"%x", &PID) > 0) ||
						(!isHex && swscanf(filter, L"%i", &PID) > 0))
					{
						// Successfully parsed the PID
						
						// Parse the process
						process->dump_process(PID);

						
					}else{
						fwprintf(stderr, L"Failed to parse filter argument as a valid PID: %s.\n", filter);
					}
				}else{
					fwprintf(stderr, L"Error. No PID was specified. Example usage:\n\tstrings2 -pid 419 > process_strings.txt\n", filter);
				}
			}else if( flagSystemDump )
			{
				// Extract strings from the whole system
				process->dump_system();
			}

			delete process;
		}else if (pipedInput)
		{
			// Set "stdin" to have binary mode:
			int result = _setmode( _fileno( stdin ), _O_BINARY );
			if( result == -1 )
				fprintf(stderr, "Failed to set piped data mode to binary but will continue with processing of piped data." );

			FILE* fh = fdopen(fileno(stdin), "rb");

			if( fh != NULL )
			{
				// Process the piped input
				parser->parse_stream(fh, "piped data");
				fclose(fh);
			}else{
				// Error
				fprintf(stderr, "Invalid stream: %s.\n", "Error opening the piped input: %s.\n", strerror(errno));
			}
		}else if(filter != NULL)
		{
			// Split the filter into the directory and filename filter halves
			char path[MAX_PATH+1] = {'.',0};
			wchar_t* last_slash = wcsrchr( filter, '\\' );
			if( last_slash == NULL || wcsrchr( filter, '/' ) > last_slash )
				last_slash = wcsrchr( filter, '/' );
			
			if( last_slash != NULL )
			{
				// Copy the path
				sprintf_s( path, MAX_PATH+1, "%S", filter );
				path[ last_slash - filter] = 0;

				// Move the filter
				memmove(filter, last_slash + 1, (wcslen(last_slash+1)+1)*2);
			}

			// Process the specified files
			processFolder( path, filter, flagRecursive, parser );
		}
		
		// Cleanup the string parser
		delete parser;
	}
	

	return 0;
}

