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
#ifndef _stat_utils_h
#define _stat_utils_h

#include "exception.h"

namespace imago
{   
   //Some statistic functions. Results are needed mainly in integer form.
   class StatUtils
   {
   public:

      /**
       * @brief   Calculates arithmetic mean of @c int array
       *
       * @exception  ERROR Thrown when array is empty
       *
       * @return  arithmetic mean value
       */
      template<typename ForwardIterator> 
      static typename ForwardIterator::value_type arithMean( ForwardIterator begin, ForwardIterator end )
      {
         int count = std::distance(begin, end);

         if (count == 0)
            throw LogicException("count == 0");

         typename ForwardIterator::value_type res = 0;

         for (ForwardIterator i = begin; i != end; ++i)
            res += *i;

         res /= count;

         return res;
      }

      /**
       * @brief   Calculates arithmetic mean of @c int array without any zero elements
       *
       * @exception  ERROR Thrown when array is empty
       *
       * @return   arithmetic mean value
       */
      template<typename ForwardIterator> 
      static typename ForwardIterator::value_type arithMeanNoZero( ForwardIterator begin, ForwardIterator end )
      {
         int count = std::distance(begin, end);

         if (count == 0)
            throw LogicException("count == 0");

         typename ForwardIterator::value_type res = 0;

         for (ForwardIterator i = begin; i != end; ++i)
            if (*i != 0)
               res += *i;

         res /= count;

         return res;
      }

      /**
       * @brief   Calculates harmonic mean of @c int array
       *
       * @exception  ERROR Thrown when array is empty
       *
       * @return  harmonic mean value
       */
      template<typename ForwardIterator> 
      static typename ForwardIterator::value_type harmonicMean( ForwardIterator begin, ForwardIterator end )
      {
         int count = std::distance(begin, end);
         double sum = 0;

         if (count == 0)
            throw LogicException("count == 0");

         typename ForwardIterator::value_type res = count;

         for (ForwardIterator i = begin; i != end; ++i)
            if (*i != 0)
               sum += 1.0 / *i;

         res /= sum;

         return res;
      }

      /**
       * @brief  Calculates interquartile mean of @c int array part (IQM)
       *
       * @remark    Array should be sorted previously
       *
       * @param start - array part starting index
       * @param count - number of elements in array part
       *
       * @exception  ERROR Thrown when array is empty
       *
       * @return  IQM value
       */
      template<typename ForwardIterator> 
      static double /*typename ForwardIterator::value_type*/ interMean( ForwardIterator begin, ForwardIterator end )
      {
         int count = std::distance(begin, end);

         if (count == 0)
            throw LogicException("count == 0");

         if (count == 1)
            return *begin;

         int l = count / 4, r = 3 * count / 4, m;
         double u;

         m = count % 4;

         if (m == 0)
            r--;

         u = -0.25 * m + 1;

         /*typename ForwardIterator::value_type*/ double res = 0;

         for (ForwardIterator i = begin + l + 1; i != begin + r; ++i)
         {
            res += *i;
         }

         res += (*(begin + l) + *(begin + r)) * u;
         u = 2.0 / count;
         res *= u;

         return res;
      }

	  /**
       * @brief   Calculates arithmetic mean of @c int array
       *
       * @exception  ERROR Thrown when array is empty
       *
       * @return  arithmetic mean value
       */
      template<typename ForwardIterator> 
      static typename ForwardIterator::value_type Median( ForwardIterator begin, ForwardIterator end )
      {
         int count = std::distance(begin, end);

         if (count == 0)
            throw LogicException("count == 0");

         typename ForwardIterator::value_type res = 0;

		 std::sort(begin, end);

		 res = count % 2 == 0 ? (*(begin+count/2 ) + *(begin + count/2 + 1))/2 :
			 *(begin + count/2);

         return res;
      }
   };
}


#endif /* _stat_utils_h_ */