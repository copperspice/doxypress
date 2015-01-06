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

#include <QHash>
#include <QFile>
#include <QByteArray>
#include <qglobal.h>

#include <string.h>

#include <resourcemgr.h>
#include <util.h>
#include <doxy_build_info.h>
#include <ftextstream.h>
#include <message.h>
#include <config.h>

ResourceMgr &ResourceMgr::instance()
{
   static ResourceMgr theInstance;
   return theInstance;
}

ResourceMgr::ResourceMgr()
{
}

ResourceMgr::~ResourceMgr()
{
}

void ResourceMgr::registerResources(const Resource resources[], int numResources)
{
   for (int i = 0; i < numResources; i++) {
      m_resources.insert(resources[i].name, &resources[i]);
   }
}

bool ResourceMgr::copyCategory(const char *categoryName, const char *targetDir) const
{  
   for (auto item : m_resources) {        
      if (qstrcmp(item->category, categoryName) == 0) {

         if (! copyResource(item->name, targetDir)) {
            return false;
         }
      }
   }

   return true;
}

bool ResourceMgr::copyResourceAs(const QString &fName, const QString &targetDir, const QString &targetName) const
{
   QByteArray resData;
 
   //
   QString resourceFileName = ":/resources/" + fName;

   QFile resource(resourceFileName);
   if ( resource.open(QIODevice::ReadOnly) ) {
      resData = resource.readAll();            
   }

   //
   QString outputName = targetDir + "/" + targetName;
 
   if (! resData.isEmpty()) {

      Resource::Type type =  Resource::Verbatim;

      if (fName.endsWith(".lum")) {
         type =  Resource::Luminance;

      } else if (fName.endsWith(".luma")) {
         type =  Resource::LumAlpha;

      } else if (fName.endsWith(".css")) {
         type =  Resource::CSS;

      }

      switch (type) {

         case Resource::Verbatim: {
            QFile f(outputName);

            if (f.open(QIODevice::WriteOnly))  {

               if (f.write(resData) == resData.size() ) {
                  return true;
               }
            }
         }

         break;

         case Resource::Luminance: {
            // replace .lum by .png

            QByteArray n = fName.toUtf8();
            n = n.left(n.length() - 4) + ".png"; 

            const uchar *p = (const uchar *)resData.constData();

            int width   = (p[0] << 8) + p[1];
            int height  = (p[2] << 8) + p[3];

            ColoredImgDataItem images[2];
            images[0].name    = n;
            images[0].width   = width;
            images[0].height  = height;
            images[0].content = &p[4];
            images[0].alpha   = 0;
            images[1].name    = 0; // terminator

            writeColoredImgData(qPrintable(targetDir), images);
            return true;
         }
         break;

         case Resource::LumAlpha: {
            // replace .luma by .png

            QByteArray n = fName.toUtf8();
            n = n.left(n.length() - 5) + ".png"; 

            const uchar *p = (const uchar *)resData.constData();

            int width   = (p[0] << 8) + p[1];
            int height  = (p[2] << 8) + p[3];

            ColoredImgDataItem images[2];
            images[0].name    = n;
            images[0].width   = width;
            images[0].height  = height;
            images[0].content = &p[4];
            images[0].alpha   = &p[4 + width * height];
            images[1].name    = 0; // terminator

            writeColoredImgData(qPrintable(targetDir), images);
            return true;
         }
         break;

         case Resource::CSS: {
            QFile f(outputName);

            if (f.open(QIODevice::WriteOnly)) {
              
               FTextStream t(&f);
               resData = replaceColorMarkers(resData);

               if (fName == "navtree.css") {
                  t << substitute(resData, "$width", QByteArray().setNum(Config_getInt("TREEVIEW_WIDTH")) + "px");

               } else {
                  t << substitute(resData, "$doxygenversion", versionString);
               }

               return true;
            }
         }
         break;
      }

   } else {
      err("Requested resource '%s' is unknown\n", qPrintable(fName));

   }

   return false;
}

bool ResourceMgr::copyResource(const QString &fName, const QString &targetDir) const
{
   return copyResourceAs(fName, targetDir, fName);
}

const Resource *ResourceMgr::get(const char *name) const
{ 
   return m_resources.value(name);
}

QByteArray ResourceMgr::getAsString(const QString &fName) const
{
   QString resourceFileName = ":/resources/" + fName;

   QByteArray retval;

   QFile resource(resourceFileName);
   if ( resource.open(QIODevice::ReadOnly) ) {
      retval = resource.readAll();            
   }
              
   return retval;
}


