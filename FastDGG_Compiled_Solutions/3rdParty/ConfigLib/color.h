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
#ifndef COLOR_H_
#define COLOR_H_

#include <string>
#include <iostream>

namespace configlib
{

/*
 * color - Handle colors in configuration files.
 */
class color
{
public:
	color(unsigned long value = 0);
	color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha = 0);
	virtual ~color();
	
	/*
	 * Access for program
	 */
	unsigned long get_value() const { return clvalue; }
	void set_value(unsigned long value) { clvalue = value; }
	
	/*
	 * Needed by the configitem template
	 */
	color& operator=(const color& rhs) { clvalue = rhs.get_value(); return *this; }
	color& operator++() { ++clvalue; return *this; }
	
	/*
	 * Serialization implementation
	 */
	friend std::istream& operator>>(std::istream& is, color& object);
	friend std::ostream& operator<<(std::ostream& os, const color& object);
	
protected:
	void parse(const std::string& str);
	void push_parsing(int channel, unsigned long hold);
	
private:
	unsigned long clvalue;
};

}

#endif /*COLOR_H_*/
