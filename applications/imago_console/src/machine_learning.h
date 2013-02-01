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

#ifndef _machine_learning_h
#define _machine_learning_h

#include <string>
#include "learning_context.h"
#include "file_helpers.h"
#include "settings.h"

namespace machine_learning
{
	double getWorstAllowedDelta(int imagesCount = 0);
	std::string modifyConfig(const std::string& config, const LearningBase& learning, int iteration);
	void runSingleItem(LearningContext& ctx, LearningResultRecord& res, const std::string& image_name, int timelimit_value, bool init = false);
	bool updateResult(LearningResultRecord& result_record, LearningHistory& history);
	bool storeConfig(const LearningResultRecord& res, const std::string& prefix = "");
	bool readLearningProgress(LearningBase& base, LearningHistory& history, bool quiet = false, const std::string& filename = "learning_progress.dat");
	bool storeLearningProgress(const LearningBase& base, const LearningHistory& history, const std::string& filename = "learning_progress.dat");
	int performMachineLearning(imago::Settings& vars, const strings& imageSet, const std::string& configName);
}

#endif