#include "fonts_list.h"

namespace imago
{
	FontEntryDefinition::FontEntryDefinition(const std::string& _name, const std::string& _data)
	{
		name = _name;
		data = _data;
	}

	FontEntries::FontEntries()
	{
		#define LENGTH(x) sizeof(x) / sizeof(int)
		
		// TODO: currently not used, decide: eliminate or transform
		/*{
			const char* data[] = 
			{
				#include "imago.font.inc"
			};

			std::string temp;
			for (int i = 0; i < (sizeof(data) / sizeof(char*)) && data[i] != 0; i++)
			{
				temp += data[i];
			}

			push_back(FontEntryDefinition("base", temp)); 			
		}*/
	}

	FontEntries getFontsList()
	{
		static FontEntries result;
		return result;
	}
};