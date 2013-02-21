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
#ifndef _filters_list_h
#define _filters_list_h

#include <string>
#include <vector>
#include "image.h"
#include "settings.h"

namespace imago
{
	struct FilterEntryDefinition
	{
		typedef bool(*ConditionFunction)(const Image&);
		typedef bool(*FilterFunction)(Settings&, Image&);

		std::string name;
		std::string update_config_string;
		int priority;
		ConditionFunction condition;
		FilterFunction routine;
		
		FilterEntryDefinition(const std::string& _name, int _priority, FilterFunction _f, 
			                  const std::string& _config = "", ConditionFunction _c = NULL);
	};

	class FilterEntries : public std::vector<FilterEntryDefinition>
	{
	public:
		FilterEntries();
	};

	FilterEntries getFiltersList();
}

#endif // _filters_list_h
