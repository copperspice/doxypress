/************************************************************************
*
* Copyright (C) 2014-2020 Barbara Geller & Ansel Sermersheim
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

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <QString>
#include <QVector>

#include <types.h>

// class contains the information about the argument of a function or template
struct Argument {
   Argument() {}

   // copy constructor
   Argument(const Argument &a) {
      attrib  = a.attrib;
      type    = a.type;
      name    = a.name;
      defval  = a.defval;
      docs    = a.docs;
      array   = a.array;
      typeConstraint = a.typeConstraint;
   }

   // Assignment operator
   Argument &operator=(const Argument &a) {
      if (this != &a) {
         attrib = a.attrib;
         type   = a.type;
         name   = a.name;
         defval = a.defval;
         docs   = a.docs;
         array  = a.array;
         typeConstraint = a.typeConstraint;
      }
      return *this;
   }

   // return true if this argument is documentation and the argument has a  non empty name
   bool hasDocumentation() const {
      return ! name.isEmpty() && ! docs.isEmpty();
   }

   QString attrib;             /*!< Argument's attribute (IDL only) */
   QString type;               /*!< Argument's type */
   mutable QString canType;    /*!< Cached value of canonical type (after type resolution). Empty initially. */
   QString name;               /*!< Argument's name (may be empty) */
   QString array;              /*!< Argument's array specifier (may be empty) */
   QString defval;             /*!< Argument's default value (may be empty) */
   QString docs;               /*!< Argument's documentation (may be empty) */
   QString typeConstraint;     /*!< Used for Java generics: <T extends C> */
};

// stores information about a member which is typically found  after the argument list,
//  such as whether the member is const, volatile or pure virtual.
class ArgumentList : public QVector<Argument>
{
 public:
   // Creates an empty argument list
   ArgumentList() : constSpecifier(false), volatileSpecifier(false), pureSpecifier(false),
                    refSpecifier(RefType::NoRef), isDeleted(false)
   {
   }

   ~ArgumentList() {}

   // Does any argument of this list have documentation?
   bool hasDocumentation() const;

   // Does the member modify the state of the class? default: false.
   bool constSpecifier;

   // Is the member volatile? default: false.
   bool volatileSpecifier;

   // Is this a pure virtual member? default: false
   bool pureSpecifier;

   RefType refSpecifier;

   // C++11 style Trailing return type
   QString trailingReturnType;

   // parsing a method with = delete
   bool isDeleted;

   // methods
   bool isEmpty() const = delete;

   bool listEmpty() const {
      return QVector<Argument>::isEmpty();
   }
};

using ArgumentListIterator = QVectorIterator<Argument>;

#endif
