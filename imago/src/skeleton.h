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

#pragma once
#ifndef _skeleton_h
#define _skeleton_h

#include <deque>
#include "boost/graph/adjacency_list.hpp"

#include "comdef.h"
#include "vec2d.h"
#include "settings.h"

namespace imago
{          
   enum BondType
   {
      BT_SINGLE = 1,
      BT_DOUBLE,
      BT_TRIPLE,
      BT_AROMATIC,
      BT_SINGLE_UP,
      BT_SINGLE_DOWN,
	  BT_ARROW,
	  BT_WEDGE,
	  BT_SINGLE_UP_C, // single up bonds that need to be checked
      BT_UNKNOWN
   };

   struct Bond
   {
      double length, k;
      BondType type;

      Bond() {};

      Bond( double _length, double _k, BondType _type ) : length(_length),
         k(_k), type(_type) {};
   };

   class Skeleton
   {
   public: 

      typedef boost::property<boost::vertex_pos_t, Vec2d>
         SkeletonVertexProperties;
      typedef boost::property<boost::edge_type_t, Bond>
         SkeletonEdgeProperties;

      // note: Check if setS is really necessary. Parallel edges are forbidden in Skeleton, but add_vertex works slower.
      typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS,
         SkeletonVertexProperties, SkeletonEdgeProperties> SkeletonGraph;

      typedef boost::property_map<SkeletonGraph, boost::vertex_pos_t>
         VertexPosMap;

      typedef boost::property_map<SkeletonGraph, boost::edge_type_t>
         EdgeTypeMap;

      typedef boost::graph_traits<SkeletonGraph>::vertex_descriptor Vertex;
      typedef boost::graph_traits<SkeletonGraph>::edge_descriptor Edge;
      typedef boost::graph_traits<SkeletonGraph>::vertex_iterator VertexIterator;
      typedef boost::graph_traits<SkeletonGraph>::out_edge_iterator EdgeIterator;

      Skeleton();

      Vertex addVertex( const Vec2d &pos );      

      Edge addBond( Vertex &v1, Vertex &v2, BondType type = BT_SINGLE, bool throw_if_error = false );
      Edge addBond( const Vec2d &begin, const Vec2d &end, BondType type = BT_SINGLE, bool throw_if_error = false );

      void removeBond( Vertex &v1, Vertex &v2 );
      void removeBond( Edge &e );

      Vertex getBondBegin( const Edge &e ) const;
      Vertex getBondEnd( const Edge &e ) const;

      int getVerticesCount() const;
      int getEdgesCount() const;
      Vec2d getVertexPos( const Vertex &v1 ) const;
      BondType getBondType( const Edge &e ) const;
      Bond getBondInfo( const Edge &e ) const;
      void setBondType( Edge e, BondType t );

      void reverseEdge( const Edge &e );

      void setInitialAvgBondLength(Settings& vars, double avg_length );
      void recalcAvgBondLength();
      double bondLength() { return _avg_bond_length; } 
      
	  void modifyGraph(Settings& vars);

      void clear();

      operator SkeletonGraph(){ return _g; }
      SkeletonGraph &getGraph() { return _g; }
      ~Skeleton();

	  int getWarningsCount() const { return _warnings; }
	  int getDissolvingsCount() const { return _dissolvings; }
	  
	  void calcShortBondsPenalty(const Settings& vars);
	  void calcCloseVerticiesPenalty(const Settings& vars);

   protected:

      SkeletonGraph _g;
	  int _warnings, _dissolvings;
      
   private:
      friend class DoubleBondMaker;
      friend class TripleBondMaker;
      friend class MultipleBondChecker;
      
      void _reconnectBonds( Vertex from, Vertex to );
	  bool _checkMidBonds( Vertex from, Vertex to );
	  void _reconnectBondsRWT( Vertex from, Vertex to, BondType new_t);
      void _repairBroken(const Settings& vars);
      void _findMultiple(const Settings& vars);

   public:
      void _joinVertices(double eps);
      bool _dissolveShortEdges (double coeff,const bool has2nb = false);
      void deleteBadTriangles( double eps );

   private:
      bool _dissolveIntermediateVertices (const Settings& vars);
      double _avgEdgeLendth (const Vertex &v, int &nnei);
      typedef boost::tuple<bool, Edge, Edge> MakersReturn;
      //MakersReturn _makeDouble( std::pair<Edge, Edge> edges );
      MakersReturn _makeTriple( boost::tuple<Edge, Edge, Edge> edges );
      bool _isParallel( const Edge &first, const Edge &second ) const;
	  bool _isEqualDirection( const Edge &first, const Edge &second ) const;
      bool _isEqualDirection( const Vertex &b1,const Vertex &e1,const Vertex &b2,const Vertex &e2)  const;
	  bool _isSegmentIntersectedByEdge(const Settings& vars, Vec2d &b, Vec2d &e, std::deque<Edge> edges);
	  void _processInlineDoubleBond(const Settings& vars);

	  void _connectBridgedBonds(const Settings& vars);
	  
      double _avg_bond_length,
             _parLinesEps,
         _addVertexEps,
         _min_bond_length, _min_bond_h2nb_length;
      std::vector<Vec2d> _vertices_big_degree;

      Skeleton( const Skeleton &g );
   };
}


#endif /* _skeleton_h_ */
