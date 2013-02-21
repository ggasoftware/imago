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
#ifndef _virtual_fs_h
#define _virtual_fs_h

#include <string>
#include <vector>

namespace imago
{
	struct VirtualFSRecord
	{
		std::string filename;
		std::vector<unsigned char> data;
	};

	class VirtualFS : public std::vector<VirtualFSRecord>
	{
	public:
		bool createNewFile(const std::string& filename, const std::string& data);
		bool appendData(const std::string& filename, const std::string& data);		
		
		// gets internal data to external storage
		void getData(std::vector<char>& output) const;

		// sets internal state by specified input
		void setData(std::vector<char>& input);
		
		// if non-empty, the trailing slash is required
		void storeOnDisk(const std::string& folder = "") const;
	};
};

#endif //_virtual_fs_h
