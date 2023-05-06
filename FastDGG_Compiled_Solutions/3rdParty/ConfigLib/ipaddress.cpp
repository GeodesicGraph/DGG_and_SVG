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
#include "ipaddress.h"

namespace configlib
{

#define IP_ADDRESS 0
#define IP_MASK 1
#define PORT_NUMBER 8
#define MASK_BITS 9

/*
 * Constructor - default constructor
 */
 ipaddress::ipaddress()
 : ipaddr(0)
 , ipport(-1)
 , ipmask(0)
 {
 }

/*
 * Constructor - parses an ip address in a string
 */
ipaddress::ipaddress(const std::string& value)
{
	parse(value);
}

/*
 * Constructor - Create an ip address numerically.
 */
ipaddress::ipaddress(unsigned long address, long port /*= -1*/, unsigned long mask /*= 0*/)
: ipaddr(address)
, ipport(port)
, ipmask(mask)
{
}

/*
 * Destructor -
 */
ipaddress::~ipaddress()
{
}

/*
 * operator= - assign each value to avoid defaults
 */
ipaddress& ipaddress::operator=(const ipaddress& rhs)
{
	ipaddr = rhs.get_address();
	ipport = rhs.get_port();
	ipmask = rhs.get_mask();
	return *this;
}

/*
 * operator>> - Serialize in the address.  hard limited to 512 bytes, more
 * 		than enough.
 */
std::istream& operator>>(std::istream& is, ipaddress& object)
{
	char address[512];
	is.getline(address, 512);
	object.parse(address);
	return is;
}

/*
 * operator<< - Serialize the address. Output must be in a form that 
 * 		operator>> can handle.
 */
std::ostream& operator<<(std::ostream& os, const ipaddress& object)
{
	int shift;
	
	// Write as a.a.a.a:p m.m.m.m by default.
	// TODO: Make this smarter?
	for (shift = 24;0 <= shift;shift -= 8)
	{
		os << (int)((object.get_address() >> shift) & 0xFF);
		if (0 < shift)
			os << ".";
	}
	// Only ouput a port if it is valid
	if (-1 != object.get_port())
		os << ":" << (int)object.get_port();
	// Only output a mask if there is one.  An all blank mask is
	// meaningless and can therefore be used safely as "invalid."
	if (0 != object.get_mask())
	{
		os << " ";
		for (shift = 24;0 <= shift;shift -= 8)
		{
			os << (int)((object.get_mask() >> shift) & 0xFF);
			if (0 < shift)
				os << ".";
		}
	}
	return os;
}

/*
 * parse - parse an ip address. Valid forms are
 * 		a.a.a.a - fully qualified ip address
 * 		a.a.a   - Class B address with 16 bit bottom
 * 		a.a		- Class A address with 24 bit bottom
 * 		:p		- port number (must appear immediately after ip address)
 * 		m.m.m.m - fully qualified mask
 * 		m.m.m.	- Class C mask with 8 unmasked bits
 * 		m.m.	- Class B mask with 16 unmasked bits
 * 		m.		- Class A mask with 24 unmasked bits
 * 		/m		- number of bits to mask (must appear immediately after
 * 					ip address or port number.
 */
void ipaddress::parse(const std::string& str)
{
	// What are we currently parsing?
	// 0 == ip address
	// 1 == ip mask
	// 8 == port number
	// 9 == mask bits
	// Default to ip address
	int what_parsing = IP_ADDRESS;
	
	int shift = 24;
	
	// hold keeps the temporary value before
	// storing it in the ipaddress
	unsigned long hold = 0;
	
	ipaddr = 0;
	
	//Parse to /, : or space, that is the ip address
	std::size_t pos = str.find_first_of("0123456789.");
	while (pos < str.length())
	{
		if ('.' == str[pos])
		{
			// Shift the input for the current ip address
			// position.
			push_parsing(what_parsing, hold << shift);
			hold = 0;
			shift -=8;
		}
		else if ((' ' == str[pos]) || ('\t' == str[pos])) 
		{
			// Don't shift, since whatever came before is
			// the bottom of an ip address or a port number
			push_parsing(what_parsing, hold);
			// Switch parsing to an ip address mask
			what_parsing = IP_MASK;
			hold = 0;
			shift = 24;
			pos = str.find_first_not_of(" \t", pos);
		}
		else if (':' == str[pos])
		{
			// Don't shift, since whatever came before is
			// the bottom of an ip address or a port number
			push_parsing(what_parsing, hold);
			// Switch to parsing a port number
			what_parsing = PORT_NUMBER;
			hold = 0;
			pos = str.find_first_of("0123456789", pos);
		}
		else if ('/' == str[pos])
		{
			// Don't shift, since whatever came before is
			// the bottom of an ip address or a port number
			push_parsing(what_parsing, hold);
			// Switch to parsing a mask bit count
			what_parsing = MASK_BITS;
			hold = 0;
			pos = str.find_first_of("0123456789", pos);
		}
		else if (('0' <= str[pos]) && ('9' >= str[pos]))
		{
			// Only parse decimal digits for now.  Allowing hex
			// leads to an issue if the address is something
			// like 12.23.34.45 since it is ambiguous if this
			// is hex or decimal.
			hold = ((10 * hold) - '0') + str[pos];
		}
		
		++pos;
	}
}

/*
 * push_parsing - Push the current hold value onto whatever is being parsed.
 */
void ipaddress::push_parsing(int what_parsing, unsigned long data)
{
	if (IP_ADDRESS == what_parsing)
		ipaddr += data;
	else if (IP_MASK == what_parsing)
		ipmask += data;
	else if (PORT_NUMBER == what_parsing)
		ipport = data;
	else if (MASK_BITS == what_parsing)
		ipmask = 0xFFFFFFFF << (32 - data);
}

}
