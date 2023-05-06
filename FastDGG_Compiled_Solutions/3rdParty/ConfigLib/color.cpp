/*  The following is the Simplified BSD License
 * 
 * Copyright (c) 2008, Warren Kurt vonRoeschlaub
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions 
 * are met:
 * 
 *  * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer 
 *       in the documentation and/or other materials provided with the 
 *       distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "color.h"

namespace configlib
{

/*
 * Constructor - color from long
 */
color::color(unsigned long value /* = 0 */)
: clvalue(value)
{
}

/*
 * Constructor - color from rgb/a
 */  
color::color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha /*= 0*/)
: clvalue(((unsigned long)alpha << 24) | ((unsigned long)red << 16) |
		  ((unsigned long)green << 8) | (unsigned long)blue) 
{
}

/*
 * Destructor
 */
color::~color()
{
}

/*
 * operator>> - Read from stream
 */
std::istream& operator>>(std::istream& is, color& object)
{
	char line[512];
	is.getline(line, 512);
	object.parse(line);
	return is;
}

/*
 * operator>> - Write out rgb/a format
 */
std::ostream& operator<<(std::ostream& os, const color& object)
{
	// Write as "r g b" triple or "r g b a" quad if the alpha channel
	// is non-zero
	os << (int)((object.get_value() >> 16) & 0x000000FF) << " ";
	os << (int)((object.get_value() >> 8) & 0x000000FF) << " ";
	os << (int)(object.get_value() & 0x000000FF);
	if (object.get_value() & 0xFF000000)
		os << " " << (int)((object.get_value() >> 24) & 0x000000FF);
	return os;
}

/*
 * parse - Parse color from a string. Valid forms are
 *     #xxxxxxxx - HTML style color code
 *     r g b     - rgb triple (use 0x to do hexadecimal)
 *     r g b a   - rgb triple with alpha value
 *     l         - long value unsigned int color (use 0x for hexadecimal)
 */
void color::parse(const std::string& str)
{
	// The current channel being read, or -1 for the full value.
	// See the function push_parsing for a complete list.
	// Start with red = channel 1
	int channel = 1;
	// The currently held value for the current channel
	unsigned long hold = 0;
	// Start in base 10
	int base = 10;
	
	// Scan the input for the first non-space character.
	std::size_t pos = str.find_first_not_of(" \t");
	while (pos < str.length())
	{
		if ('#' == str[pos])
		{
			// Web style color
			base = 16;
			hold = 0;
			// Use channel -1 to indicate next channel is alpha
			channel = -1;
		}
		else if (('x' == str[pos]) || ('X' == str[pos]))
			base = 16;
		else if ((' ' == str[pos]) || ('\t' == str[pos]))
		{
			// A number is complete, push it into the current slot
			push_parsing(channel, hold);
			channel++;
			hold = 0;
			base = 10;
		}
		else if (('0' <= str[pos]) && ('9' >= str[pos]))
			hold = ((hold * base) - '0') + str[pos];
		else if (('A' <= str[pos]) && ('F' >= str[pos]))
			hold = ((hold * base) - 'A' + 10) + str[pos];
		else if (('a' <= str[pos]) && ('f' >= str[pos]))
			hold = ((hold * base) - 'a' + 10) + str[pos];
		
		pos++;
	}
	
	// Push the final hold value
	if (0 < hold)
		push_parsing(channel, hold);
}

/*
 * push_parsing - push the current hold value into the current channel
 */
void color::push_parsing(int channel, unsigned long hold)
{
	switch (channel)
	{
		case -1 : // All bits
			clvalue = hold;
			break;
			
		case 1 : // Red channel
			clvalue = (clvalue & 0xFF00FFFF) | (hold << 16);
			break;
			
		case 2 : // Green channel
			clvalue = (clvalue & 0xFFFF00FF) | (hold << 8);
			break;
			
		case 3 : // Blue channel
			clvalue = (clvalue & 0xFFFFFF00) | hold;
			break;
		
		case 0 : // Alpha channel for all bits
		case 4 : // Alpha channel
			clvalue = (clvalue & 0x00FFFFFF) | (hold << 24);
			break;
	}
}

}
