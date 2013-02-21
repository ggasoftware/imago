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

#pragma once
#ifndef _line_priority_queue_h
#define	_line_priority_queue_h

#include "stl_fwd.h"
#include <queue>
#include <vector>
#include "rectangle.h"
#include "log_ext.h"

namespace imago
{
	struct SegmentIndx
	  {
		  std::pair<Vec2d, Vec2d> _lineSegment;
		  int _indx;
	  };
	  
	  class LineSegmentComparer
	  {
	  public:
		  LineSegmentComparer()
		  {
			  _rec = Rectangle(0, 0, 0, 0);
			  _sortByPolyLine = false;
		  }

		  void SetRectangle(Rectangle rec)
		  {
			  _rec = rec;
		  }

		  void SetPolyline(std::deque<Vec2d> polyline)
		  {
			  _polyline = polyline;
		  }
		  
		  LineSegmentComparer(Rectangle rec)
		  {
			  SetRectangle(rec);
		  }
		  
		  bool operator() (const SegmentIndx &lhs, const SegmentIndx &rhs) const
		  {
			  bool retVal;
			  if(_isSortByPolyLine())
				  retVal =  CompareByPolyline(lhs, rhs);
			  else
				  retVal = CompareByRectangle(lhs, rhs);

			  return retVal;
		  }
		  
		  bool _isSortByPolyLine() const
		  {
			  return _sortByPolyLine;
		  }
		  void setSortByPolyLine(bool enable)
		  {
			  _sortByPolyLine = enable;
		  }
	  private:
		  bool CompareByPolyline(const SegmentIndx &lhs, const SegmentIndx &rhs) const
		  {			  
			  double d1 = CompareSegmentByPoly(lhs);
			  double d2 = CompareSegmentByPoly(rhs);

			  if(d1 >= imago::DIST_INF-imago::EPS && d2 >= imago::DIST_INF-imago::EPS)
			  {
				  logEnterFunction();
				  getLogExt().appendText("Lines not set exception");
				  throw ImagoException("Lines not set");
			  }

			  return d1 > d2;
		  }

		  bool CompareByRectangle(const SegmentIndx &lhs, const SegmentIndx &rhs) const
		  {
			  double d1 = Algebra::distance2rect(lhs._lineSegment.first, _rec);
			  double d2 = Algebra::distance2rect(lhs._lineSegment.second, _rec);

			  double d3 = Algebra::distance2rect(rhs._lineSegment.first, _rec);
			  double d4 = Algebra::distance2rect(rhs._lineSegment.second, _rec);

			  double min1 = d1 < d2 ? d1  : d2;
			  double min2 = d3 < d4 ? d3 : d4;

			  return min1 > min2;
		  }

		  double CompareSegmentByPoly(const SegmentIndx &si) const
		  {
			  double retVal = DIST_INF;

			  for(size_t i = 0; i < _polyline.size(); i+=2)
			  {
				  Vec2d p1 = _polyline[i];
				  Vec2d p2 = _polyline[i+1];

				  double d1 = Algebra::distance2segment(p1, si._lineSegment.first, si._lineSegment.second);
				  double d2 = Algebra::distance2segment(p2, si._lineSegment.first, si._lineSegment.second);

				  double min = d1 < d2 ? d1 : d2;
				  if(min < retVal)
					  retVal = min;
			  }

			  return retVal;
		  }
		  Rectangle _rec;
		  std::deque<Vec2d> _polyline;
		  bool _sortByPolyLine;
	  };

	  class PriorityQueue: public std::priority_queue<SegmentIndx, std::vector<SegmentIndx>, LineSegmentComparer>
	  {
	  public:
		  PriorityQueue(){

		  }
		  
		  void UpdateComparer(const Rectangle &rec)
		  {
			  SetRectangle(rec);
		  }

		  void UpdateComparer(const std::deque<Vec2d> polyLine)
		  {
			  SetPolyline(polyLine);
		  }

	  private:
		  
		  void SetRectangle(const Rectangle &rec)
		  {
			  std::vector<SegmentIndx> segs;
			  
			  
			  if(!this->empty())
			  {
				  do{
					  segs.push_back(this->top());
					  this->pop();
				  }while(!this->c.empty());
				    
				  _rec = rec;
				  
				  this->comp.SetRectangle(_rec);
				  this->comp.setSortByPolyLine(false);
				  
				  for(size_t i = 0;i<segs.size();i++)
				  {
					  this->push(segs[i]);
				  }
			  }
		  }

		  void SetPolyline(const std::deque<Vec2d> &polyline)
		  {
			  std::vector<SegmentIndx> segs;

			  if(!this->empty())
			  {
				  do{
					  segs.push_back(this->top());
					  this->pop();
				  }while(!this->c.empty());
				    
				  
				  this->comp.SetPolyline(polyline);
				  this->comp.setSortByPolyLine(true);
				  
				  for(size_t i = 0;i<segs.size();i++)
				  {
					  this->push(segs[i]);
				  }
			  }
		  }


		  Rectangle _rec;
	  };
}

#endif //_line_priority_queue_h
