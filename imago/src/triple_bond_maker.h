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
#ifndef _triple_bond_maker_h
#define _triple_bond_maker_h

#include "boost/tuple/tuple.hpp"
#include "skeleton.h"
#include "settings.h"

namespace imago
{
   class TripleBondMaker
   {
   private:
      typedef Skeleton::Vertex Vertex;
      typedef Skeleton::Edge Edge;
      typedef Skeleton::SkeletonGraph Graph;
   public:
      TripleBondMaker( const Settings& settings, Skeleton &s );

      typedef boost::tuple<int, Edge, Edge> Result;

      Result operator()( boost::tuple<Edge, Edge, Edge> edges );
      
      virtual ~TripleBondMaker();
   private:
      TripleBondMaker( const TripleBondMaker & );

      Edge first, second, third;
      Edge empty;
      Vertex fb, fe, sb, se, tb, te;
      
      Result _validateVertices();
      
      Skeleton &_s;
      Graph &_g;
      double _avgBondLength;

	  const Settings& vars;
   };
}

#endif /* _triple_bond_maker_h */

