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

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <ctype.h>
#include <string>

#include "exception.h"
#include "comdef.h"
#include "scanner.h"

using namespace imago;

Scanner::~Scanner()
{
}

int Scanner::readInt1()
{
   std::string buf;
   char c;
   int result;

   buf.clear();

   while (!isEOF())
   {
      c = readChar();
      if (!isdigit(c) && c != '-' && c != '+')
         break;
      buf.push_back(c);
   }

   buf.push_back(0);

   if (sscanf(buf.c_str(), "%d", &result) < 1)
      throw IOException("readInt(): error parsing " + buf);

   return result;
}

int Scanner::readInt()
{
   std::string buf;
   char c;
   int result;

   buf.clear();

   c = readChar();

   if (c == '+' || c == '-' || isdigit(c))
      buf.push_back(c);

   while (isdigit(lookNext()))
      buf.push_back(readChar());

   buf.push_back(0);

   if (sscanf(buf.c_str(), "%d", &result) < 1)
      throw IOException("readInt(): error parsing " + buf);

   return result;
}

int Scanner::readUnsigned()
{
   int result = 0;
   bool was_digit = false;

   while (!isEOF())
   {
      char c = readChar();
      if (isdigit(c))
      {
         was_digit = true;
         result = (int)(c - '0') + result * 10;
      }
      else
      {
         seek(-1, SEEK_CUR);
         break;
      }
   }

   if (!was_digit)
      throw IOException("readUnsigned(): no digits");

   return result;
}

double Scanner::readDouble()
{
   std::string buf;
   double result;

   buf.clear();

   while (!isEOF())
   {
      char c = readChar();

      if (!isdigit(c) && c != '-' && c != '+' && c != '.')
         break;
      buf.push_back(c);
   }

   buf.push_back(0);

   if (sscanf(buf.c_str(), "%lf", &result) < 1)
      throw IOException("readDouble(): error parsing " + buf);

   return result;
}

bool Scanner::tryReadDouble( double &value )
{
   std::string buf;

   int pos = tell();

   buf.clear();

   while (!isEOF())
   {
      char c = readChar();

      if (!isdigit(c) && c != '-' && c != '+' && c != '.')
         break;
      buf.push_back(c);
   }

   buf.push_back(0);

   if (sscanf(buf.c_str(), "%lf", &value) < 1)
   {
      seek(pos, SEEK_SET);
      return false;
   }

   return true;
}

void Scanner::readWord( std::string &word, const char *delimiters )
{
   word.clear();

   while (!isEOF())
   {
      int next = lookNext();

      if (next == -1)
         break;

      if (delimiters == 0 && isspace((char)next))
         break;

      if (delimiters != 0 && strchr(delimiters, (char)next) != NULL)
         break;

      word.push_back(readChar());
   }

   word.push_back(0);
}

char Scanner::readChar()
{
   char c;

   read(sizeof(char), &c);

   return c;
}

byte Scanner::readByte()
{
   byte c;

   read(1, &c);
   return c;
}

bool Scanner::skipString()
{
   char c;

   if (isEOF())
      return false;

   while (!isEOF())
   {  
      c = readChar();
      if (c == '\n')
      {
         if (lookNext() == '\r')
            skip(1);
         return true;
      }
      if (c == '\r')
      {
         if (lookNext() == '\n')
            skip(1);
         return true;
      }
   }

   return false;
}

void Scanner::skipSpace()
{
   while (isspace(lookNext()))
      skip(1);
}

void Scanner::readBinaryString( std::string &out)
{
   char c;

   out.clear();

   while (!isEOF())
   {  
      c = readChar();

      if (c == '\0')
      {
		  break;
      }

      out.push_back(c);
   }
}


void Scanner::readString( std::string &out, bool append_zero )
{
   char c;

   out.clear();

   while (!isEOF())
   {  
      c = readChar();

      if (c == '\r')
      {
         if (lookNext() == '\n')
            continue;
         break;
      }
      if (c == '\n')
         break;

      out.push_back(c);
   }

   if (append_zero)
      out.push_back(0);
}

void Scanner::readCharsFix( int n, char *chars_out )
{
   read(n, chars_out);
}

word Scanner::readBinaryWord()
{
   word res;

   read(sizeof(word), &res);

   return res;
}

dword Scanner::readBinaryDword()
{
   dword res;

   read(sizeof(dword), &res);

   return res;
}

int Scanner::readBinaryInt()
{
   int res;

   read(sizeof(int), &res);

   return res;
}

float Scanner::readBinaryFloat()
{
   float res;

   read(sizeof(float), &res);

   return res;
}

double Scanner::readBinaryDouble()
{
   double res;

   read(sizeof(double), &res);

   return res;
}


short Scanner::readPackedShort()
{
   byte high = readByte();

   if (high < 128)
      return high;

   byte low = readByte();

   high -= 128;

   return high * (short)256 + low;
}

void Scanner::readAll( std::string &arr )
{
   arr.resize(length());

   char *res = new char[arr.size()];

   read(arr.size(), res);

   for (int i = 0; i < length(); i++)
      arr[i] = res[i];

   delete []res;
}

bool Scanner::isSingleLine( Scanner &scanner )
{
   int pos = scanner.tell();

   scanner.skipString();

   bool res = scanner.isEOF();

   scanner.seek(pos, SEEK_SET);
   return res;
}

FileScanner::FileScanner( const char *format, ... ) : Scanner()
{
   char filename[MAX_TEXT_LINE];

   va_list args;

   va_start(args, format);
   vsnprintf(filename, sizeof(filename), format, args);
   va_end(args);

   _file_len = 0;
   _file = fopen(filename, "rb");

   if (_file == NULL)
      throw FileNotFoundException("can't open file " + std::string(filename));

   fseek(_file, 0, SEEK_END);
   _file_len = ftell(_file);
   fseek(_file, 0, SEEK_SET);
}

int FileScanner::lookNext()
{
   char res;
   size_t nread = fread(&res, 1, 1, _file);

   if (nread == 0)
      return -1;

   fseek(_file, -1, SEEK_CUR);

   return res;
}

int FileScanner::tell()
{
   return ftell(_file);
}

void FileScanner::read( int length, void *res )
{
   size_t nread = fread(res, 1, length, _file);

   if (nread != (size_t)length)
      throw IOException("FileScanner::read() error");
}

void BufferScanner::skip( int n )
{
   _offset += n;

   if (_size >= 0 && _offset > _size)
      throw IOException("skip() passes after end of buffer");
}

void BufferScanner::seek( int pos, int from )
{
   if (from == SEEK_SET)
      _offset = pos;
   else if (from == SEEK_CUR)
      _offset += pos;
   else // SEEK_END
   {
      if (_size < 0)
         throw IOException("can not seek from end: buffer is unlimited");
      _offset = _size - pos;
   }

   if ((_size >= 0 && _offset > _size) || _offset < 0)
      throw IOException("size < offset after seek()");
}

byte BufferScanner::readByte ()
{
   if (_size >= 0 && _offset >= _size)
      throw IOException("readByte(): end of buffer");

   return _buffer[_offset++];
}

bool FileScanner::isEOF()
{
   if (_file == NULL)
      return true;

   return ftell(_file) == _file_len;
}

void FileScanner::skip( int n )
{
   int res = fseek(_file, n, SEEK_CUR);

   if (res != 0)
      throw IOException("skip() passes after end of file");
}

void FileScanner::seek( int pos, int from )
{
   fseek(_file, pos, from);
}

int FileScanner::length()
{
   return _file_len;
}

FileScanner::~FileScanner ()
{
   if (_file != NULL)
      fclose(_file);
}

void BufferScanner::_init( const char *buffer, int size )
{
   if (size < -1 || (size > 0 && buffer == 0))
      throw IOException("incorrect parameters in BufferScanner constructor");

   _buffer = buffer;
   _size = size;
   _offset = 0;
}

BufferScanner::BufferScanner( const char *buffer, int buffer_size )
{
   _init(buffer, buffer_size);
}

BufferScanner::BufferScanner( const byte *buffer, int buffer_size )
{
   _init((const char *)buffer, buffer_size);
}

BufferScanner::BufferScanner( const char *str )
{
   _init(str, (int)strlen(str));
}

BufferScanner::BufferScanner( const std::string &arr )
{
   _init(arr.c_str(), arr.size());
}

bool BufferScanner::isEOF()
{
   if (_size < 0)
      throw IOException("isEOF() called to unlimited buffer");
   return _size >= 0 && _offset >= _size;
}

void BufferScanner::read( int length, void *res )
{
   if (_size >= 0 && _offset + length > _size)
      throw IOException("BufferScanner::read() error");

   memcpy(res, &_buffer[_offset], length);
   _offset += length;
}

int BufferScanner::lookNext()
{
   if (_size >= 0 && _offset >= _size)
      return -1;

   return _buffer[_offset];
}

int BufferScanner::length()
{
   return _size;
}

int BufferScanner::tell()
{
   return _offset;
}

const void *BufferScanner::curptr()
{
   return _buffer + _offset;
}