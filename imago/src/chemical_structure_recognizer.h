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
#ifndef _chemical_structure_recognizer_h
#define _chemical_structure_recognizer_h

#include "image.h"
#include "character_recognizer.h"
#include "stl_fwd.h"
#include "settings.h"

namespace imago
{
   class Molecule;
   class Segment;
   class CharacterRecognizer;
   
   class ChemicalStructureRecognizer
   {
   public:
      ChemicalStructureRecognizer();
      
      void setImage( Image &img );
      void recognize( Settings& vars, Molecule &mol); 
      void image2mol( Settings& vars, Image &img, Molecule &mol );
	  void extractCharacters (Settings& vars, Image& img);
      const CharacterRecognizer &getCharacterRecognizer() { return  _cr; };

      ~ChemicalStructureRecognizer();

   private:
      CharacterRecognizer _cr;
      Image _origImage;

	  bool removeMoleculeCaptions(const Settings& vars, Image& img, SegmentDeque& layer_symbols, SegmentDeque& layer_graphics);
	  void segmentate(const Settings& vars, Image& img, SegmentDeque& segments, bool connect_mode = false);
	  void storeSegments(const Settings& vars, SegmentDeque& layer_symbols, SegmentDeque& layer_graphics);
	  bool isReconnectSegmentsRequired(const Settings& vars, const Image& img, const SegmentDeque& segments);
      
      ChemicalStructureRecognizer( const ChemicalStructureRecognizer &csr );
   };
}


#endif /* _chemical_structure_recognizer_h */
