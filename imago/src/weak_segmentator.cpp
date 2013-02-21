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

#include "weak_segmentator.h"
#include <queue>
#include <string.h>
#include "log_ext.h"
#include "pixel_boundings.h"
#include "thin_filter2.h"
#include "segment_tools.h"

namespace imago
{	
	Points2i WeakSegmentator::getLookupPattern(int range, bool fill)
	{
		Points2i result;
		for (int dx = -range; dx <= range; dx++)
			for (int dy = -range; dy <= range; dy++)
			{
				if (dx == 0 && dy == 0)
					continue;

				if (fill)
				{
					// add all points
					result.push_back(Vec2i(dx, dy));
				}
				else
				{
					// draw crosshair-like figure
					if (dx == 0 || dy == 0 || (int)(1 + sqrt((double)(dx*dx + dy*dy))) == range)
						result.push_back(Vec2i(dx, dy));					
				}
			}
		return result;
	}

	void WeakSegmentator::decorner(Image &img, byte set_to)
	{
		logEnterFunction();

		for (int y = 0; y < img.getHeight(); y++)
		{
			for (int x = 0; x < img.getWidth(); x++)
			{
				if (!img.isFilled(x,y))
					continue;

				if (SegmentTools::getInRange(img, Vec2i(x,y), 1).size() > 2)
					img.getByte(x,y) = set_to;
			}
		}

		getLogExt().appendImage("Decorner", img);
	}

	int WeakSegmentator::appendData(const Image& img, const Points2i& lookup_pattern, bool reconnect)
	{
		logEnterFunction();
			
		int added_pixels = 0;

		for (int y = 0; y < height(); y++)
			for (int x = 0; x < width(); x++)
				if (at(x,y) == 0 && (img.getByte(x,y) != 255))
				{
					int id = SegmentPoints.size()+1;
					fill(img, id, x, y, lookup_pattern, reconnect);			
					added_pixels += SegmentPoints[id].size();
				}

		getLogExt().append("Currently added pixels", added_pixels);
		getLogExt().append("Total segments count", SegmentPoints.size());

		return added_pixels;
	}

	bool WeakSegmentator::needCrop(const Settings& vars, Rectangle& crop, int winSize)
	{
		logEnterFunction();

		int area_pixels = round(width() * height() * vars.weak_seg.RectangularCropAreaTreshold);
		for (size_t id = 1; id <= SegmentPoints.size(); id++)
		{			
			Rectangle bounds;
			if (getRectangularArea(id) > area_pixels && hasRectangularStructure(vars, id, bounds, winSize))
			{
				getLogExt().append("Has rectangular structure, id", id);	
				bounds.adjustBorder(winSize*2);
				crop = bounds;
				return true;
			}
		}
		return false;
	}

	int WeakSegmentator::getRectangularArea(int id)
	{
		RectShapedBounding b(SegmentPoints[id]);		
		return b.getBounding().width * b.getBounding().height;
	}		

	bool WeakSegmentator::hasRectangularStructure(const Settings& vars, int id, Rectangle& bound, int winSize)
	{
		Points2i& p = SegmentPoints[id];
		
		std::vector<int> map_x;
		std::vector<int> map_y;

		for (Points2i::iterator it = p.begin(); it != p.end(); ++it)
		{
			if (it->x >= (int)map_x.size())
				map_x.resize(it->x + 1);
			map_x[it->x]++;

			if (it->y >= (int)map_y.size())
				map_y.resize(it->y + 1);
			map_y[it->y]++;
		}

		double x1c, x2c, y1c, y2c;			
		if (get2centers(map_x, x1c, x2c) && get2centers(map_y, y1c, y2c))
		{
			// now update maps
			map_x.clear();
			map_y.clear();

			for (Points2i::iterator it = p.begin(); it != p.end(); ++it)
			{
				if (it->y > y1c && it->y < y2c)
				{
					if (it->x >= (int)map_x.size())
						map_x.resize(it->x + 1);
					map_x[it->x]++;
				}
				if (it->x > x1c && it->x < x2c)
				{
					if (it->y >= (int)map_y.size())
						map_y.resize(it->y + 1);
					map_y[it->y]++;
				}
			}
			// and centers
			if (get2centers(map_x, x1c, x2c) && get2centers(map_y, y1c, y2c) &&
				fabs(x1c - x2c) > 2*winSize && fabs(y1c - y2c) > 2*winSize)
			{
				int good = 0, bad = 0;
				for (Points2i::iterator it = p.begin(); it != p.end(); ++it)
					if ((fabs(it->x - x1c) < winSize || fabs(it->x - x2c) < winSize) ||
						(fabs(it->y - y1c) < winSize || fabs(it->y - y2c) < winSize))
						good++;
					else
						bad++;
				if ((double)good / (good+bad) > vars.weak_seg.RectangularCropFitTreshold)
				{
					bound = Rectangle((int)x1c, (int)y1c, (int)x2c, (int)y2c, 0);
					return true;
				}
			}
		}

		return false;
	}

	void WeakSegmentator::fill(const Image& img, int& id, int sx, int sy, const Points2i& lookup_pattern, bool reconnect)
	{
		std::queue<Vec2i> v;
		v.push(Vec2i(sx,sy));
		while (!v.empty())
		{
			Vec2i cur = v.front();
			v.pop(); // remove top

			if (at(cur.x,cur.y) == 0)
			{
				at(cur.x,cur.y) = id;
				SegmentPoints[id].push_back(cur);

				for (size_t w = 0; w < lookup_pattern.size(); w++)
				{
					int dx = lookup_pattern[w].x;
					int dy = lookup_pattern[w].y;
					Vec2i t(cur.x + dx,cur.y + dy);
					if (inRange(t.x, t.y))
					{
						if (at(t.x, t.y) == 0)
						{					
							if (img.isFilled(t.x, t.y))
							{
								if (reconnect && (abs(dx) > 1 || abs(dy) > 1) )
								{
									v.push(Vec2i(cur.x + dx/2,cur.y + dy/2));
								}
								v.push(t);
							}
						}
						else if (at(t.x, t.y) != id)
						{
							int merge_id = at(t.x, t.y);
							for (size_t u = 0; u < SegmentPoints[id].size(); u++)
							{
								Vec2i p = SegmentPoints[id][u];
								at(p.x, p.y) = merge_id;
								SegmentPoints[merge_id].push_back(p);
							}
							SegmentPoints.erase(SegmentPoints.find(id));
							id = merge_id;
						} // if
					} // inRange
				} // for
			} // if
		} // while
	}

	bool WeakSegmentator::get2centers(const std::vector<int>& data, double &c1, double& c2) // c1 < c2
	{
		double mean = 0.0, count = 0.0;
		for (size_t u = 0; u < data.size(); u++)
		{
			mean += u * data[u];
			count += data[u];
		}			
			
		if (count < 1)
			return false;

		mean /= count;
		c1 = 0.0;
		c2 = 0.0;
		double count1 = 0.0;
		double count2 = 0.0;

		for (size_t u = 0; u < data.size(); u++)
		{
			if (u < mean)
			{
				c1 += u * data[u];
				count1 += data[u];
			}
			else
			{
				c2 += u * data[u];
				count2 += data[u];
			}
		}

		if (count1 < 1 || count2 < 1)
			return false;

		c1 /= count1;
		c2 /= count2;
		return true;
	}
}
