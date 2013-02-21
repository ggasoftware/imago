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

#include "recognition_distance.h"
#include <algorithm>
#include "log_ext.h"
#include <float.h>

namespace imago
{

	void RecognitionDistance::mergeTables(const RecognitionDistance& second)
	{
		for (RecognitionDistance::const_iterator it = second.begin(); it != second.end(); ++it)
		{
			if (this->find(it->first) == this->end())
				(*this)[it->first] = it->second;
			else
			{
				(*this)[it->first] = std::min((*this)[it->first], it->second);
			}
		}
	}

	void RecognitionDistance::adjust(double factor, const std::string& sym_set)
	{
		if (absolute(factor - 1.0) < EPS)
			return;

		getLogExt().append("Distance map adjust for " + sym_set, factor);

		for (size_t u = 0; u < sym_set.size(); u++)
			if (this->find(sym_set[u]) != this->end())
				(*this)[sym_set[u]] *= factor;
	}

	void RecognitionDistance::fillVector(PrVec &out) const
	{
		for (RecognitionDistance::const_iterator it = this->begin(); it != this->end(); ++it)
			out.push_back(std::make_pair(it->first, it->second));
	}

	double RecognitionDistance::getQuality() const
	{		
		double min1 = 999.0, min2 = 1000.0;
	
		for (RecognitionDistance::const_iterator it = this->begin(); it != this->end(); ++it)
		{
			if (it->second < min1)
			{
				min2 = min1;
				min1 = it->second;
			}
			else if (it->second < min2)
				min2 = it->second;
		}

		double result = min2 - min1;
		if (result > 1.0)
			result = 1.0;

		return result;
	}

	std::string RecognitionDistance::getRangedBest(double max_diff) const
	{
		std::string result;
	
		PrVec temp;
		fillVector(temp);	
	
		if (!temp.empty())
		{
			std::sort(temp.begin(), temp.end(), sortCompareFunction);
			result = temp[0].first;
			for (size_t u = 1; u < temp.size(); u++)
				if (temp[u].second < temp[0].second + max_diff)
					result += temp[u].first;
		}

		return result;
	}

	char RecognitionDistance::getBest(double* dist) const
	{
		double d = DBL_MAX;
		char result = 0;
		for (RecognitionDistance::const_iterator it = this->begin(); it != this->end(); ++it)
		{
			if (it->second < d)
			{
				d = it->second;
				result = it->first;
			}
		}
		if (dist != NULL)
			*dist = d;
		return result;
	}
}
