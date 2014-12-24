/*************************************************************************
 *
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <QByteArray>
#include <QStringList>
#include <QHash>
#include <QList>

#include <ftextstream.h>

// convenience macros for access the config options
#define Config_getString(val)  Config::instance()->getString(__FILE__,__LINE__,val)
#define Config_getInt(val)     Config::instance()->getInt(__FILE__,__LINE__,val)
#define Config_getList(val)    Config::instance()->getList(__FILE__,__LINE__,val)
#define Config_getEnum(val)    Config::instance()->getEnum(__FILE__,__LINE__,val)
#define Config_getBool(val)    Config::instance()->getBool(__FILE__,__LINE__,val)

/** 
 *  This object holds the global static variables read from a user supplied configuration file.
 *  The static member instance() can be used to get a pointer to the one and only instance.
 *
 *  Set all variables to their default values by calling Config::instance()->init()
 *
 */
class Config
{
 public:  
   /*! Returns the one and only instance of this class */
   static Config *instance() {
      if (m_instance == 0) {
         m_instance = new Config;
      }

      return m_instance;
   }

   /*! Delete the instance */
   static void deleteInstance() {
      delete m_instance;
      m_instance = 0;
   }

   QByteArray json_ReadFile(QString fName);
   void parseConfig();
   
   /*!
    *  @name Getting configuration values.
    *  @{
    */

   /*! Returns the value of the string option with name \a fileName.
    *  The arguments \a num and \a name are for debugging purposes only.
    *  There is a convenience function Config_getString() for this.
    */
   QByteArray &getString(const char *fileName, int num, const char *name) const;

   /*! Returns the value of the list option with name \a fileName.
    *  The arguments \a num and \a name are for debugging purposes only.
    *  There is a convenience function Config_getList() for this.
    */
   QStringList &getList(const char *fileName, int num, const char *name) const;

   /*! Returns the value of the enum option with name \a fileName.
    *  The arguments \a num and \a name are for debugging purposes only.
    *  There is a convenience function Config_getEnum() for this.
    */
   QByteArray &getEnum(const char *fileName, int num, const char *name) const;

   /*! Returns the value of the integer option with name \a fileName.
    *  The arguments \a num and \a name are for debugging purposes only.
    *  There is a convenience function Config_getInt() for this.
    */
   int &getInt(const char *fileName, int num, const char *name) const;

   /*! Returns the value of the boolean option with name \a fileName.
    *  The arguments \a num and \a name are for debugging purposes only.
    *  There is a convenience function Config_getBool() for this.
    */
   bool &getBool(const char *fileName, int num, const char *name) const;
  
   /* @} */
    
  
  
   /*! Writes a template configuration to stream \a t. If \a shortIndex
    *  is \c true the description of each configuration option will
    *  be omitted.
    */
   void writeTemplate(FTextStream &t, bool shortIndex, bool updateOnly);

   void setHeader(const char *header) {
      m_header = header;
   }

   
   // internal API
   
   /*! Converts the string values read from the configuration file
    *  to real values for non-string type options (like int, and bools)
    */
   void convertStrToVal();

   /*! Replaces references to environment variable by the actual value
    *  of the environment variable.
    */
   void substituteEnvironmentVars();

   /*! Checks if the values of the variable are correct, adjusts them
    *  if needed, and report any errors.
    */
   void check();

   /*! Initialize config variables to their default value */
   void init();

   /*! Parse a configuration data in string \a str.
    *  \returns true if successful, or false if the string could not be
    *  parsed.
    */
   //bool parseString(const char *fn,const char *str);
   bool parseString(const char *fn, const char *str, bool upd = false);

   /*! Parse a configuration file with name \a fn.
    *  \returns true if successful, false if the file could not be
    *  opened or read.
    */
   bool parse(const char *fn, bool upd = false);

   /*! Called from the constructor, will add doxygen's default options
    *  to the configuration object
    */
   void create();
     

  private:

   Config() { 
      m_initialized = false;
      create();
   }

   ~Config() 
   { }
 
   QHash<QString, QString> m_cfg_String;
   QHash<QString, bool> m_cfg_Bool;

   void checkFileName(const char *);
  
   static Config *m_instance;

   QByteArray m_userComment;  
   QByteArray m_header;

   bool m_initialized;
};

#endif
