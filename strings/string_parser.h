#pragma once
#include <string>
#include "windows.h"
#include "DynArray.h"
#include "print_buffer.h"
#include "string_hashes.h"

using namespace std;

#define MAX_STRING_SIZE 0x2000
#define BLOCK_SIZE 0x50000


// Quick way of checking if a character value is displayable ascii
static bool isAscii[0x100] =
	/*          0     1     2     3        4     5     6     7        8     9     A     B        C     D     E     F     */
	/* 0x00 */ {false,false,false,false,   false,false,false,false,   false,true ,true ,false,   false,true ,false,false,
	/* 0x10 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0x20 */  true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,
	/* 0x30 */  true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,
	/* 0x40 */  true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,
	/* 0x50 */  true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,
	/* 0x60 */  true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,
	/* 0x70 */  true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,true ,   true ,true ,true ,false,
	/* 0x80 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0x90 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0xA0 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0xB0 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0xC0 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0xD0 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0xE0 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false,
	/* 0xF0 */  false,false,false,false,   false,false,false,false,   false,false,false,false,   false,false,false,false};

struct STRING_OPTIONS
{
	bool printAsciiOnly;
	bool printUnicodeOnly;
	bool printFile;
	bool printType;
	bool printNormal;
	bool printASM;
	bool printUniqueLocal;
	bool printUniqueGlobal;
	bool escapeNewLines;
	int minCharacters;
};

class string_parser
{
	// Maybe add XOR methods for extracting strings?
	enum EXTRACT_TYPE
	{
	  EXTRACT_RAW,
	  EXTRACT_ASM
	};

	enum STRING_TYPE
	{
	  TYPE_UNDETERMINED,
	  TYPE_ASCII,
	  TYPE_UNICODE
	};
	
	STRING_OPTIONS options;
	print_buffer* printer;
	

	int extractImmediate( char* immediate, int immediateSize, STRING_TYPE &stringType, unsigned char* outputString );
	int extractString( unsigned char*  buffer, long bufferSize, long offset, unsigned char* outputString, int outputStringSize, int &outputStringLength, EXTRACT_TYPE &extractType, STRING_TYPE & stringType);
	bool processContents( unsigned char* buffer, long numRead, LPCSTR filepath );
public:
	string_parser( STRING_OPTIONS options );
	bool parse_block( unsigned char* buffer, unsigned int buffer_length, LPCSTR datasource );
	bool parse_stream( FILE* fh, LPCSTR datasource );
	~string_parser(void);
};