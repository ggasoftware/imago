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
#ifndef _euclidean_graph_h
#define _euclidean_graph_h

#include "boost/bind.hpp"
#include "boost/graph/adjacency_list.hpp"

#include "comdef.h"
#include "vec2d.h"
#include "segment.h"

namespace imago
{
   namespace segments_graph
   {
      typedef boost::property<boost::vertex_pos_t, Vec2d, 
         boost::property<boost::vertex_seg_ptr_t, Segment*> > 
         SegmentsGraphVertexProperties;

      typedef boost::adjacency_list<boost::vecS, boost::vecS,
         boost::undirectedS, SegmentsGraphVertexProperties, 
         boost::property<boost::edge_weight_t, double> > SegmentsGraph;

      typedef boost::property_map<SegmentsGraph, boost::vertex_pos_t>
         VertexPosMap;
      typedef boost::property_map<SegmentsGraph, boost::vertex_seg_ptr_t>
         VertexSegMap;

      typedef boost::graph_traits<SegmentsGraph>::vertex_descriptor Vertex;

      Vertex add_segment( Segment *seg, SegmentsGraph &g );

      template <typename InputIterator>
      inline void add_segment_range( InputIterator begin, InputIterator end,
                                                          SegmentsGraph &g )
      {
         std::for_each(begin, end, boost::bind(&add_segment, _1, boost::ref(g)));
      }
   }
}


#endif /* _segments_graph_h_ */