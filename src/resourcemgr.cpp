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
#include <dox_build_info.h>
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

bool ResourceMgr::copyResourceAs(const char *name, const char *targetDir, const char *targetName) const
{
   QByteArray pathName = QByteArray(targetDir) + "/" + targetName;
   const Resource *res = m_resources.value(name);

   if (res) {
      switch (res->type) {

         case Resource::Verbatim: {
            QFile f(pathName);

            if (f.open(QIODevice::WriteOnly) && f.write((const char *)res->data, res->size) == res->size) {
               return true;
            }
         }
         break;

         case Resource::Luminance: {
            // replace .lum by .png

            QByteArray n = name;

            n = n.left(n.length() - 4) + ".png"; 

            uchar *p = (uchar *)res->data;
            int width   = (p[0] << 8) + p[1];
            int height  = (p[2] << 8) + p[3];

            ColoredImgDataItem images[2];
            images[0].name    = n;
            images[0].width   = width;
            images[0].height  = height;
            images[0].content = &p[4];
            images[0].alpha   = 0;
            images[1].name    = 0; // terminator

            writeColoredImgData(targetDir, images);
            return true;
         }
         break;

         case Resource::LumAlpha: {
            // replace .luma by .png

            QByteArray n = name;
            n = n.left(n.length() - 5) + ".png"; 

            uchar *p = (uchar *)res->data;
            int width   = (p[0] << 8) + p[1];
            int height  = (p[2] << 8) + p[3];

            ColoredImgDataItem images[2];
            images[0].name    = n;
            images[0].width   = width;
            images[0].height  = height;
            images[0].content = &p[4];
            images[0].alpha   = &p[4 + width * height];
            images[1].name    = 0; // terminator

            writeColoredImgData(targetDir, images);
            return true;
         }
         break;

         case Resource::CSS: {
            QFile f(pathName);

            if (f.open(QIODevice::WriteOnly)) {
               QByteArray buffer(res->data, res->size); 
               
               FTextStream t(&f);
               buffer = replaceColorMarkers(buffer);

               if (qstrcmp(name, "navtree.css") == 0) {
                  t << substitute(buffer, "$width", QByteArray().setNum(Config_getInt("TREEVIEW_WIDTH")) + "px");

               } else {
                  t << substitute(buffer, "$doxygenversion", versionString);
               }

               return true;
            }
         }
         break;
      }

   } else {
      err("requested resource '%s' not compiled in!\n", name);

   }

   return false;
}

bool ResourceMgr::copyResource(const char *name, const char *targetDir) const
{
   return copyResourceAs(name, targetDir, name);
}

const Resource *ResourceMgr::get(const char *name) const
{ 
   return m_resources.value(name);
}

QByteArray ResourceMgr::getAsString(const char *name) const
{
   const Resource *strucRes = m_resources.value(name);

   QByteArray retval;

   if (strucRes) {      
      retval = QByteArray(strucRes->data, strucRes->size);      
   }

   return retval;
}


