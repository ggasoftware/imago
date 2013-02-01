#include "file_helpers.h"
#include <algorithm>
#include "scanner.h"


#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>
#include <errno.h>
#endif

namespace file_helpers
{
	bool getReferenceFileName(const std::string& image, std::string& output)
	{
		output = image;

		// get last slash position
		size_t pos_slash = image.rfind('/');
		size_t temp = image.rfind('\\');
		if (pos_slash == std::string::npos || 
			(temp != std::string::npos && temp > pos_slash) )
		{
			pos_slash = temp;
		}

		if (pos_slash == std::string::npos)
		{
			pos_slash = 0; // no slash
		}

		size_t pos_dot = image.find('.', pos_slash);
		if (pos_dot == std::string::npos)
		{
			return false;
		}

		output = image.substr(0, pos_dot) + ".mol";
		return true;
	}

	size_t getLastSlashPos(const std::string& filename)
	{		
		size_t pos_slash = filename.rfind('/');
		size_t temp = filename.rfind('\\');
		if (pos_slash == std::string::npos || 
			temp != std::string::npos && temp > pos_slash)
		{
			pos_slash = temp;
		}
		return pos_slash;
	}

	bool getOnlyFileName(const std::string& image, std::string& output)
	{
		output = image;

		size_t pos_slash = getLastSlashPos(image);

		if (pos_slash == std::string::npos)
		{
			pos_slash = 0; // no slash
		}

		size_t pos_dot = image.find('.', pos_slash);
		if (pos_dot == std::string::npos)
		{
			return false;
		}

		output = image.substr(0, pos_dot);
		return true;
	}


	int getDirectoryContent(const std::string& dir, strings &files, bool recursive)
	{
		DIR *dp;
		struct dirent *dirp;
	
		strings todo;

		if((dp = opendir(dir.c_str())) == NULL) 
		{
			return errno;
		}

		while ((dirp = readdir(dp)) != NULL) 
		{
			if (dirp->d_type == DT_REG)
			{
				files.push_back(dir + "/" + std::string(dirp->d_name));
			}
			else if (recursive && dirp->d_type == DT_DIR)
			{
				if (dirp->d_name[0] != '.')
				{
					todo.push_back(dir + "/" + dirp->d_name);				
				}
			}
		}
		closedir(dp);

		for (size_t u = 0; u < todo.size(); u++)
		{
			getDirectoryContent(todo[u], files, recursive);
		}

		return 0;
	}


	bool isSupportedImageType(const std::string& filename)
	{
		size_t idx = filename.rfind('.');
		if (idx != std::string::npos)
		{
			std::string ext = filename.substr(idx+1);
			std::transform(ext.begin(), ext.end(), ext.begin(), toupper);

			// from OpenCV documentation:
			if (ext == "BMP"  ||  
				ext == "DIB"  ||
				ext == "JPEG" || 
				ext == "JPG"  ||
				ext == "JPE"  ||
				ext == "PNG"  || 
				ext == "PBM"  ||
				ext == "PGM"  ||
				ext == "PPM"  ||
				ext == "SR"   ||
				ext == "RAS"  ||
				ext == "TIFF" ||
				ext == "TIF") // nice vertical lines
			{
				return true;
			}
		}
		return false;
	}

	void filterOnlyImages(strings& files)
	{
		strings out; // O(N)

		for (size_t u = 0; u < files.size(); u++)
		{
			if (isSupportedImageType(files[u]))
			{
				out.push_back(files[u]);
			}
		}

		files = out;
	}
}

