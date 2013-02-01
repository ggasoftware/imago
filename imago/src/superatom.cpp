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

#include "superatom.h"

namespace imago
{
	void Atom::addLabel(const RecognitionDistance& dist)
	{
		labels.push_back(CharacterRecognitionEntry(dist));
	}

	void Atom::addLabel(const char c)
	{
		RecognitionDistance dist;
		dist[c] = 1.0;
		labels.push_back(CharacterRecognitionEntry(dist));
	}

	void Atom::setLabel(const std::string& str)
	{
		labels.clear();
		for (size_t u = 0; u < str.size(); u++)
			addLabel(str[u]);
	}
      
	char Atom::getLabelFirst() const
	{
		if (labels.size() > 0)
			return labels[0].selected_character;
		else
			return 0;
	}

	char Atom::getLabelSecond() const
	{
		if (labels.size() > 1)
			return labels[1].selected_character;
		else
			return 0;
	}
      
    Atom::Atom()
    {
        isotope = count = 0;
        charge = 0;
    }

	std::string Atom::getPrintableForm(bool expanded) const
	{
		const Atom& a = *this;

		std::string molecule;

		if (a.getLabelFirst() != 0) 
			molecule.push_back(a.getLabelFirst());

		if (a.getLabelSecond() != 0) 
			molecule.push_back(a.getLabelSecond());

		if ((a.getLabelFirst() == 'R' || a.getLabelFirst() == 'Z') && a.getLabelSecond() == 0)
		{
			if (a.charge > 0)
			{
				char buffer[32];
				sprintf(buffer, "%i", a.charge);
				molecule += buffer;
			}
		}
		else
		{
			if (expanded && a.charge != 0)
			{
				char buffer[32];
				if (a.charge < 0)
					sprintf(buffer, "%i", a.charge);
				else
					sprintf(buffer, "+%i", a.charge);
				molecule += buffer;
			}					
		}
		if (expanded && a.count != 0)
		{
			char buffer[32];
			sprintf(buffer, "x%i", a.count);
			molecule += buffer;
		}
		if (expanded && a.isotope != 0)
		{
			char buffer[32];
			sprintf(buffer, "{isotope:%i}", a.isotope);
			molecule += buffer;
		}
		return molecule;
	}

	std::string Superatom::getPrintableForm(bool expanded) const
	{
		std::string molecule;
		for (size_t i = 0; i < atoms.size(); i++)
		{
			molecule += atoms[i].getPrintableForm(expanded);
		};
		return molecule;
	}

	Superatom::Superatom(const Atom& a1)
	{
		atoms.push_back(a1);
	}

	Superatom::Superatom(const Atom& a1, const Atom& a2)
	{
		atoms.push_back(a1);
		atoms.push_back(a2);
	}

	Superatom::Superatom(const Atom& a1, const Atom& a2, const Atom& a3)
	{
		atoms.push_back(a1);
		atoms.push_back(a2);
		atoms.push_back(a3);
	}
}