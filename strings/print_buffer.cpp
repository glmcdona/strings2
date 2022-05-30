#include "StdAfx.h"
#include "print_buffer.h"
#include <string>

print_buffer::print_buffer(int buffer_size)
{
	this->m_buffer_size = buffer_size;
	this->m_buffer = new char[buffer_size];
	this->m_space_used = 0;
}

void print_buffer::add_string(const char* string, size_t length)
{
	// Digest the buffer if it is full
	if( m_space_used + length + 1 >= m_buffer_size )
		digest();

	// Copy the string if there is room
	if( m_space_used + length + 1 >= m_buffer_size )
	{
		// Digest this string without buffering it
		fwrite(string, length, 1, stdout);
	}else{
		// Add it to the buffer
		memcpy( m_buffer + m_space_used, string, length );
		m_space_used += length;
		m_buffer[m_space_used] = 0;
	}
}

void print_buffer::add_string(const char* string)
{
	int length = strlen(string);
	add_string(string, length);
}

void print_buffer::add_string(string string)
{
	add_string(string.c_str(), string.length());
}

void print_buffer::add_json_string(string json)
{
	// Json string building is formed as an array of json objects.
	// We prepend "[" at the first log, comma delmit, then post-fix a "]" upon completion.
	if (m_is_start)
	{
		add_string("[" + json);
		m_is_start = false;
		m_add_json_close = true;
	}
	else
	{
		add_string("," + json);
	}
}

void print_buffer::digest()
{
	if( m_space_used > 0 )
	{
		// Print the current buffer
		fwrite( m_buffer, 1, m_space_used, stdout);
		fflush( stdout );
		m_buffer[0] = 0;
		m_space_used = 0;
	}
}

print_buffer::~print_buffer(void)
{
	if (m_add_json_close)
		add_string("]");
	digest();
	delete[] m_buffer;

	
}
