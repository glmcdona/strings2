// Class for extracted strings
#pragma once
#include <string>
#include <codecvt>
#include "string_model.h"
#include <unordered_set>

using namespace std;

enum STRING_TYPE
{
	TYPE_UNDETERMINED,
	TYPE_UTF8,
	TYPE_WIDE_STRING
};



class extracted_string
{
private:
	STRING_TYPE m_type;
	std::string m_string; // Supports Utf8
	size_t m_size_in_bytes;
	int m_offset_start;
	int m_offset_end;

public:
	extracted_string();
	extracted_string(const char* string, size_t size_in_bytes, STRING_TYPE type, int offset_start, int offset_end);
	extracted_string(const wchar_t* string, size_t size_in_bytes, STRING_TYPE type, int offset_start, int offset_end);

	float get_proba_interesting();
	size_t get_size_in_bytes();
	string get_string();
	STRING_TYPE get_type();
	string get_type_string();
	int get_offset_start();
	int get_offset_end();

	bool is_interesting();

	~extracted_string();
};
