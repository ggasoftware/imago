#include <map>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

#include "recognition_context.h"

namespace imago
{
   typedef std::map<qword, RecognitionContext *> ContextMap;
   static ContextMap _contexts;
   static boost::mutex _contexts_mutex;

   RecognitionContext *getContextForSession( qword sessionId )
   {
      boost::lock_guard<boost::mutex> lock(_contexts_mutex);
      ContextMap::iterator it;
      if ((it = _contexts.find(sessionId)) == _contexts.end())
         return NULL;

      return it->second;
   }

   void setContextForSession( qword sessionId, RecognitionContext *context )
   {
      boost::lock_guard<boost::mutex> lock(_contexts_mutex);
      ContextMap::iterator it;
      if ((it = _contexts.find(sessionId)) == _contexts.end())
         _contexts.insert(std::make_pair(sessionId, context));
      else 
         it->second = context;
   }

   void deleteRecognitionContext(qword sessionId, RecognitionContext *context)
   {
      boost::lock_guard<boost::mutex> lock(_contexts_mutex);
      delete context;
      _contexts.erase(_contexts.find(sessionId));
   }

   struct _ContextCleanup
   {
      ~_ContextCleanup()
      {
         BOOST_FOREACH(ContextMap::value_type item, _contexts)
         {
            delete item.second;
         }
      }
   };

   static _ContextCleanup _deleter;
}