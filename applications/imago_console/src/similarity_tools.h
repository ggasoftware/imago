/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef _similarity_tools_h
#define _similarity_tools_h

#include <string>
#include "learning_context.h"

namespace similarity_tools
{
	void   setExternalSimilarityTool(const std::string& executable, const std::string& param = "");
	double getSimilarity(const LearningContext& ctx);
}

#endif // _similarity_tools_h
