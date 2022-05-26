#pragma once
#include <string>
#include "windows.h"
#include "print_buffer.h"
#include "binary2strings.hpp"

using namespace std;

constexpr auto MAX_STRING_SIZE = 0x2000;
constexpr auto BLOCK_SIZE = 0x50000;

struct STRING_OPTIONS
{
	bool print_utf8;
	bool print_wide_string;
	bool print_string_type;
	bool print_interesting;
	bool print_not_interesting;
	bool print_filename;
	bool print_filepath;
	bool print_span;
	wstring json_file_output;
	int min_chars;
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
	
	STRING_OPTIONS m_options;
	print_buffer* m_printer;
	
public:
	string_parser( STRING_OPTIONS options );
	bool parse_block( unsigned char* buffer, unsigned int buffer_length, string data_source );
	bool parse_stream( FILE* fh, string data_source );
	~string_parser(void);
};