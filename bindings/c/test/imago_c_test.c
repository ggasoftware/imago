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

#include "imago_c.h"

int main (int argc, char *argv[])
{
   int warnings;
   int ret;
   qword session;

   if (argc < 3)
   {
      fprintf(stderr, "Usage: <source image> <dest molfile>\n");
      return -1;
   }

   session = imagoAllocSessionId();
   imagoSetSessionId(session);

   printf("Loading image...");
   ret = imagoLoadImageFromFile(argv[1]);
   if (ret != 1)
   {
      fprintf(stderr, "Cannot load image\n");
      return -1;
   }
   printf("OK\n");

   printf("Recognizing image...");
   ret = imagoRecognize(&warnings);
   if (ret != 1)
   {
      fprintf(stderr, "Cannot recognize image\n");
      return -1;
   }
   printf("OK\n");
   printf("Number of warnings: %d\n", warnings);

   printf("Saving recongized molecule...");
   ret = imagoSaveMolToFile(argv[2]);
   if (ret != 1)
   {
      fprintf(stderr, "Cannot save molecule\n");
      return -1;
   }
   printf("OK\n");

   imagoReleaseSessionId(session);
   return 0;
}
