/*************************************************************************
 *
 * Copyright (C) 2014-2016 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <classdef.h>
#include <stringmap.h>

struct CallContext {  
   QString name;
   QString type;
   QSharedPointer<Definition> d;
};


/*! Represents a stack of variable to class mappings as found in the code. 
 *  Each scope is enclosed in pushScope() and popScope() calls.
 *  Variables are added by calling addVariables() and you can search
 *  for variables using findVariable().
 */
class VariableContext
{
 public:
   static QSharedPointer<ClassDef> dummyContext();     
   
   VariableContext()  
   { 
   }
   
   virtual ~VariableContext() 
   { 
   }
    
   void pushScope() 
   {
      m_scopes.append(new StringMap<QSharedPointer<ClassDef>>);
    }

   void popScope() {
      if (m_scopes.count() > 0) {
         m_scopes.removeAt(m_scopes.count() - 1);
      }
   }

   void clear() {
      m_scopes.clear();
      m_globalScope.clear();
   }

    void clearExceptGlobal() {
      m_scopes.clear();
   }

   void addVariable(const QString &type, const QString &name);
   QSharedPointer<ClassDef> findVariable(const QString &name);

   int count() const {
      return m_scopes.count();
   }
    
 private:
   StringMap<QSharedPointer<ClassDef>>          m_globalScope;
   QList<StringMap<QSharedPointer<ClassDef>> *> m_scopes;
};