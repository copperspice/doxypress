/************************************************************************
*
* Copyright (c) 2014-2021 Barbara Geller & Ansel Sermersheim
* Copyright (c) 1997-2014 Dimitri van Heesch
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

#ifndef CONFIG_H
#define CONFIG_H

#include <QByteArray>
#include <QDir>
#include <QHash>
#include <QString>
#include <QStringList>

class Config
{
   public:
      Config() = delete;

      static void load_Defaults();
      static bool parseConfig(const QString &fName);
      static QDir getConfigDir();
      static QString getFullName(const QString &fName);

      static bool getBool(const QString &name);
      static QString getEnum(const QString &name);
      static int getInt(const QString &name);
      static QStringList getList(const QString &name);
      static QString getString(const QString &name);

      static void setBool(const QString &name, bool data);
      static void setList(const QString &name, const QStringList &data);

      static Qt::CaseSensitivity getCase(const QString &name);

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

         struc_CfgEnum(const char *data, DataSource src) {
            value = QString::fromLatin1(data);
            type  = src;
         }

         struc_CfgEnum(QString data, DataSource src) {
            value = std::move(data);
            type  = src;
         }

         QString value;
         DataSource type;
      };

      struct struc_CfgList {
         QStringList value;
         DataSource type;
      };

      struct struc_CfgString {

         struc_CfgString(const char *data, DataSource src) {
            value = QString::fromLatin1(data);
            type  = src;
         }

         struc_CfgString(QString data, DataSource src) {
            value = std::move(data);
            type  = src;
         }

         QString value;
         DataSource type;
      };

   private:
      static QHash<QString, struc_CfgBool>   m_cfgBool;
      static QHash<QString, struc_CfgInt>    m_cfgInt;
      static QHash<QString, struc_CfgEnum>   m_cfgEnum;
      static QHash<QString, struc_CfgList>   m_cfgList;
      static QHash<QString, struc_CfgString> m_cfgString;

      static QDir m_configDir;

      static QByteArray json_ReadFile(const QString &fName);

      static bool read_ProjectFile(const QString &fName);
      static void msgVerify();
      static bool preVerify();
      static bool verify();

      static QStringList getAbbreviateBrief();
      static QStringList getFilePatterns();
      static QStringList getSuffixSource();
      static QStringList getSuffixHeader();
      static QStringList getSuffixExclude();
      static QStringList getDotImageFormat();
      static QStringList getMathJaxFormat();
      static QStringList getLatexPaperType();
      static QStringList getRtfPaperType();

      static void cleanUpPaths(QStringList &str);

      static void loadCmd_Aliases();
      static void escapeAliases();
      static void expandAliases();

      static void loadRenameNS_Aliases();
};

#endif
