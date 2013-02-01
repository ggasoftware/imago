/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include <cstring>
#include <ctype.h>
#include "label_logic.h"
#include "segment.h"
#include "character_recognizer.h"
#include "label_combiner.h"
#include "image_utils.h"
#include "log_ext.h"
#include "chemical_validity.h"

using namespace imago;

const std::string exact_as_lower = "OSWVPZXCV"; 
const std::string can_not_be_capital = "XJUVWQ";

LabelLogic::LabelLogic( const CharacterRecognizer &cr ) : _cr(cr), _satom(NULL), _cur_atom(NULL)
{
}

LabelLogic::~LabelLogic()
{
}

void LabelLogic::setSuperatom( Superatom *satom )
{
	_cur_atom = NULL;
	_satom = satom;
	_addAtom();
}

std::string substract(const std::string& fullset, const std::string& reduction)
{
	std::string result;
	for (std::string::const_iterator it = fullset.begin(); it != fullset.end(); ++it)
		if (reduction.find(*it) == std::string::npos)
			result += *it;
	return result;
}

void LabelLogic::process_ext(const Settings& vars, Segment *seg, int line_y )
{
	logEnterFunction();
	getLogExt().appendSegmentWithYLine(vars, "segment with baseline", *seg, line_y);

	RecognitionDistance pr = _cr.recognize(vars, *seg);

	{ 
		// adjust using image params
		if (line_y >= 0)
		{
			double underline = SegmentTools::getPercentageUnderLine(*seg, line_y);
			getLogExt().append("Percentage under baseline", underline);
			pr.adjust(1.0 - vars.labels.weightUnderline * (underline - vars.labels.underlinePos), CharacterRecognizer::digits);		
		}
	
		if (vars.dynamic.CapitalHeight > 0)
		{
			double ratio = (double)SegmentTools::getRealHeight(*seg) / (vars.dynamic.CapitalHeight - 1);
			getLogExt().append("Height ratio", ratio);
			double base = 1.0 - vars.labels.ratioWeight * 1.0;
			pr.adjust(base + vars.labels.ratioWeight * ratio , CharacterRecognizer::lower + CharacterRecognizer::digits);	
		
			// complicated cases: there are some symbols with exactly the same representation in lower and upper cases
			if (ratio > vars.labels.ratioCapital)
			{				
				std::string lower_em = lower(exact_as_lower);
				if (lower_em.find(pr.getBest()) != std::string::npos)
				{
					pr.adjust(vars.labels.capitalAdjustFactor, exact_as_lower);
				}
			}
		}
	}

	{ 
		// adjust using chemical structure logic
		if (_cur_atom->getLabelFirst() != 0 && _cur_atom->getLabelSecond() == 0)
		{
			// can be a capital or digit or small
			int idx = _cur_atom->getLabelFirst() - 'A';
			if (idx >= 0 && idx < 26)
			{
				if (_cur_atom->getLabelFirst() == 'C')
					pr.adjust(vars.labels.adjustInc, "l");
				// decrease probability of unallowed characters
				// TODO: not implemented
				//pr.adjust(vars.labels.adjustDec, substract(CharacterRecognizer::lower, comb[idx]));		
			}
		} 
		else if (_cur_atom->getLabelFirst() == 0)
		{
			// should be a capital letter, increase probability of allowed characters			
			pr.adjust(vars.labels.adjustInc, substract(CharacterRecognizer::upper, can_not_be_capital));
		}
	}

	int attempts_count = 0;

	retry:

	if (attempts_count++ > vars.labels.adjustAttemptsCount)
	{
		getLogExt().append("Probably unrecognizable. Attempts count reached", attempts_count);
		return;
	}

	getLogExt().append("Ranged best candidates", pr.getRangedBest());
	getLogExt().append("Quality", pr.getQuality());

	char ch = pr.getBest();
	if (CharacterRecognizer::upper.find(ch) != std::string::npos) // it also includes 'tricky' symbols
	{
		_addAtom();
		if (!_multiLetterSubst(ch)) // 'tricky'
		{
			getLogExt().append("Added first label", ch);
			_cur_atom->addLabel(pr);
		}
		else
		{
			getLogExt().append("Done multichar subst", ch);
		}		
	} 
	else if (CharacterRecognizer::lower.find(ch) != std::string::npos)
	{		
		if (_cur_atom->getLabelSecond() != 0)
		{
			getLogExt().appendText("Small letter comes after another small, fixup & retry");
			pr.adjust(vars.labels.adjustDec, CharacterRecognizer::lower);
			goto retry;
		}
		else if (_cur_atom->getLabelFirst() == 0)
		{
			getLogExt().appendText("Small specified for non-set captial, fixup & retry");
			pr.adjust(vars.labels.adjustDec, CharacterRecognizer::lower);
			goto retry;
		}
		else
		{
			getLogExt().append("Added second label", ch);
			_cur_atom->addLabel(pr);
		}
	}
	else if (CharacterRecognizer::digits.find(ch) != std::string::npos)
	{
		if (_cur_atom->count != 0)
		{
			getLogExt().appendText("Count specified twice, fixup & retry");
			pr.adjust(vars.labels.adjustDec, CharacterRecognizer::digits);
			goto retry;
		}
		else if (_cur_atom->getLabelFirst() == 0)
		{
			getLogExt().appendText("Count specified for non-set atom, fixup & retry");
			pr.adjust(vars.labels.adjustDec, CharacterRecognizer::digits);
			goto retry;
		}
		else
		{
			int digit = ch - '0';
			if (_cur_atom->getLabelFirst() == 'R' && _cur_atom->getLabelSecond() == 0)
			{
				_cur_atom->charge = _cur_atom->charge*10 + digit;
				getLogExt().append("Initialized R-group index", _cur_atom->charge);
			}
			else
			{
				_cur_atom->count = _cur_atom->count*10 + digit;
				getLogExt().append("Initialized atom count", _cur_atom->count);
			}
		}
	}
	else if (CharacterRecognizer::brackets.find(ch) != std::string::npos) // brackets
	{
		_addAtom();
		_cur_atom->addLabel(pr);		
		getLogExt().append("Added bracket", ch);
	}
	else if (CharacterRecognizer::charges.find(ch) != std::string::npos) // charges
	{
		_cur_atom->charge = (ch == '+') ? +1 : -1;
		getLogExt().append("Initialized atom charge", _cur_atom->charge);
		// TODO: charges like "-3"
	}
	else 
	{
		getLogExt().append("Current char not in supported set, increase probability of supported ones", ch);
		pr.adjust(vars.labels.adjustInc, CharacterRecognizer::all);
		goto retry;
	}
}

void LabelLogic::_addAtom()
{
	if (_cur_atom && _cur_atom->getLabelFirst() == 0 && _cur_atom->getLabelSecond() == 0)
		return;

	_satom->atoms.resize(_satom->atoms.size() + 1);
	_cur_atom = &_satom->atoms[_satom->atoms.size() - 1];
}

bool LabelLogic::_multiLetterSubst(char sym)
{
	switch(sym)
	{
	case '$':
		_cur_atom->setLabel("Cl");
		return true;
	case '%':
		_cur_atom->setLabel("H");
		_cur_atom->count = 2;
		return true;
	case '^':
		_cur_atom->setLabel("H");
		_cur_atom->count = 3;
		return true;
	case '&':
		_cur_atom->setLabel("O");
		_addAtom();
		_cur_atom->setLabel("C");
		return true;
	case '#':
		_cur_atom->setLabel("N");
		_addAtom();
		_cur_atom->setLabel("H");
		return true;
	case '=':
		// special chars to ignore
		return true;
	}

	return false;
}

void LabelLogic::_postProcessLabel(Label& label)
{
	logEnterFunction();

	Superatom &sa = label.satom;

	if (sa.atoms.size() > 0)
	{		
		getLogExt().append("Molecule", sa.getPrintableForm());

		ChemicalValidity validator;
		double pr = validator.getLabelProbability(sa);
		getLogExt().append("probability", pr);

		if (pr < EPS)
		{
			getLogExt().appendText("Got wrong label!");	
			validator.updateAlternative(sa);
			getLogExt().append("Used as alternative", sa.getPrintableForm());
		}
	}

	// old hack removing extra hydrogen
	if (sa.atoms.size() == 2)
	{
		for (size_t i = 0; i < 2; i++)
		{
			Atom &atom = sa.atoms[i];
			if (atom.getLabelFirst() == 'H' && atom.getLabelSecond() == 0 &&
				atom.charge == 0 && atom.isotope == 0)
			{
				sa.atoms.erase(sa.atoms.begin() + i);
				break;
			}
		}
	}
	
	// more safe hack for empty labels
	if (sa.atoms.size() == 0)
	{
		Atom placeholder;
		placeholder.setLabel("H");
		sa.atoms.push_back(placeholder);
	}
	else if (sa.atoms.size() == 1 && sa.atoms[0].getLabelFirst() == 0)
	{
		sa.atoms[0].setLabel("H");
	}
}


void LabelLogic::recognizeLabel(const Settings& vars, Label& label )
{
   logEnterFunction();

   setSuperatom(&label.satom);

   {
	   Segment temp(vars.general.ImageWidth, vars.general.ImageHeight, 0, 0);
	   temp.fillWhite();
	   for (size_t i = 0; i < label.symbols.size(); i++)
		   ImageUtils::putSegment(temp, *label.symbols[i]);
	   getLogExt().appendSegmentWithYLine(vars, "Source label", temp, label.baseline_y);
   }

   getLogExt().append("symbols count", label.symbols.size());
   if (label.multiline)
   {
	   getLogExt().appendText("Multiline label");
   }
   else
   {
	   getLogExt().append("label.baseline_y", label.baseline_y);
   }

   for (size_t i = 0; i < label.symbols.size(); i++)
   {
      try
      {       
		  process_ext(vars, label.symbols[i], label.multiline ? -1 : label.baseline_y);
      }
      catch(ImagoException &e)
      {
		  getLogExt().append("Exception", e.what());
      }      
   }

   _postProcessLabel(label);
}
