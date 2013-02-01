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

#include <cmath>
#include <cstdarg>
#include <algorithm>
#include <string>

#include <opencv2/opencv.hpp>
//#include <opencv/highgui.h>

#include "image.h"
#include "image_utils.h"
#include "image_draw_utils.h"
#include "output.h"
#include "scanner.h"
#include "segment.h"
#include "thin_filter2.h"
#include "vec2d.h"
#include "log_ext.h"
#include "failsafe_png.h"
#include "stat_utils.h"

namespace imago
{
   bool ImageUtils::testSlashLine(const Settings& vars, Segment &img, double *angle, double eps )
   {
	   logEnterFunction();

      double density, thetha, r;

	  getLogExt().appendSegment("segment", img);

      Image tmp;   

      tmp.copy(img);   
      ThinFilter2(tmp).apply();   
   
      thetha = HALF_PI + atan2((double)img.getHeight(), (double)img.getWidth());
      r = 0;
      density = tmp.density();
      ImageDrawUtils::putLine(tmp, thetha, r, eps, 255);
      density = tmp.density() / density;

	  if (density < vars.utils.SlashLineDensity)
      {
         if (angle != 0)
            *angle = thetha;
         return true;
      }

      tmp.copy(img);
      ThinFilter2(tmp).apply();   

      thetha = -thetha;
      r = cos(thetha) * img.getWidth();
      density = tmp.density();
      ImageDrawUtils::putLine(tmp, thetha, r, eps, 255);
      density = tmp.density() / density;
   
      if (density < vars.utils.SlashLineDensity)
      {
         if (angle != 0)
            *angle = thetha;
         return true;
      }

      if (angle != 0)
         *angle = thetha;

      return false;
   }

   void ImageUtils::putSegment( Image &img, const Segment &seg, bool careful )
   {
      int i, j, img_cols = img.getWidth(),
         seg_x = seg.getX(), seg_y = seg.getY(),
         seg_rows = seg.getHeight(), seg_cols = seg.getWidth(),
         img_size = img.getWidth() * img.getHeight();

      for (j = 0; j < seg_rows; j++)
         for (i = 0; i < seg_cols; i++)
         {
			 int y = j+seg_y;
			 int x = i+seg_x;
            //int address = (j + seg_y) * img_cols + (i + seg_x);

            //if (address < img_size)
			 if (y >= 0 && y < img.getHeight() && x >= 0 && x < img.getWidth())
            { 
               if (careful)
               {
				   if (img.getByte(x,y) == 255)
                     img.getByte(x,y) = seg.getByte(i, j);
               }
               else
			   {
                  img.getByte(x,y) = seg.getByte(i, j);
			   }
            }
         }
   }

   void ImageUtils::cutSegment( Image &img, const Segment &seg, bool forceCut, byte val )
   {
      int i, j, img_cols = img.getWidth(),
         seg_x = seg.getX(), seg_y = seg.getY(),
         seg_rows = seg.getHeight(), seg_cols = seg.getWidth();

      for (j = 0; j < seg_rows; j++)
         for (i = 0; i < seg_cols; i++)
         {
			 int y = j+seg_y;
			 int x = i+seg_x;
            //int address = (j + seg_y) * img_cols + (i + seg_x);

			 if (y >= 0 && y < img.getHeight() && x >= 0 && x < img.getWidth())
			 {
				if (seg.getByte(i, j) == 0)
					if (img.getByte(x,y) == 0 || forceCut)
						img.getByte(x,y) = val;
			 }
         }
   }

   void ImageUtils::copyImageToMat ( const Image &img, cv::Mat &mat)
   {
	   img.copyTo(mat);

	   /*
      int w = img.getWidth();
      int h = img.getHeight();

      mat.create(h, w, CV_8U);
      int i, j;

      for (i = 0; i < w; i++)
         for (j = 0; j < h; j++)
            mat.at<unsigned char>(j, i) = img.getByte(i, j);*/
   }

   void ImageUtils::copyMatToImage (const cv::Mat &mat, Image &img)
   {
	   mat.copyTo(img);

	   /*
      int w = mat.cols;
      int h = mat.rows;

      img.init(w, h);
      int i, j;

      for (i = 0; i < w; i++)
         for (j = 0; j < h; j++)
            img.getByte(i, j) = mat.at<unsigned char>(j, i);*/
   }

   void ImageUtils::loadImageFromBuffer( const std::vector<byte> &buffer, Image &img )
   {
      cv::Mat mat = cv::imdecode(cv::Mat(buffer), 0);
      if (mat.empty())
         throw ImagoException("Image data is invalid");
      copyMatToImage(mat, img);
   }

   void ImageUtils::loadImageFromFile( Image &img, const char *format, ... )
   {
	   logEnterFunction();

      char str[MAX_TEXT_LINE];
      va_list args;

      va_start(args, format);   
      vsnprintf(str, sizeof(str), format, args);
      va_end(args);

      const char *FileName = str;
      img.clear(); 
   
      std::string fname(FileName);

      if (fname.length() < 5)
         throw ImagoException("Unknown file format " + fname);

      FILE *f = fopen(fname.c_str(), "r");
      if (f == 0)
         throw FileNotFoundException(fname.c_str());
      fclose(f);

      cv::Mat mat = cv::imread(fname, -1 /*BGRA*/);

      if (mat.empty())
	  {
		  getLogExt().appendText("CV returned empty mat");
		  if (failsafePngLoadFile(fname, img))
		  {			  
			  getLogExt().appendText("... but failsafePngLoad helps");
		  }
		  else
		  {
			  throw ImagoException("Image file is invalid");
		  }
	  }
	  else
	  {
		  if (mat.type() == CV_8UC4)
		  {
			  getLogExt().append("Image type", "CV_8UC4 / BGRA");
			  for (int row = 0; row < mat.rows; row++)
				  for (int col = 0; col < mat.cols; col++)
				  {
					  cv::Vec4b& v = mat.at<cv::Vec4b>(row, col);
					  if (v[3] == 0) // transparent
					  {
						  v[0] = v[1] = v[2] = 255; // to white
					  }
				  }
			  cv::cvtColor(mat, mat, cv::COLOR_BGRA2GRAY);
		  }
		  else if (mat.type() == CV_8UC3)
		  {
			  getLogExt().append("Image type", "CV_8UC3 / BGR");
			  cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
		  }
		  else if (mat.type() == CV_8UC1)
		  {
			  getLogExt().append("Image type", "CV_8UC1 / GRAY");
		  }
		  else
		  {
			  getLogExt().appendText("Unknown image type, attempt to reload as grayscale");
			  mat = cv::imread(fname, 0 /*Grayscale*/);
		  }
		  copyMatToImage(mat, img);
	  }
   }

   void ImageUtils::saveImageToFile( const Image &img, const char *format, ... )
   {
      char str[MAX_TEXT_LINE];
      va_list args;

      va_start(args, format);   
      vsnprintf(str, sizeof(str), format, args);
      va_end(args);

      std::string fname(str);
      cv::Mat mat;
      copyImageToMat(img, mat);
      cv::imwrite(fname, mat);
   }

   void ImageUtils::saveImageToBuffer( const Image &img, const std::string &ext, std::vector<byte> &buffer )
   {
      cv::Mat mat;
      copyImageToMat(img, mat);
      cv::imencode(ext, mat, buffer);
   }

	struct _AngRadius
	{
	   float ang;
	   float radius;
	};

	static int _cmp_ang (const void *p1, const void *p2)
	{
	   const _AngRadius &f1 = *(const _AngRadius *)p1;
	   const _AngRadius &f2 = *(const _AngRadius *)p2;

	   if (f1.ang < f2.ang)
		  return -1;
	   return 1;
	}

   bool ImageUtils::isThinCircle (const Settings& vars, Image &seg, double &radius, bool asChar)
   {
	   logEnterFunction();

	   int w = seg.getWidth();
	   int h = seg.getHeight();
	   int i, j;
	   float centerx = 0, centery = 0;
	   int npoints = 0;

	   for (j = 0; j < h; j++)
	   {
		  for (i = 0; i < w; i++)
		  {
			 if (seg.getByte(i, j) == 0)
			 {
				centerx += i;
				centery += j;
				npoints++;
			 }
		  }
	   }

	   if (npoints == 0)
		  throw ImagoException("Empty fragment");

	   centerx /= npoints;
	   centery /= npoints;

	   _AngRadius *points = new _AngRadius[npoints + 1];
	   int k = 0;
	   float avg_radius = 0;

	   for (i = 0; i < w; i++)
		  for (j = 0; j < h; j++)
		  {
			 if (seg.getByte(i, j) == 0)
			 {
				float radius = sqrt((i - centerx) * (i - centerx) +
									(j - centery) * (j - centery));
				points[k].radius = radius;
				avg_radius += radius;
				float cosine = (i - centerx) / radius;
				float sine = (centery - j) / radius;
				float ang = (float)atan2(sine, cosine);
				if (ang < 0)
				   ang += TWO_PI_f;
				points[k].ang = ang;
				k++;
			 }
		  }

	   qsort(points, npoints, sizeof(_AngRadius), _cmp_ang);
   
	   points[npoints].ang = points[0].ang + TWO_PI_f;
	   points[npoints].radius = points[0].radius;

	   for (i = 0; i < npoints; i++)
	   {
		  float gap = points[i + 1].ang - points[i].ang;
		  float r1 = fabs(points[i].radius);
		  float r2 = fabs(points[i + 1].radius);
		  float gapr = 1.f;

		  if (r1 > r2 && r2 > EPS)
			 gapr = r1 / r2;
		  else if (r2 < r1 && r1 > EPS)
			 gapr = r2 / r1;

		  double c = asChar ? vars.routines.Circle_AsCharFactor : 1.0;

		  if (gapr > vars.routines.Circle_GapRadiusMax * c)
		  {
			  getLogExt().append("Radius gap", gapr);
			 delete[] points;
			 return false;
		  }

		  if (gap > vars.routines.Circle_GapAngleMax * c && gap < 2 * PI - vars.routines.Circle_GapAngleMax * c)
		  {
			  getLogExt().append("C-like gap", gap);
			 delete[] points;
			 return false;
		  }
	   }

	   avg_radius /= npoints;

	   if (avg_radius < vars.routines.Circle_MinRadius)
	   {
		   getLogExt().append("Degenerated circle", avg_radius);
		  delete[] points;
		  return false;
	   }

	   float disp = 0;

	   for (i = 0; i < npoints; i++)
		  disp += (points[i].radius - avg_radius) * (points[i].radius - avg_radius);

	   disp /= npoints;
	   float ratio = sqrt(disp) / avg_radius;

	   #ifdef DEBUG
	   printf("avgr %.3f dev %.3f ratio %.3f\n",
			  avg_radius, sqrt(disp), ratio);
	   #endif

	   getLogExt().append("avg_radius", avg_radius);
	   radius = avg_radius;
	   getLogExt().append("Ratio", ratio);

	   delete[] points;
	   if (ratio > vars.routines.Circle_MaxDeviation)
		  return false; // not a circle
	   return true;
	}


	double ImageUtils::estimateLineThickness(Image &bwimg, int grid)
	{
		int w = bwimg.getWidth();
		int h = bwimg.getHeight();
		int d = grid;

		IntVector lthick;

		if(w < d)
			d = std::max<int>(w >>1, 1) ; 
		{
			int startseg = -1;
			for(int i = 0; i < w ; i += d)
			{
				for(int j = 0; j < h; j++)
				{
					byte val = bwimg.getByte(i, j);
					if(val == 0 && (startseg == -1))
						startseg = j;
					if((val > 0 || j==(h-1)) && startseg != -1)
					{
						lthick.push_back(j - startseg + 1);
						startseg = -1;
					}
				}
			}
		}

		if(h > d)
			d = grid;
		else
			d = std::max<int>(h >>1, 1) ; 

		{
			int startseg = -1;
			for(int j = 0; j< h; j+=d)
			{
				for(int i = 0; i < w; i++)
				{
					byte val = bwimg.getByte(i, j);
					if(val == 0 && (startseg == -1))
						startseg = i;
					if((val > 0 || i==(w-1)) && startseg != -1)
					{
						lthick.push_back(i - startseg + 1);
						startseg = -1;
					}
				}
			}
		}
		std::sort(lthick.begin(), lthick.end());
		double thickness = 0;
		if(lthick.size() > 0)
			thickness = StatUtils::interMean(lthick.begin(), lthick.end());
	
		return thickness;
	}

}