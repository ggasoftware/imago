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
#ifndef _approximator_h
#define	_approximator_h

#include "vec2d.h"
#include "stl_fwd.h"
#include "settings.h"

namespace imago
{
   class BaseApproximator
   {
   public:
       virtual void apply(const Settings& vars, double eps, const Points2d &input, Points2d &output ) const = 0;
   };

   class CvApproximator: public BaseApproximator
   {
   public:
      void apply(const Settings& vars, double eps, const Points2d &input, Points2d &output ) const;
   };

   class DPApproximator: public BaseApproximator
   {
      void _apply_int( double eps, const Points2d &input, Points2d &output ) const; 
   public:
      void apply(const Settings& vars, double eps, const Points2d &input, Points2d &output ) const;
   };
}
#endif /* _approximator_h */
