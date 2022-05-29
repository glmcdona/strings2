// Class for extracted strings
#include "stdafx.h"
#include "extracted_string.hpp"

using namespace std;

std::wstring_convert<std::codecvt_utf8<wchar_t>> _converter;

extracted_string::extracted_string()
{
	m_type = TYPE_UNDETERMINED;
	m_string = (std::string)NULL;
	m_size_in_bytes = 0;
	m_offset_start = 0;
	m_offset_end = 0;
}

extracted_string::extracted_string(const char* string, size_t size_in_bytes, STRING_TYPE type, int offset_start, int offset_end)
{
	m_type = type;
	m_string = std::string(string, size_in_bytes);
	m_size_in_bytes = size_in_bytes;
	m_offset_start = offset_start;
	m_offset_end = offset_end;
}

extracted_string::extracted_string(const wchar_t* string, size_t size_in_bytes, STRING_TYPE type, int offset_start, int offset_end)
{
	m_type = type;

	// Convert to UTF8 string
	m_string = _converter.to_bytes(string, string + size_in_bytes / 2);
	//m_string = _wchar_to_utf8(string, size_in_bytes);

	m_size_in_bytes = size_in_bytes;
	m_offset_start = offset_start;
	m_offset_end = offset_end;
}

float extracted_string::get_proba_interesting()
{
	// Returns a probability of the string being interesting, 0.0 to 1.0.
	// An interesting string is non-gibberish. Gibberish is mostly erroneous
	// short extracted strings from binary content.

	// The model is trained to only support strings of length 4 to 7. Longer
	// strings are asssumed to be interesting, shorter assumed gibberish..
	int l = m_string.length();
	if (l > 16)
		return 1.0f;
	if (l < 4)
		return 0.0f;

	// Score the features
	// 	118 character unigrams (character ranges 0x9 to 0x7e)
	// 	118 + 118*118 character bigrams
	// 	1 for the total number of characters in string
	// 	1 for the total number of > 0x128 ascii code
	//  1 for distinct character count
	float score = string_model::bias;
	unordered_set<wchar_t> cc; // Character counts 
	for (size_t i = 0; i < l; i++)
	{
		// Count distinct characters
		cc.insert(m_string[i]);

		if (m_string[i] >= 0x9 && m_string[i] <= 0x7E)
		{
			// Unigram
			score += string_model::weights[m_string[i] - 0x9];

			// Bigram
			if (i + 1 < l && m_string[i + 1] >= 0x9 && m_string[i + 1] <= 0x7E)
			{
				score += string_model::weights[118 + (m_string[i] - 0x9) + 118 * (m_string[i + 1] - 0x9)];
			}
		}
		else
		{
			// Number of non-latin unicode characters
			score += string_model::weights[118 + 118 + 118 * 118 + 1];
		}
	}

	// Add the string length weight
	score += string_model::weights[118 + 118 + 118 * 118] * (float)l;

	// Add the distinct character count weight
	score += string_model::weights[118 + 118 + 118 * 118 + 2] * (float)cc.size();

	// Convert it to a probability
	return 1.0f / (1.0f + exp(-score));
}

size_t extracted_string::get_size_in_bytes()
{
	return m_size_in_bytes;
}

string extracted_string::get_string()
{
	return m_string;
}

bool extracted_string::is_interesting()
{
	return get_proba_interesting() > 0.5f;
}

STRING_TYPE extracted_string::get_type()
{
	return m_type;
}

string extracted_string::get_type_string()
{
	if (m_type == TYPE_UTF8)
	{
		return "UTF8";
	}
	else if (m_type == TYPE_WIDE_STRING)
	{
		return "WIDE_STRING";
	}
	else
	{
		return "UNDETERMINED";
	}
}

int extracted_string::get_offset_start()
{
	return m_offset_start;
}

int extracted_string::get_offset_end()
{
	return m_offset_end;
}

extracted_string::~extracted_string()
{
	// Nothing to do
}