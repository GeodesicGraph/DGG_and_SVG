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
#ifndef IPADDRESS_H_
#define IPADDRESS_H_

#include <iostream>
#include <string>

namespace configlib
{

/*
 * ipaddress - Handle IP Addresses in configuration files.
 */
class ipaddress
{	
public:
	ipaddress();
	ipaddress(const std::string& value);
	ipaddress(unsigned long address, long port = -1, unsigned long mask = 0);
	virtual ~ipaddress();
	
	unsigned long get_address() const { return ipaddr; }
	long          get_port()    const { return ipport; }
	unsigned long get_mask()    const { return ipmask; }
	
	void set_address(unsigned long address) { ipaddr = address; }
	void set_port(long port) { ipport = port; }
	void set_mask(unsigned long mask) { ipmask = mask; }
	
	ipaddress& operator=(const ipaddress& rhs);
	ipaddress& operator++() { ++ipaddr; return *this; }
	
	friend std::istream& operator>>(std::istream& is, ipaddress& object);
	friend std::ostream& operator<<(std::ostream& os, const ipaddress& object);
	
protected:
	void parse(const std::string& str);
	void push_parsing(int what_parsing, unsigned long data);
	
private:
	unsigned long ipaddr;
	long		  ipport;
	unsigned long ipmask;
};

/*
 * A useful constant for IP addresses, the loopback address
 */
const static ipaddress LOOPBACK(0x7F000001);

}

#endif /*IPADDRESS_H_*/
