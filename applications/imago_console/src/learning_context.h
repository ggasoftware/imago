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

#ifndef _learning_context_h
#define _learning_context_h

#include <string>
#include <vector>
#include <map>

struct LearningContext
{
	bool valid;

	double similarity;
	//std::string vars;
	
	double best_similarity_achieved;
	//std::string best_vars;
	
	std::string reference_file;
	std::string output_file;
		
	double stability;
	double score_stability;

	double time;
	double average_time;

	int attempts;

	LearningContext()
	{
		valid = false;
		similarity = best_similarity_achieved = 0.0;
		score_stability = stability = 1.0;
		time = average_time = 0.0;
		attempts = 0;
	}
};

typedef std::map<std::string, LearningContext> LearningBase;

struct LearningResultRecord
{
	std::string config;
	double average_time;
	double average_score;
	int ok_count;
	int valid_count;
	int work_iteration;

	bool operator<(const LearningResultRecord& second) const
	{
		return ok_count < second.ok_count ||
			      (ok_count == second.ok_count &&
				   average_score < second.average_score);
	}

	LearningResultRecord()
	{
		average_time = average_score = 0.0;
		work_iteration = ok_count = valid_count = 0;		
	}
};

typedef std::vector<LearningResultRecord> LearningHistory;

#endif // _learning_context_h
