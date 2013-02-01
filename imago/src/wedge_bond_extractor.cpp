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

#include <vector>
#include <deque>

#include "boost/graph/iteration_macros.hpp"
#include "boost/foreach.hpp"

#include <opencv2/opencv.hpp>

#include "comdef.h"
#include "log_ext.h"
#include "image.h"
#include "image_utils.h"
#include "image_draw_utils.h"
#include "segment.h"
#include "stat_utils.h"
#include "molecule.h"
#include "skeleton.h"
#include "vec2d.h"
#include "wedge_bond_extractor.h"
#include "settings.h"
#include "algebra.h"

using namespace imago;

void edge_summary(imago::Skeleton &g)
{
	if(getLogExt().loggingEnabled())
	{
		std::map<std::string, int> bondtypes;
		Skeleton::SkeletonGraph graph = g.getGraph();
		BGL_FORALL_EDGES(b, graph, Skeleton::SkeletonGraph)
		{
			BondType bt = g.getBondType(b);
			std::string bts;
			switch(bt){
			case BT_SINGLE: bts = "SINGLE";
				break;
			case BT_DOUBLE: bts = "DOUBLE";
				break;
			case BT_TRIPLE: bts = "TRIPLE";
				break;
			case BT_AROMATIC: bts = "AROMATIC";
				break;
			case BT_SINGLE_UP: bts = "SINGLE_UP";
				break;
			case BT_SINGLE_DOWN: bts = "SINGLE_DOWN";
				break;
			case BT_ARROW: bts = "ARROW";
				break;
			case BT_WEDGE: bts = "WEDGE";
				break;
			case BT_SINGLE_UP_C: bts = "SINGLE_UP_C";
				break;
			case BT_UNKNOWN: bts = "UNKNOWN";
				break;
			}
			bondtypes[bts]++;
		}
		getLogExt().appendMap("bond types", bondtypes);
	}
}

WedgeBondExtractor::WedgeBondExtractor( SegmentDeque &segs, Image &img ) : _segs(segs), _img(img)
{
}


struct PointsComparator : public std::binary_function<WedgeBondExtractor::SegCenter,WedgeBondExtractor::SegCenter,bool>
{
	int PointsCompareDist;

	PointsComparator(int dist)
	{
		PointsCompareDist = dist;
	}

	inline bool operator()(const WedgeBondExtractor::SegCenter& c, const WedgeBondExtractor::SegCenter& d)
	{
		Vec2d a = c.center, b = d.center;

		bool res;

		if (a.x > b.x)  
			res = true;
		if (a.x < b.x) 
			res = false;   

		if (fabs(a.x - b.x) <= PointsCompareDist)
		{
			if (a.y > b.y)
				res = true;
			if (a.y < b.y)
				res = false;
			if (fabs(a.y - b.y) <= PointsCompareDist)
				res = false;
		}

		return res;
	}
};


void WedgeBondExtractor::_fitSingleDownBorders( Vec2d &p1, Vec2d &p2, Vec2d &v1, Vec2d &v2 )
{
   _intersectionContext ic;

   ic.img = &_img;

   ic.white_found = false;
   ic.intersection_point.x = -1;
   ic.intersection_point.y = -1;
   ImageDrawUtils::putLineSegment(p1, v1, 255, &ic, _intersectionFinderPlotCallBack);

   if (ic.intersection_point.x != -1 && ic.intersection_point.y != -1)
      p1 = ic.intersection_point;

   ic.white_found = false;
   ic.intersection_point.x = -1;
   ic.intersection_point.y = -1;
   ImageDrawUtils::putLineSegment(p2, v2, 255, &ic, _intersectionFinderPlotCallBack);

   if (ic.intersection_point.x != -1 && ic.intersection_point.y != -1)
      p2 = ic.intersection_point;
}

int WedgeBondExtractor::singleDownFetch(const Settings& vars, Skeleton &g )
{
   int sdb_count = 0;
   double eps = vars.wbe.SingleDownEps, angle;   

   std::vector<SegCenter> segs_info;
   std::vector<Segment *> to_delete_segs;

   for (SegmentDeque::iterator it = _segs.begin(); it != _segs.end(); ++it)
   {
      if (ImageUtils::testSlashLine(vars, **it, &angle, eps))
      {
         Vec2d a = (*it)->getCenter();
         segs_info.push_back(SegCenter(it, a, angle));
      }
   }

   if (segs_info.empty())
      return 0;

   for (size_t i = 0; i < segs_info.size(); i++)
   {
      segs_info[i].seginfo_index = i;
   }

   for (size_t i = 0; i < segs_info.size(); i++)
      for (size_t j = i + 1; j < segs_info.size(); j++)
      {
		  if (segs_info[i].used && segs_info[j].used && fabs(segs_info[i].angle - segs_info[j].angle) < vars.wbe.SomeTresh)
         {
            Vec2d p1 = segs_info[i].center, p2 = segs_info[j].center;   

            std::vector<SegCenter> cur_points;

            cur_points.push_back(segs_info[i]);
            cur_points.push_back(segs_info[j]);
            
            for (size_t k = 0; k < segs_info.size(); k++)
            {
               Vec2d p3;

               if (k != i && k != j && segs_info[k].used)
               {
                  p3 = segs_info[k].center;

				  if (absolute(p1.x - p2.x) <= vars.wbe.SingleDownCompareDist)
                  {
                     if (absolute(p1.x - p3.x) <= vars.wbe.SingleDownCompareDist || absolute(p3.x - p2.x) <= vars.wbe.SingleDownCompareDist)
                     {
                        cur_points.push_back(segs_info[k]);
                        continue;
                     }
                  }

                  if (absolute(p1.y - p2.y) <= vars.wbe.SingleDownCompareDist)
                  {
                     if (absolute(p1.y - p3.y) <= vars.wbe.SingleDownCompareDist || absolute(p3.y - p2.y) <= vars.wbe.SingleDownCompareDist)
                     {
                        cur_points.push_back(segs_info[k]);
                        continue;
                     }
                  }

                  double ch1 = (p1.x - p3.x) * (p2.y - p1.y);
                  double ch2 = (p1.x - p2.x) * (p3.y - p1.y);

				  if (absolute(ch1 - ch2) <= vars.wbe.SingleDownAngleMax)
                     cur_points.push_back(segs_info[k]);
               }
            }

			std::sort(cur_points.begin(), cur_points.end(), PointsComparator(vars.wbe.PointsCompareDist));

            if ((int)cur_points.size() >= vars.wbe.MinimalSingleDownSegsCount)
            {
               std::vector<IntPair> same_dist_pairs;
               DoubleVector distances(cur_points.size() - 1);

               for (size_t k = 0; k < cur_points.size() - 1; k++)
                  distances[k] = Vec2d::distance(cur_points[k + 1].center, cur_points[k].center);

               for (size_t k = 0; k < distances.size();)
               {
                  int l = k + 1;
                  IntPair p;

                  for (; l != (int)distances.size(); l++)
                  {
					  if (fabs(distances[l - 1] - distances[l]) > vars.wbe.SingleDownDistancesMax)
                        break;
                  }

                  p.first = k;
                  p.second = l;

                  same_dist_pairs.push_back(p);

                  k += l - k;
               }

               for (size_t k = 0; k < same_dist_pairs.size(); k++)
               {
                  IntPair p = same_dist_pairs[k];

                  if (p.second - p.first > 1)
                  {
                     double ave_dist = 0;

                     for (int l = p.first; l < p.second; l++)
                     {
                        ave_dist += distances[l];
                     }

                     ave_dist /= p.second - p.first;

					 if (ave_dist > vars.wbe.SingleDownLengthMax)
                        continue;

                     if (!segs_info[cur_points[p.first].seginfo_index].used ||
                         !segs_info[cur_points[p.second].seginfo_index].used)
                         continue;

                     Vec2d p1 = cur_points[p.first].center;
                     Vec2d p2 = cur_points[p.second].center; 
					 
                     double length = Vec2d::distance(p1, p2);

					 Vec2d orient;
                     orient.diff(p2, p1);
                     orient = orient.getNormalized();

                     orient.scale(length);

                     Vec2d v1 = p1, v2 = p2;

                     v1.diff(v1, orient);
                     v2.sum(v2, orient);

                     _fitSingleDownBorders(p1, p2, v1, v2);
                     g.addBond(p2, p1, BT_SINGLE_DOWN); 

                     sdb_count++;

                     for (int l = p.first; l <= p.second; l++)
                     {
                        // mark elements to delete from deque 
                        to_delete_segs.push_back(*cur_points[l].seg_iterator);
                        segs_info[cur_points[l].seginfo_index].used = false;
                     }
                  }
               }                             
            }
            cur_points.clear();
        }
      }

   // delete elements from queue (note: iterators invalidation after erase)
   for (SegmentDeque::iterator it = _segs.begin(); it != _segs.end();)
   {
      std::vector<Segment *>::iterator res = std::find(to_delete_segs.begin(), to_delete_segs.end(), *it);

      if (res != to_delete_segs.end())
      {
         delete *it;
         it = _segs.erase(it);
      }
      else
         ++it;
   }

   g.recalcAvgBondLength();

   Skeleton::SkeletonGraph graph = g.getGraph();   
  
   return sdb_count;
}

int WedgeBondExtractor::_radiusFinder( const Vec2d &v )
{
   _CircleContext cc;
   cc.img = &_img;
   cc.done = 0;
   int r = 1;

   while (!cc.done)
   {
      r++;
      ImageDrawUtils::putCircle(round(v.x), round(v.y), r, 0, &cc, _radiusFinderPlotCallback);
   }

   return r + 1;
}

bool WedgeBondExtractor::_radiusFinderPlotCallback( int x, int y, int color, void *userdata )
{
   _CircleContext *cc = (_CircleContext*) userdata;
   int w = cc->img->getWidth(), h = cc->img->getHeight();

   if (x >= 0 && y >= 0 && x < w && y < h)
   {
      if (cc->img->getByte(x, y) != 0) //Not black
         cc->done = 1;
   }
   else
      cc->done = 1;

   return true;
}

bool WedgeBondExtractor::_intersectionFinderPlotCallBack( int x, int y, int color, void *userdata )
{
   _intersectionContext *ic = (_intersectionContext *)userdata;
   int w = ic->img->getWidth(), h = ic->img->getHeight();

   if (x >= 0 && y >= 0 && x < w && y < h)
   {
      if (ic->img->getByte(x, y) == 255)
         ic->white_found = true;

      if (ic->img->getByte(x, y) == 0)
      {
         if (ic->white_found)
         {
            if (ic->intersection_point.x == -1 && ic->intersection_point.y == -1)
            {
               ic->intersection_point.x = x;
               ic->intersection_point.y = y;
            }
         }
      }
   }
   
   return true;
}

void WedgeBondExtractor::fixStereoCenters( Molecule &mol )
{
	logEnterFunction();

   Skeleton::SkeletonGraph &graph = mol.getSkeleton();
   const Molecule::ChemMapping &labels = mol.getMappedLabels();
   std::vector<Skeleton::Edge> to_reverse_bonds;
   BGL_FORALL_EDGES(b, graph, Skeleton::SkeletonGraph)
   {
      Bond b_bond = boost::get(boost::edge_type, graph, b);
      BondType type = b_bond.type;

      if (type == BT_SINGLE_DOWN || type == BT_SINGLE_UP_C)
      {
         bool begin_stereo = false, end_stereo = false;         
         Skeleton::Vertex v1 = boost::source(b, graph), 
            v2 = boost::target(b, graph);

         if (_checkStereoCenter(v1, mol))
            begin_stereo = true;

         if (_checkStereoCenter(v2, mol))
            end_stereo = true;

         if (!begin_stereo)
         {
            if (!end_stereo)
               mol.setBondType(b, BT_SINGLE);
			else
               to_reverse_bonds.push_back(b);
         }       
		 else
			 if(type == BT_SINGLE_UP_C)
				 mol.setBondType(b, BT_SINGLE_UP);
      }
   }

   BOOST_FOREACH( Skeleton::Edge e, to_reverse_bonds )
   {
	   Bond b_bond = boost::get(boost::edge_type, graph, e);
	   BondType type = b_bond.type;

	   if (type == BT_SINGLE_UP_C)
		   mol.setBondType(e, BT_SINGLE_UP);
      
	   mol.reverseEdge(e);
   }

   edge_summary(mol);
}

bool WedgeBondExtractor::_checkStereoCenter( Skeleton::Vertex &v, 
   Molecule &mol )
{
   _Configuration conf;
   const Skeleton::SkeletonGraph &graph = mol.getSkeleton();
   const Molecule::ChemMapping &labels = mol.getMappedLabels();

   Molecule::ChemMapping::const_iterator elem = labels.find(v);

   if (elem == labels.end())
   {
      conf.label_first = 'C';
      conf.label_second = 0;
      conf.charge = 0;
   }
   else
   {
	   conf.label_first = elem->second->satom.atoms[0].getLabelFirst();
	   conf.label_second = elem->second->satom.atoms[0].getLabelSecond();
	   conf.charge = elem->second->satom.atoms[0].charge;
   }

   conf.degree = boost::in_degree(v, graph);
   conf.n_double_bonds = 0;

   std::pair<Skeleton::EdgeIterator, Skeleton::EdgeIterator> p;

   p = boost::out_edges(v, graph);

   for (Skeleton::EdgeIterator it = p.first; it != p.second; ++it)
   {
      BondType type = boost::get(boost::edge_type, graph, *it).type;

      if (type == BT_DOUBLE)
         conf.n_double_bonds++;
   }

   static const _Configuration allowed_stereocenters [] = 
   {
      {'C', 0, 0, 3, 0 },
      {'C', 0, 0, 4, 0 },
      {'S', 'i', 0, 3, 0 },
      {'S', 'i', 0, 4, 0 },
      {'N', 0, 1, 3, 0 },
      {'N', 0, 1, 4, 0 },
      {'N', 0, 0, 3, 0 },
      {'N', 0, 0, 4, 2 },
      {'S', 0, 1, 3, 0 },
      {'S', 0, 0, 3, 1 },
      {'P', 0, 0, 3, 0 },
      {'P', 0, 1, 4, 0 },
      {'P', 0, 0, 4, 1 },
   };

   int arr_size = sizeof(allowed_stereocenters) / sizeof(allowed_stereocenters[0]);

   for (int i = 0; i < arr_size; i++)
   {
      if (allowed_stereocenters[i].label_first == conf.label_first && 
         allowed_stereocenters[i].label_second == conf.label_second && 
         allowed_stereocenters[i].charge == conf.charge &&
         allowed_stereocenters[i].degree == conf.degree && 
         allowed_stereocenters[i].n_double_bonds == conf.n_double_bonds)
      {
         return true;
      }
   }
   
   return false;
}

int WedgeBondExtractor::getVertexValence(Skeleton::Vertex &v, Skeleton &mol)
{
	std::deque<Skeleton::Vertex> neighbors;
	boost::graph_traits<Skeleton::SkeletonGraph>::adjacency_iterator b_e, e_e;
				
	boost::tie(b_e, e_e) = boost::adjacent_vertices(v, mol.getGraph());
	neighbors.assign(b_e, e_e);

	int retVal = 0;

	for(size_t i = 0; i < neighbors.size(); i++)
	{
		Skeleton::Edge edge = boost::edge(neighbors[i], v, mol.getGraph()).first;
		BondType bType = mol.getBondType(edge);
		switch(bType)
		{
		case BT_DOUBLE:
			retVal += 2;
		case BT_TRIPLE:
			retVal += 3;
		default:
			retVal++;	
		}
	}
	return retVal;
}

void WedgeBondExtractor::singleUpFetch(const Settings& vars, Skeleton &g )
{   
	logEnterFunction();

   int count = 0;

   Skeleton::SkeletonGraph &graph = g.getGraph();
   
   if (g.getEdgesCount() >= 1)
   {
      Image img;
      img.copy(_img);
	  _bond_length = vars.dynamic.AvgBondLength;

      _bfs_state.resize(_img.getWidth() * _img.getHeight());
      IntVector iqm_thick;

      BGL_FORALL_VERTICES(v, graph, Skeleton::SkeletonGraph) 
      {
         Vec2d v_vec2d = g.getVertexPos(v);

         int r = _radiusFinder(v_vec2d);
         _thicknesses.insert(std::make_pair(v, r));
         iqm_thick.push_back(r);
      }

      std::sort(iqm_thick.begin(), iqm_thick.end());
      _mean_thickness = StatUtils::interMean(iqm_thick.begin(), iqm_thick.end());
	 
      BGL_FORALL_EDGES(b, graph, Skeleton::SkeletonGraph)
      {
		  Skeleton::Vertex v1 = boost::source(b, graph),
			  v2 = boost::target(b, graph);
		 
		 BondType bond_type = BT_SINGLE;

         if (_isSingleUp(vars, g, b, bond_type))
         {
			 if(bond_type == BT_WEDGE)
			 {
				 int b_val =  getVertexValence(v1, g);
				 int e_val = getVertexValence(v2, g);
				 if(e_val == b_val)
					 continue;
			 }
            count++;
            g.setBondType(b, bond_type);
         }
      }

      _thicknesses.clear();
      _bfs_state.clear();
   }

   for(size_t i = 0; i < _bonds_to_reverse.size(); i++)
   {
	   g.reverseEdge(_bonds_to_reverse[i]);
   }
   _bonds_to_reverse.clear();

   CurateSingleUpBonds(g);
   edge_summary(g);
   getLogExt().append("Single-up bonds", count);
}

bool WedgeBondExtractor::_isSingleUp(const Settings& vars, Skeleton &g, Skeleton::Edge &e1, BondType &return_type )
{
	logEnterFunction();
	
   Bond bond = g.getBondInfo(e1);

   if (bond.type != BT_SINGLE)
      return false;

   Vec2d bb = g.getVertexPos(g.getBondBegin(e1));
   Vec2d ee = g.getVertexPos(g.getBondEnd(e1));
   int r1 = _thicknesses[g.getBondBegin(e1)],
	   r2 = _thicknesses[g.getBondEnd(e1)];
   int max_r = r1 > r2 ? r1 : r2;
   int min_r = r1 < r2 ? r1 : r2;

   double coef = vars.wbe.SingleUpDefCoeff;
   if (_bond_length < vars.wbe.SingleUpIncLengthTresh)
	   coef = vars.wbe.SingleUpIncCoeff;

   if (Vec2d::distance(bb, ee) < _bond_length * coef)
      return false;
   double interpolation_factor = 0.1;

   Vec2d b(bb), e(ee);
   b.interpolate(bb, ee, interpolation_factor); //vars.wbe.SingleUpInterpolateEps);
   e.interpolate(ee, bb, interpolation_factor); //vars.wbe.SingleUpInterpolateEps);
   b.x = round(b.x);
   b.y = round(b.y);
   e.x = round(e.x);
   e.y = round(e.y);
   Vec2d n;
   n.diff(e, b);
   n = n.getNormalized();
   int size = round(abs(n.x * (e.x - b.x)+ n.y * (e.y - b.y)));
   std::vector<int> profile(size);
   
   int w = _img.getWidth();
   int h = _img.getHeight();
   Image img(w, h);
   img.fillWhite();

   Points2d visited;
   std::deque<Vec2d> queue;

   queue.push_back(b);
   while (!queue.empty())
   {
      Vec2d cur = queue.front();

      queue.pop_front();

      visited.push_back(cur);

      double dp = Vec2d::distance(cur, e);
	  dp = sqrt(dp * dp + 1) + vars.wbe.SingleUpMagicAddition;
      for (int i = round(cur.x) - 1; i <= round(cur.x) + 1; i++)
      {
         for (int j = round(cur.y) - 1; j <= round(cur.y) + 1; j++)
         {
            if (i == round(cur.x) && j == round(cur.y))
               continue;

            if (i < 0 || j < 0 || i >= w || j >= h)
               continue;

            if (_img.getByte(i, j) != 255 && !_bfs_state[j * w + i])
            {
               Vec2d v(i, j);
               double dist = Vec2d::distance(v, e);
               if (dist <= dp)
               {
                  queue.push_back(v);
                  _bfs_state[j * w + i] = 1;
				  img.getByte(i, j) = 0;
				  int indx = round(n.x * (i - b.x) + n.y * (j - b.y));
				  if(indx > -1 && indx < (int)profile.size())
					  profile[indx]++;
               }
            }
         }
      }

	  
   }
   getLogExt().appendVector("profile ", profile);
   getLogExt().appendImage("image profile", img);

   double y_mean = 0, x_mean = 0;
   int startProfile = round(vars.wbe.SingleUpInterpolateEps * profile.size());
   int endProfile = profile.size() - startProfile;
   
   int psize = endProfile - startProfile;//(profile.size() - 1);
   
   for(int i = startProfile; i < endProfile; i++)
   {
	   
	   if( profile[i] == 0)
		   psize--;
	   else
	   {
		   x_mean += i;
		   y_mean += profile[i];
	   }
   }


   if( psize < (int)profile.size() / 4 )
	   return false;

   y_mean /= psize;
   x_mean /= psize;

   double Sxx=0, Sxy=0;
   double max_val = 0;
   for(int i = startProfile; i < (startProfile + psize); i++)
   {
	   if(profile[i] != 0 )
	   {
		   double xx = i - x_mean;
		   double xy = (profile[i] - y_mean) * (i - x_mean);
		   Sxx += xx * xx;
		   Sxy += xy;
		   if(max_val < profile[i])
			   max_val = profile[i];
	   }
   }

   double b_coeff = Sxy / Sxx;
   getLogExt().append("Slope coefficient", b_coeff);
   
   for (size_t i = 0; i < visited.size(); i++)
   {
	   int y = round(visited[i].y);
	   int x = round(visited[i].x);
	   if (y >= 0 && x >= 0 && y < _img.getHeight() && x < _img.getWidth())
		   _bfs_state[y * w + x] = 0;
   }

   if( abs(b_coeff) > vars.wbe.SingleUpSlopeThresh && (y_mean > vars.dynamic.LineThickness || max_r / min_r > 2) )
   {
	   return_type = BT_SINGLE_UP;
	   if( b_coeff < 0 )
		   _bonds_to_reverse.push_back(e1);//g.reverseEdge(e1);
	   return true;
   }
   else
	   if( y_mean / vars.dynamic.LineThickness > vars.wbe.SingleUpThickThresh)
	   {
		   return_type = BT_WEDGE;
		   return true;
	   }
   return false;
}

void WedgeBondExtractor::CurateSingleUpBonds(Skeleton &graph)
{
	Skeleton::SkeletonGraph &g = graph.getGraph();
	BGL_FORALL_EDGES(e, g, Skeleton::SkeletonGraph)
	{
		BondType edge_type = graph.getBondType(e);
		if(edge_type == BT_SINGLE_UP)
		{
			Skeleton::Vertex b_v = graph.getBondBegin(e);
			Skeleton::Vertex e_v = graph.getBondEnd(e);
			int v1 = getVertexValence(b_v, graph),
				v2 = getVertexValence(e_v, graph);
			if(v1 == v2 && v1 == 1)
				graph.setBondType(e, BT_SINGLE);
		}
		if(edge_type == BT_WEDGE)
		{
			Skeleton::Vertex b_v = graph.getBondBegin(e);
			Skeleton::Vertex e_v = graph.getBondEnd(e);
			bool has_single_begin = false,
				has_single_end = false;

			std::deque<Skeleton::Vertex> neighbors;
			boost::graph_traits<Skeleton::SkeletonGraph>::adjacency_iterator b_e, e_e;
			boost::tie(b_e, e_e) = boost::adjacent_vertices(b_v, g);
			neighbors.assign(b_e, e_e);
			
			//check edges from beginning vertex
			for(size_t i = 0; i < neighbors.size(); i++)
			{
				if(neighbors[i] != e_v)
				{
					Skeleton::Edge edge = boost::edge(neighbors[i], b_v, g).first;
					if(graph.getBondType(edge) == BT_SINGLE_UP)
					{
						has_single_begin = true;
						break;
					}
				}
			}

			//check edges from ending vertex
			boost::tie(b_e, e_e) = boost::adjacent_vertices(e_v, g);
			neighbors.assign(b_e, e_e);
			for(size_t i = 0; i < neighbors.size(); i++)
			{
				if(neighbors[i] != b_v)
				{
					Skeleton::Edge edge = boost::edge(neighbors[i], e_v, g).first;
					if(graph.getBondType(edge) == BT_SINGLE_UP)
					{
						has_single_end = true;
						break;
					}
				}
			}

			if( has_single_begin && has_single_end)
				graph.setBondType(e, BT_SINGLE);
		}
	}

	BGL_FORALL_EDGES(e, g, Skeleton::SkeletonGraph)
	{
		BondType edge_type = graph.getBondType(e);
		if(edge_type == BT_WEDGE)
			graph.setBondType(e, BT_SINGLE_UP_C);
	}
}

void WedgeBondExtractor::fetchArrows(const Settings& vars, Skeleton &g )
{
	Skeleton::SkeletonGraph &graph = g.getGraph();
	BGL_FORALL_EDGES(e, graph, Skeleton::SkeletonGraph)
	{
		BondType edge_type = g.getBondType(e);
		if(edge_type == BT_SINGLE_UP)
		{
			Skeleton::Vertex b = g.getBondBegin(e);
			Skeleton::Vertex e_v = g.getBondEnd(e);
			int b_deg = boost::degree(b, graph);
			int e_deg = boost::degree(e_v, graph);
			int min_deg = b_deg < e_deg ? b_deg : e_deg;
			int max_deg = b_deg > e_deg ? b_deg : e_deg;

			if(min_deg == 1  && max_deg == 2)
			{
				Skeleton::Vertex v = b;
				if(e_deg == 2)
					v = e_v;
				std::deque<Skeleton::Vertex> neighbors;
				boost::graph_traits<Skeleton::SkeletonGraph>::adjacency_iterator b_e, e_e;
				boost::tie(b_e, e_e) = boost::adjacent_vertices(v, graph);
				neighbors.assign(b_e, e_e);

				for(size_t i = 0; i < neighbors.size(); i++)
				{
					if(neighbors[i] != b && neighbors[i] != e_v && 
						boost::degree(neighbors[i], graph) == 1)
					{
						Skeleton::Edge edge = boost::edge(neighbors[i], v, graph).first;

						if (g.getBondType(edge) == BT_SINGLE)
						{
							Vec2d v1 = g.getVertexPos(neighbors[i]);
							Vec2d v2 = g.getVertexPos(v);
							Vec2d v3 = g.getVertexPos(v);
							Vec2d v4 = g.getVertexPos(b_deg == 1 ? b : e_v);

							if (Algebra::SegmentsOnSameLine(vars, v1, v2, v3, v4))
							{
								// arrow found 
								// TODO: handle properly
								g.removeBond(edge);
								g.removeBond(v, (b_deg == 1 ? b : e_v));
								g.addBond(neighbors[i], (b_deg == 1 ? b : e_v), BT_ARROW, true);
							}
						}
					}
				}
			}
		}
	}
}

WedgeBondExtractor::~WedgeBondExtractor()
{
}
