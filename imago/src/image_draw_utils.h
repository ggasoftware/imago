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
#ifndef _image_draw_utils_h
#define _image_draw_utils_h

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/iteration_macros.hpp"

#include "comdef.h"
#include "vec2d.h"

namespace imago
{
    class Image;
    
    class ImageDrawUtils
    {
    public:
        static void putLine( Image &img, double thetha, double r, double eps, byte color );

        static void putLineSegment( const Vec2i &p1, const Vec2i &p2, int color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) );
        static void putLineSegment( Image &img, const Vec2i &p1, const Vec2i &p2, byte color );
        
        static void putCircle( Image &img, int cx, int cy, int r, byte color );
        static void putCircle( int cx, int cy, int r, byte color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) );

        template <class EuclideanGraph>
        static void putGraph( Image &img, const EuclideanGraph &g )
        {
           typename boost::property_map<EuclideanGraph,
                                        boost::vertex_pos_t>::const_type
                              positions = boost::get(boost::vertex_pos, g);
           BGL_FORALL_VERTICES_T(v, g, EuclideanGraph)
           {
              Vec2d pos = positions[v];
              ImageDrawUtils::putCircle(img, round(pos.x), round(pos.y), 4, 100);
           }
           BGL_FORALL_EDGES_T(e, g, EuclideanGraph)
           {
              Vec2d b_pos = positions[boost::source(e, g)],
                    e_pos = positions[boost::target(e, g)];
              ImageDrawUtils::putLineSegment(img, b_pos, e_pos, 100);
           }
        }

    private:
        static void _plot8points( int cx, int cy, int x, int y, byte color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) );
        static void _plot4points( int cx, int cy, int x, int y, byte color, void *userdata, bool (*plot)( int x, int y, int color, void *userdata ) );

        static bool _imagePlot( int x, int y, int color, void *userdata );
    };
};


#endif /* _image_draw_utils_h */

