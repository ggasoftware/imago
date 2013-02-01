#include "similarity_tools.h"

#include "exception.h"
#include "platform_tools.h"

#include "indigo.h"

namespace similarity_tools
{

	std::string similarity_tool_exe = "";
	std::string similarity_tool_param = "";

	void setExternalSimilarityTool(const std::string& tool, const std::string& param)
	{
		similarity_tool_exe = tool;
		similarity_tool_param = param;
	}

	std::string quote(const std::string input)
	{
		std::string result = input;
		if (!result.empty() && result[0] != '\"')
			result = '\"' + result + '\"';
		return result;
	}

	double getSimilarity(const LearningContext& ctx)
	{
      double result = 0.0;
		if (!similarity_tool_exe.empty())
		{
			std::string params = quote(ctx.output_file) + " " + quote(ctx.reference_file);
			if (!similarity_tool_param.empty())
			{
				params = quote(similarity_tool_param) + " " + params;
			}
			int retVal = platform::CALL(similarity_tool_exe, params);
			if (retVal >= 0 && retVal <= 100)
				return retVal;
			else
				throw imago::FileNotFoundException("Failed to call similarity tool " 
						  + similarity_tool_exe + " (" + imago::ImagoException::str(retVal) + ")");
		}
		else
		{
         int outm = indigoLoadMoleculeFromFile(ctx.output_file.c_str());
         if (outm == -1)
         {
            indigoFreeAllObjects();
				throw imago::IOException("Failed to load " + ctx.output_file + ":" + indigoGetLastError());
         }
         int refm = indigoLoadMoleculeFromFile(ctx.reference_file.c_str());
         if (refm == -1)
         {
            indigoFreeAllObjects();
				throw imago::IOException("Failed to load " + ctx.reference_file + ":" + indigoGetLastError());
         }

         indigoNormalize(refm, "");
         indigoNormalize(outm, "");

         float sim1 = indigoSimilarity(refm, outm, "normalized-edit");
         indigoAromatize(refm);
         indigoAromatize(outm);

         float sim2 = indigoSimilarity(refm, outm, "normalized-edit");

         result = 100.0 * std::max(sim1, sim2);

         // Clear all the objects used by Indigo
         indigoFreeAllObjects();
		}

		return result;
	}
}
