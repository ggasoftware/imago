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
#ifndef _double_bond_maker_h
#define _double_bond_maker_h

#include "boost/graph/graph_traits.hpp"
#include "boost/tuple/tuple.hpp"
#include "skeleton.h"
#include "settings.h"

namespace imago
{
   class DoubleBondMaker
   {
   private:
      typedef Skeleton::Vertex Vertex;
      typedef Skeleton::Edge Edge;
      typedef Skeleton::SkeletonGraph Graph;
   public:
      DoubleBondMaker( const Settings& settings, Skeleton &s );

      typedef boost::tuple<int, Edge, Edge> Result;

      Result operator()( std::pair<Edge, Edge> edges );
      
      virtual ~DoubleBondMaker();
   private:
      Edge empty;
      Edge first, second;
      Bond bf, bs;
      Vertex fb, fe, sb, se;
      Vec2d fb_pos, fe_pos, sb_pos, se_pos;
      
      Result _validateVertices();
      
      DoubleBondMaker( const DoubleBondMaker & );

      Result _simple();

      Result _hard();

      void _disconnect( Vertex a, Vertex b, const Vertex *third );
      Skeleton &_s;
      Graph &_g;
      double _avgBondLength;
	  
	  const Settings& vars;
   };

   template <class Graph> class DoubleBondComparator
   {
   public:
      DoubleBondComparator( Graph &g ) : _g(g)
      {
         types = boost::get(boost::edge_type, _g);
      }
      typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
      int operator() ( const std::pair<Edge, Edge> &a,
                       const std::pair<Edge, Edge> &b )
      {
         double average_len_a = 0.5 * (types[a.first].length + types[a.second].length);
         double average_len_b = 0.5 * (types[b.first].length + types[b.second].length);
         return average_len_a > average_len_b;
      }
   private:
      typename boost::property_map<Graph, boost::edge_type_t>::type
         types;
      Graph &_g;
   };
}

#endif /* _double_bond_maker_h */

