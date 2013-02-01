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
#ifndef _multiple_bond_checker_h
#define _multiple_bond_checker_h

#include "skeleton.h"
#include "settings.h"

namespace imago
{
   class MultipleBondChecker
   {
   private:
      typedef Skeleton::SkeletonGraph Graph;
      typedef Skeleton::Vertex Vertex;
      typedef Skeleton::Edge Edge;
   public:
      MultipleBondChecker(const Settings& vars, Skeleton &s );
      ~MultipleBondChecker();

      bool checkDouble(const Settings& vars, Edge frst, Edge scnd );
      bool checkTriple(const Settings& vars, Edge thrd );
   private:
      Edge first, second, third;
      Vertex fb, fe, sb, se, tb, te;
      Vec2d fb_pos, fe_pos, sb_pos, se_pos, tb_pos, te_pos;
      Bond bf, bs, bt;

      double _multiBondErr;
      double _avgBondLength;
      double _parLinesEps;
      Skeleton &_s;
      Graph &_g;
   };
}

#endif /* _multiple_bond_checker_h */
