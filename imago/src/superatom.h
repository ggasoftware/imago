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
#ifndef _superatom_h
#define _superatom_h

#include <vector>
#include <string>
#include "vec2d.h"
#include "recognition_tree.h"

namespace imago
{
	struct Atom
	{
		// list of used characters with their probabilities
		CharactersRecognitionGroup labels;	   

		// adds a character to atom label by probability table
		void addLabel(const RecognitionDistance& dist);

		// adds a character to atom label by exact value
		void addLabel(const char c);

		// sets the whole label by specified string
		void setLabel(const std::string& str);
      
		// returns first label char (the most probable: selected_character) or NULL (if none specified)
		char getLabelFirst() const;

		// returns second label char (the most probable: selected_character) or NULL (if none specified)
		char getLabelSecond() const;

		// chemical configuration
		int charge, isotope, count;

		// returns printable form of the atom's content
		// expanded - use chemical configuration too
		std::string getPrintableForm(bool expanded = true) const;
		      
		Atom();
	};

	struct Superatom
	{
		// list of used atoms in order
		std::vector<Atom> atoms;

		// returns printable form of the whole molecule content
		// apply 'expanded' rule to all subatoms
		std::string getPrintableForm(bool expanded = true) const;

		Superatom() {};

		// some useful constructors to create superatom from atoms fast
		Superatom(const Atom& a1);
		Superatom(const Atom& a1, const Atom& a2);
		Superatom(const Atom& a1, const Atom& a2, const Atom& a3);
	};
}


#endif	/* _superatom_h */
