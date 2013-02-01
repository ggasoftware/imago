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

#include "segments_graph.h"

namespace imago
{
   namespace segments_graph
   {
      Vertex add_segment( Segment *seg, SegmentsGraph &g )
      {
         Vec2d pos = seg->getCenter();

         Vertex v = add_vertex(g);
         VertexPosMap::type positions = boost::get(boost::vertex_pos, g);
         VertexSegMap::type segments = boost::get(boost::vertex_seg_ptr, g);
         positions[v] = pos;
         segments[v] = seg;
         return v;
      }
   }
}