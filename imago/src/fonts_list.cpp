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

#include "fonts_list.h"

namespace imago
{
	FontEntryDefinition::FontEntryDefinition(const std::string& _name, const std::string& _data)
	{
		name = _name;
		data = _data;
	}

	FontEntries::FontEntries()
	{
		#define LENGTH(x) sizeof(x) / sizeof(int)
		
		// TODO: currently not used, decide: eliminate or transform
		/*{
			const char* data[] = 
			{
				#include "imago.font.inc"
			};

			std::string temp;
			for (int i = 0; i < (sizeof(data) / sizeof(char*)) && data[i] != 0; i++)
			{
				temp += data[i];
			}

			push_back(FontEntryDefinition("base", temp)); 			
		}*/
	}

	FontEntries getFontsList()
	{
		static FontEntries result;
		return result;
	}
};