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

#pragma once
#ifndef _output_h
#define _output_h

#include <cstdio>
#include <cstdarg>
#include <string>

#include "stl_fwd.h"

namespace imago
{
   class Output
   {
   public:
      explicit Output();
      virtual ~Output();

      virtual void write( const void *data, int size ) = 0;
      virtual void seek( int offset, int from ) = 0;
      virtual int tell() = 0;

      void writeByte( byte value );
      void writeChar( char value );
      void writeBinaryDouble( double value );
      void writeBinaryInt( int value );
      void writeBinaryString( const char *string );

      void writeString( const char *string );
      void writeStringCR( const char *string );
      void writeCR();

      void printf( const char *format, ... );
      void vprintf( const char *format, va_list args );
      void skip( int count );
   };

   class FileOutput : public Output
   {
   public:
      FileOutput();
      explicit FileOutput( const char *format, ... );
      virtual ~FileOutput();

      void reopen( const char *format, ... );
      void flush();
      
      virtual void write( const void *data, int size );
      virtual void seek( int offset, int from );
      virtual int tell();

   private:
      FILE *_f;
   };

   class ArrayOutput : public Output
   {
   public:
      explicit ArrayOutput( std::string &arr );
      virtual ~ArrayOutput();

      virtual void write( const void *data, int size );
      virtual void seek( int offset, int from );
      virtual int tell();

   private:
      std::string &_buf;
   };

   class StandardOutput : public Output
   {
   public:
      StandardOutput();
      virtual ~StandardOutput();

      virtual void write( const void *data, int size );
      virtual void seek( int offset, int from );
      virtual int tell();

   private:
      int _pos;
   };
}


#endif /* _output_h */