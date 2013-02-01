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
#ifndef _graphics_detector_h
#define _graphics_detector_h

#include "vec2d.h"
#include "stl_fwd.h"
#include "label_combiner.h"
#include "settings.h"

namespace imago
{
   struct LineSegment
   {
      Vec2d b;
      Vec2d e;
   };
   
   class Segment;
   class Image;
   class Molecule;
   class BaseApproximator;

   class GraphicsDetector
   {
   public:
      GraphicsDetector();
      GraphicsDetector( const BaseApproximator *approximator, double eps );
      void extractRingsCenters(const Settings& vars, SegmentDeque &segments, Points2d &ring_centers ) const;
      void analyzeUnmappedLabels( std::deque<Label> &unmapped_labels, 
         Points2d &ring_centers );
      void detect(const Settings& vars, const Image &img, Points2d &lsegments ) const;
      ~GraphicsDetector();

   private:
      const BaseApproximator *_approximator;
      double _approx_eps;
      void _decorner( Image &img ) const;
      void _extractPolygon(const Settings& vars, const Segment &seg, Points2d &poly ) const;
      int _countBorderBlackPoints( const Image &img ) const;
      GraphicsDetector( const GraphicsDetector & );
   };
}
#endif /* _graphics_detector_h */

