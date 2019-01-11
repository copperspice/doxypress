/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
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