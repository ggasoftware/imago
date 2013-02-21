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

/**
 * @file   probability_separator.h
 * 
 * @brief  Probabilistic estimate of segments
 */

#pragma once
#ifndef _probability_separator_h
#define _probability_separator_h

#include "complex_contour.h"
#include "segment.h"
#include "settings.h"

namespace imago
{
	class ProbabilitySeparator
	{
	public:
		static void CalculateProbabilities(const Settings& vars, Image& seg,
			double& char_probability, double& bond_probability, 
			double char_apriory = 0.5, double bond_apriory = 0.5);

	private:
		static int getAngleDirection(ComplexNumber vec);
	};
}

#endif