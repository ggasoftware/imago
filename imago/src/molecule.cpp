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

#include "boost/foreach.hpp"
#include "boost/graph/iteration_macros.hpp"

#include "algebra.h"
#include "molecule.h"
#include "skeleton.h"
#include "exception.h"
#include "segment.h"
#include "log_ext.h"
#include "settings.h"

using namespace imago;


Molecule::Molecule()
{
}

const Molecule::ChemMapping &Molecule::getMappedLabels() const
{
   return _mapping;
}

Molecule::ChemMapping &Molecule::getMappedLabels()
{
   return _mapping;
}

const std::deque<Label> &Molecule::getLabels() const
{
   return _labels;
}

std::deque<Label> &Molecule::getLabels()
{
   return _labels;
}

const Skeleton::SkeletonGraph & Molecule::getSkeleton() const
{
   return _g;
}

Skeleton::SkeletonGraph & Molecule::getSkeleton()
{
   return _g;
}

bool testNear(Vec2d &point, Rectangle &rec, double margin)
{
	double top = rec.y - margin;  //std::max<int>(0, rec.y - margin);
	double left = rec.x - margin; //std::max<int>(0, rec.x - margin);

	return point.x < (rec.x + rec.width + margin) &&
		point.x > left && 
		point.y < (rec.y + rec.height + margin) &&
		point.y > top;
}

//returns true if edge is directed to rectangle
// end is the closest point to rectangle
bool testCollision(Vec2d &beg, Vec2d &end, Rectangle &rec)
{
	Vec2d perp(-(end.y - beg.y), end.x - beg.x);
	try
	{
		perp = perp.getNormalized();
	}
	catch(DivizionByZeroException &e)
	{
		getLogExt().appendText(e.what());
		return false;
	}
	//perp.diff(perp, end);

	Vec2d vec1(rec.x, rec.y), vec2(rec.x + rec.width, rec.y),
		vec3(rec.x, rec.y + rec.height), vec4(rec.x+rec.width, rec.y+rec.height);
	vec1.diff(vec1, end);
	vec2.diff(vec2, end);
	vec3.diff(vec3, end);
	vec4.diff(vec4, end);

	double a1 = Vec2d::dot(perp, vec1);
	double a2 = Vec2d::dot(perp, vec2);
	double a3 = Vec2d::dot(perp, vec3);
	double a4 = Vec2d::dot(perp, vec4);

	double min = std::min<double>(a1, 
		std::min<double>(a2, 
		std::min(a3, a4)));
	double max = std::max<double>(a1, 
		std::max<double>(a2, 
		std::max(a3, a4)));
	return min*max < 0;
}

void Molecule::mapLabels(const Settings& vars, std::deque<Label> &unmapped_labels )
{
   double space, space2;
   double bl = bondLength();
   if (bl > vars.molecule.LengthValue_long)
	   space = vars.molecule.LengthFactor_long;
   else if (bl > vars.molecule.LengthValue_medium)
	   space = vars.molecule.LengthFactor_medium; 
   else
	   space = vars.molecule.LengthFactor_default; 

//   printf("****: %lf %lf\n", bl, space);

   std::vector<Skeleton::Vertex> nearest;

   labels.assign(_labels.begin(), _labels.end());
   
   for (size_t i = 0; i < labels.size(); ++i)
   {
      Label &l = labels[i];
#ifdef DEBUG
      printf("LABELS: %d %d\n", l.rect.x, l.rect.y);
#endif


      nearest.clear();
	  space = l.MaxSymbolWidth() * vars.molecule.SpaceMultiply;
	  space2 = l.rect.width < l.rect.height ? l.rect.width : l.rect.height;
	   
	     boost::property_map<SkeletonGraph, boost::vertex_pos_t>::type
                              positions = boost::get(boost::vertex_pos, _g);

      BGL_FORALL_EDGES(e, _g, SkeletonGraph)
      {
		  if (vars.checkTimeLimit())
			  throw ImagoException("Timelimit exceeded");

         double d1, d2;
         d1 = d2 = DIST_INF;

		 if(boost::degree(boost::source(e, _g), _g) > 1 &&
			 boost::degree(boost::target(e, _g), _g) > 1)
			 continue;

		 if (boost::degree(boost::source(e, _g), _g) == 1)
            d1 = Algebra::distance2rect(boost::get(positions,
                                                 boost::source(e, _g)), l.rect);

         if (boost::degree(boost::target(e, _g), _g) == 1)
            d2 = Algebra::distance2rect(boost::get(positions,
                                                 boost::target(e, _g)), l.rect);

		 if (d1 <= d2 && ((testCollision(boost::get(positions, boost::target(e, _g)), boost::get(positions, boost::source(e, _g)), l.rect) &&
			 testNear(boost::get(positions, boost::source(e, _g)), l.rect, space)) ||
			 testNear(boost::get(positions, boost::source(e, _g)), l.rect, space2/2)))
            nearest.push_back(boost::source(e, _g));
         else if (d2 < d1 && ((testCollision(boost::get(positions, boost::source(e, _g)), boost::get(positions, boost::target(e, _g)), l.rect) &&
			 testNear(boost::get(positions, boost::target(e, _g)), l.rect, space)) ||
			 testNear(boost::get(positions, boost::target(e, _g)), l.rect, space2/2)))
            nearest.push_back(boost::target(e, _g));
	  }

	  BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
	  {
		  if(boost::degree(v, _g) != 2)
			  continue;
		  if(!testNear(boost::get(positions, v), l.rect, space2/2))
			  continue;

		  std::deque<Skeleton::Vertex> neighbors;
		  boost::graph_traits<Skeleton::SkeletonGraph>::adjacency_iterator b_e, e_e;
		  boost::tie(b_e, e_e) = boost::adjacent_vertices(v, _g);
		  neighbors.assign(b_e, e_e);

		  Skeleton::Edge edge1 = boost::edge(neighbors[0], v, _g).first;
		  Skeleton::Edge edge2 = boost::edge(neighbors[1], v, _g).first;
		  BondType t1 = getBondType(edge1);
		  BondType t2 = getBondType(edge2);

		  if( t1 != BT_SINGLE || t2 != BT_SINGLE)
			  continue;

		  Vec2d p_v = boost::get(positions, v);
		  Vec2d p1 = boost::get(positions, neighbors[0]);
		  Vec2d p2 = boost::get(positions, neighbors[1]);

		  Vec2d ap1, ap2;
		  ap1.diff(p1, p_v);
		  ap2.diff(p2, p_v);

		  if( testCollision(p1, p_v, l.rect) && 
			  testCollision(p2, p_v, l.rect) &&
			  testNear(p_v, l.rect, space) &&
			  !testNear( p1, l.rect, space) &&
			  !testNear( p2, l.rect, space) )
		  {
			
			  removeBond(edge1);
			  Vertex v_d = addVertex( p_v );
			  addBond(neighbors[0], v_d, BT_SINGLE, true);
			  nearest.push_back(v_d);
			  nearest.push_back(v);
		  }
	  }

      int s = nearest.size();
      if (s == 0)
      {
         unmapped_labels.push_back(l);
         continue;
      }

      if (s == 1)
      {
         _mapping[nearest[0]] = &l;
         continue;
      }

      Vec2d middle;
      for (int j = 0; j < s; j++)
      {
         for (int k = j + 1; k < s; k++)
         {
			 if (vars.checkTimeLimit())
			  throw ImagoException("Timelimit exceeded");
            Skeleton::Vertex a, b;
            Skeleton::Vertex c, d;
            a = nearest[j];
            b = nearest[k];
            c = *boost::adjacent_vertices(a, _g).first;
            d = *boost::adjacent_vertices(b, _g).first;

            Vec2d n1, n2;
            Vec2d v_a, v_b, v_c, v_d;
            v_a = boost::get(positions, a);
            v_b = boost::get(positions, b);
            v_c = boost::get(positions, c);
            v_d = boost::get(positions, d);
            n1.diff(v_a, v_c);
            n2.diff(v_b, v_d);

            Vec2d m;

			double ang = Vec2d::angle(n1, n2);
			if (fabs(ang) < vars.molecule.AngleTreshold ||
                fabs(ang - PI) < vars.molecule.AngleTreshold )
			{
				m.middle(v_a, v_b);
			}
            else
			{
                m.copy(Algebra::linesIntersection(vars, v_a, v_c, v_b, v_d));
			}

            middle.add(m);
         }
      }
      
      middle.scale(2.0 / (s * (s - 1)));
      Vertex newVertex = addVertex(middle);
      for (int j = 0; j < s; j++)
      {
         Vertex e = nearest[j];
         Vertex b = *boost::adjacent_vertices(e, _g).first;
         Edge bond = boost::edge(e, b, _g).first;
         BondType t = boost::get(boost::edge_type, _g, bond).type;

         boost::remove_edge(bond, _g);
         boost::remove_vertex(e, _g);
         addBond(b, newVertex, t);
      }
      _mapping[newVertex] = &l;
   }

   std::deque<Skeleton::Vertex> deck;

   //Removing dots without labels
   

   BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
   {
      if (boost::degree(v, _g) == 0 && _mapping.find(v) == _mapping.end())
         deck.push_back(v);
   }
   


   BOOST_FOREACH(Skeleton::Vertex v, deck)
      boost::remove_vertex(v, _g);
}

void Molecule::aromatize( Points2d &aromatic_centers )
{      
   BOOST_FOREACH( Vec2d arom_center, aromatic_centers )
   {
      Vertex begin_vertex = (Vertex)0; 
      double distance = DIST_INF;
  
      BGL_FORALL_VERTICES(v, _g, SkeletonGraph)
      {
         double tmp = Vec2d::distance(arom_center, getVertexPos(v));
   
         if (tmp < distance)
         {
            distance = tmp;
            begin_vertex = v;
         }
      }

      Vertex cur_vertex = begin_vertex, prev_vertex = (Vertex)0, next_vertex = (Vertex)0;

	  if (cur_vertex == NULL)
		  return;
   
      std::vector<Edge> aromatized_bonds;

      do
      {         
         distance = DIST_INF;

         BGL_FORALL_ADJ(cur_vertex, u, _g, SkeletonGraph)
         {
            Vec2d bond_middle;
            bond_middle.middle(getVertexPos(cur_vertex), getVertexPos(u));
            double tmp = Vec2d::distance(arom_center, bond_middle); 

            if (tmp < distance)
            {
               if (u != prev_vertex)
               {
                  distance = tmp;
                  next_vertex = u;
               }
            }
         } 

         if (next_vertex == cur_vertex || next_vertex == (Vertex)NULL)
            break;

         std::pair<Edge, bool> p = boost::edge(cur_vertex, next_vertex, _g);

         if (p.second)
            aromatized_bonds.push_back(p.first);

         if ((int)aromatized_bonds.size() >= getEdgesCount())
            break;

         prev_vertex = cur_vertex;
         cur_vertex = next_vertex;

      } while (cur_vertex != begin_vertex);

      //TODO: Aromatizing only closed contours! Not sure if it's true.
      if (cur_vertex == begin_vertex)
         BOOST_FOREACH( Edge e, aromatized_bonds )
            setBondType(e, BT_AROMATIC);

      aromatized_bonds.clear();
   }
}

void Molecule::clear()
{
   Skeleton::clear();
   _labels.clear();
   _mapping.clear();
}



Molecule::~Molecule()
{
   clear();
}
