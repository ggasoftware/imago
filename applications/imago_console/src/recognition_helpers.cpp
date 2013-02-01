#include "recognition_helpers.h"
#include "output.h"
#include "chemical_structure_recognizer.h"
#include "molecule.h"
#include "prefilter_entry.h"
#include "superatom_expansion.h"
#include "log_ext.h"
#include "image_utils.h"
#include "indigo.h"
#include "indigo-renderer.h"

// Required for indigo-renderer:
#ifdef _WIN32
#pragma comment(lib, "msimg32.lib")
#endif

namespace recognition_helpers
{
	void dumpVFS(imago::VirtualFS& vfs, const std::string& filename)
	{
		// store all the vfs contents in one single file (including html, images, etc)
		if (!vfs.empty())
		{
			imago::FileOutput filedump(filename.c_str());
			std::vector<char> data;		
			vfs.getData(data); 
			filedump.write(&data.at(0), data.size());
		}
	}

	void applyConfig(bool verbose, imago::Settings& vars, const std::string& config)
	{
		if (!config.empty())
		{
			if (verbose)
				printf("Loading configuration cluster [%s]... ", config.c_str());

			bool result = vars.forceSelectCluster(config);

			if (verbose)
			{
				if (result)
					printf("OK\n");
				else
					printf("FAIL\n");
			}
		}
		else
		{
			vars.selectBestCluster();
		}
	}


	RecognitionResult recognizeImage(bool verbose, imago::Settings& vars, const imago::Image& src, const std::string& config)
	{
		std::vector<RecognitionResult> results;
		
		imago::ChemicalStructureRecognizer _csr;
		imago::Molecule mol;

		for (int iter = 0; ; iter++)
		{
			bool good = false;

			vars.general.StartTime = 0;

			try
			{
				imago::Image img;

				if (iter == 0)
				{
					if (!imago::prefilterEntrypoint(vars, img, src))
						break;
				}
				else
				{
					if (!imago::applyNextPrefilter(vars, img, src))
						break;
				}

				applyConfig(verbose, vars, config);
				_csr.image2mol(vars, img, mol);

				RecognitionResult result;
				result.molecule = imago::expandSuperatoms(vars, mol);
				result.warnings = mol.getWarningsCount() + mol.getDissolvingsCount() / vars.main.DissolvingsFactor;
				
				if (vars.dynamic.CapitalHeight < vars.main.MinGoodCharactersSize &&
					!vars.general.ImageAlreadyBinarized)
				{
					result.warnings += vars.main.WarningsForTooSmallCharacters;
				}

				results.push_back(result);

				good = result.warnings <= vars.main.WarningsRecalcTreshold;				
			
				if (verbose)
					printf("Filter [%u] done, warnings: %u, good: %u.\n", vars.general.FilterIndex, result.warnings, good);
			}
			catch (std::exception &e)
			{
				if (verbose)
					printf("Filter [%u] exception '%s'.\n", vars.general.FilterIndex, e.what());

			}

			if (good)
				break;
		} // for

		RecognitionResult result;
		result.warnings = 999; // just big number to override
		// select the best one
		for (size_t u = 0; u < results.size(); u++)
		{
			if (results[u].warnings < result.warnings)
			{
				result = results[u];
			}
		}
		return result;
	}


	int performFilterTest(imago::Settings& vars, const std::string& imageName)
	{
		int result = 0; // ok mark
		try
		{
			imago::Image image;
			imago::ImageUtils::loadImageFromFile(image, imageName.c_str());
			
			imago::Image out;
			if (!imago::prefilterEntrypoint(vars, out, image))
				return 2;

			imago::ImageUtils::saveImageToFile(out, (imageName + ".filtered.png").c_str());
		}
		catch (std::exception &e)
		{
			puts(e.what());
			result = 1;
		}
		return result;
	}

	int performFileAction(bool verbose, imago::Settings& vars, const std::string& imageName, const std::string& configName,
						  const std::string& outputName)
	{
		logEnterFunction();

		int result = 0; // ok mark
		imago::VirtualFS vfs;

		vars.general.StartTime = 0; // reset timelimit

		if (vars.general.ExtractCharactersOnly)
		{
			if (verbose)
				printf("Characters extraction from image '%s'\n", imageName.c_str());
		}
		else
		{
			if (verbose)
				printf("Recognition of image '%s'\n", imageName.c_str());
		}

		try
		{
			imago::Image image;	  

			if (vars.general.LogVFSEnabled)
			{
				imago::getLogExt().SetVirtualFS(vfs);
			}

			imago::ImageUtils::loadImageFromFile(image, imageName.c_str());

			if (vars.general.ExtractCharactersOnly)
			{
				imago::Image out;
				imago::prefilterEntrypoint(vars, out, image);
				applyConfig(verbose, vars, configName);
				imago::ChemicalStructureRecognizer _csr;
				_csr.extractCharacters(vars, out);
			}
			else
			{
				RecognitionResult result = recognizeImage(verbose, vars, image, configName);		
				imago::FileOutput fout(outputName.c_str());
				fout.writeString(result.molecule.c_str());
				if (imago::getLogExt().loggingEnabled())
				{
					int molObj = indigoLoadMoleculeFromString(result.molecule.c_str());
					if (molObj != -1)
					{
						indigoSetOption("render-output-format", "png");
						indigoSetOption("render-background-color", "255, 255, 255");
						std::string outputImg = imago::getLogExt().generateImageName();
						indigoRenderToFile(molObj, outputImg.c_str());
						imago::getLogExt().appendImageFile("Result image:", outputImg);
						indigoFree(molObj);
					}
				}
			}

		}
		catch (std::exception &e)
		{
			result = 2; // error mark
			puts(e.what());
		}

		dumpVFS(vfs, "log_vfs.txt");

		return result;
	}
}