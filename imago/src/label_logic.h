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

#pragma once
#ifndef _label_logic_h
#define	_label_logic_h

#include "superatom.h"
#include "character_recognizer.h"
#include "settings.h"

namespace imago
{
   struct Superatom;
   class Segment;
   class CharacterRecognizer;
   struct Label;

   class LabelLogic
   {
   public:
      LabelLogic( const CharacterRecognizer &cr );
      ~LabelLogic();
            
      void recognizeLabel(const Settings& vars, Label &label );

   protected:
	  void process_ext(const Settings& vars, Segment *seg, int line_y );

   private:	  
      const CharacterRecognizer &_cr;
      Superatom *_satom;
      Atom *_cur_atom;
      LabelLogic( LabelLogic & );

	  void _addAtom();
	  void setSuperatom( Superatom *satom );
      void _predict(const Settings& vars,  const Segment *seg, std::string &letters );      
	  void _postProcessLabel(Label& label);
	  bool _multiLetterSubst(char sym);
   };
}

#endif /* _label_logic_h */

