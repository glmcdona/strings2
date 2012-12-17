#pragma once

#include <stdio.h>
#include <string.h>

class print_buffer
{
	int buffer_size;
	int space_used;
	char* buffer;
public:
	void addStrings(char* string1, char* string2, char* string3, char* string4, char* string5);
	void addStrings(char* string1, char* string2, char* string3, char* string4);
	void addStrings(char* string1, char* string2, char* string3);
	void addStrings(char* string1, char* string2);
	void addString(char* string, int length);
	void addString(char* string);
	void addLine(char* string, int length);
	void addLine(char* string);
	void digest();
	print_buffer(int buffer_size);
	~print_buffer(void);
};
