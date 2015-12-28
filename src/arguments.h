/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim
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

#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <QList>
#include <QString>

class StorageIntf;

/*! \brief This class contains the information about the argument of a
 *         function or template
 *
 */
struct Argument {
   /*! Construct a new argument. */
   Argument() {}

   /*! Copy an argument (does a deep copy of all strings) */
   Argument(const Argument &a) {
      attrib  = a.attrib;
      type    = a.type;
      name    = a.name;
      defval  = a.defval;
      docs    = a.docs;
      array   = a.array;
      typeConstraint = a.typeConstraint;
   }

   /*! Assignment of an argument (does a deep copy of all strings) */
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

   /*! return true if this argument is documentation and the argument has a
    *  non empty name.
    */
   bool hasDocumentation() const {
      return ! name.isEmpty() && ! docs.isEmpty();
   }

   QString attrib;          /*!< Argument's attribute (IDL only) */
   QString type;            /*!< Argument's type */
   QString canType;         /*!< Cached value of canonical type (after type resolution). Empty initially. */
   QString name;            /*!< Argument's name (may be empty) */
   QString array;           /*!< Argument's array specifier (may be empty) */
   QString defval;          /*!< Argument's default value (may be empty) */
   QString docs;            /*!< Argument's documentation (may be empty) */
   QString typeConstraint;  /*!< Used for Java generics: <T extends C> */
};

/*! \brief This class represents an function or template argument list.
 *
 *  This class also stores some information about member that is typically
 *  put after the argument list, such as whether the member is const,
 *  volatile or pure virtual.
 */
class ArgumentList : public QList<Argument>
{
 public:
   /*! Creates an empty argument list */
   ArgumentList() : constSpecifier(false), volatileSpecifier(false), pureSpecifier(false), isDeleted(false) 
   {
   }

   /*! Destroys the argument list */
   ~ArgumentList() {}

   /*! Does any argument of this list have documentation? */
   bool hasDocumentation() const;

   /*! Does the member modify the state of the class? default: false. */
   bool constSpecifier;

   /*! Is the member volatile? default: false. */
   bool volatileSpecifier;

   /*! Is this a pure virtual member? default: false */
   bool pureSpecifier;

   /*! C++11 style Trailing return type? */
   QString trailingReturnType;

   /*! method with =delete */
   bool isDeleted;
   
   static ArgumentList *unmarshal(StorageIntf *s);
   static void marshal(StorageIntf *s, const ArgumentList &argList);
};

using ArgumentListIterator = QListIterator<Argument>;

#endif
