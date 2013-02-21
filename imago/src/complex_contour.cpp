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

#include "complex_contour.h"
#include "settings.h"
#include "approximator.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/graph_traits.hpp>
#include "algebra.h"
#include "log_ext.h"
#include <opencv2/opencv.hpp>

using namespace imago;

ComplexContour::ComplexContour(void)
{
}


ComplexContour::~ComplexContour(void)
{
}

ComplexNumber& ComplexContour::getContour(int shift)
{
	return *(_contours[shift % _contours.size()]);
}

const ComplexNumber& ComplexContour::getContour(int shift) const
{
	return *(_contours[shift % _contours.size()]);
}

double ComplexContour::DiffR2(const ComplexContour& lc) const
{
	double max1 = 0;
	double max2 = 0;
	double sum = 0;
	for(size_t i = 0; i< _contours.size();i++)
	{
		double v1 = _contours[i].getRadius();
		double v2 = lc.getContour(i).getRadius();
		if(v1 > max1) max1 = v1;
		if(v2 > max2) max2 = v2;
		double v = v1 - v2;
		sum += v *v;
	}

	double max = std::max(max1, max2);
	return 1. - sum / _contours.size() / max / max;
}


double ComplexContour::Norm() const
{
	double result = 0;
	for(size_t i = 0;i < _contours.size();i++)
	{
		result += _contours[i].getRadius2();
	}
	return sqrt(result);
}

ComplexNumber ComplexContour::Dot(const ComplexContour& c, int shift) const
{
	ComplexNumber cn = ComplexNumber(0, 0);
	for(size_t i=0;i<_contours.size();i++)
		cn = cn + ComplexNumber::Dot(_contours[i], c.getContour(shift + i));

	return cn;
}

std::vector<ComplexNumber> ComplexContour::InterCorrelation(const ComplexContour& c)
{
	int count = _contours.size();
	std::vector<ComplexNumber> retVal;
	for(int i=0;i< count;i++)
		retVal.push_back(Dot(c, i));
	return retVal;
}

std::vector<ComplexNumber> ComplexContour::AutoCorrelation(bool normalize)
{
	int count = _contours.size()/2;
	double maxNorm = 0;
	std::vector<ComplexNumber> acf;
	for(int i=0; i<count; i++)
	{
		ComplexNumber cn = Dot(*this, i);
		
		acf.push_back(cn);

		double normaSq = acf[i].getRadius2();
		if(normaSq > maxNorm)
			maxNorm = normaSq;

	}

	if(normalize && maxNorm > 0)
	{
		double maxNormaSq = sqrt(maxNorm);
		for(size_t i=0;i< acf.size();i++)
			acf[i] /= maxNormaSq;
	}

	return acf;
}

ComplexNumber ComplexContour::FindMaxNorm() const
{
	double max = 0.;
	ComplexNumber res(0, 0);
	for(size_t i =0;i<_contours.size();i++)
		if(_contours[i].getRadius() > max)
		{
			max = _contours[i].getRadius();
			res = _contours[i];
		}
				  
return res;
}

void ComplexContour::Scale(double scale)
{
	for(size_t i=0; i< _contours.size();i++)
		_contours[i] *= scale;
}

void ComplexContour::Normalize()
{
	double max = FindMaxNorm().getRadius();
	if(max > 0)
		Scale(1.0/max);
}

void ComplexContour::NormalizeByPerimeter()
{
	double perimeter = 0;
	for(size_t i = 0; i < _contours.size(); i++)
		perimeter += _contours[i].getRadius();
	if(perimeter > 0)
		Scale(1.0 / perimeter);
}

double ComplexContour::getNorm() const
{
	double result = 0.;
	for(size_t i=0;i<_contours.size();i++)
		result += _contours[i].getRadius2();
	return std::sqrt(result);
}

double ComplexContour::Distance(const ComplexContour& c)
{
	double n1 = this->Norm();
	double n2 = c.Norm();
	return n1 * n1 + n2 * n2 - 2 * (Dot(c).getReal());
}

void ComplexContour::EqualizeUp(int n)
{
	ComplexNumber currPoint = _contours[0];
	int count = _contours.size();

	std::vector<ComplexNumber> newCont;

	
	int times = (n - count) / count;
	int span = (n - count) % count;

	for(int i=0; i < count;i ++)
	{
		int slice = i < span ? times + 1 : times;

		if(slice > 0)
		{
			double k = 1.0 / (1.0 + slice);

			ComplexNumber c = _contours[i];
			c *= k;
			for(int k = 0 ; k <= slice; k++)
				newCont.push_back(c);
		}
		else
			newCont.push_back(_contours[i]);
	}

	/*for(int i=0;i<n;i++)
	{
		double index = i * count / double(n);
		int j = (int)index;
		double k = index -j;

		if(j == count - 1)
			newCont.push_back(_contours[j]);
		else
		{
			ComplexNumber c1 = _contours[j];
			ComplexNumber c2 = _contours[j+1];
			c1 *= (1-k);
			c2 *=k;
			newCont.push_back(c1 + c2);
		}
	}
*/
	_contours = newCont;
}

void ComplexContour::EqualizeDown(int n)
{
	int count = _contours.size();
	ComplexNumber currPoint = _contours[0];

	std::vector<ComplexNumber> newCont;

	for(int i = 0; i< n; i++)
		newCont.push_back(ComplexNumber());

	for(int i = 0; i < count; i++)
	{
		newCont[i * n / count] += _contours[i];
	}

	_contours = newCont;
}

void ComplexContour::Equalize(int n)
{
	if(n > (int)_contours.size())
		EqualizeUp(n);
	else
		if(n < (int)_contours.size())
			EqualizeDown(n);
}


void cvRetrieveContour(Image& img, Points2d &lines, int eps)
{
	int w = img.getWidth(), h = img.getHeight();
   cv::Mat mat = cv::Mat::zeros(cv::Size(w + 2, h + 2), CV_8U);
   
   for (int i = 0; i < w; i++)
      for (int j = 0; j < h; j++)
         mat.at<unsigned char>(j+1, i+1) = 255 - img.getByte(i, j);
   
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Point> newcont;
    std::vector<cv::Vec4i> hierarchy;
	cv::findContours(mat, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);
	size_t maxLengthContour = 0, maxLength = 0;
	
	if (!contours.empty())
	{
		for(size_t i = 0; i < contours.size(); i++)
		{
			if(contours[i].size() > maxLength)
			{
				maxLength = contours[i].size();
				maxLengthContour = i;
			}
		}
	
		cv::Mat m(contours[maxLengthContour]);
		cv::approxPolyDP(contours[maxLengthContour], newcont, eps, false);
	
		for(size_t i = 0;i < newcont.size(); i++)
		{
			lines.push_back(Vec2d(newcont[i].x, newcont[i].y));
		}
	}
}

ComplexContour ComplexContour::RetrieveContour(const Settings& vars, Image& seg, bool fine_detail)
{
	logEnterFunction();
	std::vector<ComplexNumber> contours;
	double lnThickness = vars.dynamic.LineThickness;

	Points2d lines;
	
	double eps = (lnThickness / 2.0 > 2.0) ? (lnThickness / 2.0) : 2.0;

	if(fine_detail)
		eps = 2.0;
	
	cvRetrieveContour(seg, lines, round(eps)); 

	Skeleton graph;
	Vec2d lastPoint;

	// add lines to graph
	if (!lines.empty())
	{
		imago::Skeleton::Vertex vStart = graph.addVertex(lines[0]);
		imago::Skeleton::Vertex vStart1 = vStart;
	   for (size_t i = 1; i < lines.size() ; i++)
	   {
		   if (vars.checkTimeLimit())
				throw ImagoException("Timelimit exceeded");

		   imago::Skeleton::Vertex vEnd = graph.addVertex(lines[i]);
		   try
		   {
			   graph.addBond(vStart, vEnd, BT_SINGLE, true);
		   }// no need to add already existing edge - continue
		   catch(LogicException ex)
		   {
			   getLogExt().append("Error while adding bond", i);
		   }
		   vStart = vEnd;
	   }

	   try
	   {
		   graph.addBond(vStart, vStart1, BT_SINGLE, true);
	   }// no need to add already existing edge
	   catch(LogicException ex)
	   {
		   getLogExt().append("Error while adding bond", -1);
	   }
	}
	else
	{
		throw LogicException("No contours");
	}

	Skeleton::SkeletonGraph _g = graph.getGraph();
	getLogExt().appendSkeleton(vars, "retrieved contour", _g);
	Skeleton::Vertex vert1 = *(_g.m_vertices.begin());
	Skeleton::Vertex vert2;
	Skeleton::Vertex vertIt = vert1;

	std::deque<Skeleton::Vertex> neighbours;
	std::deque<Skeleton::Vertex>::iterator minIt;
	boost::graph_traits<Skeleton::SkeletonGraph>::adjacency_iterator b, e;
  
	//Find start vertex with minimal degree
	int minDeg = boost::num_vertices(_g);
	BGL_FORALL_VERTICES(v, _g, Skeleton::SkeletonGraph)
	{
		int deg = boost::degree(v, _g);
		if(minDeg > deg)
		{
			minDeg = deg;
			vert1 = v;
		}
	}

	vertIt = vert1;
	boost::tie(b, e) = boost::adjacent_vertices(vert1, _g);
	neighbours.assign(b, e);

	// TODO: FIND appropriate vert2 for a clokcwise traversal 

	vert2 = *(neighbours.begin());

	do
	{
		if (vars.checkTimeLimit())
			throw ImagoException("Timelimit exceeded");

		double min_angle = 2*PI;
		
			//add contour
			Vec2d bpos = boost::get(boost::vertex_pos, _g, vert1);
			Vec2d epos = boost::get(boost::vertex_pos, _g, vert2);

			contours.push_back(ComplexNumber(bpos.x, bpos.y));
			contours.push_back(ComplexNumber(epos.x, epos.y));

			//find adjacent vertices of vert2
			std::deque<Skeleton::Vertex> neighbours2;
			std::deque<Skeleton::Vertex>::iterator vit2;
			boost::graph_traits<Skeleton::SkeletonGraph>::adjacency_iterator b2, e2;
			boost::tie(b2, e2) = boost::adjacent_vertices(vert2, _g);
			neighbours2.assign(b2, e2);

			minIt = neighbours2.begin();

			//find adjacent vertex to vert2  with minimum angle with (vert1, vert2) edge
			for(vit2 = neighbours2.begin(); vit2 != neighbours2.end(); ++vit2)
			{
				if (vars.checkTimeLimit())
					throw ImagoException("Timelimit exceeded");

				Skeleton::Vertex v = *(vit2);

				if(v == vert1)
					continue;

				Vec2d vpos = boost::get(boost::vertex_pos, _g, v);

				Vec2d e1, e2, e;
				e1.diff(bpos, epos);
				e2.diff(vpos, epos);

				double angle1 = atan2(e1.y, e1.x);
				if(angle1 < 0 )
					angle1 = 2 * PI + angle1;
				double angle2 = atan2(e2.y, e2.x);
				if(angle2 < 0)
					angle2 = 2 * PI + angle2;

				double angle = angle1 - angle2;

				angle = angle < 0 ? 2*PI + angle : angle;

				if( min_angle > angle)
				{
					min_angle = angle;
					minIt = vit2;
				}
			}

			vert1 = vert2;
			vert2 = *(minIt);

		
		/*const Skeleton::Vertex &beg = boost::source(edge, _g);
		const Skeleton::Vertex &end = boost::target(edge, _g);

		Vec2d bpos = boost::get(boost::vertex_pos, _g, beg);

		contours.push_back(ComplexNumber(bpos.x, bpos.y));*/

	}while(vert1 != vertIt);

	std::string directions;
	double pi_8 = imago::PI / 8.0;

	std::vector<ComplexNumber> diffCont;
	//int i;
	for(size_t i=1;i < contours.size();i+=2)
	{
		ComplexNumber c = contours[i] - contours[i-1];
		double angle = c.getAngle();
		if(angle < 0)
			angle  += 2 * PI;
		directions += " ";
		if(angle < pi_8 || angle >= 15.0 * pi_8)
			directions += "E";
		else
			if(angle >= pi_8 && angle < 3.0 * pi_8)
				directions += "NE";
			else
				if(angle >= 3.0 * pi_8 && angle < pi_8 * 5.0)
					directions += "N";
				else
					if(angle >= pi_8 * 5.0 && angle < pi_8 * 7.0)
						directions += "NW";
					else
						if(angle >= pi_8 * 7.0 && angle < pi_8 * 9.0)
							directions += "W";
		
		if(angle >= 9.0 * pi_8 && angle < 11.0 * pi_8)
				directions += "SW";
			else
				if(angle >= 11.0 * pi_8 && angle < pi_8 * 13.0)
					directions += "S";
				else
					if(angle >= pi_8 * 13.0 && angle < pi_8 * 15.0)
						directions += "SE";

		diffCont.push_back(c);
	}

	getLogExt().appendText(directions);

	return ComplexContour(diffCont);
}