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

#include "prefilter_basic.h"
#include <opencv2/opencv.hpp>
#include "image.h"
#include "image_utils.h"
#include "pixel_boundings.h"
#include "segment_tools.h"
#include "weak_segmentator.h"
#include "chemical_structure_recognizer.h"
#include "log_ext.h"
#include "filters_list.h"
#include "prefilter_entry.h"

namespace imago
{
	namespace prefilter_basic
	{
		bool prefilterBinarizedFullsize(Settings& vars, Image &image)
		{
			logEnterFunction();		

			getLogExt().appendImage("Source", image);

			int white_count = 0, black_count = 0, others_count = 0;
			for (int y = 0; y < image.getHeight(); y++)
				for (int x = 0; x < image.getWidth(); x++)
					if (image.getByte(x, y) == 0)
						black_count++;
					else if (image.getByte(x, y) == 255)
						white_count++;
					else 
						others_count++;

			getLogExt().append("white_count", white_count);
			getLogExt().append("black_count", black_count);
			getLogExt().append("others_count", others_count);

			if (vars.prefilterCV.MaxNonBWPixelsProportion * others_count < black_count + white_count)
			{	
				getLogExt().appendText("image is binarized");
				if (others_count > 0)
				{
					int gap = vars.prefilterCV.BinarizerFrameGap;
					getLogExt().appendText("Fixup other colors");
					for (int y = 0; y < image.getHeight(); y++)
					{
						for (int x = 0; x < image.getWidth(); x++)
						{
							if (image.getByte(x,y) != 0 && image.getByte(x,y) != 255)
							{
								if (x > gap && y > gap && x + gap < image.getWidth() && y + gap < image.getHeight() &&
									image.getByte(x, y) < vars.prefilterCV.BinarizerThreshold )
								{
									image.getByte(x, y) = 0;
								}
								else
								{
									image.getByte(x, y) = 255;
								}
							}
						}
					}
				}

				if (black_count > 2 * white_count)
				{
					if (white_count == 0)
					{
						getLogExt().appendText("image is probably wrongly loaded");
						return false;
					}
					else
					{
						getLogExt().appendText("image is inversed");
						for (int y = 0; y < image.getHeight(); y++)
						{
							for (int x = 0; x < image.getWidth(); x++)
							{
								if (image.getByte(x,y) == 0)
								{
									image.getByte(x, y) = 255;
								}
								else
								{
									image.getByte(x, y) = 0;
								}
							}
						}
					}
				}

				// this code allows to crop image in rectangular border
				// useful only for 1 image from Image2Structure set
				// but works quite fast.
				if (image.getWidth() > vars.csr.SmallImageDim && image.getHeight() > vars.csr.SmallImageDim)
				{
					WeakSegmentator ws(image.getWidth(), image.getHeight());
					ws.appendData(image);

					Rectangle viewport;
					if (ws.needCrop(vars, viewport, vars.prefilterCV.MaxRectangleCropLineWidthAlreadyBinarized) 
						&& viewport.height > vars.csr.SmallImageDim && viewport.width > vars.csr.SmallImageDim)
					{
						getLogExt().appendText("Crop appended");
						image.crop(viewport.x1(), viewport.y1(), viewport.x2(), viewport.y2());
					}
				}

				getLogExt().appendImage("Result", image);

				return true;
			}
			else
			{
				getLogExt().appendText("image requires processing");
				return false;
			}
		}	

		bool prefilterBasicForceDownscale(Settings& vars, Image &image)
		{
			logEnterFunction();

			double base_ratio = 2.0;
			double rescale_ratio = 1.0;

			{
				double remp_rescale_ratio = std::max(image.cols, image.rows) / vars.csr.RescaleImageDimensions;
				if (remp_rescale_ratio > rescale_ratio)
					rescale_ratio = remp_rescale_ratio;
			}

			if (vars.dynamic.CapitalHeight <= EPS)
			{
				base_ratio = 4.0;
			}
			else
			{
				// 16 gives 93.139
				const double preferred_cap_height = 16.0;
				double temp_base_ratio = vars.dynamic.CapitalHeight / preferred_cap_height;
				if (temp_base_ratio > base_ratio)
					base_ratio = temp_base_ratio;
			}

			double ratio = base_ratio * rescale_ratio;
			getLogExt().append("ratio", ratio);

			cv::resize(image, image, cv::Size((int)(image.cols / ratio), (int)(image.rows / ratio)),
				0.0, 0.0, cv::INTER_AREA);

			return prefilterBasicFullsize(vars, image);
		}

		bool prefilterBasicFullsize(Settings& vars, Image& raw)
		{
			static const int CV_THRESH_BINARY = 0;

			logEnterFunction();

			cv::Mat grayFrame;
			ImageUtils::copyImageToMat(raw, grayFrame);
		
			cv::Mat reduced2x((grayFrame.rows+1)/2, (grayFrame.cols+1)/2, CV_8U);
			cv::pyrDown(grayFrame, reduced2x);

			cv::Mat smoothed2x(grayFrame.rows, grayFrame.cols, CV_8U);
			cv::pyrUp(reduced2x, smoothed2x);		

			cv::Mat strong;
			cv::adaptiveThreshold(smoothed2x, strong, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, (vars.prefilterCV.StrongBinarizeSize) + (vars.prefilterCV.StrongBinarizeSize) % 2 + 1, vars.prefilterCV.StrongBinarizeTresh);
			getLogExt().appendMat("strong", strong);

			cv::Mat weak;
			cv::adaptiveThreshold(smoothed2x, weak,   255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, (vars.prefilterCV.WeakBinarizeSize)   + (vars.prefilterCV.WeakBinarizeSize) % 2 + 1,   vars.prefilterCV.WeakBinarizeTresh);	
			getLogExt().appendMat("weak",   weak);

			cv::Mat otsu;
			if (vars.prefilterCV.UseOtsuPixelsAddition)
			{
				cv::threshold(smoothed2x, otsu, vars.prefilterCV.OtsuThresholdValue, 255, cv::THRESH_OTSU);
				getLogExt().appendMat("otsu",   otsu);
			}

			Image* output = NULL;
			Rectangle viewport;
			int tresholdPassSum = 0, tresholdPassCount = 0;

			int borderX = raw.getWidth()  / vars.prefilterCV.BorderPartProportion + 1;
			int borderY = raw.getHeight() / vars.prefilterCV.BorderPartProportion + 1;

			for (int iter = 0; iter <= (vars.prefilterCV.UseOtsuPixelsAddition ? 1 : 0); iter++)
			{
				Image bin;
				if (iter == 0)
					ImageUtils::copyMatToImage(weak, bin);
				else
					ImageUtils::copyMatToImage(otsu, bin);


				WeakSegmentator ws(raw.getWidth(), raw.getHeight());
				ws.appendData(bin);

				if (output == NULL)
				{
					viewport = Rectangle(0, 0, raw.getWidth(), raw.getHeight());
					Rectangle temp;
					if (ws.needCrop(vars, temp, vars.prefilterCV.MaxRectangleCropLineWidth) &&
						temp.width > vars.csr.SmallImageDim && temp.height > vars.csr.SmallImageDim)
					{
						getLogExt().appendText("Crop appended");
						viewport = temp;
					}

					output = new Image(viewport.width, viewport.height);
					output->fillWhite();
				}

				for (WeakSegmentator::SegMap::const_iterator it = ws.SegmentPoints.begin(); it != ws.SegmentPoints.end(); ++it)
				{
					const Points2i& p = it->second;
		
					int good = 0, bad = 0;
					for (size_t u = 0; u < p.size(); u++)
					{
						if (p[u].x > borderX && p[u].y > borderY
							&& p[u].x < raw.getWidth() - borderX && p[u].y < raw.getHeight() - borderY
							&& strong.at<unsigned char>(p[u].y, p[u].x) == 0)
							good++;
						else
							bad++;
					}

					if (vars.prefilterCV.MaxBadToGoodRatio * good > bad && good > vars.prefilterCV.MinGoodPixelsCount)
					{
						if (getLogExt().loggingEnabled())
						{
							std::map<std::string, int> temp;
							temp["Segment id"] = it->first;
							temp["Good points"] = good;
							temp["Bad points"] = bad;
							getLogExt().appendMap("Append segment", temp);
						}

						for (size_t u = 0; u < p.size(); u++)
						{
							int x = p[u].x - viewport.x;
							int y = p[u].y - viewport.y;
							if (x >= 0 && y >= 0 && x < output->getWidth() && y < output->getHeight())
							{
								tresholdPassSum += raw.getByte(x, y);
								tresholdPassCount++;
								output->getByte(x, y) = 0;
							}
						}
					}
				}
			}

			if (output)
			{
				getLogExt().appendImage("output", *output);

				raw.copy(*output);

				delete output;
				output = NULL;

				return true;
			}

			return false;
		}
	} // end namespace

}

