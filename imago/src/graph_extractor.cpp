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

#include <deque>

#include "boost/foreach.hpp"

#include "comdef.h"
#include "log_ext.h"
#include "graph_extractor.h"
#include "graphics_detector.h"
#include "image_utils.h"
#include "image_draw_utils.h"
#include "segment.h"
#include "skeleton.h"

using namespace imago;

void GraphExtractor::extract(Settings& vars, const GraphicsDetector &gd, const SegmentDeque &segments, Skeleton &graph )
{
	logEnterFunction();

   Image tmp;
   int w = 0, h = 0;

   // recreate image from segments
   BOOST_FOREACH( Segment *s, segments )
   {
      if (s->getX() + s->getWidth() >= w)
         w = s->getX() + s->getWidth();
      if (s->getY() + s->getHeight() >= h)
         h = s->getY() + s->getHeight();
   }

   tmp.init(w + 10, h + 10);
   tmp.fillWhite();

   BOOST_FOREACH( Segment *s, segments )
   {
      ImageUtils::putSegment(tmp, *s, true);
   }

   getLogExt().appendImage("Working image", tmp);

   extract(vars, gd, tmp, graph);
}

void GraphExtractor::extract(Settings& vars, const GraphicsDetector &gd, const Image &img, Skeleton &graph )
{
	logEnterFunction();

   double avg_size = 0;
   Points2d lsegments;

   gd.detect(vars, img, lsegments);

   if (!lsegments.empty())
   {
	   for (size_t i = 0; i < lsegments.size() / 2; i++)
	   {
		  Vec2d &p1 = lsegments[2 * i];
		  Vec2d &p2 = lsegments[2 * i + 1];

		  double dist = Vec2d::distance(p1, p2);

		  if (dist > 2.0)
			 avg_size += dist;
	   }

	   avg_size /= (lsegments.size() / 2.0);

	   graph.setInitialAvgBondLength(vars, avg_size);

	   for (size_t i = 0; i < lsegments.size() / 2; i++)
	   {
		  Vec2d &p1 = lsegments[2 * i];
		  Vec2d &p2 = lsegments[2 * i + 1];

		  double dist = Vec2d::distance(p1, p2);

		  if (dist > vars.graph.MinimalDistTresh)
			 graph.addBond(p1, p2);      
	   }

	   getLogExt().appendSkeleton(vars, "Source skeleton", (Skeleton::SkeletonGraph)graph);	   

	   graph.modifyGraph(vars);

	   getLogExt().appendSkeleton(vars, "Modified skeleton", (Skeleton::SkeletonGraph)graph);
   }
   
}
