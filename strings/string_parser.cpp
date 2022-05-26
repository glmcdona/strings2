#include "StdAfx.h"
#include "string_parser.h"
#include <errno.h>

bool string_parser::parse_block(unsigned char* buffer, unsigned int buffer_length, string data_source)
{
	if( buffer != NULL && buffer_length > 0)
	{
		// Process this buffer
		return this->processContents( buffer, buffer_length, data_source);
	}
	return false;
}

string_parser::string_parser(STRING_OPTIONS options)
{
	m_printer = new print_buffer(0x100000);
	this->m_options = options;
}

bool string_parser::parse_stream(FILE* fh, string data_source)
{
	if( fh != NULL )
	{
		
		unsigned char* buffer;
		int numRead;

		// Allocate the buffer
		buffer = new unsigned char[BLOCK_SIZE];

		do
		{
			// Read the stream in blocks of 0x50000, assuming that a string does not border the regions.
			numRead = fread( buffer, 1, BLOCK_SIZE, fh);

			if( numRead > 0 )
			{
				// We have read in the full contents now, lets process it.
				this->processContents( buffer, numRead, data_source);
			}
		}while( numRead == BLOCK_SIZE );

		// Clean up
		delete[] buffer;
		return true;
	}else{
		// Failed to open file
		fprintf(stderr,"Invalid stream: %s.\n", strerror(errno));
		return false;
	}
}

string_parser::~string_parser(void)
{
	delete m_printer;
}
