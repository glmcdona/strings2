#pragma once
#include <string>
#include "windows.h"
#include "print_buffer.h"
#include "binary2strings.hpp"
#include "json.hpp"
#include <algorithm>
#include <errno.h>

using namespace std;

constexpr auto MAX_STRING_SIZE = 0x2000;
constexpr auto BLOCK_SIZE = 5e+7; // 50MB

struct STRING_OPTIONS
{
	bool print_utf8 = true;
	bool print_wide_string = true;
	bool print_string_type = false;
	bool print_interesting = true;
	bool print_not_interesting = false;
	bool print_filename = false;
	bool print_filepath = false;
	bool print_span = false;
	bool print_json = false;
	bool escape_new_lines = false;
	int min_chars = 4;
	size_t offset_start = 0;
	size_t offset_end = 0;
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
	bool parse_block( unsigned char* buffer, unsigned int buffer_length, string name_short, string name_long, unsigned long long base_address);
	bool parse_stream( FILE* fh, string name_short, string name_long);
	~string_parser(void);
};