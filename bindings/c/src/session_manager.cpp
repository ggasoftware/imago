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

#include <iostream>
#include <boost/thread.hpp>

#include "session_manager.h"

using namespace imago;

boost::mutex SessionManager::_mutex;
boost::thread_specific_ptr<qword> SessionManager::_curSID;
SessionManager SessionManager::_instance;


SessionManager::SessionManager()
{
   _freeSID = 0;
}

qword SessionManager::getSID()
{
   qword *pid = _curSID.get();
   qword id;

   if (pid == 0)
   {
      id = allocSID();
      setSID(id);
   }
   else
      id = *pid;

   return id;
}

qword SessionManager::allocSID()
{
   lock_guard lock(_mutex);
   qword id;

   if (_availableSIDs.size() > 0)
   {
      id = _availableSIDs.front();
      _availableSIDs.pop_front();
   }
   else
   {
      while (_activeSessions.find(_freeSID) !=  _activeSessions.end())
         ++_freeSID;

      id = _freeSID;
      ++_freeSID;
   }

   _activeSessions.insert(id);
   return id;
}

void SessionManager::setSID( qword id )
{
   lock_guard lock(_mutex);

   if (_activeSessions.find(id) == _activeSessions.end())
   {
      //keep working or throw an exception?
      //throw WrongSessionIdException();
      _activeSessions.insert(id);
   }

   qword *pId = _curSID.get();
   if (pId == 0)
   {
      _curSID.reset(new qword(id));
      pId = _curSID.get();
   }
   else
      *pId = id;
}

void SessionManager::releaseSID( qword id )
{
   lock_guard lock(_mutex);

   IdSet::iterator curSessionIt = _activeSessions.find(id);
   if (curSessionIt == _activeSessions.end())
   {
      std::cerr << "Trying to release unallocated session " << id << "\n";
      return;
   }

   _activeSessions.erase(curSessionIt);
   _availableSIDs.push_back(id);
}

SessionManager &SessionManager::getInstance()
{
   return _instance;
}

SessionManager::~SessionManager()
{
   _activeSessions.clear();
   _availableSIDs.clear();
   _curSID.reset(0);
}

