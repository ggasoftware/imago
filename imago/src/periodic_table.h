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
#ifndef _periodic_table_h
#define _periodic_table_h

#include <string>
#include <vector>

namespace imago
{
    static const unsigned char MAX_ATOM_ELEMENT_NUMBER = 109;

    class AtomSymbolMap
    {
		// indexed by string like "Ag" interpretted as unsigned short, the first symbol must be in UPPER case
        unsigned char SymbolMap[0xFFFF];

		// inserts element into 'Elements' and into 'SymbolMap'
		void insert(const std::string& element);

	public:
		std::vector<std::string> Elements;

    public:
        AtomSymbolMap();
        		
		// fast lookup for specified element in table
		bool lookup(const std::string& str) const;
    };

    extern AtomSymbolMap AtomMap;   
}

#endif //_periodic_table_h
