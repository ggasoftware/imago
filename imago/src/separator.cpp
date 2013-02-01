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

#include <list>
#include <vector>
#include <algorithm>
#include <deque>
#include <cstdio>
#include <cmath>
#include <stack>
#include <queue>

#include "boost/foreach.hpp"
#include <opencv2/opencv.hpp>

#include "approximator.h"
#include "comdef.h"
#include "log_ext.h"
#include "graphics_detector.h"
#include "exception.h"
#include "image.h"
#include "image_utils.h"
#include "image_draw_utils.h"
#include "separator.h"
#include "segment.h"
#include "segmentator.h"
#include "stat_utils.h"
#include "thin_filter2.h"
#include "graph_extractor.h"
#include "stat_utils.h"
#include "algebra.h"
#include "character_recognizer.h"
#include "molecule.h"
#include "probability_separator.h"

using namespace imago;

Separator::Separator( SegmentDeque &segs, const Image &img ) : _segs(segs), _img(img)
{
   std::sort(_segs.begin(), _segs.end(), _segmentsComparator);    
}

void _getHuMomentsC(const Image &img, double hu[7])
{
   int w = img.getWidth(), h = img.getHeight();
   cv::Mat mat(h, w, CV_8U);
   for (int k = 0; k < h; k++)
   {
      for (int l = 0; l < w; l++)
      {
         mat.at<imago::byte> (k, l) = 255 - img.getByte(l, k);
      }
   }
   cv::Moments moments = cv::moments(mat, true);
   cv::HuMoments(moments, hu);
}

int Separator::HuClassifier(const Settings& vars, Image &im)
{
	double hu[7];
	_getHuMomentsC(im, hu);

	if (hu[1] > vars.separator.hu_1_1 || (hu[1] < vars.separator.hu_1_2 && hu[0] < vars.separator.hu_0_1))
		return SEP_BOND;
	if (hu[1] < vars.separator.hu_1_3 && hu[0] > vars.separator.hu_0_2)
		return SEP_SYMBOL;

	return SEP_SUSPICIOUS;
}

bool Separator::_bIsTextContext(const Settings& vars, SegmentDeque &layer_symbols, imago::Rectangle rec)
{
	Segment* firstNear = NULL,
		*secNear = NULL;

	double dist1 = imago::DIST_INF;
	double dist2 = imago::DIST_INF;

	Vec2i cntr(rec.x + rec.width/2, rec.y+rec.height/2);

	//find first pair of symbols closer to rec
	BOOST_FOREACH(Segment *s, layer_symbols)
	{
		imago::Rectangle srec = s->getRectangle();
		Vec2i sc = s->getCenter();
		
		double dist = Vec2i::distance(sc, cntr);

		if(dist < dist1)
		{
			dist1= dist;
			firstNear = s;
		}

		if(dist > dist1 && dist< dist2)
		{
			dist2 = dist;
			secNear = s;
		}
	}

	if(firstNear == NULL && secNear == NULL)
		return false;

	bool xfirstSeparable = Algebra::rangesSeparable(rec.x, rec.x+rec.width, firstNear->getX(), firstNear->getX() + firstNear->getWidth());
	bool yfirstSeparable = Algebra::rangesSeparable(rec.y, rec.y+rec.height, firstNear->getY(), firstNear->getY() + firstNear->getHeight());
	
	if(xfirstSeparable &&  !yfirstSeparable && dist1 < vars.dynamic.CapitalHeight)
		return true;

	if(xfirstSeparable  && !yfirstSeparable &&
		rec.height < vars.dynamic.CapitalHeight + vars.separator.ltFactor1 * vars.dynamic.LineThickness && 
		rec.height > vars.dynamic.CapitalHeight * vars.separator.capHeightMin &&
		dist1 < vars.separator.capHeightMax * vars.dynamic.CapitalHeight && vars.separator.extRatioMin > ((double)rec.width / (double)rec.height))
		return true;
	return false;
}

double HuMoms[2][2] = {
	{0.9419354839,	0.0682196339},
	{0.0326612903,	0.8860232945}
};

double Ratioclass[2][2] = {
	{0.9826612903, 0.6114808652},
	{0.0096774194, 0.340266223}
};

double KNNclass[2][2] = {
	{0.6596774194, 0.2978369384},
	{0.3403225806, 0.7021630616}
};

double Probclass[2][2] = {
	{0.964516129, 0.3693843594},
	{0.035483871, 0.6306156406}
};

int GetClass(imago::Separator::ClassifierResults cres)
{
	double symClass = 1.0, bondClass = 1.0;
	symClass *= (Probclass[1][cres.Probability] * KNNclass[1][cres.KNN]);
	bondClass *= (Probclass[0][cres.Probability] * KNNclass[0][cres.KNN]);
	if(cres.Ratios < 2)
	{
		symClass *= Ratioclass[1][cres.Ratios];
		bondClass *= Ratioclass[0][cres.Ratios];
	}

	if(cres.HuMoments < 2)
	{
		symClass *= HuMoms[1][cres.HuMoments];
		bondClass *= HuMoms[0][cres.HuMoments];
	}

	if(symClass > bondClass)
		return 1;	//SEP_SYMBOL;
	return 0;		//SEP_BOND;
}

int Separator::ClusterLines(const Settings& vars,Points2d& inputLines, IntVector& outClasses)
{
	std::vector<double> lengths;
	// find the minimum and max of the line segments
	for (size_t i = 0; i < inputLines.size() / 2; i++)
	{
      Vec2d &p1 = inputLines[2 * i];
      Vec2d &p2 = inputLines[2 * i + 1];

      double dist = Vec2d::distance(p1, p2);
	  lengths.push_back(dist);
   }

	double min = imago::DIST_INF;
	double max = 0;
	for(size_t i=0;i<lengths.size();i++)
	{
		if(lengths[i] < min)
			min = lengths[i];
		if(lengths[i] > max)
			max = lengths[i];
	}
	if(fabs(min - max) < 0.01) // eps
		return -1;

	// Clustering line segments in 2 groups
	double c1 = min, c2 = max, c1_o = min, c2_o = max;
	
	for (size_t i = 0; i < inputLines.size() / 2; i++)
		outClasses.push_back(0);
	int count1 = 0, count2=0;

	do
	{
		c1_o = c1;
		c2_o = c2;

		for (size_t i = 0; i < inputLines.size() / 2; i++)
		{
		  Vec2d &p1 = inputLines[2 * i];
		  Vec2d &p2 = inputLines[2 * i + 1];

		  double maxX, maxY, minX, minY;
		  if(p1.x > p2.x)
		  {
			  maxX = p1.x;
			  minX = p2.x;
		  }
		  else
		  {
			  maxX = p2.x;
			  minX = p1.x;
		  }

		  if(p1.y > p2.y)
		  {
			  maxY = p1.y;
			  minY = p2.y;
		  }
		  else
		  {
			  maxY = p2.y;
			  minY = p1.y;
		  }

		  double ratio = (maxX - minX)/(maxY - minY);

		  double dist = Vec2d::distance(p1, p2);
		  double dc1 = fabs(dist - c1);
		  double dc2 = fabs(dist - c2);
		  if(dc1 < dc2 && (dist < vars.dynamic.CapitalHeight || 
			  (dist < vars.dynamic.CapitalHeight * vars.separator.getRatio2 && ratio < vars.estimation.MinSymRatio)))
			  outClasses[i] = 0;
		  else
			  outClasses[i] = 1;
		}
		count1 = 0;
		count2=0;
		
		double sum1=0, sum2 = 0;
		
		for(size_t i=0; i < outClasses.size(); i++)
			if(outClasses[i] == 0)
			{
				count1++;
				sum1 += Vec2d::distance(inputLines[2*i], inputLines[2*i+1]);
			}
			else
			{
				count2++;
				sum2 += Vec2d::distance(inputLines[2*i], inputLines[2*i+1]);
			}
		c1 = sum1 / count1;
		c2 = sum2 / count2;
	
	}while(fabs(c1 - c1_o) > 0.1 || fabs(c2 - c2_o) > 0.1); // eps
	
	if(count1 == 0 || count2 == 0)
		return -1;

	if(fabs(c1 - c2) < vars.dynamic.LineThickness)
		return -1;

	return 0;
}

void Separator::SeparateStuckedSymbols(const Settings& vars, SegmentDeque &layer_symbols, SegmentDeque &layer_graphics, CharacterRecognizer &rec  )
{
	logEnterFunction();

	Points2d lsegments;
	
	double line_thick = vars.dynamic.LineThickness;
    CvApproximator cvApprox;
	
    GraphicsDetector gd(&cvApprox, line_thick * vars.separator.gdConst);

	Image timg(_img.getWidth(), _img.getHeight());
	timg.fillWhite();

	SegmentDeque::iterator sit;

	// put the graphic layer on the image
	for(sit = layer_graphics.begin();sit != layer_graphics.end(); ++sit)
	{
		ImageUtils::putSegment(timg, *(*sit));
	}

	//approximate graphics with line segments
	gd.detect(vars, timg, lsegments);

	if(lsegments.empty())
		return;
	
	IntVector classes;

	int clusterResult = ClusterLines(vars, lsegments, classes);

	if(clusterResult != 0)
		return;
	
	if(getLogExt().loggingEnabled())
	{
		imago::Image	smallLines(vars.general.ImageWidth, vars.general.ImageHeight), 
						largeLines(vars.general.ImageWidth, vars.general.ImageHeight);
		smallLines.fillWhite();
		largeLines.fillWhite();
		for(size_t i = 0; i < classes.size(); i++)
		{
			if(classes[i] == 0)
				imago::ImageDrawUtils::putLineSegment(smallLines, lsegments[2 * i], lsegments[ 2 * i + 1], 0);
			else
				imago::ImageDrawUtils::putLineSegment(largeLines, lsegments[2 * i], lsegments[ 2 * i + 1], 0);
		}

		getLogExt().appendImage(std::string("clustered small lines"), smallLines);
		getLogExt().appendImage(std::string("clustered large lines"), largeLines); 
	}

	std::vector<Rectangle> symbRects;
	IntVector LineCount;
	std::vector<bool> visited;
	for(size_t i=0;i<classes.size(); i++)
		visited.push_back(false);
	int ri = -1;

	PriorityQueue pq;
	IntDeque symInds;
	for(size_t i = 0;i<classes.size();i++)
		if(classes[i] == 0)
		{
			symInds.push_back(i);
			Vec2d p1 = lsegments[2 * i];
			Vec2d p2 = lsegments[2 * i + 1];
			SegmentIndx si;
			si._indx = i;
			si._lineSegment.first = p1;
			si._lineSegment.second = p2;
			pq.push(si);
		}
	

	typedef std::deque<Vec2d> polygon;

	std::deque<polygon> RectPoints;

	// integrate the results by joining close to each other segments
	
	int i = -1, currInd;
	int ncount;

	do
	{
		if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

		i++;
		if(i == symInds.size())
			break;
		currInd = symInds[i];
		
		if(visited[currInd])
			continue;
		
		Vec2d &p1 = lsegments[2 * currInd];
		Vec2d &p2 = lsegments[2 * currInd + 1];

		double d_x = p1.x < p2.x ? p1.x : p2.x;
		double d_y = p1.y < p2.y? p1.y:p2.y;
		double d_w = absolute(p1.x - p2.x) + 1;
		double d_h = absolute(p1.y - p2.y) + 1;
		Rectangle rec(round(d_x), round(d_y), round(d_w), round(d_h));

		symbRects.push_back(rec);
		RectPoints.push_back(polygon());
		LineCount.push_back(1);
		ri++;
		RectPoints[ri].push_back(p1);
		RectPoints[ri].push_back(p2);

		bool added = false;
		visited[currInd] = true;

		pq.UpdateComparer(RectPoints[ri]);
		int j = 0;
		
		do{
			if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

			added = false;
			if(pq.empty())
				break;
			
			{
				SegmentIndx si = pq.top();

				int currInd2 = si._indx;
				if(visited[currInd2] || currInd == currInd2)
				{
					pq.pop();
					added = true;
					continue;
				}
			
				Vec2d &sp1 = lsegments[2 * currInd2];
				Vec2d &sp2 = lsegments[2 * currInd2 + 1];

				double dist1 = Algebra::distance2rect(sp1, symbRects[ri]);
				double dist2 = Algebra::distance2rect(sp2, symbRects[ri]);
			
				//update rectangle
				if(dist1 < line_thick || dist2 < line_thick)
				{
					double left = symbRects[ri].x < sp1.x ? symbRects[ri].x : sp1.x; 
					left = left < sp2.x ? left : sp2.x;
				
					double right = (symbRects[ri].x + symbRects[ri].width) > sp1.x ? (symbRects[ri].x  + symbRects[ri].width): sp1.x; 
					right = right > sp2.x ? right : sp2.x;

					double top = symbRects[ri].y < sp1.y ?  symbRects[ri].y : sp1.y; 
					top = top < sp2.y ? symbRects[ri].y : sp2.y;

					double bottom = (symbRects[ri].y + symbRects[ri].height) > sp1.y ? (symbRects[ri].y + symbRects[ri].height):sp1.y;
					bottom = bottom > sp2.y ? bottom : sp2.y;

					int r_l = round(left);
					int r_t = round(top);
					int r_w = round(right) - r_l + 1;
					int r_h = round(bottom) - r_t + 1;

					symbRects[ri] = Rectangle(r_l, r_t, r_w, r_h);

					RectPoints[ri].push_back(sp1);
					RectPoints[ri].push_back(sp2);
					LineCount[ri]++;
					visited[currInd2] = true;
					pq.UpdateComparer(RectPoints[ri]);
					pq.pop();

					added = true;
				}
			}

		}while(added);
		
		ncount = 0;
		for(size_t nc=0;nc<visited.size();nc++)
			ncount += visited[nc];

	}while(ncount !=  symInds.size());

	if(symbRects.empty())
		return;

	bool found_symbol = false;
		
	double adequate_ratio_min = vars.estimation.MinSymRatio;

	for(size_t i=0;i< symbRects.size(); i++)
	{
		if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

		bool isTextContext = _bIsTextContext(vars, layer_symbols, symbRects[i]);

		// TODO: check width/height bound
		if(LineCount[i] < 2 && (!isTextContext || ((double)symbRects[i].width / symbRects[i].height) >= 1/*adequate_ratio_min*/))
			continue;

		if(LineCount[i] == 2 )
		{
			if(Algebra::segmentsParallel(RectPoints[i][0], RectPoints[i][1],
				RectPoints[i][2], RectPoints[i][3], 0.1))
				continue;
			Vec2d p1 = RectPoints[i][0];
			Vec2d p2 = RectPoints[i][1];
			if(Algebra::distance2segment(p1, RectPoints[i][2], RectPoints[i][3]) > line_thick &&
				Algebra::distance2segment(p2, RectPoints[i][2], RectPoints[i][3]) > line_thick)
				continue;
			Line l1 = Algebra::points2line(p1, p2);
			Line l2 = Algebra::points2line(RectPoints[i][2], RectPoints[i][3]);
			Vec2d pintersect = Algebra::linesIntersection(vars, l1, l2);
			if(absolute(pintersect.y - symbRects[i].y) < (symbRects[i].height / 2) )
				continue;
		}

		if(LineCount[i] == 3)
		{
			if(Algebra::segmentsParallel(RectPoints[i][0], RectPoints[i][1],
				RectPoints[i][2], RectPoints[i][3], 0.13) || 
				Algebra::segmentsParallel(RectPoints[i][4], RectPoints[i][5],
				RectPoints[i][2], RectPoints[i][3], 0.13) || 
				Algebra::segmentsParallel(RectPoints[i][0], RectPoints[i][1],
				RectPoints[i][4], RectPoints[i][5], 0.13))
				continue;
		}

		int left = round(  (symbRects[i].x - line_thick < 0) ? 0 : (symbRects[i].x - line_thick));
		int top =  round(  (symbRects[i].y - line_thick < 0) ? 0 : (symbRects[i].y - line_thick));
		int right = round( (symbRects[i].x + symbRects[i].width + line_thick > timg.getWidth()) ? timg.getWidth() :
				            (symbRects[i].x + symbRects[i].width + line_thick) );
		int bottom = round((symbRects[i].y + symbRects[i].height + line_thick > timg.getHeight()) ? timg.getHeight() :
				            (symbRects[i].y + symbRects[i].height  + line_thick) );

		Image extracted(right - left + 1, bottom - top + 1),
			_2BClassified(right - left + 1, bottom - top + 1);
		extracted.fillWhite();
		_2BClassified.fillWhite();


		timg.extractRect(left, top, right, bottom, extracted); 
			
		SegmentDeque segs;
		Segment *s = NULL;
		Segmentator::segmentate(extracted, segs);

		for(SegmentDeque::iterator it = segs.begin(); it!=segs.end(); ++it)
		{
			for(size_t n = 0; n < RectPoints[i].size(); n++)
			{
				if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

				Vec2d pt = RectPoints[i][n];
				int dx = round(pt.x - left);
				int dy = round(pt.y - top);
					
				if((*it)->getX() <= dx && ((*it)->getWidth() + (*it)->getX()) > dx && 
					(*it)->getY() <= dy && ((*it)->getHeight() + (*it)->getY()) > dy)
				{
					int sx = dx - (*it)->getX();
					int sy = dy - (*it)->getY();

					if((*it)->getByte(sx, sy) == 0)
					{
						ImageUtils::putSegment(_2BClassified, *(*it));
						break;
					}
				}
			}
				
			delete *it;
		}

		segs.clear();

		//_2BClassified.crop();
		s = new Segment();
		//s ->init( _2BClassified.getWidth(),  _2BClassified.getHeight());
		s->copy(_2BClassified); // TODO: check
		//memcpy(s->getData(),  _2BClassified.getData(), sizeof(byte) *  _2BClassified.getWidth() *  _2BClassified.getHeight());
		s->getX() = left;
		s->getY() = top;

		imago::Points2d linesegs;
		GraphicsDetector gd2(&cvApprox, round(line_thick / 2.0));
		gd2.detect(vars, _2BClassified, linesegs);

		getLogExt().appendImage(std::string("assembled image"), _2BClassified);
		getLogExt().appendImage(std::string("assembled segment"), *s);

		if((int)(linesegs.size() / 2) > LineCount[i])
		{
			for(size_t k = 0; k < linesegs.size() / 2; k++)
			{
				if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

				Vec2d p1 = linesegs[2 * k];
				Vec2d p2 = linesegs[2 * k + 1];
				double xmin = p1.x > p2.x ? p2.x : p1.x;
				double xmax = p1.x > p2.x ? p1.x : p2.x;

				double ymin = p1.y > p2.y ? p2.y : p1.y;
				double ymax = p1.y > p2.y ? p1.y : p2.y;

				double w_h_ratio = (xmax - xmin) / (ymax - ymin);

				xmin += left;
				xmax += left;
				
				ymin += top;
				ymax += top;

				if(xmin > symbRects[i].x + symbRects[i].width - 1 )
				{
					int limit = round(xmin - left);
					if(w_h_ratio < 0.5)
						limit += round(line_thick / 2.0);
					for(int m = 0; m < s->getHeight(); m++)
						for(int n = limit; n < s->getWidth(); n++) 
							s->getByte(n, m) = 255;
					getLogExt().appendSegment(std::string("after removing right redudant lines"), *s);
				}
				else
					if(xmax < symbRects[i].x)
					{
						int limit = round(xmax - left);
						if(w_h_ratio < 0.5)
							limit += round(line_thick / 2.0);
						for(int m = 0; m < s->getHeight(); m++)
							for(int n = 0; n < limit; n++) 
								s->getByte(n, m) = 255;
						getLogExt().appendSegment(std::string("after removing left redudant lines"), *s);
					}
					else
						if(ymin > symbRects[i].y + symbRects[i].height)
						{
							int limit = round(ymin - top);
							if(w_h_ratio > 0.5)
								limit += round(line_thick / 2.0);
							for(int m = limit; m < s->getHeight(); m++)
								for(int n = 0; n < s->getWidth(); n++) 
									s->getByte(n, m) = 255;
							getLogExt().appendSegment(std::string("after removing bottom redudant lines"), *s);
						}
						else
							if(ymax < symbRects[i].y)
							{
								int limit = round(ymax - top);
								if(w_h_ratio > 0.5)
									limit += round(line_thick / 2.0);
								for(int m = 0; m < limit; m++)
									for(int n = 0; n < s->getWidth(); n++) 
										s->getByte(n, m) = 255;
								getLogExt().appendSegment(std::string("after removing top redudant lines"), *s);
							}
			}
		}
	
		linesegs.clear();

		ClassifierResults cres;

		imago::Segment *scopy = new Segment(*s);
		scopy->crop();

		try
		{
			ClassifySegment(vars, layer_symbols, rec, scopy, cres);
			delete scopy;
		}
		catch(ImagoException ex)
		{
			delete s;
			delete scopy;
			continue;
		}
		//	classify object
		if(GetClass(cres) == SEP_SYMBOL || isTextContext)
		{
			imago::ImageUtils::cutSegment(timg, *s, false, 255);
			layer_symbols.push_back(s);
			found_symbol = true;					 
		}
		else
			delete s;
	}

	if(found_symbol)
	{
		layer_graphics.clear();
			
		Segmentator::segmentate(timg, layer_graphics);
	}
	
}

int Separator::PredictGroup(const Settings& vars, Segment *seg, int mark,  SegmentDeque &layer_symbols)
{
	logEnterFunction();
	int retVal = mark;
	double bond_prob, sym_prob;
	double aprior = vars.p_estimator.DefaultApriority; //0.5

	double capital_height = 0;
	if( layer_symbols.size() > 0 )
	{
		BOOST_FOREACH(Segment *s, layer_symbols)
			capital_height += s->getHeight();

		capital_height /= layer_symbols.size();
	}
	else
		capital_height =  vars.dynamic.CapitalHeight;


	if(mark == SEP_SYMBOL)
		aprior = vars.p_estimator.ApriorProb4SymbolCase;// 0.8;
	else
	{
		double maxEdge = seg->getHeight() > seg ->getWidth() ? seg->getHeight() : seg->getWidth();
		double surfaceRatio =  maxEdge / capital_height;

		if(surfaceRatio < vars.p_estimator.MinRatio2ConsiderGrPr) //0.25)
			surfaceRatio = 1.0 / surfaceRatio;
				
		aprior = 1.0 - 1.0 / 
			( 1 + std::exp( - ( surfaceRatio - vars.p_estimator.LogisticLocation )/vars.p_estimator.LogisticScale ) );
	}

	try
	{
		ProbabilitySeparator::CalculateProbabilities(vars, *seg, sym_prob, bond_prob, aprior, 1.0 - aprior);
		if(bond_prob > sym_prob)
			retVal = SEP_BOND;
		else
			retVal = SEP_SYMBOL;
		
		getLogExt().append("Graphic probability ", bond_prob);
		getLogExt().append("Character probability ", sym_prob);

		getLogExt().append("Probabilistic estimation", retVal);	

	}
	catch (LogicException &e)
	{
		getLogExt().append("CalculateProbabilities logic exception", e.what());
	}
	catch (std::exception &e)
	{
		getLogExt().append("CalculateProbabilities general exception", e.what());
	}

	return retVal;
}


void Separator::ClassifySegment(const Settings& vars, SegmentDeque &layer_symbols, CharacterRecognizer &rec, Segment* seg, ClassifierResults &cresults)
{
	logEnterFunction();

	if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

	int cap_height = (int)vars.dynamic.CapitalHeight;

	int sym_height_err = (int)vars.estimation.SymHeightErr;
	double adequate_ratio_max = vars.estimation.MaxSymRatio;
	double adequate_ratio_min = vars.estimation.MinSymRatio;
    
    /* Classification procedure */
	
	Segment* s = seg;

	getLogExt().appendSegment("Segment", *s);

	//thin segment
	Image temp;
	temp.copy(*s);
	//s->extractRect(0, 0, s->getWidth(), s->getHeight(), temp); // TODO: check
	
	int votes[2] = {0, 0};

	int mark = HuClassifier(vars, temp);

	cresults.HuMoments = mark;
	
	if(mark < 2)
		votes[mark]++;

	if(mark == SEP_SYMBOL && 
		(!(s->getHeight() >= cap_height - sym_height_err && s->getHeight() <= cap_height + sym_height_err && s->getHeight() <= cap_height * 2 && s->getWidth() <= cap_height)
		|| s->getHeight() < vars.separator.capHeightRatio *cap_height)
		)
		mark = SEP_SUSPICIOUS;
	cresults.KNNRatios = mark;

	int mark1 = mark;
	getLogExt().append("mark1", mark1);
		
	//if(mark == SEP_SUSPICIOUS || mark == SEP_BOND)
	{
		ThinFilter2 tfilt(temp);
		tfilt.apply();

		Segment *thinseg = new Segment();
		thinseg->copy(*s);

		if (s->getHeight() >= cap_height - sym_height_err && 
			s->getHeight() <= cap_height + sym_height_err &&
			s->getHeight() <= cap_height * 2 &&
			s->getWidth() <= vars.separator.capHeightRatio2 * cap_height) 
		{
			if (thinseg->getRatio() > vars.separator.getRatio1 && thinseg->getRatio() < vars.separator.getRatio2)
			{
				if (_analyzeSpecialSegment(vars, thinseg))
				{
					mark = SEP_BOND;
				}
			}

			if (thinseg->getRatio() > adequate_ratio_max)
				if (ImageUtils::testSlashLine(vars, *thinseg, 0, vars.separator.testSlashLine1))
					mark = SEP_BOND;
				else
					mark = SEP_SPECIAL;
			else
				if (thinseg->getRatio() < adequate_ratio_min)
					if (_testDoubleBondV(vars, *thinseg))
						mark = SEP_BOND;
					else
						mark = SEP_SUSPICIOUS;
				else
					if (ImageUtils::testSlashLine(vars, *thinseg, 0, vars.separator.testSlashLine2))
						mark = SEP_BOND;
					else 
						mark = SEP_SYMBOL;
		}
		else
			mark = SEP_BOND;
		delete thinseg;
	}

	if((mark == SEP_SUSPICIOUS || mark == SEP_BOND) && mark < 2)
		votes[mark]++;
	
	cresults.Ratios = mark;

	getLogExt().append("mark", mark);

	int segs = _getApproximationSegmentsCount(vars, s) - 1;

	if (segs > vars.separator.minApproxSegsStrong)
	{		
		getLogExt().append("cap_height", cap_height);
		getLogExt().append("Height", s->getHeight());
		double wh = (double)s->getWidth() / (double)s->getHeight();
		getLogExt().append("Width/height", wh);

		bool two_chars_probably = 
			wh > vars.separator.extRatioMax &&
			wh < vars.separator.ext2charRatio * vars.separator.extRatioMax;
		
		if (s->getHeight() > vars.separator.extCapHeightMin * cap_height && 
			s->getHeight() < vars.separator.extCapHeightMax * cap_height &&
									wh > vars.separator.extRatioMin && 
			(two_chars_probably || wh < vars.separator.extRatioMax) )
		{				
			char ch;

			bool matches = false;
			bool strict = false;				

			if (rec.isPossibleCharacter(vars, *s, false, &ch))
			{
				if (two_chars_probably && ch != '#' && ch != '$' && ch != '&')
				{
					getLogExt().append("[strict] Segment passed as 2-chars, but recognized as", ch);
				}
				else
				{
					matches = true;
					strict = true;
				}
			}
			else if (rec.isPossibleCharacter(vars, *s, true, &ch))
			{
				if (two_chars_probably && ch != '#' && ch != '$' && ch != '&')
				{
					getLogExt().append("[loose] Segment passed as 2-chars, but recognized as", ch);
				}
				else
				{
					matches = true;
					strict = false;
				}
			}
			else if (two_chars_probably)
			{
				// calculate split					
				int mid = s->getWidth() / 2;
				int gap = mid / 2;
				int best_x = -1;
				int best_intersect = s->getHeight();
				for (int xv = 0; xv < gap; xv++)
				{
					for (int xs = -1; xs <= 1; xs += 2)
					{
						int x = mid + xv * xs;
						int y_intersect = 0;
						for (int y = 0; y < s->getHeight(); y++)
						{
							if (s->getByte(x, y) == 0)
								y_intersect++;
						}
						if (y_intersect < best_intersect)
						{
							best_intersect = y_intersect;
							best_x = x;
						}
					}
				}
				if (best_x > 0)
				{
					Segment* s1 = new Segment();
					Segment* s2 = new Segment();
					s->splitVert(best_x, *s1, *s2);
						
					getLogExt().appendSegment("Split: S1", *s1);
					getLogExt().appendSegment("Split: S2", *s2);

					if (rec.isPossibleCharacter(vars, *s1, true, &ch) &&
						rec.isPossibleCharacter(vars, *s2, true, &ch))
					{
						getLogExt().appendText("Both are symbols");
						if (segs > vars.separator.minApproxSegsWeak)
						{							
							getLogExt().appendText("Segments criteria passed");
							layer_symbols.push_back(s1);
							layer_symbols.push_back(s2);
							cresults.KNN = SEP_SYMBOL;
							cresults.Processed = true;
							//continue;
						}
					}
					if(!cresults.Processed)	
					{
						delete s1;
						delete s2;						
					}
				}
			}

			if (matches)
			{
				int segs = _getApproximationSegmentsCount(vars, s) - 1;
				getLogExt().append("Approx segs", segs);
				if (segs > (strict ? vars.separator.minApproxSegsStrong : vars.separator.minApproxSegsWeak))
				{
					getLogExt().appendText("Segment marked as symbol");
					mark = SEP_SYMBOL;
					votes[SEP_SYMBOL]++;
				}
			}			
		}

		cresults.KNN = mark;
	}

	if(mark1 != SEP_SYMBOL && cresults.KNN < 2 && cresults.KNN > -1)
		votes[mark]++;

	//s->setSymbolProbability(mark < 2 ? (double)mark : 0.0);

	if (vars.general.UseProbablistics) // use probablistic method			
	{
		mark = PredictGroup(vars, s, votes[SEP_SYMBOL] > votes[SEP_BOND] ? SEP_SYMBOL : SEP_BOND, layer_symbols);
		if(mark < 2)
			votes[mark]++;			
		cresults.Probability = mark;
	}

	if (vars.separator.UseVoteArray)
	{
		mark = votes[SEP_SYMBOL] > votes[SEP_BOND] ? SEP_SYMBOL : SEP_BOND;
	}

	
	char ch;
	if(cresults.KNN == -1 || cresults.KNN == 3)
	{
		cresults.KNN = rec.isPossibleCharacter(vars, *s, true, &ch) ? 1 : 0;
		if(ch == '(' || ch == ')' || ch == '[' || ch == ']')
		{
			cresults.HuMoments = SEP_SYMBOL;
			cresults.Probability = SEP_SYMBOL;
			mark = SEP_SYMBOL;
		}
	}
	if(cresults.Probability == -1)
		cresults.Probability = PredictGroup(vars, s, votes[SEP_SYMBOL] > votes[SEP_BOND] ? SEP_SYMBOL : SEP_BOND, layer_symbols);
	
	cresults.OverAll = mark;
}

void Separator::Separate(Settings& vars, CharacterRecognizer &rec, SegmentDeque &layer_symbols, SegmentDeque &layer_graphics )
{
   logEnterFunction();
   
   int guessedHeight = -1;

   if (_segs.size() == 0)
   {
	   getLogExt().appendText("Warning, _segs.size is 0!");	   
	   return;
   }
   else
   {
	   bool _heightRestricted = false;
	   guessedHeight = _estimateCapHeight(vars, _heightRestricted);
	   vars.dynamic.CapitalHeight = guessedHeight;
	   getLogExt().append("Capital height", vars.dynamic.CapitalHeight);
   }

	double height_sum = 0.0;
	int height_count = 0;
	bool classified_at_least_one_char = false;	

	pre_classify:

   for (SegmentDeque::iterator it = _segs.begin(); it != _segs.end(); it++)
   {
	   if (std::find(layer_graphics.begin(), layer_graphics.end(), *it) != layer_graphics.end())
		   continue;

	   if (std::find(layer_symbols.begin(), layer_symbols.end(), *it) != layer_symbols.end())
		   continue;

	   Segment *s = *it;
	   RecognitionDistance rd = rec.recognize(vars, *s, CharacterRecognizer::all + CharacterRecognizer::graphics);
	   double dist;
	   char c = rd.getBest(&dist);
	   if (CharacterRecognizer::graphics.find(c) != std::string::npos && dist < vars.characters.DistanceAbsolutelySure)
	   {
		   layer_graphics.push_back(s);
		   getLogExt().appendText("Classified as graphics on first stage");
	   }
	   else if (CharacterRecognizer::like_bonds.find(c) == std::string::npos
		        && (vars.dynamic.CapitalHeight < 0 ||
				s->getHeight() > vars.characters.HeightMinBound * vars.dynamic.CapitalHeight  
				&& s->getHeight() < vars.characters.HeightMaxBound * vars.dynamic.CapitalHeight ) )
		{
			if (dist < vars.characters.DistanceAbsolutelySure)
			{
				if (CharacterRecognizer::upper.find(c) != std::string::npos)
				{
					height_sum += s->getHeight();
					height_count += 1;
					getLogExt().appendText("Classified as capital letter on first stage");
				}
				else
				{			
					getLogExt().appendText("Classified as symbol on first stage");
				}

				classified_at_least_one_char = true;
				layer_symbols.push_back(s);
			}
		}
		else
		{
			getLogExt().appendText("Not classified on first stage");
	   }	   
   }

   if (height_count >= vars.characters.ReestimateMinimalCharacters || (height_count > 0 && vars.dynamic.CapitalHeight < 0)) 
	{
		getLogExt().appendText("Re-estimate cap height");
		double height = height_sum / height_count;
		vars.dynamic.CapitalHeight = height;
		getLogExt().append("Height updated", vars.dynamic.CapitalHeight);
	}

   if (!classified_at_least_one_char)
   {
	   if (vars.dynamic.CapitalHeight > 0)
	   {
		   vars.dynamic.CapitalHeight = -1; // reset estimated height;
		   getLogExt().appendText("Restart classification!");
		   goto pre_classify;
	   }
   }

   if (vars.dynamic.CapitalHeight < 0 && guessedHeight > 0)
   {
	   getLogExt().appendText("Give first guessed height a try");
	   vars.dynamic.CapitalHeight = guessedHeight;
   }

     
   {
	  for (SegmentDeque::iterator it = _segs.begin(); it != _segs.end(); it++)
	  {

	   if (std::find(layer_graphics.begin(), layer_graphics.end(), *it) != layer_graphics.end())
		   continue;

	   if (std::find(layer_symbols.begin(), layer_symbols.end(), *it) != layer_symbols.end())
		   continue;

		  if (vars.checkTimeLimit())
			  throw ImagoException("Timelimit exceeded");

		  Segment *s = *it;
		  ClassifierResults cres;

		  if(vars.dynamic.CapitalHeight == -1)
		  {
			layer_graphics.push_back(s);
			continue;
		  }
		  else
			ClassifySegment(vars, layer_symbols, rec, s, cres);

		  if(!cres.Processed)
		  {
			  switch (cres.OverAll)
			  {
			  case SEP_BOND:
				  layer_graphics.push_back(s);
				  break;
			  case SEP_SYMBOL:
					layer_symbols.push_back(s);
				  break;
			  default:
				  layer_graphics.push_back(s);
			  }
		  }
      }
   }

   SeparateStuckedSymbols(vars, layer_symbols, layer_graphics, rec);

   std::sort(layer_symbols.begin(), layer_symbols.end(), _segmentsComparator);
}

bool Separator::_analyzeSpecialSegment(const Settings& vars, Segment *cur_seg)
{
	return _getApproximationSegmentsCount(vars, cur_seg) <= vars.separator.specialSegmentsTreat;
}

int Separator::_getApproximationSegmentsCount(const Settings& vars, Segment *seg)
{
   Image tmp;
   CvApproximator cvApprox;
   GraphicsDetector gd(&cvApprox, vars.dynamic.LineThickness * vars.separator.gdConst);
   Points2d lsegments;
   tmp.copy(*seg);
   gd.detect(vars, tmp, lsegments);
   return lsegments.size();
}

int Separator::_estimateCapHeight(const Settings& vars, bool &restrictedHeight)
{
	logEnterFunction();

   typedef std::vector<IntPair> PairIntVector; 
   PairIntVector seq_pairs;
   IntVector heights, seq_lengths;
   IntPair p;

   BOOST_FOREACH( Segment *s, _segs )
   {
	   if (s->getHeight() >= vars.characters.MinimalRecognizableHeight)
	   {
		   heights.push_back(s->getHeight());
	   }
      
#ifdef DEBUG
      printf("%d ", s->getHeight());
#endif
   }

   getLogExt().appendVector("Heights", heights);   

#ifdef DEBUG
   puts("");
#endif

   int seg_ver_eps = vars.estimation.SegmentVerEps;
   getLogExt().append("Seg_ver_eps", seg_ver_eps);

   for (size_t i = 0; i < heights.size(); )
   {
      size_t j = i + 1;
      for (; j < heights.size(); j++)
      {
         if (absolute(heights[j - 1] - heights[j]) > seg_ver_eps) 
            break;
      }

      p.first = i;
      p.second = j;

#ifdef DEBUG
      printf("%d %d\n", i, j);
#endif

      seq_pairs.push_back(p);

      i += j - i;
   }

   seq_lengths.resize(seq_pairs.size());

   for (size_t i = 0; i < seq_pairs.size(); i++)
   {      
      seq_lengths[i] = seq_pairs[i].second - seq_pairs[i].first;
   }

   IntVector symbols_found;
   DoubleVector densities(symbols_found.size(), 0.0);
   PairIntVector symbols_graphics(seq_pairs.size());

   int symbols_seq = -1, max_seq_length_i;

   while (true)
   {
      int maximum;
      double density;
      IntPair p;
      IntVector::iterator iter = std::max_element(seq_lengths.begin(), seq_lengths.end());
	  if (iter == seq_lengths.end())
		  break;
      
      maximum = *iter;
      max_seq_length_i = std::distance(seq_lengths.begin(), iter);

      if (maximum == -1)
         break;

      seq_lengths[max_seq_length_i] = -1;
      p = seq_pairs[max_seq_length_i];

      density = 0;
      
      if (_checkSequence(vars, p, symbols_graphics[max_seq_length_i], density))
      {
         densities.push_back(density);
         symbols_found.push_back(max_seq_length_i);
      }
   }

   if (symbols_found.empty())
   {
	   getLogExt().appendText("No symbols found");
	   return -1;
   }

   int count = 0;
   symbols_seq = -1;

   for (size_t i = 0; i < symbols_found.size(); i++)
      if (symbols_graphics[symbols_found[i]].first > count)
      {
         symbols_seq = i;
         count = symbols_graphics[symbols_found[i]].first;
      }

   if (count == 0)
   {
	   getLogExt().appendText("Can not determine");
      return -1;
   }


   double temp = StatUtils::interMean(heights.begin() + seq_pairs[symbols_found[symbols_seq]].first, 
                                      heights.begin() + seq_pairs[symbols_found[symbols_seq]].second);    
#ifdef DEBUG
   printf("SSQ: %d\nTEMP: %lf", symbols_seq, temp);
#endif
   int cap_height = (int)temp;

   double cur_density = densities[symbols_seq];

   for (size_t i = 0; i < symbols_found.size(); i++)
   {
      if (absolute(count - symbols_graphics[symbols_found[i]].first) < 2)
      {  
         if (densities[i] > cur_density)
         {
            symbols_seq = i;
            cap_height = (int)(StatUtils::interMean(heights.begin() + seq_pairs[symbols_found[symbols_seq]].first, 
                                              heights.begin() + seq_pairs[symbols_found[symbols_seq]].second)); 
            cur_density = densities[i];
         }
      }
   }

   getLogExt().append("Return", cap_height);

   double cap_height_limit = std::max(vars.general.ImageWidth, vars.general.ImageHeight) * vars.estimation.MaxSymbolHeightPercentsOfImage;
   if (cap_height > cap_height_limit)
   {
	   cap_height = round(cap_height_limit);
	   restrictedHeight = true;
	   getLogExt().append("Limited to", cap_height);
   }
   else
	   restrictedHeight = false;

   return cap_height;
}

bool Separator::_checkSequence(const Settings& vars, IntPair &checking, IntPair &symbols_graphics, double &density )
{
   if (checking.second - checking.first == 1)
   {
	   if (_segs[checking.first]->getDensity() < vars.separator.minDensity)
      {
         symbols_graphics.first = 1;
         symbols_graphics.second = 0;
         density += _segs[checking.first]->getDensity();

         return true;
      }
   }

   double adequate_ratio_max = vars.estimation.MaxSymRatio;
   double adequate_ratio_min = vars.estimation.MinSymRatio;

   for (int i = checking.first; i < checking.second; i++)
   {
	   if (_segs[i]->getDensity() > vars.separator.maxDensity && (_segs[i]->getHeight() > _segs[i]->getWidth()))
      {
         if (!_testDoubleBondV(vars, *_segs[i]))
         {
            symbols_graphics.first++;
            density += _segs[i]->getDensity();
         }
      }

      if (_segs[i]->getRatio() >= adequate_ratio_min && 
         _segs[i]->getRatio() <= adequate_ratio_max)
      {         
         if (!ImageUtils::testSlashLine(vars, *_segs[i], 0, 1))
         {
            symbols_graphics.first++;
            density += _segs[i]->getDensity();
         }
         else
            symbols_graphics.second++;
      }
   }

   //Some symbols found in sequence
   if (symbols_graphics.first)
   {
      density /= symbols_graphics.first;
      return true;
   }
   else
      return false;
}

bool Separator::_testDoubleBondV(const Settings& vars, Segment &segment )
{
	logEnterFunction();

   bool ret = false;
   SegmentList segs;
   Segment tmp, segment_tmp;

   getLogExt().appendSegment("segment", segment);

   segment_tmp.emptyCopy(segment);
   segment_tmp.getY() = 0;
   segment_tmp.getX() = segment.getX();
   tmp.init(_img.getWidth(), segment.getHeight());
   tmp.fillWhite();

   {
      int x, y, rows = tmp.getHeight(), cols = tmp.getWidth(),
         y0 = segment.getY();

      for (y = 0; y < rows; y++)
         for (x = 0; x < cols; x++)
			 if (y + y0 < _img.getHeight())
				tmp.getByte(x, y) = _img.getByte(x, y + y0);
   }

   ImageUtils::putSegment(tmp, segment_tmp, false);
   Segmentator::segmentate(tmp, segs);

   BOOST_FOREACH( Segment *s, segs )
   {
	   if (s->getRatio() <= vars.estimation.MinSymRatio)
		   if (absolute(s->getX() - segment.getX()) < vars.estimation.DoubleBondDist) 
         {
            ret = true;
            break;
         }
   }

   BOOST_FOREACH( Segment *s, segs )
      delete s;

   segs.clear();

   return ret;
}

bool Separator::_segmentsComparator( Segment *a, Segment *b )
{
   return a->getHeight() < b->getHeight();
}
