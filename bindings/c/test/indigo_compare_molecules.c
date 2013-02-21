/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

 #include <stdio.h>

#include "indigo.h"

int compareMolecules (const char *m1_filename, const char *m2_filename)
{
   int m1 = indigoLoadMoleculeFromFile(m1_filename);
   int m2 = indigoLoadMoleculeFromFile(m1_filename);
   if (indigoExactMatch(m1, m2, "") == 0)
      return 0;
   return 1;
}

int main (int argc, char *argv[])
{
   if (argc < 2)
   {
      fprintf(stderr, "Usage: <source molfile> <reference file>\n");
      return -1;
   }

   if (compareMolecules(argv[2], argv[3]) == 0)
   {
      fprintf(stderr, "Recognized molecule doesn't match: %s and %s\n", argv[2], argv[3]);
      return -1;
   }
   printf("Matched.\n");

   return 0;
}
