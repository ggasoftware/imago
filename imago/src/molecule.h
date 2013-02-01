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
#ifndef _molecule_h
#define _molecule_h

#include <map>

#include "comdef.h"
#include "skeleton.h"
#include "label_combiner.h"
#include "settings.h"

namespace imago
{
   class Molecule : public Skeleton
   {
   public:

      typedef std::map<Skeleton::Vertex, Label*> ChemMapping;

      Molecule();

      const ChemMapping &getMappedLabels() const;
      ChemMapping &getMappedLabels();

      const std::deque<Label> &getLabels() const;
      std::deque<Label> &getLabels();

      const SkeletonGraph &getSkeleton() const;
      SkeletonGraph &getSkeleton();

      void aromatize( Points2d &p );

      void mapLabels(const Settings& vars, std::deque<Label> &unmapped_labels );

      void clear();

      ~Molecule();

   private:
      ChemMapping _mapping;
      std::deque<Label> _labels;
      std::vector<Label> labels;
   };
}


#endif /* _molecule_h */
