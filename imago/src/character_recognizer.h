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

#pragma once
#ifndef _character_recognizer_h
#define _character_recognizer_h

#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include "segment.h"
#include "stl_fwd.h"
#include "recognition_distance.h"
#include "segment_tools.h"
#include "settings.h"

namespace imago
{
   class Segment;   
	
   class CharacterRecognizer
   {
   public:
	  bool isPossibleCharacter(const Settings& vars, const Segment& seg, 
		                       bool loose_cmp = false, char* result = NULL);  

      RecognitionDistance recognize(const Settings& vars, const Segment &seg, 
									const std::string &candidates = all) const;

	  virtual ~CharacterRecognizer() { };

      static const std::string upper; 
	  static const std::string lower;
	  static const std::string digits;
	  static const std::string charges;
	  static const std::string brackets;
	  static const std::string all;
	  static const std::string graphics;	  
	  static const std::string like_bonds;

   private:
	   static qword getSegmentHash(const Segment &seg);
   };

   namespace CharacterRecognizerImp
   {
	   	const int REQUIRED_SIZE = 30;
		const int PENALTY_SHIFT = 1;
		const int PENALTY_STEP  = 1;

		// used for technical reasons, do not modify
		const int PENALTY_WHITE_FACTOR = 32;
		const int CHARACTERS_OFFSET = 32;
		const int INTERNAL_ARRAY_DIM = REQUIRED_SIZE + 2*PENALTY_SHIFT;
		const int INTERNAL_ARRAY_SIZE = INTERNAL_ARRAY_DIM * INTERNAL_ARRAY_DIM;

		struct MatchRecord
		{
			unsigned char penalty_ink[INTERNAL_ARRAY_SIZE];
			unsigned char penalty_white[INTERNAL_ARRAY_SIZE];
			std::string text;
			double wh_ratio;
		};

		typedef std::vector<MatchRecord> Templates;

		void calculatePenalties(const cv::Mat1b& img, unsigned char* penalty_ink, unsigned char* penalty_white);
		double compareImages(const cv::Mat1b& img, const unsigned char* penalty_ink, const unsigned char* penalty_white);
		cv::Mat1b prepareImage(const Settings& vars, const cv::Mat1b& src, double &ratio);
		bool initializeTemplates(const Settings& vars, const std::string& path, Templates& templates);		
		RecognitionDistance recognizeMat(const Settings& vars, const cv::Mat1b& image, const Templates& templates);		
   };
}

#endif /* _character_recognizer_h */
