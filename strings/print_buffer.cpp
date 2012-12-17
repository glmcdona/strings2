#include "StdAfx.h"
#include "print_buffer.h"

print_buffer::print_buffer(int buffer_size)
{
	this->buffer_size = buffer_size;
	this->buffer = new char[buffer_size];
	this->space_used = 0;
}

void print_buffer::addString(char* string, int length)
{
	// Digest the buffer if it is full
	if( space_used + length + 1 >= buffer_size )
		digest();

	// Copy the string if there is room
	if( space_used + length + 1 >= buffer_size )
	{
		// Digest this string without buffering it
		fwrite(string, length, 1, stdout);
	}else{
		// Add it to the buffer
		memcpy( buffer + space_used, string, length );
		space_used += length;
		buffer[space_used] = 0;
	}
}

void print_buffer::addString(char* string)
{
	int length = strlen(string);
	addString(string, length);
}

void print_buffer::addStrings(char* string1, char* string2, char* string3, char* string4, char* string5)
{
	addString(string1);
	addString(string2);
	addString(string3);
	addString(string4);
	addString(string5);
}

void print_buffer::addStrings(char* string1, char* string2, char* string3, char* string4)
{
	addString(string1);
	addString(string2);
	addString(string3);
	addString(string4);
}

void print_buffer::addStrings(char* string1, char* string2, char* string3)
{
	addString(string1);
	addString(string2);
	addString(string3);
}

void print_buffer::addStrings(char* string1, char* string2)
{
	addString(string1);
	addString(string2);
}

void print_buffer::addLine(char* string, int length)
{
	// Digest the buffer if it is full
	if( space_used + length + 3 >= buffer_size )
		digest();

	// Copy the string if there is room
	if( space_used + length + 3 >= buffer_size )
	{
		// Digest this string without buffering it
		printf( "%s", string );
	}else{
		// Add it to the buffer
		memcpy( buffer + space_used, string, length );
		space_used += length + 2;
		buffer[space_used - 2] = '\r';
		buffer[space_used - 1] = '\n';
		buffer[space_used] = 0;
	}
}

void print_buffer::addLine(char* string)
{
	int length = strlen(string);
	addLine(string, length);
}

void print_buffer::digest()
{
	if( space_used > 0 )
	{
		// Print the current buffer
		fwrite( buffer, 1, space_used, stdout);
		fflush( stdout );
		buffer[0] = 0;
		space_used = 0;
	}
}

print_buffer::~print_buffer(void)
{
	digest();
	delete[] buffer;
}
