#include "filters_list.h"
#include "prefilter_basic.h"
#include "prefilter_retinex.h"

namespace imago
{
	FilterEntryDefinition::FilterEntryDefinition(const std::string& _name, int _priority, FilterFunction _f, const std::string& _config, ConditionFunction _c)
	{
		name = _name;
		priority = _priority;
		routine = _f;
		condition = _c;
		update_config_string = _config;
	}

	FilterEntries::FilterEntries()
	{
		push_back(FilterEntryDefinition("prefilter_binarized", 1,   prefilter_basic::prefilterBinarizedFullsize, 
			                            "general.ImageAlreadyBinarized = 1;"));	

		push_back(FilterEntryDefinition("prefilter_retinex",   2,   prefilter_retinex::prefilterRetinexDownscaleOnly));		

		push_back(FilterEntryDefinition("prefilter_basic_s",   3,   prefilter_basic::prefilterBasicForceDownscale));

		push_back(FilterEntryDefinition("prefilter_basic",     4,   prefilter_basic::prefilterBasicFullsize));
	}

	FilterEntries getFiltersList()
	{
		static FilterEntries result;
		return result;
	}
};
