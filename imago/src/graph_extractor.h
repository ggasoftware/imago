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
#ifndef _graph_extractor_h
#define _graph_extractor_h

#include "stl_fwd.h"
#include "settings.h"

namespace imago
{
   class Graph;
   class Skeleton;
   class Image;
   class GraphicsDetector;
   
   struct GraphExtractor
   {
      static void extract( Settings& vars, const GraphicsDetector &gd,
                           const SegmentDeque &segments, Skeleton &graph );

      static void extract( Settings& vars, const GraphicsDetector &gd,
                           const Image &img, Skeleton &graph );
   };
}


#endif /* _graph_extractor_h */
