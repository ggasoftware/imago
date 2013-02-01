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
#ifndef _image_utils_h
#define _image_utils_h

#include <vector>
#include "settings.h"

namespace cv 
{
   class Mat;
}

namespace imago
{
   class Image;
   class Segment;

   class ImageUtils
   {
   public:
      static void copyImageToMat( const Image &img, cv::Mat &mat );
      static void copyMatToImage( const cv::Mat &mat, Image &img );

      static void loadImageFromFile( Image &img, const char *FileName, ... );
      static void saveImageToFile( const Image &img, const char *FileName, ... );

      static void loadImageFromBuffer( const std::vector<byte> &buffer, Image &img);
      static void saveImageToBuffer( const Image &img, const std::string &format, std::vector<byte> &buffer );

      static void putSegment( Image &img, const Segment &seg, bool careful = true );
      static void cutSegment( Image &img, const Segment &seg, bool forceCut = false, byte val = 255 );

      static bool testSlashLine(const Settings& vars, Segment &img, double *angle, double eps );
      static bool isThinCircle(const Settings& vars, Image &seg, double &radius, bool asChar = false);
	  static double estimateLineThickness(Image &bwimg, int grid);
   };
}


#endif /* _image_utils_h_ */