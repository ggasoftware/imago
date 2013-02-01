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

#include <cstdio>
#include <cstdarg>
#include <string>
#include <cstring>
#include <vector>

#include "comdef.h"
#include "exception.h"
#include "output.h"

using namespace imago;

Output::Output()
{
}

void Output::writeByte( byte value )
{
   write(&value, sizeof(byte));
}

void Output::writeChar( char value )
{
   write(&value, sizeof(char));
}

void Output::writeBinaryDouble( double value )
{
   write(&value, sizeof(double));
}

void Output::writeBinaryInt( int value )
{
   write(&value, sizeof(int));
}

void Output::writeString( const char *string )
{
   int n = (int)strlen(string);

   write(string, n);
}

void Output::writeBinaryString( const char *string )
{
   writeString(string);
   writeByte(0);
}

void Output::writeCR()
{
   writeChar('\n');
}

void Output::writeStringCR( const char *string )
{
   writeString(string);
   writeCR();
}

void Output::printf( const char* format, ... )
{
   va_list args;
  
   va_start(args, format);
   vprintf(format, args);
   va_end(args);
}

void Output::vprintf( const char *format, va_list args )
{
   char str[MAX_TEXT_LINE];
   int n = vsnprintf(str, sizeof(str), format, args);

   write(str, n);
}

void Output::skip( int count )
{
   seek(count, SEEK_CUR);
}

Output::~Output()
{
}

FileOutput::FileOutput()
{
   _f = 0;
}

FileOutput::FileOutput( const char *format, ... )
{
   char filename[MAX_TEXT_LINE];

   va_list args;

   va_start(args, format);
   vsnprintf(filename, sizeof(filename), format, args);
   va_end(args);

   _f = fopen(filename, "wb");

   if (_f == NULL)
      throw FileNotFoundException(filename);
}

void FileOutput::write( const void *data, int size )
{
   if (!_f)
      throw LogicException("FileOutput isn't opened");

   if (size < 1)
      return;

   int res = fwrite(data, size, 1, _f);

   if (res == -1)
      throw IOException("file writing error");
}

void FileOutput::seek( int offset, int from )
{
   if (!_f)
      throw LogicException("FileOutput isn't opened");

   fseek(_f, offset, from);
}

int FileOutput::tell()
{
   if (!_f)
      throw LogicException("FileOutput isn't opened");

   return ftell(_f);
}

void FileOutput::flush()
{
   fflush(_f);
}

void FileOutput::reopen( const char *format, ... )
{
   char filename[MAX_TEXT_LINE];

   va_list args;

   va_start(args, format);
   vsnprintf(filename, sizeof(filename), format, args);
   va_end(args);

   if (_f)
      fclose(_f);

   _f = fopen(filename, "wb");

   if (_f == NULL)
      throw FileNotFoundException("can't open file " + std::string(filename));
}

FileOutput::~FileOutput()
{
   if (_f)
      fclose(_f);
}

ArrayOutput::ArrayOutput( std::string &arr ) : _buf(arr)
{
   _buf.clear();
}

void ArrayOutput::write( const void *data, int size )
{
   int old_size = _buf.size();

   _buf.resize(old_size + size);

   for (int i = 0; i < size; i++)
      _buf[i + old_size] = *((byte *)data + i);
}

void ArrayOutput::seek( int offset, int from )
{
   throw ImagoException("no seek in Array Output");
}

int ArrayOutput::tell()
{
   return _buf.size();
}

ArrayOutput::~ArrayOutput()
{
}

StandardOutput::StandardOutput()
{
   _pos = 0;
}

void StandardOutput::write( const void *data, int size )
{
   int ret_size = (int)fwrite(data, size, 1, stdout);

   if (ret_size != 1)
      throw IOException("error writing in standard output");

   _pos += size;
}

void StandardOutput::seek( int offset, int from )
{
   throw LogicException("no seek in standard output");
}

int StandardOutput::tell()
{
   return _pos;
}

StandardOutput::~StandardOutput()
{
}

