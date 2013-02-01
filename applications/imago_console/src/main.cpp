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

#include "file_helpers.h"
#include "recognition_helpers.h"
#include "machine_learning.h"
#include "similarity_tools.h"
#include "settings.h"
#include "log_ext.h"

int main(int argc, char **argv)
{
	imago::Settings vars;
	vars.general.TimeLimit = 8000; // ms, default timelimit value

	if (argc <= 1)
	{
		printf("Usage: %s [option]* [batches] [mode] [image_path] [-o output_file]\n", argv[0]);				
		printf("\n MODE SWITCHES: \n");
		printf("  image_path: full path to image to recognize (may be omitted if other switch is specified) \n");
		printf("  -o output_file: save single recognition result to the specified file \n");
		printf("  -characters: extracts only characters from image(s) and store in ./characters/ \n");
		printf("  -learn dir_name: process machine learning for specified collection \n");
		printf("  -compare molfile1 molfile2: calculate similarity between molfiles \n");
		printf("    -retcode: returns similarity 0..100 in ERRORLEVEL \n");
		printf("\n OPTION SWITCHES: \n");
		printf("  -config cfg_file: use specified configuration cluster file \n");		
		printf("  -log: enables debug log output to ./log.html \n");
		printf("  -logvfs: stores log in single encoded file ./log_vfs.txt \n");		
		printf("  -noexp: do not expand chemical abbreviations \n");		
		printf("  -pr: use probablistic separator (experimental) \n");
		printf("  -tl time_in_ms: timelimit per single image process (default is %u) \n", vars.general.TimeLimit);
		printf("  -similarity tool [-sparam additional_parameters]: override the default comparison method \n");
		printf("  -pass: don't process images, only print their filenames \n");
		printf("  -override config_string: override config by applying specified string \n");
		printf("\n BATCHES: \n");
		printf("  -dir dir_name: process every image from dir dir_name \n");
		printf("    -rec: process directory recursively \n");
		printf("    -images: skip non-supported files from directory \n");				
		printf("\n SHORTCUTS: \n");
		printf("  -learnd dir_name: -learn -dir dir_name -images \n");
		return 0;
	}

	std::string image = "";
	std::string dir = "";
	std::string config = "";
	std::string sim_tool = "";
	std::string sim_param = "";
	std::string molfile1 = "";
	std::string molfile2 = "";
	std::string override_cfg = "";
	std::string output = "molecule.mol";

	bool next_arg_dir = false;
	bool next_arg_config = false;
	bool next_arg_sim_tool = false;
	bool next_arg_sim_param = false;
	bool next_arg_tl = false;	
	bool next_arg_override_cfg = false;
	bool next_arg_output = false;
	int next_arg_compare = 0; // two args

	bool mode_recursive = false;
	bool mode_pass = false;
	bool mode_learning = false;
	bool mode_filter = false;
	bool mode_retcode = false;
	bool mode_test_filter_only = false;

	for (int c = 1; c < argc; c++)
	{
		std::string param = argv[c];

		if (param.empty())
			continue;
		
		if (param == "-l" || param == "-log")
			vars.general.LogEnabled = true;

		else if (param == "-logvfs")
			vars.general.LogVFSEnabled = true;

		else if (param == "-noexp")
			vars.general.ExpandAbbreviations = false;

		else if (param == "-pr" || param == "-probablistic")
			vars.general.UseProbablistics = true;

		else if (param == "-dir")
			next_arg_dir = true;

		else if (param == "-tl")
			next_arg_tl = true;

		else if (param == "-similarity")
			next_arg_sim_tool = true;

		else if (param == "-o")
			next_arg_output = true;

		else if (param == "-sparam")
			next_arg_sim_param = true;		

		else if (param == "-compare")
			next_arg_compare = 2; // expected two params

		else if (param == "-r" || param == "-rec")
			mode_recursive = true;

		else if (param == "-i" || param == "-images")
			mode_filter = true;

		else if (param == "-learn" || param == "-optimize")
			mode_learning = true;

		else if (param == "-retcode")
			mode_retcode = true;

		else if (param == "-filter")
			mode_test_filter_only = true;

		else if (param == "-override")
			next_arg_override_cfg = true;

		else if (param == "-learnd")
		{
			mode_learning = true;
			mode_recursive = true;
			mode_filter = true;
			next_arg_dir = true;
		}

		else if (param == "-pass")
			mode_pass = true;

		else if (param == "-config")
			next_arg_config = true;

		else if (param == "-characters")
			vars.general.ExtractCharactersOnly = true;

		else 
		{
			if (next_arg_compare)
			{
				if (next_arg_compare == 2)
				{
					molfile1 = param;
				}
				else if (next_arg_compare == 1)
				{
					molfile2 = param;
				}
				next_arg_compare--;
			}			
			else if (next_arg_output)
			{
				output = param;
				next_arg_output = false;
			}
			else if (next_arg_config)
			{
				config = param;
				next_arg_config = false;
			}
			else if (next_arg_dir)
			{
				dir = param;
				next_arg_dir = false;
			}
			else if (next_arg_sim_tool)
			{
				sim_tool = param;
				next_arg_sim_tool = false;
			}
			else if (next_arg_sim_param)
			{
				sim_param = param;
				next_arg_sim_param = false;
			}
			else if (next_arg_tl)
			{
				vars.general.TimeLimit = atoi(param.c_str());
				next_arg_tl = false;
			}
			else if (next_arg_override_cfg)
			{
				if (!override_cfg.empty())
					override_cfg += "\n";
				override_cfg += param;
				next_arg_override_cfg = false;
			}
			else
			{
				if (param[0] == '-' && param.find('.') == std::string::npos)
				{
					printf("Unknown option: '%s'\n", param.c_str());
					return 1;
				}
				else
				{
					if (image.empty())
					{
						image = param;
					}
					else
					{
						printf("Image file is already specified ('%s'), the second definition unallowed ('%s')\n", image.c_str(), param.c_str());
						return 1;
					}
				}
			}
		}
	}

	imago::getLogExt().setLoggingEnabled(vars.general.LogEnabled);

	similarity_tools::setExternalSimilarityTool(sim_tool, sim_param);

	if (!override_cfg.empty())
		vars.fillFromDataStream(override_cfg);	

	if (mode_test_filter_only)
	{
		return recognition_helpers::performFilterTest(vars, image);
	}
	else if (!molfile1.empty() && !molfile2.empty())
	{
		// compare mode
		LearningContext temp;
		temp.output_file = molfile1;
		temp.reference_file = molfile2;
		try
		{
			double res = similarity_tools::getSimilarity(temp);
			if (mode_retcode)
			{
				return (int)(res + imago::EPS);
			}
			else
			{
				printf("Similarity: %g\n", res);
			}
		}
		catch(imago::ImagoException &e)
		{
			printf("%s\n", e.what());			
		}
		return 0;
	}	
	else if (!dir.empty())
	{
		// dir mode

		strings files;
		
		if (file_helpers::getDirectoryContent(dir, files, mode_recursive) != 0)
		{
			printf("[ERROR] Can't get the content of directory '%s'\n", dir.c_str());
			return 2;
		}

		if (mode_filter || mode_learning)
		{
			file_helpers::filterOnlyImages(files);
		}

		if (mode_learning)
		{			
			return machine_learning::performMachineLearning(vars, files, config);
		}
		else // process or pass
		{
			for (size_t u = 0; u < files.size(); u++)
			{
				if (mode_pass)
				{
					printf("Skipped file '%s'\n", files[u].c_str());
				}
				else
				{
					std::string output_file = files[u] + ".result.mol";
					recognition_helpers::performFileAction(true, vars, files[u], config, output_file);	
				}
			}
		}
	}
	else if (!image.empty())
	{
		// single item mode
		return recognition_helpers::performFileAction(true, vars, image, config, output);	
	}		
	
	return 1; // "nothing to do" error
}
