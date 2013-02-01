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
#ifndef _scanner_h
#define _scanner_h

#include <cstdio>
#include <cstdarg>
#include "comdef.h"

namespace imago
{
   class Scanner
   {
   public:
      virtual ~Scanner ();

      virtual void read( int length, void *res ) = 0;
      virtual void skip( int n ) = 0;
      virtual bool isEOF() = 0;
      virtual int  lookNext() = 0;
      virtual void seek( int pos, int from ) = 0;
      virtual int  length() = 0;
      virtual int  tell() = 0;

      virtual byte readByte();
      virtual void readAll( std::string &arr );

	  void readBinaryString( std::string &out );
      void readString( std::string &out, bool append_zero );

      char  readChar();
      word  readBinaryWord();
      int   readBinaryInt();
      dword readBinaryDword();
      float readBinaryFloat();
	  double readBinaryDouble();
      short readPackedShort();

      bool  skipString();
      void  readCharsFix( int n, char *chars_out );
      void  skipSpace();

      double readDouble();
      bool  tryReadDouble( double &value );
      int readInt();
      int readInt1();
      int readUnsigned ();

      // when delimiters = 0, any isspace() character is considered delimiter
      void readWord( std::string &word, const char *delimiters );

      static bool isSingleLine( Scanner &scanner );
   };

   class FileScanner : public Scanner
   {
   public:
      explicit FileScanner( const char *format, ... );
      virtual ~FileScanner ();

      virtual void read( int length, void *res );
      virtual bool isEOF();
      virtual void skip( int n );
      virtual int  lookNext();
      virtual void seek( int pos, int from );
      virtual int  length();
      virtual int  tell();

   private:
      FILE *_file;
      int   _file_len;

      // no implicit copy
      FileScanner( const FileScanner & );
   };

   class BufferScanner : public Scanner
   {
   public:
      explicit BufferScanner( const char *buffer, int buffer_size );
      explicit BufferScanner( const byte *buffer, int buffer_size );
      explicit BufferScanner( const char *str );
      explicit BufferScanner( const std::string &arr );

      virtual bool isEOF();
      virtual void read( int length, void *res );
      virtual void skip( int n );
      virtual int  lookNext();
      virtual void seek( int pos, int from );
      virtual int  length();
      virtual int  tell();
      virtual byte readByte();

      const void *curptr();
   private:
      const char *_buffer;
      int   _size;
      int   _offset;

      void _init ( const char *buffer, int length );

      // no implicit copy
      BufferScanner( const BufferScanner & );
   };
}

#endif /* _scanner_h */