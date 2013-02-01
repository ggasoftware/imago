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

#ifndef _recognition_helpers_h
#define _recognition_helpers_h

#include <string>
#include "virtual_fs.h"
#include "settings.h"
#include "image.h"

namespace recognition_helpers
{
	void dumpVFS(imago::VirtualFS& vfs, const std::string& filename);
	void applyConfig(bool verbose, imago::Settings& vars, const std::string& config);

	struct RecognitionResult
	{
		std::string molecule;
		int warnings;
	};

	RecognitionResult recognizeImage(bool verbose, imago::Settings& vars, const imago::Image& src,
		                             const std::string& config);

	int performFilterTest(imago::Settings& vars, const std::string& imageName);

	int performFileAction(bool verbose, imago::Settings& vars, const std::string& imageName, 
		                  const std::string& configName, const std::string& outputName = "molecule.mol");

}

#endif