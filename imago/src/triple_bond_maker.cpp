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

#include <set>
#include "boost/foreach.hpp"
#include <cstdio>
#include "triple_bond_maker.h"
#include "algebra.h"
#include "settings.h"

using namespace imago;

TripleBondMaker::TripleBondMaker( const Settings& settings, Skeleton &s ) : _s(s), _g(_s.getGraph()), vars(settings)
{
   _avgBondLength = _s.bondLength();
}

TripleBondMaker::~TripleBondMaker()
{
}

TripleBondMaker::Result TripleBondMaker::_validateVertices()
{
   std::set<Vertex> vertices;
   boost::graph_traits<Graph>::vertex_iterator vi, vi_end;
   boost::tie(vi, vi_end) = boost::vertices(_g);
   vertices.insert(vi, vi_end);
   std::set<Vertex>::iterator it_end = vertices.end();
   bool firstValid, secondValid, thirdValid;
   firstValid = secondValid = thirdValid = true;
   if (vertices.find(fb) == it_end ||
       vertices.find(fe) == it_end)
      firstValid = false;

   if (vertices.find(sb) == it_end ||
       vertices.find(se) == it_end)
      secondValid = false;

   if (vertices.find(tb) == it_end ||
       vertices.find(te) == it_end)
      thirdValid = false;   

   if (firstValid && secondValid && thirdValid)
      return boost::make_tuple(3, empty, empty);
   else if (firstValid && secondValid)
      return boost::make_tuple(2, first, second);
   else if (firstValid && thirdValid)
      return boost::make_tuple(2, first, third);
   else if (thirdValid && secondValid)
      return boost::make_tuple(2, second, third);
   else if (firstValid)
      return boost::make_tuple(1, first, empty);
   else if (secondValid)
      return boost::make_tuple(1, second, empty);
   else if (thirdValid)
      return boost::make_tuple(1, third, empty);
   else
      return boost::make_tuple(0, empty, empty);
}

TripleBondMaker::Result TripleBondMaker::operator ()(
                           boost::tuple<Edge,Edge,Edge> edges)
{
   std::set<Vertex> toRemove;
   boost::tie(first, second, third) = edges;

   fb = boost::source(first, _g), fe = target(first, _g);
   sb = boost::source(second, _g), se = target(second, _g);
   tb = boost::source(third, _g), te = target(third, _g);

   Result ret = _validateVertices();

   if (boost::get<0>(ret) != 3)
      return ret;

   //Validating edges
   if (!boost::edge(fb, fe, _g).second ||
       !boost::edge(sb, se, _g).second ||
       !boost::edge(tb, te, _g).second)
      return boost::make_tuple(0, empty, empty);
   
   boost::property_map<Graph, boost::vertex_pos_t>::type
      positions = boost::get(boost::vertex_pos, _g);

   Vec2d fb_pos = positions[fb], fe_pos = positions[fe],
         sb_pos = positions[sb], se_pos = positions[se],
         tb_pos = positions[tb], te_pos = positions[te];

   if (Vec2d::distance(fb_pos, sb_pos) > Vec2d::distance(fb_pos, se_pos))
   {
      std::swap(sb, se);
      std::swap(sb_pos, se_pos);
   }
   if (Vec2d::distance(sb_pos, tb_pos) > Vec2d::distance(sb_pos, te_pos))
   {
      std::swap(tb, te);
      std::swap(tb_pos, te_pos);
   }
   
   Vec2d p1, p2;
   double l1, l2;

   boost::tie(p1, p2) = Algebra::segmentProjection(sb_pos, se_pos,
                                                   fb_pos, fe_pos);

   l1 = Vec2d::distance(fb_pos, p1);
   l2 = Vec2d::distance(p2, fe_pos);

   bool left = l1 > vars.mbond.TripleLeftLengthTresh * _avgBondLength;
   bool right = l2 > vars.mbond.TripleRightLengthTresh * _avgBondLength;
   
   boost::remove_edge(first, _g);
   boost::remove_edge(second, _g);
   boost::remove_edge(third, _g);

   if (left || right)
   {
      toRemove.insert(sb);
      toRemove.insert(se);
      toRemove.insert(tb);
      toRemove.insert(te);
   }
      
   if (left && right)
   {
      Vertex v1 = _s.addVertex(p1), v2 = _s.addVertex(p2);

      Edge e1 = _s.addBond(fb, v1),
           e2 = _s.addBond(v1, v2, BT_TRIPLE),
           e3 = _s.addBond(v2, fe);

      ret = boost::make_tuple(2, e1, e3);
   }
   else if (left)
   {
      Vertex v = _s.addVertex(p1);

      Edge e1 = _s.addBond(fb, v, BT_SINGLE),
           e2 = _s.addBond(v, fe, BT_TRIPLE);
      
      ret = boost::make_tuple(1, e1, empty);
   }
   else if (right)
   {
      Vertex v = _s.addVertex(p2);
      
      Edge e1 = _s.addBond(fb, v, BT_TRIPLE),
           e2 = _s.addBond(v, fe, BT_SINGLE);
      ret = boost::make_tuple(1, e2, empty);
   }
   else
   {
      p1.set((fb_pos.x + sb_pos.x + tb_pos.x) / 3,
             (fb_pos.y + sb_pos.y + tb_pos.y) / 3);
      p2.set((fe_pos.x + se_pos.x + te_pos.x) / 3,
             (fe_pos.y + se_pos.y + te_pos.y) / 3);
      
      Vertex v1 = _s.addVertex(p1), v2 = _s.addVertex(p2);
      Vertex *mbegin, *mend;
      double dist;

      if ( (l1 = Vec2d::distance(p1, fb_pos)) <
           (l2 = Vec2d::distance(p1, sb_pos)))
         mbegin = &fb, mend = &fe, dist = l1;
      else
         mbegin = &sb, mend = &se, dist = l2;

      if (Vec2d::distance(p1, tb_pos) < dist)
         mbegin = &tb, mend = &te;

      _s._reconnectBonds(*mbegin, v1);
      _s._reconnectBonds(*mend, v2);

      toRemove.insert(fb);
      toRemove.insert(fe);
      toRemove.insert(sb);
      toRemove.insert(se);
      toRemove.insert(tb);
      toRemove.insert(te);
      
      Edge e = _s.addBond(v1, v2, BT_TRIPLE);

      ret = boost::make_tuple(0, empty, empty);
   }

   BOOST_FOREACH(Vertex v, toRemove)
      if (boost::degree(v, _g) == 0)
         boost::remove_vertex(v, _g);
   
   return ret;
}
