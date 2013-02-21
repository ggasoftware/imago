/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
 *
 * This file is part of Imago OCR project.
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
#ifndef _failsafe_png_h
#define _failsafe_png_h

#include "image.h"
#include <string>

namespace imago
{
	bool failsafePngLoadBuffer(const unsigned char* buffer, size_t buf_size, Image& img);
	bool failsafePngLoadFile(const std::string& fname, Image& img);
}

#endif // _failsafe_png_h
