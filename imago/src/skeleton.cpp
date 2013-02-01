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
#include <set>
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/iteration_macros.hpp"
#include "boost/foreach.hpp"

#include "comdef.h"
#include "algebra.h"
#include "log_ext.h"
#include "skeleton.h"
#include "double_bond_maker.h"
#include "triple_bond_maker.h"
#include "multiple_bond_checker.h"
#include "image_draw_utils.h"
#include "image_utils.h"
#include "settings.h"

using namespace imago;

Skeleton::Skeleton()
{	
}

void Skeleton::setInitialAvgBondLength(Settings& vars, double avg_length )
{
   _avg_bond_length = avg_length;
   double mult = vars.skeleton.BaseMult;

   // TODO: depends on hard-set constants (something more adaptive required here)

   if (_avg_bond_length < vars.skeleton.ShortBondLen)
	   mult = vars.skeleton.ShortMul;
   else if (_avg_bond_length < vars.skeleton.MediumBondLen)
	   mult = vars.skeleton.MediumMul;
   else if (_avg_bond_length < vars.skeleton.LongBondLen)
	   mult = vars.skeleton.LongMul;
   
   _addVertexEps = mult * _avg_bond_length;
}

void Skeleton::recalcAvgBondLength()
{
   int bonds_num = num_edges(_g);

   if (bonds_num == 0)
      return;

   _avg_bond_length = 0;
   _min_bond_length = DIST_INF;

   BGL_FORALL_EDGES(e, _g, SkeletonGraph)
   {
      double len = (boost::get(boost::edge_type, _g, e)).length;
      _avg_bond_length += len;
      if (_min_bond_length > len)
         _min_bond_length = len;
   }

   _avg_bond_length /= bonds_num;
}

Skeleton::Edge Skeleton::addBond( Vertex &v1, Vertex &v2, BondType type, bool throw_if_error )
{
   std::pair<Edge, bool> p;

   p = boost::edge(v1, v2, _g);
   if (p.second)
   {
	   // Graph already has the edge
	   if (throw_if_error)
	   {
		   throw LogicException("Already has edge");
	   }
	   else
	   {
		   _warnings++;
		   std::ostringstream temp;
		   temp << "WARNING: Already has edge (" << v1 << ", " << v2 << ")";
		   getLogExt().appendText(temp.str());
		   return p.first;
	   }
   }

   p = boost::add_edge(v1, v2, _g);

   Vec2d begin = boost::get(boost::vertex_pos, _g, v1),
         end = boost::get(boost::vertex_pos, _g, v2);

   if (!p.second)
   {
	   std::ostringstream temp;
	   temp << "ERROR: Can't add bond (" << begin.x << ", " <<  begin.y << ") (" << end.x << ", " << end.y << ")";
	   getLogExt().appendText(temp.str());

      throw LogicException("Can't add bond");
   }

   Edge e = p.first;

   int dx = round(end.x - begin.x), dy = round(end.y - begin.y);
   double k = 0;

   if (dx == 0)
      k = HALF_PI;
   else
   {
      double angle = atan((double) dy / dx);
      k = angle;
   }

   Bond b(Vec2d::distance(begin, end), k, type);

   boost::put(boost::edge_type, _g, e, b);

   return e;
}

Skeleton::Edge Skeleton::addBond( const Vec2d &begin, const Vec2d &end,
                                  BondType type, bool throw_if_error )
{
   Vertex v1 = addVertex(begin), v2 = addVertex(end);

   return addBond(v1, v2, type, throw_if_error);
}

void Skeleton::removeBond( Vertex &v1, Vertex &v2 )
{
   boost::remove_edge(v1, v2, _g);
}

void Skeleton::removeBond( Edge &e )
{
   boost::remove_edge(e, _g);
}

Skeleton::Vertex Skeleton::addVertex( const Vec2d &pos )
{
   Vertex v = boost::add_vertex(_g);

   boost::put(boost::vertex_pos, _g, v, pos);

   return v;
}

Vec2d Skeleton::getVertexPos( const Vertex &v1 ) const
{
   return boost::get(boost::vertex_pos, _g, v1);
}

int Skeleton::getVerticesCount() const
{
   return boost::num_vertices(_g);
}

int Skeleton::getEdgesCount() const
{
   return boost::num_edges(_g);
}

Bond Skeleton::getBondInfo( const Edge &e ) const
{
   return boost::get(boost::edge_type, _g, e);
}

Skeleton::Vertex Skeleton::getBondBegin( const Skeleton::Edge &e ) const
{
   return boost::source(e, _g);
}

Skeleton::Vertex Skeleton::getBondEnd( const Skeleton::Edge &e ) const
{
   return boost::target(e, _g);
}

void Skeleton::_repairBroken(const Settings& vars)
{
   double coef;
   recalcAvgBondLength();

   // TODO: depends on hard-set constants (something more adaptive required here)

   double toSmallErr;
   if (_avg_bond_length > vars.skeleton.LongBondLen)
	   toSmallErr = vars.skeleton.LongSmallErr;
   else if (_avg_bond_length > vars.skeleton.MediumBondLen)
	   toSmallErr = vars.skeleton.MediumSmallErr;
   else
	   toSmallErr = vars.skeleton.BaseSmallErr;

   std::deque<Vertex> toRemove;

   BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
   {
      if (boost::degree(v, _g) != 2)
         continue;
      
      boost::graph_traits<SkeletonGraph>::adjacency_iterator
         vi = boost::adjacent_vertices(v, _g).first;
      Vertex x, y;
      x = *vi;
      y = *(++vi);
      Edge e1, e2;
      Bond e1b, e2b;
      e1 = boost::edge(x, v, _g).first;
      e2 = boost::edge(v, y, _g).first;
      e1b = getBondInfo(e1);
      e2b = getBondInfo(e2);

      if (e1b.length < toSmallErr * _avg_bond_length &&
          e2b.length < toSmallErr * _avg_bond_length)
         continue;

	  coef = vars.skeleton.BrokenRepairCoef1;
	  if (e1b.length < vars.skeleton.BrokenRepairFactor * toSmallErr * _avg_bond_length ||
          e2b.length < vars.skeleton.BrokenRepairFactor * toSmallErr * _avg_bond_length)
         coef = vars.skeleton.BrokenRepairCoef2;

      Vec2d x_pos, y_pos, v_pos;
      x_pos = boost::get(boost::vertex_pos, _g, x);
      y_pos = boost::get(boost::vertex_pos, _g, y);
      v_pos = boost::get(boost::vertex_pos, _g, v);
      Vec2d v1, v2;
      v1.diff(x_pos, v_pos);
      v2.diff(y_pos, v_pos);

      bool found = false;
      try
      {
         double angle = Vec2d::angle(v1, v2);
		 if (angle > PI - coef * vars.skeleton.BrokenRepairAngleEps)
            found = true;
      }
      catch (DivizionByZeroException &)
      {}

      if (found)
      {
         Edge e;
         Vertex to;
         if (e1b.length < e2b.length)
            e = e1, to = x;
         else
            e = e2, to = y;

         boost::remove_edge(e, _g);
         _reconnectBonds(v, to);
         toRemove.push_back(v);
      }
   }

   BOOST_FOREACH(Vertex v, toRemove)
      boost::remove_vertex(v, _g);
}

bool Skeleton::_isEqualDirection( const Edge &first, const Edge &second ) const
{
   Bond f = boost::get(boost::edge_type, _g, first),
        s = boost::get(boost::edge_type, _g, second);

   return (fabs(f.k - s.k) < _parLinesEps);
}

bool Skeleton::_isEqualDirection( const Vertex &b1,const Vertex &e1,const Vertex &b2,const Vertex &e2)  const
{
   Vec2d begin1 = boost::get(boost::vertex_pos, _g, b1),
         end1 = boost::get(boost::vertex_pos, _g, e1);

   Vec2d begin2 = boost::get(boost::vertex_pos, _g, b2),
         end2 = boost::get(boost::vertex_pos, _g, e2);
   
   int dx1 = round(end1.x - begin1.x), dy1 = round(end1.y - begin1.y);
   double k1 = 0;

   if (dx1 == 0)
      k1 = HALF_PI;
   else
   {
      double angle1 = atan((double) dy1 / dx1);
      k1 = angle1;
   }

   int dx2 = round(end2.x - begin2.x), dy2 = round(end2.y - begin2.y);
   double k2 = 0;

   if (dx2 == 0)
      k2 = HALF_PI;
   else
   {
      double angle2 = atan((double) dy2 / dx2);
      k2 = angle2;
   }

   return((fabs(k1 - k2) < _parLinesEps) && (sign(dx1) == sign(dx2)));
}


bool Skeleton::_isParallel( const Edge &first, const Edge &second ) const
{
   Bond f = boost::get(boost::edge_type, _g, first),
        s = boost::get(boost::edge_type, _g, second);

   return (fabs(f.k - s.k) < _parLinesEps ||
                    fabs(fabs(f.k - s.k) - PI) < _parLinesEps);
}

void Skeleton::calcShortBondsPenalty(const Settings& vars)
{
	logEnterFunction();

	int probablyWarnings = 0;
	int minSize = (std::max)((int)vars.dynamic.CapitalHeight / 2, vars.main.MinGoodCharactersSize);
	BGL_FORALL_EDGES(edge, _g, SkeletonGraph)
	{
		const Vertex &beg = boost::source(edge, _g);
		const Vertex &end = boost::target(edge, _g);

		double edge_len = boost::get(boost::edge_type, _g, edge).length;

		if (edge_len < minSize / 2)
			probablyWarnings += 2;
		else if (edge_len < minSize)
			probablyWarnings += 1;
	}
	getLogExt().append("probablyWarnings", probablyWarnings);
	_warnings += probablyWarnings / 2;
	getLogExt().append("_warnings updated", _warnings);	
}

void Skeleton::calcCloseVerticiesPenalty(const Settings& vars)
{
	logEnterFunction();

	int probablyWarnings = 0;
	BGL_FORALL_VERTICES(one, _g, SkeletonGraph)
	{
		BGL_FORALL_VERTICES(two, _g, SkeletonGraph)
		{
			if (one == two)
				continue;

			double dist = Vec2d::distance(getVertexPos(one), getVertexPos(two));

			if (!boost::edge(one, two, _g).second)
			{
				if (dist < vars.dynamic.CapitalHeight / 4)
					probablyWarnings += 2;
				else if (dist < vars.dynamic.CapitalHeight / 2)
					probablyWarnings += 1;
			}
		}		
	}
	getLogExt().append("probablyWarnings", probablyWarnings);
	_warnings += probablyWarnings / 2; // each counted twice
	getLogExt().append("_warnings updated", _warnings);	
}

bool Skeleton::_dissolveShortEdges (double coeff, const bool has2nb)
{

   BGL_FORALL_EDGES(edge, _g, SkeletonGraph)
   {
      const Vertex &beg = boost::source(edge, _g);
      const Vertex &end = boost::target(edge, _g);

      double edge_len = boost::get(boost::edge_type, _g, edge).length;
      double max_edge_beg = 0, max_edge_end = 0;
	  bool  pb_e = false, pb_b = false;


      // find the longest edge going from the beginning of our edge
      {
		 bool state_conected_b = false;
         std::deque<Vertex> neighbors_b;
         boost::graph_traits<SkeletonGraph>::adjacency_iterator b_b, e_b;
         boost::tie(b_b, e_b) = boost::adjacent_vertices(beg, _g);
         neighbors_b.assign(b_b, e_b);

		 if(neighbors_b.size() > 1)
			 for (size_t i = 0; i < neighbors_b.size(); i++)
			 {
				 if(neighbors_b[i] != end)
				 {
					 Edge ee = boost::edge(neighbors_b[i], beg, _g).first; // order is significant for taking edge with eqval direction
					 double len = boost::get(boost::edge_type, _g, ee).length;
					 state_conected_b = state_conected_b | _checkMidBonds(neighbors_b[i], beg);

					 if (len > max_edge_beg)
						 max_edge_beg = len;
					 if(!pb_b)
						 pb_b = _isEqualDirection(end, beg, neighbors_b[i], beg);
				 }
			 }
      

      // find the longest edge going from the end of our edge
      
		 bool state_conected_e = false;
         std::deque<Vertex> neighbors_e;
         boost::graph_traits<SkeletonGraph>::adjacency_iterator b_e, e_e;
         boost::tie(b_e, e_e) = boost::adjacent_vertices(end, _g);
         neighbors_e.assign(b_e, e_e);
		 if(neighbors_e.size() > 1)
			 for (size_t i = 0; i < neighbors_e.size(); i++)
			 {				 
				 if(neighbors_e[i] != beg)
				 {
					 Edge ee = boost::edge(neighbors_e[i], end, _g).first; // order is significant for taking edge with eqval direction
					 double len = boost::get(boost::edge_type, _g, ee).length;

					 if (len > max_edge_end)
						 max_edge_end = len;
					 state_conected_e = state_conected_e | _checkMidBonds(neighbors_e[i], end);
					 if(!pb_e)
						 pb_e = _isEqualDirection(beg, end, neighbors_e[i], end);
				 }
			 }
      
		  if(has2nb)
		  {
			  if (edge_len < max_edge_beg * (coeff) &&
				  edge_len < max_edge_end * (coeff) &&
				  edge_len < _avg_bond_length * coeff)
			  {
				  _dissolvings++;

				  std::ostringstream temp;
				  temp << "dissolving edge len: " << edge_len << ", max_edge_beg: " << max_edge_beg << ", max_edge_end: " << max_edge_end;
				  getLogExt().appendText(temp.str());

				  // dissolve the edge
				  if (max_edge_end < max_edge_beg)
				  {          
					  _reconnectBonds(end, beg);
					  boost::remove_vertex(end, _g);
				  }
				  else
				  {
					  _reconnectBonds(beg, end);
					  boost::remove_vertex(beg, _g);
				  }
				  return true;
			  }
			  else
			  {				  
				  if(( max_edge_end < _avg_bond_length * coeff) &&
					  (edge_len*(coeff) > max_edge_end) && absolute(max_edge_end) > EPS)
				  {
					  bool ret = false;
					   if(neighbors_e.size() > 1 && !state_conected_e)
						   for (size_t i = 0; i < neighbors_e.size(); i++)
						   {
							   if(neighbors_e[i] != beg)
							   {
								   _reconnectBonds(neighbors_e[i], end);
								   boost::remove_vertex(neighbors_e[i], _g);
								   ret = true;
							   }
						   }
					  if(ret)
						  return ret;
				  }

				  if(( max_edge_beg < _avg_bond_length * coeff) &&
				  (edge_len*(coeff) > max_edge_beg) && absolute(max_edge_beg) > EPS)
				  {

					  bool ret = false;
					   if(neighbors_b.size() > 1 && !state_conected_b)
						   for (size_t i = 0; i < neighbors_b.size(); i++)
						   {
							   if(neighbors_b[i] != end)
							   {
								   _reconnectBonds(neighbors_b[i], beg);
								   boost::remove_vertex(neighbors_b[i], _g);
								   ret = true;
							   }
						   }
					  if(ret)
						  return ret;
				  }
			  }
			  if(edge_len < _avg_bond_length * (coeff))
			  {
				  BondType type = getBondType(edge);
				  if(pb_e && !state_conected_b && type == BT_SINGLE)
				  {
					  _reconnectBonds(beg, end);
					  boost::remove_vertex(beg, _g);
					  return true;
					  
				  }
				  if(pb_b && !state_conected_e && type == BT_SINGLE)
				  {
					  _reconnectBonds(end, beg);
					  boost::remove_vertex(end, _g);
					  return true;
				  }
			  }
		  }
		  else
		  {
 			  if (edge_len < max_edge_beg * coeff ||
				  edge_len < max_edge_end * coeff)
			  {
				  _dissolvings++;

				  std::ostringstream temp;
				  temp << "dissolving edge len: " << edge_len << ", max_edge_beg: " << max_edge_beg << ", max_edge_end: " << max_edge_end;
				  getLogExt().appendText(temp.str());
				  
				  // dissolve the edge
				 if (max_edge_end < max_edge_beg)
				 {
					_reconnectBonds(end, beg);
					boost::remove_vertex(end, _g);
				 }
				 else
				 {
					_reconnectBonds(beg, end);
					boost::remove_vertex(beg, _g);
				 }
				 return true;
			  }
		  }
	  }

   }
   return false;
}

bool Skeleton::_dissolveIntermediateVertices (const Settings& vars)
{
   boost::graph_traits<SkeletonGraph>::vertex_iterator vi, vi_end;
   boost::tie(vi, vi_end) = boost::vertices(_g);

   Vertex to_dissolve;
   double min_err = 10000; // inf
   
   for (; vi != vi_end; ++vi)
   {
	   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

      const Vertex &vertex = *vi;

      std::deque<Vertex> neighbors;
      boost::graph_traits<SkeletonGraph>::adjacency_iterator b, e;
      boost::tie(b, e) = boost::adjacent_vertices(vertex, _g);
      neighbors.assign(b, e);

      if (neighbors.size() != 2)
         continue;

      //TODO: Need something more accurate

	  const Edge &edge1 = boost::edge(vertex, neighbors[0], _g).first;
      const Edge &edge2 = boost::edge(vertex, neighbors[1], _g).first;

      const Vertex &beg1 = boost::source(edge1, _g);
      const Vertex &beg2 = boost::source(edge2, _g);
      const Vertex &end1 = boost::target(edge1, _g);
      const Vertex &end2 = boost::target(edge2, _g);
      
      Vec2d dir1, dir2;

      if (beg1 == beg2 || end1 == end2)
      {
         dir1.diff(boost::get(boost::vertex_pos, _g, end1),
                   boost::get(boost::vertex_pos, _g, beg1));
         dir2.diff(boost::get(boost::vertex_pos, _g, end2),
                   boost::get(boost::vertex_pos, _g, beg2));
      }
      else if (beg1 == end2 || beg2 == end1)
      {
         dir1.diff(boost::get(boost::vertex_pos, _g, end1),
                   boost::get(boost::vertex_pos, _g, beg1));
         dir2.diff(boost::get(boost::vertex_pos, _g, beg2),
                   boost::get(boost::vertex_pos, _g, end2));
      }
      else
      {
         throw ImagoException("Edges are not adjacent");
      }

      double d = Vec2d::dot(dir1, dir2);
      double n1 = dir1.norm();
      double n2 = dir2.norm();
      double maxn = std::max(n1, n2);
      
      if (n1 * n2 > EPS) 
         d /= n1 * n2;
      
      double ang = acos(d);

      if (ang < PI * 3 / 4)
         continue;

      double err = n1 * n2 * sin(ang) / (maxn * maxn);

      if (err < min_err)
      {
         min_err = err;
         to_dissolve = vertex;
      }
   }
   
   if (min_err < vars.skeleton.DissolveMinErr) 
   {
	   _dissolvings++;
      
	   getLogExt().append("dissolving vertex, err", min_err);

      std::deque<Vertex> neighbors;
      boost::graph_traits<SkeletonGraph>::adjacency_iterator b, e;
      boost::tie(b, e) = boost::adjacent_vertices(to_dissolve, _g);
      neighbors.assign(b, e);
      addBond(neighbors[0], neighbors[1], BT_SINGLE);
      boost::clear_vertex(to_dissolve, _g); 
      boost::remove_vertex(to_dissolve, _g);
      return true;
   }

   return false;
}

void Skeleton::_findMultiple(const Settings& vars)
{
   DoubleBondMaker  _makeDouble(vars, *this);
   TripleBondMaker  _makeTriple(vars, *this);
   MultipleBondChecker _checker(vars, *this);
   
   std::vector<Edge> toProcess;
   boost::property_map<SkeletonGraph, boost::edge_type_t>::type types =
      boost::get(boost::edge_type, _g);
   boost::graph_traits<SkeletonGraph>::edge_iterator ei, ei_e;
   boost::tie(ei, ei_e) = boost::edges(_g);
   for (; ei != ei_e; ++ei)
      if (boost::get(types, *ei).type == BT_SINGLE)
         toProcess.push_back(*ei);

   std::map<Edge, bool> used;
   do
   {
      std::vector<std::pair<Edge, Edge> > doubleBonds;
      std::vector<boost::tuple<Edge, Edge, Edge> > tripleBonds;

      int end = toProcess.size();
      for (int ii = 0; ii < end; ii++)
      {
		  if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

         Edge i = toProcess[ii];
         if (used[i])
            continue;

         BGL_FORALL_EDGES(j, _g, SkeletonGraph)
         {
            if (i == j || boost::get(types, j).type != BT_SINGLE ||
                used[j])
               continue;

            if (!_checker.checkDouble(vars, i, j))
               continue;

            {
               std::pair<double, Edge*> arr[3];
               arr[0].first = boost::get(boost::edge_type, _g, i).length;
               arr[0].second = &i;
               arr[1].first = boost::get(boost::edge_type, _g, j).length;
               arr[1].second = &j;

               if (arr[1].first > arr[0].first)
                  std::swap(arr[0], arr[1]);

               bool is_triple = false;

               BGL_FORALL_EDGES(k, _g, SkeletonGraph)
               {
				   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

                  if (k == i || k == j ||
                      boost::get(types, k).type != BT_SINGLE ||
                      used[k])
                     continue;
                  
                  if (!_checker.checkTriple(vars, k))
                     continue;
                  
                  //Check degrees!
                  arr[2].first = boost::get(boost::edge_type, _g, k).length;
                  arr[2].second = &k;

                  if (arr[2].first > arr[0].first)
                  {
                     std::swap(arr[2], arr[0]);
                     std::swap(arr[1], arr[2]);
                  } else if (arr[2].first > arr[1].first)
                     std::swap(arr[1], arr[2]);

                  is_triple = true;
                  break;
               }

               if (is_triple)
               {
                  used[i] = used[j] = used[*arr[2].second] = true;
                  tripleBonds.push_back(boost::make_tuple(*arr[0].second,
                                                          *arr[1].second,
                                                          *arr[2].second));
               }
               else
               {
                  used[i] = used[j] = true;
                  doubleBonds.push_back(std::make_pair(*arr[0].second,
                                                       *arr[1].second));
               }
               break;
            }
         }
      }

      toProcess.erase(toProcess.begin(), toProcess.begin() + end);
      //Copy-paste
      for (size_t i = 0; i < tripleBonds.size(); ++i)
      {
         TripleBondMaker::Result ret = _makeTriple(tripleBonds[i]);

         int add = boost::get<0>(ret);
         if (add == 0)
            continue;
         
         used[boost::get<0>(tripleBonds[i])] = false;
         used[boost::get<1>(tripleBonds[i])] = false;
         used[boost::get<2>(tripleBonds[i])] = false;

         if (add == 1)
         {
            toProcess.push_back(boost::get<1>(ret));
         }
         else if (add == 2)
         {
            toProcess.push_back(boost::get<1>(ret));
            toProcess.push_back(boost::get<2>(ret));
         }
         
      }
      
      std::sort(doubleBonds.begin(), doubleBonds.end(),
                DoubleBondComparator<SkeletonGraph>(getGraph()));
            
      for (int i = 0; i < (int)doubleBonds.size(); ++i)
      {
         DoubleBondMaker::Result ret = _makeDouble(doubleBonds[i]);
         
         int add = boost::get<0>(ret);
         if (add == 0)
            continue;

         used[doubleBonds[i].first] = false;
         used[doubleBonds[i].second] = false;
         if (add == 1)
         {
            toProcess.push_back(boost::get<1>(ret));
         }
         else if (add == 2)
         {
            toProcess.push_back(boost::get<1>(ret));
            toProcess.push_back(boost::get<2>(ret));
         } 
      }

      types = boost::get(boost::edge_type, _g);
   } while (false);
   //} while (toProcess.size() != 0);
   _processInlineDoubleBond(vars);
}

void Skeleton::_processInlineDoubleBond(const Settings& vars)
{
	std::vector<Edge> toProcess, singles;
	std::vector<Edge>::iterator it, foundIt;

	double toSmallErr;
   if (_avg_bond_length > vars.skeleton.LongBondLen)
	   toSmallErr = vars.skeleton.LongSmallErr;
   else if (_avg_bond_length > vars.skeleton.MediumBondLen)
	   toSmallErr = vars.skeleton.MediumSmallErr;
   else
	   toSmallErr = vars.skeleton.BaseSmallErr;
   toSmallErr *= _avg_bond_length;

	BGL_FORALL_EDGES(j, _g, SkeletonGraph)
	{
		if(getBondType(j) == BT_SINGLE)
			singles.push_back(j);
		if(getBondType(j) == BT_DOUBLE)
			toProcess.push_back(j);
	}


	for(size_t i = 0;i < toProcess.size();i++)
	{
		bool foundInlineEdge = false;
		Vertex p1, p2, p3, p4;
		Vec2d p1b, p1e, p2b, p2e;

		for(it = singles.begin(); it != singles.end(); ++it)
		{
			if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

			Vec2d midOfSingle;

			p1= getBondBegin(toProcess[i]);
			p2= getBondEnd(toProcess[i]);
			p3= getBondBegin((*it));
			p4= getBondEnd((*it));

			p1b = getVertexPos(p1);
			p1e = getVertexPos(p2);
			p2b = getVertexPos(p3);
			p2e = getVertexPos(p4);

			midOfSingle.middle(p2b, p2e);

			if(Algebra::SegmentsOnSameLine(vars, p1b, p1e, p2b, p2e) && 
				(Vec2d::distance(midOfSingle, p1b) + Vec2d::distance(midOfSingle, p1e)) < Vec2d::distance(p2b, p2e))
			{
				foundIt = it;
				foundInlineEdge = true;
				break;
			}
		}
		if(foundInlineEdge)
		{
			removeBond(*foundIt);
			singles.erase(foundIt);

			if(Vec2d::distance(p1b, p2b) < Vec2d::distance(p1e, p2b))
			{
				if(Vec2d::distance(p1b, p2b) > toSmallErr)
					addBond(p1, p3);
				
				if(Vec2d::distance(p1e, p2e) > toSmallErr)
					addBond(p2, p4);
				
			}
			else
			{
				if(Vec2d::distance(p1b, p2e) > toSmallErr)
					addBond(p1, p4);
				if(Vec2d::distance(p1e, p2b) > toSmallErr)
					addBond(p2, p3);
			}
			
			setBondType(toProcess[i], BT_TRIPLE);
		}


	}
}

void Skeleton::_reconnectBonds( Vertex from, Vertex to )
{
   std::deque<Vertex> neighbours;
   boost::graph_traits<SkeletonGraph>::adjacency_iterator b, e;
   boost::tie(b, e) = boost::adjacent_vertices(from, _g);
   neighbours.assign(b, e);

   for (size_t i = 0; i < neighbours.size(); i++)
   {
      Vertex v = neighbours[i];
      Edge e = boost::edge(from, v, _g).first;
      BondType t = get(boost::edge_type, _g, e).type;
      boost::remove_edge(e, _g);

      if (v == to)
         continue;
      
      addBond(v, to, t);
   }
}

bool Skeleton::_checkMidBonds( Vertex from, Vertex to )
{
   std::deque<Vertex> neighbours;
   boost::graph_traits<SkeletonGraph>::adjacency_iterator b, e;
   boost::tie(b, e) = boost::adjacent_vertices(from, _g);
   neighbours.assign(b, e);
   bool ret = false;

   for (size_t i = 0; i < neighbours.size(); i++)
   {
      Vertex v = neighbours[i];
      Edge e = boost::edge(from, v, _g).first;

      if (v == to)
         continue;
      
      ret = true;
   }
   return ret;
}

void Skeleton::_reconnectBondsRWT( Vertex from, Vertex to, BondType new_t)
{
   std::deque<Vertex> neighbours;
   boost::graph_traits<SkeletonGraph>::adjacency_iterator b, e;
   boost::tie(b, e) = boost::adjacent_vertices(from, _g);
   neighbours.assign(b, e);

   for (size_t i = 0; i < neighbours.size(); i++)
   {
      Vertex v = neighbours[i];
      Edge e = boost::edge(from, v, _g).first;
      boost::remove_edge(e, _g);

      if (v == to)
         continue;
      
      addBond(v, to, new_t);
   }
}

double Skeleton::_avgEdgeLendth (const Vertex &v, int &nnei)
{
   std::deque<Vertex> neighbors;
   boost::graph_traits<SkeletonGraph>::adjacency_iterator b, e;
   boost::tie(b, e) = boost::adjacent_vertices(v, _g);
   neighbors.assign(b, e);

   nnei = neighbors.size();

   if (neighbors.size() < 1)
      return 0;

   double avg = 0;

   for (size_t i = 0; i < neighbors.size(); i++)
   {
      Edge e = boost::edge(v, neighbors[i], _g).first;
      avg += boost::get(boost::edge_type, _g, e).length;
   }
   return avg / neighbors.size();
}

void Skeleton::_joinVertices(double eps)
{
	logEnterFunction();
   std::deque<std::deque<Vertex> > nearVertices;
   std::deque<int> join_ind;
   boost::property_map<SkeletonGraph, boost::vertex_pos_t>::type pos =
           boost::get(boost::vertex_pos, _g);

#ifdef DEBUG
   LPRINT(0, "joining vertices, eps = %lf", eps);
#endif /* DEBUG */

   BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
   {
      Vec2d v_pos = pos[v];
      int v_nnei;
      double v_avg_edge_len = _avgEdgeLendth(v, v_nnei);

      for (size_t i = 0; i < nearVertices.size(); i++)
      {
         for (size_t j = 0; j < nearVertices[i].size(); j++)
         {
            const Vertex &nei = nearVertices[i][j];
            int nei_nnei;
            double nei_avg_edge_len = _avgEdgeLendth(nei, nei_nnei);
            double thresh = eps * (nei_nnei * nei_avg_edge_len + v_nnei * v_avg_edge_len) /
               (v_nnei + nei_nnei);

            if (v_nnei + nei_nnei > 0 &&
                Vec2d::distance(v_pos, pos[nei]) < thresh)
            {
               join_ind.push_back(i);
               break;
            }
         }
      }

      if (join_ind.size() == 0)
      {
         nearVertices.push_back(std::deque<Vertex>(1, v));
      }
      else
      {
         int first = join_ind[0];
         nearVertices[first].push_back(v);

         for (size_t i = join_ind.size() - 1; i >= 1; i--)
         {
            int ii = join_ind[i];
            nearVertices[first].insert(nearVertices[first].end(),
                                       nearVertices[ii].begin(),
                                       nearVertices[ii].end());
            nearVertices.erase(nearVertices.begin() + ii);
         }
      }
      join_ind.clear();
   }

   for (size_t i = 0; i < nearVertices.size(); i++)
   {
      size_t size = nearVertices[i].size();
      if (size == 1)
         continue;
      
      Vec2d newPos;
      for (size_t j = 0; j < size; j++)
         newPos.add(pos[nearVertices[i][j]]);
      newPos.scale(1.0 / size);

      Vertex newVertex = addVertex(newPos);

      for (size_t j = 0; j < size; j++)
      {
         _reconnectBonds(nearVertices[i][j], newVertex);
         boost::remove_vertex(nearVertices[i][j], _g);
      }
   }
}

bool Skeleton::_isSegmentIntersectedByEdge(const Settings& vars, Vec2d &b, Vec2d &e, std::deque<Edge> edges)
{
	std::deque<Edge>::iterator it;

	for(it=edges.begin();it != edges.end(); ++it)
	{
		Edge edge = *it;
		Vec2d p1 = getVertexPos(getBondBegin(edge));
		Vec2d p2 = getVertexPos(getBondEnd(edge));
		Vec2d intersection = Algebra::linesIntersection(vars, b, e, p1, p2);

		// TODO: check if intersection point is in segment p1, p2

		Vec2d diff1, diff2;
		diff1.diff(p1, intersection);
		diff2.diff(p2, intersection);
		double dotVal = Vec2d::dot(diff1, diff2);
		if( dotVal > 0)
			continue;
		
		double maxX = b.x > e.x ? b.x : e.x;
		double minX = b.x < e.x ? b.x : e.x;
		double maxY = b.y > e.y ? b.y : e.y;
		double minY = b.y < e.y ? b.y : e.y;

		if(intersection.x < maxX + 1 && intersection.x > minX-1 &&
			intersection.y < maxY + 1 && intersection.y > minY -1)
			return true;
	}
	return false;
}

void Skeleton::_connectBridgedBonds(const Settings& vars)
{
	logEnterFunction();

	std::vector<double> kFactor;
	std::vector<std::vector<Edge> > edge_groups_k;
	//group all parallel edges by similar factors
	BGL_FORALL_EDGES(edge, _g, SkeletonGraph)
	{
		Bond f = boost::get(boost::edge_type, _g, edge);
		Vec2d p1 = getVertexPos(getBondBegin(edge));
		Vec2d p2 = getVertexPos(getBondEnd(edge));
		double slope = Algebra::slope(p1, p2);
		if(f.type == BT_SINGLE)
		{
			bool found_kFactor = false;
			for(size_t i=0 ; i < kFactor.size() ; i++)
			{
				if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

				if(fabs(slope - kFactor[i]) < vars.skeleton.SlopeFact1 ||
					fabs(fabs(slope - kFactor[i]) - PI)< vars.skeleton.SlopeFact2)
				{
					edge_groups_k[i].push_back(edge);
					found_kFactor = true;
					break;
				}
			}

			if(!found_kFactor)
			{
				edge_groups_k.push_back(std::vector<Edge>());
				edge_groups_k[edge_groups_k.size() - 1].push_back(edge);
				kFactor.push_back(slope);
			}
		}
	}
	getLogExt().append("Group size of edges which could bridge:", edge_groups_k.size());
	std::deque<std::pair<Edge, Edge> > edges_to_connect;

	//check edges to be connected
	for(size_t i=0;i<edge_groups_k.size();i++)
	{
		int gr_count = edge_groups_k[i].size();
		if( gr_count == 1)
			continue;
		std::deque<Edge> otherE;
		for(size_t k=0;k<edge_groups_k.size();k++)
		{
			if(k != i)
				otherE.insert(otherE.end(), edge_groups_k[k].begin(), edge_groups_k[k].end());
		}
		for(int k=0;k<gr_count;k++)
		{
			Vec2d p1 = getVertexPos(getBondBegin(edge_groups_k[i][k]));
			Vec2d p2 = getVertexPos(getBondEnd(edge_groups_k[i][k]));

			for(int l = k + 1;l<gr_count;l++)
			{
				if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

				Vec2d sp1 = getVertexPos(getBondBegin(edge_groups_k[i][l]));
				Vec2d sp2 = getVertexPos(getBondEnd(edge_groups_k[i][l]));

				double d1 = Algebra::distance2segment(p1, sp1, sp2);
				double d2 = Algebra::distance2segment(p2, sp1, sp2);

				double min = d1 < d2 ? d1 : d2;

				double LineS = vars.dynamic.LineThickness;
				double blockS = LineS * vars.skeleton.ConnectBlockS;

				Vec2d nearP1, nearP2;
				if(d1 < d2)
					nearP1 = p1;
				else
					nearP1 = p2;

				double dd1 = Vec2d::distance<double>(nearP1, sp1);
				double dd2 = Vec2d::distance<double>(nearP1, sp2);
				
				if(dd1 < dd2)
					nearP2 = sp1;
				else
					nearP2 = sp2;

				if(min < blockS && min > vars.skeleton.ConnectFactor * LineS && 
					Algebra::SegmentsOnSameLine(vars, p1, p2, sp1, sp2) &&
					_isSegmentIntersectedByEdge(vars, nearP1, nearP2, otherE))
				{
					getLogExt().appendText("Candidate edges for bridge connections");
					getLogExt().append("Distance", min);
					edges_to_connect.push_back(std::pair<Edge, Edge>(edge_groups_k[i][l], edge_groups_k[i][k]));
				}
			}
		
		}
	}

	//connect edges
	std::deque<std::pair<Edge, Edge> >::iterator eit;
	std::vector<Vertex> verticies_to_remove;

	for(eit = edges_to_connect.begin(); eit != edges_to_connect.end(); ++eit)
	{
		Edge e1 = (*eit).first,
			e2 = (*eit).second;
		Vertex v1, v2, v3, v4;
		Vec2d p1 = getVertexPos(getBondBegin(e1));
		Vec2d p2 = getVertexPos(getBondEnd(e1));

		Vec2d sp1 = getVertexPos(getBondBegin(e2));
		Vec2d sp2 = getVertexPos(getBondEnd(e2));

		double d1 = Algebra::distance2segment(p1, sp1, sp2);
		double d2 = Algebra::distance2segment(p2, sp1, sp2);

		if(d1 < d2)
		{
			v1 = getBondBegin(e1);
			v3 = getBondEnd(e1);
			if(Vec2d::distance(p1, sp1) < Vec2d::distance(p1, sp2))
			{
				v2 = getBondBegin(e2);
				v4 = getBondEnd(e2);
			}
			else
			{
				v2 = getBondEnd(e2);
				v4 = getBondBegin(e2);
			}
		}
		else
		{
			v1 = getBondEnd(e1);
			v3 = getBondBegin(e1);
			if(Vec2d::distance(p2, sp1) < Vec2d::distance(p2, sp2))
			{
				v2 = getBondBegin(e2);
				v4 = getBondEnd(e2);
			}
			else
			{
				v2 = getBondEnd(e2);
				v4 = getBondBegin(e2);
			}
		}

		// ugly check for already removed verticies
		if (std::find(verticies_to_remove.begin(), verticies_to_remove.end(), v1) != verticies_to_remove.end() ||
			std::find(verticies_to_remove.begin(), verticies_to_remove.end(), v2) != verticies_to_remove.end() ||
			std::find(verticies_to_remove.begin(), verticies_to_remove.end(), v3) != verticies_to_remove.end() ||
			std::find(verticies_to_remove.begin(), verticies_to_remove.end(), v4) != verticies_to_remove.end())
			continue;

		if(boost::degree(v1, _g) > 1 ||
			boost::degree(v2, _g) > 1 )
		{
			continue;
		}

		addBond(v3, v4, BT_SINGLE);
		
		verticies_to_remove.push_back(v1);
		verticies_to_remove.push_back(v2);
	}

	for (size_t u = 0; u < verticies_to_remove.size(); u++)
	{
		boost::clear_vertex(verticies_to_remove[u], _g); 
		boost::remove_vertex(verticies_to_remove[u], _g);
	}
}

void Skeleton::modifyGraph(Settings& vars)
{
	logEnterFunction();

   //RecognitionSettings &rs = getSettings();

	_parLinesEps = vars.estimation.ParLinesEps;

   recalcAvgBondLength();

   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

   getLogExt().appendSkeleton(vars, "init", _g);

   _joinVertices(vars.skeleton.JoinVerticiesConst);

   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

   recalcAvgBondLength();

   _vertices_big_degree.clear();

   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");
   
   BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
   {
      if (boost::degree(v, _g) > 2)
      {
         Vec2d pos = boost::get(boost::vertex_pos, _g, v);
         _vertices_big_degree.push_back(pos);
      }
   }

   getLogExt().appendSkeleton(vars, "after join verticies", _g);

   while (_dissolveShortEdges(vars.skeleton.DissolveConst))
   {
	   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");
   }

   getLogExt().appendSkeleton(vars, "after dissolve short edges", _g);

   while (_dissolveIntermediateVertices(vars))
   {
	   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");
   }

   recalcAvgBondLength();

    getLogExt().appendSkeleton(vars, "after dissolve intermediate vertrices", _g);

    recalcAvgBondLength();

    _findMultiple(vars);

	if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");
    
	getLogExt().appendSkeleton(vars, "after find multiple", _g);

	_connectBridgedBonds(vars);

	if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

	getLogExt().appendSkeleton(vars, "after connecting bridge bonds", _g);

    recalcAvgBondLength();
   
	while (_dissolveShortEdges(vars.skeleton.Dissolve2Const))
	{
		if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");
	}

	getLogExt().appendSkeleton(vars, "after dissolve edges 2", _g);

    recalcAvgBondLength();

	_joinVertices(vars.skeleton.Join2Const);
	_joinVertices(vars.skeleton.Join3Const);

    //Shrinking short bonds (dots)
    std::vector<Edge> edgesToRemove;
    BGL_FORALL_EDGES(edge, _g, SkeletonGraph)
    {
       const Vertex &beg = boost::source(edge, _g);
       const Vertex &end = boost::target(edge, _g);
       Vec2d beg_pos = boost::get(boost::vertex_pos, _g, beg);
       const Vec2d &end_pos = boost::get(boost::vertex_pos, _g, end);
       if (boost::degree(beg, _g) == 1 && boost::degree(end, _g) == 1 &&
           boost::get(boost::edge_type, _g, edge).length < vars.skeleton.ShrinkEps * _avg_bond_length)
       {
          beg_pos.add(end_pos);
          beg_pos.scale(0.5); // average
          addVertex(beg_pos);
          edgesToRemove.push_back(edge);
       }
    }
    BOOST_FOREACH(Edge e, edgesToRemove)
    {
       Vertex beg = boost::source(e, _g);
       Vertex end = boost::target(e, _g);
       boost::remove_edge(e, _g);
       boost::remove_vertex(beg, _g);
       boost::remove_vertex(end, _g);
    }


	getLogExt().appendSkeleton(vars, "after shrinking", _g);


   // ---------------------------------------------------------
	   // analyze graph for vertex mess
	Image temp(vars.general.ImageWidth, vars.general.ImageHeight);

	double distTresh = vars.dynamic.CapitalHeight;

	   if (distTresh > _avg_bond_length/vars.skeleton.DistTreshLimFactor)
		   distTresh = _avg_bond_length/vars.skeleton.DistTreshLimFactor;

	   std::vector<Skeleton::Edge> bad_edges;
	   BGL_FORALL_EDGES(e, _g, SkeletonGraph)
	   {
		   const Skeleton::Vertex &beg = boost::source(e, _g);
		   const Skeleton::Vertex &end = boost::target(e, _g);
		   Vec2d pos_beg = boost::get(boost::vertex_pos, _g, beg);
		   Vec2d pos_end = boost::get(boost::vertex_pos, _g, end);
		   double d = Vec2d::distance(pos_beg, pos_end);
		   if (d < distTresh)
		   {
			   BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
			   {
				   if (vars.checkTimeLimit()) throw ImagoException("Timelimit exceeded");

				   if (v != beg && v != end)
				   {
					   Vec2d pos = boost::get(boost::vertex_pos, _g, v);
					   if (Vec2i::distance(pos,pos_beg) < distTresh &&
						   Vec2i::distance(pos,pos_end) < distTresh)
					   {
						   if (getLogExt().loggingEnabled())
						   {
							   ImageDrawUtils::putCircle(temp, round(pos.x), round(pos.y), 2, 0);							   
							   ImageDrawUtils::putLineSegment(temp, pos_beg, pos_end, 0);
						   }
						   bad_edges.push_back(e);
						   break;
					   }
				   }
			   }			   
		   }
	   }

	    BOOST_FOREACH(Skeleton::Edge e, bad_edges)
		{
		   Skeleton::Vertex beg = boost::source(e, _g);
		   Skeleton::Vertex end = boost::target(e, _g);
		   boost::remove_edge(e, _g);
		   if (boost::degree(beg, _g) == 0)
			   boost::remove_vertex(beg, _g);
		   if (boost::degree(end, _g) == 0)
			   boost::remove_vertex(end, _g);
		}

	   getLogExt().appendImage("Suspicious edges", temp);

	   // ---------------------------------------------------------


	   vars.dynamic.AvgBondLength = _avg_bond_length;
   
   BGL_FORALL_EDGES(edge, _g, SkeletonGraph)
   {
      const Vertex &beg = boost::source(edge, _g);
      const Vertex &end = boost::target(edge, _g);
      Vec2d beg_pos = boost::get(boost::vertex_pos, _g, beg);
      const Vec2d &end_pos = boost::get(boost::vertex_pos, _g, end);
#ifdef DEBUG
      printf("(%lf, %lf) - (%lf, %lf) | %lf\n", beg_pos.x, beg_pos.y, end_pos.x, end_pos.y, boost::get(boost::edge_type, _g, edge).length);
#endif
   }

}


void Skeleton::deleteBadTriangles( double eps )
{
   std::set<Edge> edges_to_delete;
   std::set<Vertex> vertices_to_delete;
   
   BGL_FORALL_EDGES(edge, _g, SkeletonGraph)
   {
      if (edges_to_delete.find(edge) != edges_to_delete.end())
         continue;
      
      Vertex beg = boost::source(edge, _g);
      Vertex end = boost::target(edge, _g);

      boost::graph_traits<SkeletonGraph>::adjacency_iterator b1, e1, b2, e2;
      std::set<Vertex> intrsect, beg_neigh, end_neigh;
      boost::tie(b1, e1) = boost::adjacent_vertices(beg, _g);
      boost::tie(b2, e2) = boost::adjacent_vertices(end, _g);
      beg_neigh.insert(b1, e1);
      end_neigh.insert(b2, e2);
      std::set_intersection(beg_neigh.begin(), beg_neigh.end(), end_neigh.begin(), end_neigh.end(), std::inserter(intrsect, intrsect.begin()));
      
      BOOST_FOREACH(Vertex v, intrsect)
      {
         if (v == beg || v == end || vertices_to_delete.find(v) != vertices_to_delete.end())
            continue;
         
         double l_b, l_e, l_be;
         //add asserts
         l_b = boost::get(boost::edge_type, _g, boost::edge(v, beg, _g).first).length;
         l_e = boost::get(boost::edge_type, _g, boost::edge(v, end, _g).first).length;         
         l_be = boost::get(boost::edge_type, _g, edge).length;
         if (fabs(l_b - (l_e + l_be)) < eps) //v - b
         {
            if (boost::degree(end, _g) == 2)
            {
               edges_to_delete.insert(edge);
               edges_to_delete.insert(boost::edge(v, end, _g).first);
               vertices_to_delete.insert(end);
               setBondType(boost::edge(v, beg, _g).first, BT_SINGLE_UP);
            }
            else
               edges_to_delete.insert(boost::edge(v, beg, _g).first);
            //v - e
            //edge
         }
         else if (fabs(l_e - (l_b + l_be)) < eps) //v - e
         {
            //v - b
            //edge
            if (boost::degree(beg, _g) == 2)
            {
               edges_to_delete.insert(edge);
               edges_to_delete.insert(boost::edge(v, beg, _g).first);
               vertices_to_delete.insert(beg);
               setBondType(boost::edge(v, end, _g).first, BT_SINGLE_UP);      
            }
            else
               edges_to_delete.insert(boost::edge(v, end, _g).first);
         }
         else if (fabs(l_be - (l_b + l_e)) < eps) //edge
         {
            //v - e
            //v - b
            if (boost::degree(v, _g) == 2)
            {
               edges_to_delete.insert(boost::edge(v, end, _g).first);
               edges_to_delete.insert(boost::edge(v, beg, _g).first);
               vertices_to_delete.insert(v);
               setBondType(edge, BT_SINGLE_UP);               
            }
            else
               edges_to_delete.insert(edge);
         }
      }
   }

   BOOST_FOREACH(Edge edge, edges_to_delete)
      boost::remove_edge(edge, _g);
   BOOST_FOREACH(Vertex v, vertices_to_delete)
      boost::remove_vertex(v, _g);
}

void Skeleton::setBondType( Edge e, BondType t )
{
   Bond b = boost::get(boost::edge_type, _g, e);

   b.type = t;

   boost::put(boost::edge_type, _g, e, b);
}

BondType Skeleton::getBondType( const Edge &e ) const
{
   return boost::get(boost::edge_type, _g, e).type;
}

void Skeleton::reverseEdge( const Edge &e )
{
   BondType type = getBondType(e);
   Vertex v1 = getBondBegin(e), v2 = getBondEnd(e);

   removeBond(v1, v2);
   addBond(v2, v1, type);
}

Skeleton::~Skeleton()
{
}

void Skeleton::clear()
{
   _g.clear();
   _warnings = 0;
   _dissolvings = 0;
}

