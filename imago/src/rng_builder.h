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
#ifndef _rng_builder_h
#define _rng_builder_h

#include "boost/graph/iteration_macros.hpp"
#include "boost/property_map/property_map.hpp"
#include "boost/graph/adjacency_list.hpp"

#include "stl_fwd.h"
#include "comdef.h"

namespace imago
{
   class RNGBuilder
   {
   public:
      template <class EuclideanGraph>
      static void build( EuclideanGraph &g )
      {
         typename boost::property_map<EuclideanGraph, boost::vertex_pos_t>::type 
            positions = boost::get(boost::vertex_pos, g);

         typename boost::property_map<EuclideanGraph, 
            boost::vertex_index_t>::type vert2ind = 
            boost::get(boost::vertex_index, g);

         typename boost::property_map<EuclideanGraph, 
            boost::edge_weight_t>::type weights = 
            boost::get(boost::edge_weight, g);

         int n = boost::num_vertices(g);

         std::vector<typename boost::graph_traits<
                     EuclideanGraph>::vertex_descriptor> ind2vert(n);
         DoubleVector distances(n * n, 0);

         BGL_FORALL_VERTICES_T(v, g, EuclideanGraph)
            ind2vert[vert2ind[v]] = v;

         for (int i = 0; i < n; i++)
         {
            for (int j = 0; j < n; j++)
            {
               distances[i + j * n] = Vec2d::distance(positions[ind2vert[i]],
                                                      positions[ind2vert[j]]);
            }
         }

         for (int i = 0; i < n; i++)
         {
            for (int j = 0; j < n; j++)
            {
               if (i != j)
               {
                  bool add_edge = true;
                  double d = distances[i + j * n];

                  for (int k = 0; k < n; k++)
                  {
                     if (k != i && k != j)
                     {
                        if (d > std::max(distances[i + k * n],
                                         distances[k + j * n]))
                        {
                           add_edge = false;
                           break;
                        }
                     }
                  }

                  if (add_edge)
                  {
                     bool isAdded;
                     typename boost::graph_traits<EuclideanGraph>::edge_descriptor e;

                     boost::tie(e, isAdded) = 
                     boost::add_edge(ind2vert[i], ind2vert[j], g);

                     if (!isAdded)
					 {
						 getLogExt().appendText("Warning: <RNG::build> edge is not added");
					 }

                     weights[e] = distances[i + j * n];
                  }
               }
            }
         }
      }
   };
}


#endif /* _rng_builder_h_ */