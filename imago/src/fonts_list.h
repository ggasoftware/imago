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
#ifndef _fonts_list_h
#define _fonts_list_h

#include <string>
#include <vector>

namespace imago
{
	struct FontEntryDefinition
	{		
		std::string name;
		std::string data;		

		FontEntryDefinition(const std::string& _name, const std::string& _data);
	};

	class FontEntries : public std::vector<FontEntryDefinition>
	{
	public:
		FontEntries();
	};

	FontEntries getFontsList();
}

#endif // _fonts_list_h
