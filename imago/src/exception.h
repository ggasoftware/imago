/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

/**
  * @file   exception.h
  * 
  * @brief  Declares Exception class
  */

#pragma once
#ifndef _exception_h
#define _exception_h

#include <cstdarg>
#include <cstdio>
#include <exception>
#include <stdexcept>
#include <string>

#include "comdef.h"

namespace imago
{
	class ImagoException : public std::runtime_error
	{
	public: 
		ImagoException(const std::string& error) : std::runtime_error(error) { }
		ImagoException(int errorCode) : std::runtime_error(str(errorCode)) { }
		
		static std::string str(int x)
		{
			char buffer[32];
			sprintf(buffer, "%i", x);
			return buffer;
		}
	};

	class OutOfBoundsException : public ImagoException
	{
	public:
		OutOfBoundsException(const std::string& error, int index1, int index2)
			: ImagoException(error + " index(" + str(index1) + ", " + str(index2) + ")") { }
	};

	class NoContourException : public ImagoException
	{
	public: 
		NoContourException(const std::string& error) : ImagoException(error) { }
	};

	class LabelException : public ImagoException
	{
	public: 
		LabelException(const std::string& error) : ImagoException(error) { }
	};

	class FileNotFoundException : public ImagoException
	{
	public: 
		FileNotFoundException(const std::string& error) : ImagoException(error) { }
	};

	class LogicException : public ImagoException
	{
	public: 
		LogicException(const std::string& error) : ImagoException(error) { }
		LogicException(const int errorCode) : ImagoException("code: " + str(errorCode)) { }
	};

	class IOException : public ImagoException
	{
	public: 
		IOException(const std::string& error) : ImagoException(error) { }
	};

	class DivizionByZeroException : public ImagoException
	{
	public: 
		DivizionByZeroException(const std::string& error) : ImagoException(error) { }
	};

	class OCRException : public ImagoException
	{
	public: 
		OCRException(const std::string& error) : ImagoException(error) { }
	};
}

#endif /* _exception_h */
