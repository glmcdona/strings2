#include "StdAfx.h"
#include "string_parser.h"

using namespace nlohmann;

bool string_parser::parse_block(unsigned char* buffer, unsigned int buffer_length, string name_short, string name_long)
{
	if( buffer != NULL && buffer_length > 0)
	{
		// Process this buffer
		vector<std::tuple<string, string, std::pair<int, int>, bool>> r_vect = extract_all_strings(buffer, buffer_length, this->m_options.min_chars, !this->m_options.print_not_interesting);

		
		if (m_options.print_json)
		{
			// Output the strings to a json file
			json j;
			j["name_short"] = name_short;
			j["name_long"] = name_long;
			for (int i = 0; i < r_vect.size(); i++)
			{
				j["strings"][i]["string"] = std::get<0>(r_vect[i]);
				j["strings"][i]["type"] = std::get<1>(r_vect[i]);
				j["strings"][i]["span"] = { std::get<2>( r_vect[i]).first, std::get<2>(r_vect[i]).second };
				j["strings"][i]["is_interesting"] = std::get<3>(r_vect[i]);
			}
			this->m_printer->add_json_string(j.dump());
		}
		else
		{
			// Iterate through the resulting strings, printing them
			for (int i = 0; i < r_vect.size(); i++)
			{
				bool is_interesting = std::get<3>(r_vect[i]);
				if (is_interesting && m_options.print_interesting ||
					!is_interesting && m_options.print_not_interesting)
				{
					// Add the prefixes as appropriate
					if (m_options.print_filepath)
						this->m_printer->add_string(name_long + ",");

					if (m_options.print_filename)
						this->m_printer->add_string(name_short + ",");

					if (m_options.print_string_type)
						this->m_printer->add_string(std::get<1>(r_vect[i]) + ",");

					if (m_options.print_span)
					{
						string span = "(" + to_string(std::get<2>(r_vect[i]).first) + "," + to_string(std::get<2>(r_vect[i]).second) + ")" + ",";
						this->m_printer->add_string(span);
					}

					this->m_printer->add_string(std::get<0>(r_vect[i]) + "\n");
				}
			}
		}
	}
	return false;
}

string_parser::string_parser(STRING_OPTIONS options)
{
	m_printer = new print_buffer(0x100000);
	this->m_options = options;
}

bool string_parser::parse_stream(FILE* fh, string name_short, string name_long)
{
	if( fh != NULL )
	{
		unsigned char* buffer;
		int num_read;
		long long offset = 0;

		// Adjust the start offset if specified
		if (m_options.offset_start > 0)
			fseek(fh, m_options.offset_start, SEEK_SET);

		// Allocate the buffer
		buffer = new unsigned char[BLOCK_SIZE];

		do
		{
			// Read the stream in blocks of 0x50000, assuming that a string does not border the regions.
			if (m_options.offset_end > 0)
			{
				num_read = fread(buffer, 1, min(BLOCK_SIZE, m_options.offset_end - m_options.offset_start), fh);
			}
			else
			{
				num_read = fread(buffer, 1, BLOCK_SIZE, fh);
			}
			

			if( num_read > 0 )
			{
				// We have read in the full contents now, lets process it.
				if( offset > 0 )
					this->parse_block( buffer, num_read, name_short, name_long + ":offset=" + to_string(offset));
				else
					this->parse_block(buffer, num_read, name_short, name_long);

				offset += num_read;
			}

			this->m_printer->digest();
		}while( num_read == BLOCK_SIZE );

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
