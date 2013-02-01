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
#include <cstring>
#include <string>

#include "imago_c.h"
#include "imago_version.h"
#include "indigo.h"
#include "image_utils.h"
#include "molfile_saver.h"
#include "log_ext.h"
#include "exception.h"
#include "output.h"
#include "scanner.h"
#include "comdef.h"
#include "session_manager.h"
#include "superatom_expansion.h"
#include "settings.h"
#include "failsafe_png.h"
#include "recognition_context.h"
#include "prefilter_entry.h"
#include "filters_list.h"

#define IMAGO_BEGIN try {                                                    

#define IMAGO_END   } catch ( ImagoException &e )                            \
                    {                                                        \
                       RecognitionContext *context = getCurrentContext();    \
                       std::string &error_buf = context->error_buf;          \
                       error_buf.erase();                                    \
                       ArrayOutput aout(error_buf);                          \
                       aout.writeBinaryString(e.what());                     \
                       return 0;                                             \
                    }                                                        \
                    return 1;

using namespace imago;

CEXPORT const char *imagoGetVersion()
{
   return IMAGO_VERSION;
}

CEXPORT qword imagoAllocSessionId()
{
   return SessionManager::getInstance().allocSID();
}

CEXPORT void imagoSetSessionId( qword id )
{
   SessionManager::getInstance().setSID(id);
   indigoSetSessionId(id);

   RecognitionContext *context = getCurrentContext();
      
   if (context == 0)
      setContextForSession(id, new RecognitionContext());
}

CEXPORT void imagoReleaseSessionId( qword id )
{
   indigoReleaseSessionId(id);
   RecognitionContext *context;
   if ((context = getCurrentContext()) != 0)
      deleteRecognitionContext(id, context);

   SessionManager::getInstance().releaseSID(id);
}

CEXPORT const char* imagoGetConfigsList()
{  
   RecognitionContext *context = getCurrentContext();
   
   // TODO: fill configs list

   return context->configs_list.c_str();
}

CEXPORT int imagoSetConfig( const char *Name )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();

   if (Name == NULL || strlen(Name) == 0)
   {
	   context->vars.selectBestCluster();
   }
   else
   {
	   bool loaded = context->vars.forceSelectCluster(Name);

	   if (!loaded)
		   throw ImagoException(std::string("Config not found: ") + Name);
   }

   IMAGO_END;
}

CEXPORT int imagoSetFilter( const char *Name )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   bool found = false;
   
   FilterEntries entries = getFiltersList();

   for (size_t i = 0; i < entries.size(); i++)
   {
	   if (strcmp(Name, entries[i].name.c_str()) == 0)
	   {
		   context->vars.general.FilterIndex = i;
		   found = true;
	   }
   }

   if (!found)
	   throw ImagoException(std::string("Filter not found: ") + Name);

   IMAGO_END;
}


CEXPORT int imagoLoadImageFromFile( const char *FileName )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   ImageUtils::loadImageFromFile(context->img_src, FileName);
   context->img_tmp = context->img_src;
      
   IMAGO_END;
}

CEXPORT int imagoFilterImage()
{
   IMAGO_BEGIN;
   
   RecognitionContext *context = getCurrentContext();   
   prefilterEntrypoint(context->vars, context->img_tmp, context->img_src);

   IMAGO_END;
}

CEXPORT int imagoLoadImageFromBuffer( const char *buf, const int buf_size )
{
   IMAGO_BEGIN;
   
   RecognitionContext *context = getCurrentContext();
   const unsigned char* buf_uc = (const unsigned char*)buf;
   failsafePngLoadBuffer(buf_uc, buf_size, context->img_src);
   context->img_tmp = context->img_src;

   IMAGO_END;
}

CEXPORT int imagoLoadGreyscaleRawImage( const char *buf, const int width, const int height )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   Image &img = context->img_src;

   img.clear();

   img.init(width, height);
   
   for (int y = 0; y < height; y++)
	   for (int x = 0; x < width; x++)
		   img.getByte(x,y) = buf[y * width + x];

   context->img_tmp = context->img_src;

   IMAGO_END;
}

CEXPORT int imagoGetInkPercentage(double *result)
{
	IMAGO_BEGIN;
	RecognitionContext *context = getCurrentContext();
	Image &img = context->img_tmp;
	
	size_t ink = 0;
	size_t total = img.getWidth() * img.getHeight();
	for (int y = 0; y < img.getHeight(); y++)
	{
		for (int x = 0; x < img.getWidth(); x++)
		{
			if (img.getByte(x,y) < 64)
				ink++;			
		}
	}

	if (result)
	{
		if (total > 0)
			*result = (double)ink / (double)total;
		else
			*result = 0;
	}
   
	IMAGO_END;
}

CEXPORT int imagoGetPrefilteredImageSize (int *width, int *height)
{
   IMAGO_BEGIN;
   RecognitionContext *context = getCurrentContext();
   Image &img = context->img_tmp;

   *height = img.getHeight();
   *width = img.getWidth();
   IMAGO_END;
}

CEXPORT int imagoGetPrefilteredImage (unsigned char **data, int *width, int *height)
{
   IMAGO_BEGIN;
   RecognitionContext *context = getCurrentContext();
   Image &img = context->img_tmp;
   unsigned char *buf = new unsigned char[img.getWidth() * img.getHeight()];

   *height = img.getHeight();
   *width = img.getWidth();

   for (int j = 0; j != img.getHeight(); j++)
   {
      int offset = j * img.getWidth();

      for (int i = 0; i != img.getWidth(); i++)
      {
         buf[offset + i] = img.getByte(i, j);
      }
   }

   *data = buf;
   IMAGO_END;
}

CEXPORT int imagoSaveImageToFile( const char *filename )
{
   IMAGO_BEGIN;
   RecognitionContext *context = getCurrentContext();
   if (context->img_tmp.isInit())
   {
      ImageUtils::saveImageToFile(context->img_tmp, filename);
   }

   IMAGO_END;
}

CEXPORT int imagoRecognize(int* warningsCountDataOut)
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   ChemicalStructureRecognizer &csr = context->csr;

   csr.setImage(context->img_tmp);
   csr.recognize(context->vars, context->mol);
   if (warningsCountDataOut)
   {
	   (*warningsCountDataOut) = context->mol.getWarningsCount() + context->mol.getDissolvingsCount() / context->vars.main.DissolvingsFactor;
   }
   context->molfile = expandSuperatoms(context->vars, context->mol);

   IMAGO_END;
}

CEXPORT int imagoSaveMolToFile( const char *FileName )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   
   {
	   FileOutput fout(FileName);
	   fout.writeString(context->molfile.c_str());
   }

   IMAGO_END;
}

CEXPORT int imagoSaveMolToBuffer( char **buf, int *buf_size )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   std::string &out_buf = context->molfile;
   
   *buf = new char[out_buf.size() + 1];
   memcpy(*buf, out_buf.c_str(), out_buf.size());
   *buf_size = out_buf.size();
   (*buf)[out_buf.size()] = 0;

   IMAGO_END;
}

CEXPORT int imagoSetLogging( int mode )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();

   if (mode > 0)
   {
      context->vars.general.LogEnabled = true;
      imago::getLogExt().setLoggingEnabled(true);

      if (mode == 2)
      {
         context->vars.general.LogVFSEnabled = true;
         imago::getLogExt().SetVirtualFS(context->vfs);
         context->vfs.clear();
      }
      else
      {
         context->vars.general.LogVFSEnabled = false;
         imago::getLogExt().SetNoVirtualFS();
      }
   }
   else 
   {
      context->vars.general.LogEnabled = false;
      imago::getLogExt().setLoggingEnabled(false);
   }
   
   IMAGO_END;
}

CEXPORT int imagoSetSessionSpecificData( void *data )
{
   IMAGO_BEGIN;

   RecognitionContext *context = getCurrentContext();
   context->session_specific_data = data;

   IMAGO_END;
}

CEXPORT int imagoGetSessionSpecificData( void **data )
{
   IMAGO_BEGIN;

   if (data != 0)
   {
      RecognitionContext *context = getCurrentContext();
      *data = context->session_specific_data;
   }

   IMAGO_END;
}

CEXPORT const char* imagoGetLastError()
{
   RecognitionContext *context = getCurrentContext();

   return context->error_buf.c_str();
}

CEXPORT int imagoClearLog( )
{
	IMAGO_BEGIN;

	RecognitionContext *context = getCurrentContext();
	context->vfs.clear();

	IMAGO_END;
}


CEXPORT int imagoGetLogCount( int *count )
{
   IMAGO_BEGIN;

   VirtualFS &vfs = getCurrentContext()->vfs;
   *count = vfs.size();

   IMAGO_END;
}

CEXPORT int imagoGetLogRecord( int it, char **filename, int *length, char **data)
{
   IMAGO_BEGIN;

   VirtualFS &vfs = getCurrentContext()->vfs;
   std::string &fname = vfs[it].filename;
   *filename = new char[fname.length() + 1];
   *length = vfs[it].data.size();
   *data = new char[vfs[it].data.size()];

   memcpy(*filename, fname.c_str(), fname.length());
   (*filename)[fname.length()] = 0;
   memcpy(*data, &vfs[it].data[0], *length);

   IMAGO_END;
}
