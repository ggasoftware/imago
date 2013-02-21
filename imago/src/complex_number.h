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

/**
 * @file   complex_number.h
 * 
 * @brief  Complex numbers
 */

#pragma once
#ifndef _complex_number_h
#define _complex_number_h

#include <math.h>


namespace imago{

	class ComplexNumber
	  {
	  public:
		  ComplexNumber():_a(0.0), _b(0.0)
		  {}

		  ComplexNumber(double Real, double Imaginary);
		  
		  inline double getReal() const
		  { return _a; }

		  inline void setReal(double a)
		  { _a = a;}

		  inline double getImaginary() const
		  { return _b; }

		  inline void setImaginary(double b)
		  { _b = b;}

		  static ComplexNumber Dot(const ComplexNumber& n1, const ComplexNumber& n2);

		  ComplexNumber& operator*();
		  
		  const ComplexNumber& operator*() const;

		  ComplexNumber&  operator +(const ComplexNumber& n2);

		  ComplexNumber& operator +=(const ComplexNumber& cn)
		  {
			  _a += cn.getReal();
			  _b += cn.getImaginary();
			  return *this;
		  }

		  ComplexNumber operator -(const ComplexNumber& n)
		  {
			  return ComplexNumber(_a - n.getReal(), _b - n.getImaginary());
		  }

		  ComplexNumber& operator /(double n);

		  ComplexNumber& operator*(double n);

		  ComplexNumber& operator /=(double n);

		  ComplexNumber& operator *=(double n);

		  inline double getAngle() const
		  {
			  return atan2(_b, _a);
		  }

		  inline double getRadius() const
		  {
			  return sqrt(getRadius2());
		  }

		  inline double getRadius2() const
		  {
			  return _a * _a + _b * _b;
		  }

		  ComplexNumber  operator *(const ComplexNumber& n2);

	  private:
		  double _a, _b;
	  };
}
#endif
