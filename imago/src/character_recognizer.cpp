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

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <map>
#include <cmath>
#include <cfloat>
#include <deque>
#include <opencv2/opencv.hpp>
#include <string.h> // memcpy
#include "stl_fwd.h"
#include "character_recognizer.h"
#include "segment.h"
#include "exception.h"
#include "scanner.h"
#include "segmentator.h"
#include "thin_filter2.h"
#include "image_utils.h"
#include "log_ext.h"
#include "recognition_tree.h"
#include "settings.h"
#include "fonts_list.h"
#include "file_helpers.h"
#include "platform_tools.h"

using namespace imago;

const std::string CharacterRecognizer::upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ$%^&#";
const std::string CharacterRecognizer::lower = "abcdefghijklmnopqrstuvwxyz";
const std::string CharacterRecognizer::digits = "0123456789";
const std::string CharacterRecognizer::brackets = "()[]";
const std::string CharacterRecognizer::charges = "+-";
const std::string CharacterRecognizer::all = CharacterRecognizer::upper + CharacterRecognizer::lower +
	                                         CharacterRecognizer::digits + CharacterRecognizer::charges + 
											 CharacterRecognizer::brackets + "=";
const std::string CharacterRecognizer::graphics = "!";
const std::string CharacterRecognizer::like_bonds = "lL1iVv";


bool imago::CharacterRecognizer::isPossibleCharacter(const Settings& vars, const Segment& seg, bool loose_cmp, char* result)
{
	RecognitionDistance rd = recognize(vars, seg, CharacterRecognizer::all + CharacterRecognizer::graphics);
	
	double best_dist;
	char ch = rd.getBest(&best_dist);

	if (result)
		*result = ch;

	if (CharacterRecognizer::graphics.find(ch) != std::string::npos)
		return false;

	if (CharacterRecognizer::like_bonds.find(ch) != std::string::npos)
	{
		Points2i endpoints = SegmentTools::getEndpoints(seg);
		if ((int)endpoints.size() < vars.characters.MinEndpointsPossible)
		{
			return false;
		}
	}

	if (best_dist < vars.characters.PossibleCharacterDistanceStrong && 
		rd.getQuality() > vars.characters.PossibleCharacterMinimalQuality) 
	{
		return true;
	}

	if (loose_cmp && (best_dist < vars.characters.PossibleCharacterDistanceWeak 
		          && rd.getQuality() > vars.characters.PossibleCharacterMinimalQuality))
	{
		return true;
	}

	return false;
}

qword CharacterRecognizer::getSegmentHash(const Segment &seg) 
{
	logEnterFunction();

	qword segHash = 0, shift = 0;
   
	// hash against the source pixels
	for (int y = 0; y < seg.getHeight(); y++)
	{
		for (int x = 0; x < seg.getWidth(); x++)
			if (seg.getByte(x,y) == 0) // ink
			{
				shift = (shift << 1) + x * 3 + y * 7;
				segHash ^= shift;
			}
	}

	return segHash;
}

void internalInitTemplates(imago::CharacterRecognizerImp::Templates& templates)
{	
	//unsigned int start = platform::TICKS();
	{
		#include "font.inc"
	}
	//printf("Loaded font (%u entries) in %u ms\n", templates.size(), platform::TICKS() - start);
}


RecognitionDistance CharacterRecognizer::recognize(const Settings& vars, const Segment &seg, const std::string &candidates) const
{
	logEnterFunction();
		   
	getLogExt().appendSegment("Source segment", seg);
	getLogExt().append("Candidates", candidates);

	qword segHash = getSegmentHash(seg);	
	getLogExt().append("Segment hash", segHash);
	RecognitionDistance rec;
   
	if (vars.caches.PCacheSymbolsRecognition &&
		vars.caches.PCacheSymbolsRecognition->find(segHash) != vars.caches.PCacheSymbolsRecognition->end())
	{
		rec = (*vars.caches.PCacheSymbolsRecognition)[segHash];
		getLogExt().appendText("Used cache: clean");
	}
	else
	{
		static bool init = false;
		static CharacterRecognizerImp::Templates templates;
		
		if (!init)
		{
			internalInitTemplates(templates);
			init = true;
		}

		rec = CharacterRecognizerImp::recognizeMat(vars, seg, templates);
		getLogExt().appendMap("Font recognition result", rec);

		if (vars.caches.PCacheSymbolsRecognition)
		{
			(*vars.caches.PCacheSymbolsRecognition)[segHash] = rec;
			getLogExt().appendText("Filled cache: clean");
		}
	}

	RecognitionDistance result;

	for (RecognitionDistance::iterator it = rec.begin(); it != rec.end(); it++)
	{
		if (candidates.find(it->first) != std::string::npos)
		{
			result[it->first] = it->second;
		}
	}

	if (getLogExt().loggingEnabled())
	{
		getLogExt().append("Result candidates", result.getBest());
		getLogExt().append("Recognition quality", result.getQuality());
	}

   return result;
}


namespace imago
{
	namespace CharacterRecognizerImp
	{
		typedef std::vector<cv::Point> Points;

		class CircleOffsetPoints : public std::vector<Points>
		{
		public: CircleOffsetPoints(int radius)
			{
				clear();
				resize(radius+1);

				for (int dx = -radius; dx <= radius; dx++)
					for (int dy = -radius; dy <= radius; dy++)
					{
						int distance = imago::round(sqrt((double)(imago::square(dx) + imago::square(dy))));
						if (distance <= radius)
							at(distance).push_back(cv::Point(dx, dy));
					}
			}
		};

		static CircleOffsetPoints offsets(REQUIRED_SIZE);

		void calculatePenalties(const cv::Mat1b& img, unsigned char* penalty_ink, unsigned char* penalty_white)
		{				
			for (int y = -PENALTY_SHIFT; y < REQUIRED_SIZE + PENALTY_SHIFT; y++)
				for (int x = -PENALTY_SHIFT; x < REQUIRED_SIZE + PENALTY_SHIFT; x++)
				{
					for (int value = 0; value <= 255; value += 255)
					{
						double min_dist = REQUIRED_SIZE;
						for (size_t radius = 0; radius < offsets.size(); radius++)
							for (size_t point = 0; point < offsets[radius].size(); point++)
							{
								int j = y + offsets[radius].at(point).y;
								if (j < 0 || j >= img.cols)
									continue;
								int i = x + offsets[radius].at(point).x;
								if (i < 0 || i >= img.rows)
									continue;

								if (img(j,i) == value)
								{
									min_dist = radius;
									goto found;
								}
							}

						found:

						double result = (value == 0) ? min_dist : sqrt(min_dist);
					
						int idx = (y + PENALTY_SHIFT) * INTERNAL_ARRAY_DIM + (x + PENALTY_SHIFT);

						if (value == 0)
							penalty_ink[idx] = std::min(255, CHARACTERS_OFFSET + imago::round(result));
						else
							penalty_white[idx] = std::min(255, CHARACTERS_OFFSET + imago::round(PENALTY_WHITE_FACTOR * result));
					}
				}	
		}

		double compareImages(const cv::Mat1b& img, const unsigned char* penalty_ink, const unsigned char* penalty_white)
		{
			double best = imago::DIST_INF;
			for (int shift_x = 0; shift_x <= 2 * PENALTY_SHIFT; shift_x += PENALTY_STEP)
			{
				for (int shift_y = 0; shift_y <= 2 * PENALTY_SHIFT; shift_y += PENALTY_STEP)
				{
					int sum_ink = 0;
					int sum_white = 0;
					for (int y = 0; y < img.cols; y++)
					{
						for (int x = 0; x < img.rows; x++)
						{
							int value = img(y,x);

							int idx = (y + PENALTY_SHIFT) * INTERNAL_ARRAY_DIM + (x + PENALTY_SHIFT);

							if (value == 0)
							{
								sum_ink += penalty_ink[idx] - CHARACTERS_OFFSET;
							}
							else
							{
								sum_white += penalty_white[idx] - CHARACTERS_OFFSET;
							}
						}
					}

					double result = (double)sum_ink + (double)sum_white / (double)PENALTY_WHITE_FACTOR;
					if (result < best)
						best = result;
				}
			}
			return best;
		}

		cv::Mat1b prepareImage(const Settings& vars, const cv::Mat1b& src, double &ratio)
		{
			imago::Image temp;
			cv::threshold(src, temp, vars.characters.InternalBinarizationThreshold, 255, CV_THRESH_BINARY);
			temp.crop();
			
			if (temp.cols * temp.rows == 0)
				throw ImagoException("Empty mat passed");
		
			int size_y = temp.rows;
			int size_x = temp.cols;

			ratio = (double)size_x / (double)size_y;
		
			size_y = REQUIRED_SIZE;
			size_x = REQUIRED_SIZE;

			cv::resize(temp, temp, cv::Size(size_x, size_y), 0.0, 0.0, cv::INTER_CUBIC);

			cv::threshold(temp, temp, vars.characters.InternalBinarizationThreshold, 255, CV_THRESH_BINARY);
		
			return temp;
		}	

		cv::Mat1b load(const std::string& filename)
		{
			const int CV_FORCE_GRAYSCALE = 0;
			return cv::imread(filename, CV_FORCE_GRAYSCALE);
		}		

		std::string convertFileNameToLetter(const std::string& name)
		{
			std::string temp = name;
			strings levels;

			for (size_t u = 0; u < 3; u++)
			{
				size_t pos_slash = file_helpers::getLastSlashPos(temp);

				if (pos_slash == std::string::npos)
				{
					break;
				}

				std::string sub = temp.substr(pos_slash+1);
				levels.push_back(sub);
				temp = temp.substr(0, pos_slash);			
			}

		
			if (levels.size() >= 3)
			{
				if (lower(levels[2]) == "capital")
					return upper(levels[1]);
				else if (lower(levels[2]) == "special")
					return levels[1];
				else
					return lower(levels[1]);
			}
			else if (levels.size() >= 2)
			{
				return lower(levels[1]);
			}

			// failsafe
			return name;
		}

		bool initializeTemplates(const Settings& vars, const std::string& path, Templates& templates)
		{
			strings files;
			file_helpers::getDirectoryContent(path, files, true);
			file_helpers::filterOnlyImages(files);
			for (size_t u = 0; u < files.size(); u++)
			{
				MatchRecord mr;
				cv::Mat1b image = load(files[u]);
				mr.text = convertFileNameToLetter(files[u]);
				if (!image.empty())
				{
					cv::Mat1b prepared = prepareImage(vars, image, mr.wh_ratio);
					calculatePenalties(prepared, mr.penalty_ink, mr.penalty_white);
					templates.push_back(mr);
				}
			}
			return !templates.empty();
		}

		struct ResultEntry
		{
			double value;
			std::string text;
			ResultEntry(double _value, std::string _text)
			{
				value = _value;
				text = _text;
			}
			bool operator <(const ResultEntry& second) const
			{
				if (this->value < second.value)
					return true;
				else
					return false;
			}
		};

		imago::RecognitionDistance recognizeMat(const Settings& vars, const cv::Mat1b& rect, const Templates& templates)
		{
			imago::RecognitionDistance _result;

			std::vector<ResultEntry> results;
		
			double ratio;
			cv::Mat1b img;
			try
			{
				img = prepareImage(vars, rect, ratio);
			}
			catch (ImagoException& e)
			{
				getLogExt().append("Exception", e.what());
				return _result;
			}

			for (size_t u = 0; u < templates.size(); u++)
			{
				// Imago supports only one-char-length templates, TODO: upgrade
				if (templates[u].text.size() != 1)
					continue;

				try
				{
					double distance = compareImages(img, templates[u].penalty_ink, templates[u].penalty_white);
					double ratio_diff = imago::absolute(ratio - templates[u].wh_ratio);
				
					if (ratio_diff < vars.characters.RatioDiffThresh)
					{
						results.push_back(ResultEntry(distance, templates[u].text));
					}
				}
				catch(std::exception& e)
				{
					printf("Exception: %s\n", e.what());
				}
			}

			if (results.empty())
				return _result;

			std::sort(results.begin(), results.end());

			for (int u = (int)results.size() - 1; u >= 0; u--)
			{
				if (results[u].text.size() == 1) // Imago supports only one-char-length templates
				{
					_result[results[u].text[0]] = results[u].value / vars.characters.DistanceScaleFactor;
				}
			}

			return _result;
		}
	}
}
