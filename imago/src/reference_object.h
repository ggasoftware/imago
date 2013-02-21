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
#ifndef _reference_object_h
#define _reference_object_h

#include <map>
#include <string>

namespace imago
{
	class DataTypeReference
	{
	public:
		enum ObjectType
		{
			otUndef,
			otBool,
			otInt,
			otDouble
		};

		DataTypeReference() : b_value(NULL)
		{
			type = otUndef;
		}

		explicit DataTypeReference(bool& value) : b_value(&value)
		{
			type = otBool;
		}

		explicit DataTypeReference(int& value) : i_value(&value)
		{
			type = otInt;
		}

		explicit DataTypeReference(double& value) : d_value(&value)
		{
			type = otDouble;			
		}

		ObjectType getType() const
		{
			return type;
		}

		bool* getBool() const
		{
			return b_value;
		}

		int* getInt() const
		{
			return i_value;
		}

		double* getDouble() const
		{
			return d_value;
		}

	private:
		union
		{
			bool* b_value;
			int* i_value;
			double* d_value;
		};
		ObjectType type;
	};

	typedef std::map<std::string, DataTypeReference> ReferenceAssignmentMap;
}

#endif // _reference_object_h
