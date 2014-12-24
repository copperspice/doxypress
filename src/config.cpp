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

//   #include <QDir>
//   #include <QFileDialog>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <config.h>

QByteArray Config::json_ReadFile(QString fName)
{        
   QByteArray data;

   QFile file(fName);

   if (! file.open(QFile::ReadWrite | QFile::Text)) {
      //const QString msg = tr("Unable to open Configuration File: ") +  m_jsonFname + " : " + file.errorString();
      //csError(tr("Read Json"), msg);
      return data;
   }

   file.seek(0);
   data = file.readAll();
   file.close();

   return data;
}

void Config::parseConfig()
{
   // temp test code (broom)
   QString m_jsonFname = "cfg_test.json";

   // get existing json data
   QByteArray data = json_ReadFile(m_jsonFname);

   //
   QJsonDocument doc  = QJsonDocument::fromJson(data);
   QJsonObject object = doc.object();
   
   for (auto iter = object.begin(); iter != object.end(); ++iter) {

      if (iter.value().isString()) {
         m_cfg_String.insert( iter.key(), iter.value().toString());         
      } 

      if (iter.value().isBool()) {
         m_cfg_Bool.insert( iter.key(), iter.value().toBool());         
      } 

   }
}