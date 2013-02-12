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
