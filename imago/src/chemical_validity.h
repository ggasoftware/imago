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
#ifndef _chemical_validity_h
#define _chemical_validity_h

#include <vector>
#include <string>
#include <map>
#include "superatom.h"

namespace imago
{
	class ChemicalValidity
	{
	public:
		// fills internal elements table
		ChemicalValidity();
		
		// returns probability of superatom existence
		double getLabelProbability(const Superatom& sa) const;

		// updates the non-existent atom to the most close existent alternative
		void updateAlternative(Superatom& sa) const;

	private:
		typedef std::vector<std::string> Strings;
		typedef std::map<std::string, double> Probabilities;

		struct ElementTableEntry
		{
			Strings names;
			Probabilities probability;

			// helper function to initialize both names and probability 
			void push_back(const std::string& name, double prob = 1.0);
		};
	
		ElementTableEntry elements;
		typedef std::map<std::string, Superatom> HacksMap;
		HacksMap hacks;

	protected:
		// returns optimal string split by specified dictionary
		static Strings optimalSplit(const std::string& input, const Strings& dictionary);

		// calculates split probability against the 'elements' information
		double calcSplitProbability(const Strings& split) const;

		// returns if atom (short form, one atom) is probable
		bool isProbable(const std::string& atom) const;

		// returns list of alternative characters
		std::string getAlternatives(char base_char, const RecognitionDistance& d) const;

		typedef std::vector<Atom*> AtomRefs;
		// subtask of the updateAlternative function
		bool optimizeAtomGroup(AtomRefs& data) const;
	};
}

#endif
