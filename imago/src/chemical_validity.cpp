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

#include "chemical_validity.h"
#include "comdef.h"
#include "boost/algorithm/string.hpp"
#include "periodic_table.h"
#include "log_ext.h"

namespace imago
{	
	void ChemicalValidity::ElementTableEntry::push_back(const std::string& name, double prob)
	{
		names.push_back(name);
		probability[name] = prob;
	}

	bool ChemicalValidity::isProbable(const std::string& atom) const
	{
		ChemicalValidity::Probabilities::const_iterator it = elements.probability.find(atom);
		if (it != elements.probability.end())
			return it->second > EPS;
		else
			return false;
	}

	ChemicalValidity::Strings ChemicalValidity::optimalSplit(const std::string& input, const ChemicalValidity::Strings& dictionary)
	{
		Strings result;
		for (size_t u = 0; u < dictionary.size(); u++)
		{
			const std::string& word = dictionary[u];
			
			size_t idx = input.find(word);

			if (idx != std::string::npos)
			{
				if (idx > 0)
				{
					std::string s1 = input.substr(0, idx);
					Strings r1 = optimalSplit(s1, dictionary);
					for (size_t p = 0; p < r1.size(); p++)
						result.push_back(r1[p]);
				}

				result.push_back(word);

				if (idx + word.size() < input.size())
				{
					std::string s2 = input.substr(idx + word.size());
					Strings r2 = optimalSplit(s2, dictionary);
					for (size_t p = 0; p < r2.size(); p++)
						result.push_back(r2[p]);
				}

				return result;
			}
		}

		// no dictionary strings found in input, return input as the minimal split
		for (size_t u = 0; u < input.size(); u++)
		{
			std::string temp;
			temp += input[u];
			result.push_back(temp);
		}

		return result;
	}

	double ChemicalValidity::calcSplitProbability(const Strings& split) const
	{
		double result = 1.0;
		for (size_t u = 0; u < split.size(); u++)
		{
			const std::string& item = split[u];
			if (isProbable(item))
			{
				ChemicalValidity::Probabilities::const_iterator it = elements.probability.find(item);
				result *= it->second;				
			}
			else
			{
				result = 0.0;
				break;
			}
		}
		return result;
	}

	double ChemicalValidity::getLabelProbability(const Superatom& sa) const
	{
		logEnterFunction();
		std::string molecule = sa.getPrintableForm(false);
		if (hacks.find(molecule) != hacks.end())
		{
			return 0.0;
		}
		else
		{
			Strings split = optimalSplit(molecule, elements.names);
			getLogExt().appendVector("Split", split);
			return calcSplitProbability(split);
		}
	}

	std::string ChemicalValidity::getAlternatives(char base_char, const RecognitionDistance& d) const
	{
		std::string result = d.getRangedBest(2.0);
		if (result.length() == 1 && result[0] == base_char)
		{
			// TODO: extend
			if (base_char == 'I')
				result += 'H';
			if (base_char == 'Y')
				result += 'H';
			if (base_char == 'E')
				result += 'F';
			if (base_char == 'F')
				result += 'T';
		}
		return result;
	}

	struct IterationRecord
	{
		int counter;
		int atom;
		int pos;
		std::string alts;
	};

	bool ChemicalValidity::optimizeAtomGroup(AtomRefs& data) const
	{
		logEnterFunction();

		// here we know that data contains the references to the bad atoms
		// we can do anything with them.

		// HACK for "R", probably outdated
		if (data.size() == 1 && data[0]->getPrintableForm() == "R")
		{
			return true;
		}

		std::vector<IterationRecord> bruteforce;

		std::string molecule;
		for (size_t u = 0; u < data.size(); u++)
		{
			std::string s = data[u]->getPrintableForm(false);
			molecule += s;
			for (size_t v = 0; v < data[u]->labels.size(); v++)
			{
				IterationRecord irec;
				irec.atom = u;
				irec.pos = v;
				irec.counter = 0;
				irec.alts = getAlternatives(data[u]->labels[v].selected_character, data[u]->labels[v].alternatives);
				getLogExt().append("alternatives", irec.alts);
				bruteforce.push_back(irec);
			}
		}
		
		getLogExt().append("bad part", molecule);

		if (bruteforce.empty())
			return false;

		while (true)
		{
			// increment counter
			bruteforce[0].counter++;
			size_t idx = 0;
			while (bruteforce[idx].counter >= (int)bruteforce[idx].alts.size())
			{
				bruteforce[idx].counter = 0;
				idx++;
				if (idx >= bruteforce.size())
					goto not_found;
				bruteforce[idx].counter++;
			}

			// update atoms
			for (size_t u = 0; u < bruteforce.size(); u++)
			{
				if (!bruteforce[u].alts.empty())
				{
					CharacterRecognitionEntry& cg = data[bruteforce[u].atom]->labels[bruteforce[u].pos];
					cg.selected_character = bruteforce[u].alts[bruteforce[u].counter];
				}
			}

			// assemble string
			std::string test;
			for (size_t u = 0; u < data.size(); u++)
			{
				test += data[u]->getPrintableForm(false);				
			}

			getLogExt().append("check string", test);

			if (calcSplitProbability(optimalSplit(test, elements.names)) > EPS)
			{
				getLogExt().append("passed!", test);
				return true;
			}
		}

		not_found: ;
		
		return false;
	}
		
	void ChemicalValidity::updateAlternative(Superatom& sa) const
	{
		logEnterFunction();

		// step 1: calculate split
		std::string molecule = sa.getPrintableForm(false);
		getLogExt().append("molecule", molecule);
		Strings split = optimalSplit(molecule, elements.names);
		getLogExt().appendVector("split", split);

		const HacksMap::const_iterator hacks_it = hacks.find(molecule);
		if (hacks_it != hacks.end())
		{
			getLogExt().append("Found predefined hack for", molecule);
			sa = hacks_it->second;
			return;
		}
				
		// step 2: assign split to real atoms & regroup letters from same atom
		typedef std::pair<int,int> bad_entry;
		typedef std::vector<bad_entry> bad_info;
		bad_info bad_parts; // (index, length);
		size_t pattern_processed_len = 0;
		size_t atoms_sequence_len = 0;
		size_t atoms_current_index = 0;
		for (size_t u = 0; u < split.size(); u++)
		{
			const std::string& word = split[u];
			bool good = isProbable(word);
			pattern_processed_len += word.length();
			int part_index = atoms_current_index;
			int part_length = 0;
			while (atoms_sequence_len < pattern_processed_len && atoms_current_index < sa.atoms.size())
			{
				atoms_sequence_len += sa.atoms[atoms_current_index].getPrintableForm(false).size();				
				atoms_current_index++;
				part_length++;
			}
			if (!good)
			{
				bad_parts.push_back(std::make_pair(part_index,part_length));
				getLogExt().append("bad part index", part_index);
				getLogExt().append("bad part length", part_length);
			}
		}
		
		// step 3: calculate the most probable (of all possible) combination for each bad atom groups

		std::vector<int> keep;
		for (size_t u = 0; u < sa.atoms.size(); u++)
			keep.push_back(1);

		for (bad_info::iterator it = bad_parts.begin(); it != bad_parts.end(); ++it)
		{
			AtomRefs group_items;
			for (int offset_count = 0; offset_count < it->second; offset_count++)
				group_items.push_back(&sa.atoms[it->first + offset_count]);

			if (!optimizeAtomGroup(group_items))
			{
				// we can not optimize this part, just erase atoms from output
				for (int offset_count = 0; offset_count < it->second; offset_count++)
					keep[it->first + offset_count] = 0;
			}
		}

		// step 4: reconstruct molecule using only valid parts
		Superatom result;
		for (size_t u = 0; u < sa.atoms.size(); u++)
		{
			if (keep[u])
				result.atoms.push_back(sa.atoms[u]);
		}

		sa = result;
	}

	bool sort_comparator_length_reversed(const std::string & p_lhs, const std::string & p_rhs)
    {
        const size_t lhsLength = p_lhs.length();
        const size_t rhsLength = p_rhs.length();
        return (lhsLength == rhsLength) ? (p_lhs < p_rhs) : (lhsLength > rhsLength);
    }

	ChemicalValidity::ChemicalValidity()
	{
		logEnterFunction();

		// append atoms from periodic table
		for (size_t u = 0; u < AtomMap.Elements.size(); u++)
		{
			double pr = (u < 32) ? 1.0 : 0.1;
			elements.push_back(AtomMap.Elements[u],  pr);
		}

		// append also the abbreviations
		elements.push_back("CH",    1.0);
		elements.push_back("OH",    1.0);
		elements.push_back("OTf",   0.7);
		elements.push_back("TfO",   0.7);
		elements.push_back("AcO",   0.7);
		elements.push_back("OAc",   0.7);
		elements.push_back("OAC",   0.7);
		elements.push_back("NHBoc", 0.7);
		elements.push_back("NHBOC", 0.7);
		elements.push_back("OMe",   0.7);		
		elements.push_back("Boc",   0.2);
		elements.push_back("tBu",   0.2);
		elements.push_back("R1",    0.1);
		elements.push_back("R2",    0.1);
		elements.push_back("R3",    0.1);
		elements.push_back("Z1",    0.1);		
		elements.push_back("X",     0.1);
		elements.push_back("Me",    0.2);
		elements.push_back("Et",    0.2); // whats that?

		elements.push_back("(",     0.01);
		elements.push_back(")",     0.01);

		// and also fixup these bad combinations
		elements.push_back("IN", 0.0);
		
	/*	
		// hacks usage, something like that:
		Atom N;   N.setLabel("N");
		Atom H2; H2.setLabel("H"); H2.count = 2;
		hacks["NHH"] = Superatom(N, H2);
	*/

		// elements should be sorted for optimal split function working
		std::sort(elements.names.begin(), elements.names.end(), sort_comparator_length_reversed);		
	}
};
