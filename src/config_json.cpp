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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <config_json.h>

QByteArray Config_Json::json_ReadFile(QString fName)
{        
   QByteArray data;

   QFile file(fName);

   if (! file.open(QFile::ReadWrite | QFile::Text)) {      
      return data;
   }

   file.seek(0);
   data = file.readAll();
   file.close();

   return data;
}

void Config_Json::parseConfig(QString fName)
{  
   // get existing json data
   QByteArray data = Config_Json::json_ReadFile(fName);

   //
   QJsonDocument doc  = QJsonDocument::fromJson(data);
   QJsonObject object = doc.object();
   
   for (auto iter = object.begin(); iter != object.end(); ++iter) {
      // walk the config file

/*

      X->project_name->setText( object.value("project-name").toString());
      X->project_brief->setText( object.value("project-brief").toString());
      X->project_number->setText( object.value("project-number").toString());


      if (iter.value().isString()) {
         FOO.insert( iter.key(), iter.value().toString());         
      } 

      if (iter.value().isBool()) {
         FOO.insert( iter.key(), iter.value().toBool());         
      } 
*/


   }
}