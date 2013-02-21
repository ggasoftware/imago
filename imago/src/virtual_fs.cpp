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

#include "virtual_fs.h"
#include "output.h"

namespace imago
{
	bool VirtualFS::createNewFile(const std::string& filename, const std::string& data)
	{
		VirtualFSRecord rec;
		rec.filename = filename;
		rec.data.insert(rec.data.end(), data.begin(), data.end());
		push_back(rec);
		return true;
	}

	bool VirtualFS::appendData(const std::string& filename, const std::string& data)
	{
		for (size_t u = 0; u < size(); u++)
		{
			if (filename == at(u).filename)
			{
				at(u).data.insert(at(u).data.end(), data.begin(), data.end());
				return true;
			}
		}

		return createNewFile(filename, data);
	}

	void VirtualFS::storeOnDisk(const std::string& folder) const
	{
		for (size_t u = 0; u < size(); u++)
		{
			FileOutput dump((folder + at(u).filename).c_str());
			dump.write(&at(u).data.at(0), at(u).data.size());
		}
	}

	void VirtualFS::setData(std::vector<char>& input)
	{
		clear();
		bool name = true;
		for (size_t idx = 0; idx < input.size(); )
		{
			size_t end = idx + 1;
			while (input[end] != '\n') end++;

			if (name)
			{
				VirtualFSRecord r;
				for (size_t u = idx; u < end; u++)
					r.filename += input[u];
				push_back(r);
			}
			else
			{
				VirtualFSRecord& r = at(size() - 1);
				for (size_t u = idx; u < end; u += 2)
				{
					unsigned char c = (input[u] - 'a') * 16 + (input[u+1] - 'a');
					r.data.push_back(c);
				}
			}

			idx = end+1;
			name = !name;
		}
	}

	void VirtualFS::getData(std::vector<char>& output) const
	{
		output.clear();
		for (size_t u = 0; u < size(); u++)
		{
			output.insert(output.end(), at(u).filename.begin(), at(u).filename.end());
			output.push_back('\n');
			for (size_t v = 0; v < at(u).data.size(); v++)
			{
				char c1 = 'a' + ((at(u).data.at(v) & 0xF0) / 16);
				char c2 = 'a' +  (at(u).data.at(v) & 0x0F);
				output.push_back(c1);
				output.push_back(c2);
			}
			output.push_back('\n');
		}
	}
}