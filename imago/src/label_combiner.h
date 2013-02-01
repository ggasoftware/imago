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
#ifndef _label_combiner_h
#define _label_combiner_h

#include <deque>
#include "stl_fwd.h"
#include "superatom.h"
#include "rectangle.h"
#include "segment.h"
#include "settings.h"

namespace imago
{
   class Segment;
   class Font;
   class CharacterRecognizer;

   struct Label
   {
      std::vector<Segment*> symbols;
      Rectangle rect;
      int baseline_y;
	  bool multiline;

      Superatom satom;

      Label()
      {
         baseline_y = 0;
		 multiline = false;
      }

	  int MaxSymbolWidth() const
	  {
		  int result = 0;
		  std::vector<Segment*>::const_iterator it;
		  for (it = symbols.begin(); it != symbols.end(); ++it)
			  if ((*it)->getWidth() > result)
				  result = (*it)->getWidth();
		  return result;
	  }
   };

   class LabelCombiner
   {
   public:
      LabelCombiner(Settings& vars, SegmentDeque &symbols_layer, SegmentDeque &other_layer, const CharacterRecognizer &cr );
      ~LabelCombiner();
      
	  void extractLabels( std::deque<Label> &labels );

   private:
      SegmentDeque &_symbols_layer;
	  SegmentDeque &_graphic_layer;
      const CharacterRecognizer &_cr;
	  double _capHeightStandardDeviation;

      std::deque<Label> _labels;
      void _locateLabels(const Settings& vars);
      void _fillLabelInfo(const Settings& vars, Label &l );
      static bool _segmentsComparator( const Segment* const &a,
                                       const Segment* const &b );

      static bool _segmentsCompareX( const Segment* const &a,
                                     const Segment* const &b );
      
   };
}
#endif /* _label_combiner_h */

