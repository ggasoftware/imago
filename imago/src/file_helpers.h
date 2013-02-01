/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 * 
 * This file is part of Imago toolkit.
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

#ifndef _file_helpers_h_
#define _file_helpers_h_

#include <string>
#include <vector>

typedef std::vector<std::string> strings;

namespace file_helpers
{
	size_t getLastSlashPos(const std::string& filename);
	bool getReferenceFileName(const std::string& image, std::string& output);
	bool getOnlyFileName(const std::string& image, std::string& output);
	int  getDirectoryContent(const std::string& dir, strings &files, bool recursive);
	bool isSupportedImageType(const std::string& filename);
	void filterOnlyImages(strings& files);
}

#endif // _file_helpers_h_
