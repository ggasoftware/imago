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

/**
 * @file   vec2d.h
 * 
 * @brief  2d double vector
 */

#pragma once
#ifndef _vec2d_h
#define _vec2d_h

#include <cmath>
#include "comdef.h"
#include "exception.h"

namespace imago
{
   /**
    * @brief Vector class
    */
   template<typename Type>
   class Vec2
   {
   public:
      Type x;
      Type y;

      /**
       * Default constructor
       */
      Vec2(): x(0), y(0)
      {};
      /**
       * Construct with given coordinates
       *
       * @param _x
       * @param _y
       */
      Vec2( Type _x, Type _y ): x(_x), y(_y)
      {};

      /**
       * Copy-constructor
       *
       * @param v
       */
      template<typename OtherType>
      Vec2( const Vec2<OtherType> &v ): x(static_cast<Type>(v.x)),
                                        y(static_cast<Type>(v.y))
      {};

      /**
       * @brief Sets coordinates to (_x, _y)
       *
       * @param _x
       * @param _y
       */
      void set( Type _x, Type _y )
      {
         x = _x, y = _y;
      }

      /**
       * @brief Copies coordinates from vector @a v
       *
       * @param v
       */
      void copy( const Vec2<Type> &v )
      {
         x = v.x, y = v.y;
      }

      /**
       * @brief Sets coordinates to (0, 0)
       *
       */
      void zero()
      {
         x = y = 0;
      }

      /**
       * @brief Adds vector @a v
       *
       * @param v
       */
      void add( const Vec2<Type> &v )
      {
         x += v.x, y += v.y;
      }

      template<typename OtherType>
      void add( const Vec2<OtherType> &v )
      {
         x += static_cast<Type>(v.x);
         y += static_cast<Type>(v.y);
      }
      /**
       * @brief Sets coordinates to sum of @a a and @a b
       *
       * @param a
       * @param b
       */
      void sum( const Vec2<Type> &a, const Vec2<Type> &b )
      {
         x = a.x + b.x, y = a.y + b.y;
      }

      /**
       * @brief Substracts vector @a v
       *
       * @param v
       */
      void sub( const Vec2<Type> &v )
      {
         x -= v.x, y -= v.y;
      }

      /**
       * @brief Sets coordinates to difference of vectors @a a and @a b
       *
       * @param a
       * @param b
       */
      void diff( const Vec2<Type> &a, const Vec2<Type> &b )
      {
         x = a.x - b.x, y = a.y - b.y;
      }

      void interpolate( const Vec2<Type> &b, const Vec2<Type> &e, double lambda )
      {
         if (lambda < 0 || lambda > 1.0)
            throw LogicException("invalid lambda parameter");

         x = static_cast<Type>(b.x * (1 - lambda) + e.x * lambda);
         y = static_cast<Type>(b.y * (1 - lambda) + e.y * lambda);
      }

      void middle( const Vec2<Type> &b, const Vec2<Type> &e )
      {
         interpolate(b, e, 0.5);
      }

      /**
       * @brief Scales vector
       *
       * @param s scale factor
       */
      void scale( double s )
      {
         x = static_cast<Type>(s * x);
         y = static_cast<Type>(s * y);
      }

      /**
       * @brief this = v * s;
       *
       * @param v vector
       * @param s scale factor
       */
      void scaled( const Vec2<Type> &v, double s )
      {
         x = static_cast<Type>(s * v.x);
         y = static_cast<Type>(s * v.y);
      }

      /**
       * @brief Length of vector
       *
       * @return length
       */
      double norm() const
      {
         return sqrt(static_cast<double>(x * x + y * y));
      }

      Vec2<double> getNormalized()
      {
         double n = norm();
         if (n < EPS)
            throw DivizionByZeroException("Vec2::getNormalized");

         return Vec2<double>(x / n, y / n);
      }

      void rotate( double angle )
      {
         double s = sin(angle);
         double c = cos(angle);
         double _x = x, _y = y;
         x = static_cast<Type>(c * _x - s * _y);
         y = static_cast<Type>(s * _x + c * _y);
      }

      /**
       * @brief Dot product of two vectors
       *
       * @param a first vector
       * @param b second vector
       *
       * @return dot product
       */
      static Type dot( const Vec2<Type> &a, const Vec2<Type> &b )
      {
         return a.x * b.x + a.y * b.y;
      }

      template<typename OtherType>
      static double ddot( const Vec2<Type> &a, const Vec2<OtherType> &b )
      {
         return static_cast<double>(a.x * b.x + a.y * b.y);
      }

      /**
       * @brief Angle between two vectors
       *
       * @param a first vector
       * @param b second vector
       *
       * @return Cosine of angle
       */
      template<typename OtherType>
      static double angle( const Vec2<Type> &a, const Vec2<OtherType> &b )
      {
         double d = ddot(a, b);
         double na = a.norm();
         double nb = b.norm();
         if (fabs(na * nb) < EPS)
            throw DivizionByZeroException("angle calculation");

         d = d / na / nb;

         if (absolute(d - 1) < EPS)
            return 0;

         return acos(d);
      }

      /**
       * @brief Distance between point v1 and v2
       *
       * @param a first vector
       * @param b second vector
       *
       * @return distance
       */
      template<typename OtherType>
      static double distance( const Vec2<Type> &a, const Vec2<OtherType> &b )
      {
         double dx, dy;
         dx = a.x - b.x;
         dy = a.y - b.y;

         return sqrt(dx * dx + dy * dy);
      }
   };

   typedef Vec2<double> Vec2d;
   typedef Vec2<float> Vec2f;
   typedef Vec2<int> Vec2i;

   inline bool operator== ( const Vec2d &a, const Vec2d &b )
   {
      return fabs(a.x - b.x) < EPS && fabs(a.y - b.y) < EPS;
   }
   inline bool operator== ( const Vec2i &a, const Vec2i &b )
   {
      return (a.x == b.x && a.y == b.y);
   }

}


#endif /* _vec2d_h */
