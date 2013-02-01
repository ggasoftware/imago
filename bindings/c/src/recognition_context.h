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
#ifndef _recognition_context_h
#define _recognition_context_h

#include <string>

#include "comdef.h"
#include "chemical_structure_recognizer.h"
#include "image.h"
#include "molecule.h"
#include "settings.h"
#include "virtual_fs.h"
#include "session_manager.h"

namespace imago
{
   struct RecognitionContext
   {
      ChemicalStructureRecognizer csr;
      Image img_tmp;
	  Image img_src;
      Molecule mol;
      std::string molfile;
      std::string out_buf;
      std::string error_buf;
	  std::string configs_list;
      Settings vars;
      VirtualFS vfs;
      void *session_specific_data;
      
      RecognitionContext () 
      {
         session_specific_data = 0;
         error_buf = "No error";
      }
   };

   RecognitionContext *getContextForSession(qword sessionId);
   inline RecognitionContext *getCurrentContext()
   {
      return getContextForSession(SessionManager::getInstance().getSID());
   }

   void setContextForSession(qword sessionId, RecognitionContext *context);
   void deleteRecognitionContext(qword sessionId, RecognitionContext *context);
};


#endif /* _imago_session_h */