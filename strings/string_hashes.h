#pragma once
#include <iostream>
#include <string>
#include <unordered_set>
#include "DynArray.h"
#include <stdio.h>
#include <io.h>

using namespace std;
using namespace std::tr1;

 // 10 mb
#define START_SIZE 10000000

// 1 mb
#define HASH_SIZE 1000000

struct unordered_eqstr
{
    bool operator()(const char* s1, const char* s2) const
    {
        return strcmp(s1, s2) == 0;
    }
};

struct unordered_deref
{
    size_t operator()(const char* p) const
    {
        return hash<string>()(p);
    }
};

class string_hashes
{
	// Create the hashsets
	unordered_set<string> _GlobalHashset;
	unordered_set<string> _LocalHashset;
	char* _data_block_start;
	char* _data_block_cur;
	char* _data_block_end;
	DynArray<char*> blocks;

	char* _clone_string(char* source, int length)
	{
		if( _data_block_cur + length + 1 >= _data_block_end )
		{
			// Allocate a new block
			_data_block_start = new char[START_SIZE];
			_data_block_cur = _data_block_start;
			_data_block_end = _data_block_start + START_SIZE - 1;
			blocks.Add( _data_block_start );
		}

		// Copy the string
		char* result = _data_block_cur;
		memcpy( _data_block_cur, source, length + 1 );
		_data_block_cur[length] = 0;
		_data_block_cur += length + 1;
		
		return result;
	}

public:
	string_hashes(void)
	{
		_data_block_start = new char[START_SIZE];
		_data_block_cur = _data_block_start;
		_data_block_end = _data_block_start + START_SIZE - 1;
		blocks.Add( _data_block_start );

		_LocalHashset.rehash(HASH_SIZE);
		_GlobalHashset.rehash(HASH_SIZE);
	}

	string_hashes(char* file_globalhashes)
	{
		/*_data_block_start = new char[START_SIZE];
		_data_block_cur = _data_block_start;
		_data_block_end = _data_block_start + START_SIZE - 1;
		blocks.Add( _data_block_start );

		FILE* stream = fopen( file_globalhashes, "rb" );
		if( stream != NULL )
		{
			// Deserialize the file to rebuild the hashtable
			while( !feof(stream) )
			{
				// Read the length of this string
				DWORD length = 0;
				if( fread( &length, 4, 1, stream ) > 0 )
				{
					char* string = new char[length + 1];
					if( fread( string, 1, length + 1, stream ) > 0 )
					{
						// Add this to the hashtable
						Global_Insert( string(string, length );
					}
					delete[] string;
				}
			}

			fclose(stream);
		}*/
	}

	void Serialize(char* file_globalhashes)
	{
		/*
		FILE* stream = fopen( file_globalhashes, "wb" );
		if( stream != NULL )
		{
			for (unordered_set<const char*, unordered_deref, unordered_eqstr>::const_iterator it = _GlobalHashset.begin();
				it != _GlobalHashset.end(); ++it) 
			{
				DWORD length = strlen(*it);
				fwrite( &length, 4, 1, stream );
				fwrite( *it, 1, length + 1, stream);
			}

			fclose(stream);
		}
		*/
	}
	
	bool Contains(string value)
	{
		if( _LocalHashset.count( value ) > 0 ||
			_GlobalHashset.count( value ) > 0 )
			return true;
		return false;
	}

	void Local_Clear()
	{
		_LocalHashset.clear();
	}

	void Local_Insert( string value )
	{
		// Insert a copy of this string
		if( _LocalHashset.size() + 10 > _LocalHashset.max_size() )
			_LocalHashset.rehash(_LocalHashset.max_size() + HASH_SIZE);
		_LocalHashset.insert( value );
	}

	void Global_Clear()
	{
		_GlobalHashset.clear();
	}

	void Global_Insert(string value)
	{
		// Insert a copy of this string
		if( _GlobalHashset.size() + 10 > _GlobalHashset.max_size() )
			_GlobalHashset.rehash(_GlobalHashset.max_size() + HASH_SIZE);
		_GlobalHashset.insert( value );
	}
	
	~string_hashes(void)
	{
		Local_Clear();
		Global_Clear();

		// Clean up the allocated blocks
		
	}
};
