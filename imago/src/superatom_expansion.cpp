/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "boost/foreach.hpp"
#include "boost/algorithm/string.hpp"

#include "superatom_expansion.h"

#include <stdlib.h>
#include "indigo.h"
#include "molecule.h"
#include "output.h"
#include "molfile_saver.h"
#include "log_ext.h"

namespace imago
{

std::string expandSuperatoms(const Settings& vars, const Molecule &molecule )
{
   logEnterFunction();

   std::string molString;
   ArrayOutput so(molString);
   MolfileSaver ma(so);
   ma.saveMolecule(vars, molecule);
   
   if (!vars.general.ExpandAbbreviations)
      return molString;

   indigoSetOption("treat-x-as-pseudoatom", "true");
   indigoSetOption("ignore-stereochemistry-errors", "true");

   int mol = indigoLoadMoleculeFromString(molString.c_str());

   if (mol == -1)
   {
      fprintf(stderr, "%s\n", indigoGetLastError());
      return molString;
   }

   int expCount = indigoExpandAbbreviations(mol);
   if (expCount == -1)
   {
      fprintf(stderr, "%s\n", indigoGetLastError());
      return molString;
   }

   std::string newMolfile = indigoMolfile(mol);
   indigoFree(mol);

   return newMolfile;
}

}
