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
#ifndef _session_manager_h
#define _session_manager_h

#include <set>
#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

#include "comdef.h"

namespace imago
{
   class SessionManager
   {
   public:
      static SessionManager &getInstance();

      qword getSID();
      void setSID( qword id );

      qword allocSID();
      void releaseSID( qword id );
   private:
      qword _freeSID;
      
      typedef std::deque<qword> IdContainer;
      IdContainer _availableSIDs;
      typedef std::set<qword> IdSet;
      IdSet _activeSessions;

      static boost::thread_specific_ptr<qword> _curSID;
      static SessionManager _instance;
      static boost::mutex _mutex;
      typedef boost::lock_guard<boost::mutex> lock_guard;

      SessionManager();
      SessionManager( const SessionManager& );
      ~SessionManager();
   };
}

#endif /* _session_manager_h */

