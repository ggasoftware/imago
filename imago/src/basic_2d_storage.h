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
#ifndef _basic_2d_storage_h
#define _basic_2d_storage_h

namespace imago
{
	// two-dimensional storage for specified type data
	template <class t> class Basic2dStorage
	{
	public:
		// initialize dimensions and data pointer, fill with zero, fast
		Basic2dStorage(int width, int height)
		{
			_w = width;
			_h = height;
			data = new t[_w*_h]();
		}

		// initialize dimensions and data pointer, fill with 'value'
		Basic2dStorage(int width, int height, t value)
		{
			_w = width;
			_h = height;
			data = new t[_w * _h];
			for (int u = 0; u < _w*_h; u++)
				data[u] = value;
		}

		// delete data pointer
		virtual ~Basic2dStorage()
		{
			if (data)
				delete []data;
		}

		// index operator
		const t& at(int x, int y) const
		{
			return data[x + y * _w];
		}

		// index operator
		t& at(int x, int y)
		{
			return data[x + y * _w];
		}

		// dimensions getters
		int width() const { return _w; }
		int height() const { return _h; }

		// return true if point (x,y) fits in storage
		bool inRange(int x, int y) const
		{
			return x >= 0 && y >= 0 && x < _w && y < _h;
		}

	private:
		int _w, _h;
		t* data;
	};
}

#endif
