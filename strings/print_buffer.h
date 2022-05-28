#pragma once

#include <stdio.h>
#include <string>

using namespace std;

class print_buffer
{
	bool m_is_start = true;
	bool m_add_json_close = false;
	int m_buffer_size;
	int m_space_used;
	char* m_buffer;
public:
	void add_string(const char* string, size_t length);
	void add_string(const char* string);
	void add_string(string string);

	void add_json_string(string json);

	void digest();
	print_buffer(int buffer_size);
	~print_buffer(void);
};
