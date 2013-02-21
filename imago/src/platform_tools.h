/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#pragma once
#ifndef _platform_tools_h
#define _platform_tools_h

#include <string>

namespace platform
{
	// returns error code
	int MKDIR(const std::string& directory); 

	// current time ticks in milliseconds
	unsigned int TICKS(); 

	// current available memory in kilobytes
	unsigned int MEM_AVAIL(); 

	// platform-depent line ending string
	std::string getLineEndings(); 

	// returns 'return-code' or negative error code
	int CALL(const std::string& executable, const std::string& parameters, int timelimit = 0);

	// returns true if memory allocation of 'amount' MB is failed. memory is released instantly.
	bool checkMemoryFail(int amount = 32);
}

#endif //_platform_tools_h
