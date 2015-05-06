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

#ifndef CONFIG_H
#define CONFIG_H

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>
  
class Config
{
   public:         
      Config() = delete;      

      static bool parseConfig(const QString &fName);

      static bool getBool(const QString &name);
      static QString getEnum(const QString &name);
      static int getInt(const QString &name);
      static QStringList getList(const QString &name);
      static QString getString(const QString &name);

      static void setBool(const QString &name, bool data);
      static void setList(const QString &name, const QStringList &data);

      enum DataSource { DEFAULT, PROJECT }; 

      struct struc_CfgBool {
         bool value;
         DataSource type;
      };

      struct struc_CfgInt {
         int value;
         DataSource type;
      };

      struct struc_CfgEnum {
         QString value;      
         DataSource type;
      };

      struct struc_CfgList {
         QStringList value;
         DataSource type;
      };

      struct struc_CfgString {
         QString value;
         DataSource type;
      };

   private:    
 
      static QHash<QString, struc_CfgBool>   m_cfgBool;
      static QHash<QString, struc_CfgInt>    m_cfgInt;  
      static QHash<QString, struc_CfgEnum>   m_cfgEnum;
      static QHash<QString, struc_CfgList>   m_cfgList; 
      static QHash<QString, struc_CfgString> m_cfgString;
    
      static QByteArray json_ReadFile(const QString &fName);

      static void load_Defaults();
      static bool read_ProjectFile(const QString &fName);
      static bool preVerify();
      static bool verify();

      static QStringList getAbbreviateBrief();
      static QStringList getFilePatterns();
};

#endif
