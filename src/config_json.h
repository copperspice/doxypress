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

#ifndef CONFIG_JSON_H
#define CONFIG_JSON_H

#include <QByteArray>
#include <QHash>
#include <QString>

class Config_Json
{

   public:
      QByteArray json_ReadFile(QString fName);
      void parseConfig();

   private:
       QHash<QString, QString> m_cfg_String;
       QHash<QString, bool>    m_cfg_Bool;

};

#endif