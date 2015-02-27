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
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

#include <qglobal.h>
#include <QFile>
#include <QRegExp>
#include <QTextStream>

#include <config.h>
#include <doxy_build_info.h>
#include <message.h>
#include <resourcemgr.h>
#include <util.h>

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

bool ResourceMgr::copyResourceAs(const QString &fName, const QString &targetDir, const QString &targetName) const
{
   QString outputName = targetDir + "/" + targetName;

   QByteArray resData = getAsString(fName);     
   
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
            // replace .lum with .png

            // convert file, throw out any line starting with #            
            QString data = resData;

            QRegExp comment("#.*\\n"); 
            comment.setMinimal(true);

            data.replace(comment, "");

            QRegExp blanks("\\s+"); 
            QStringList dataList = data.split(blanks);

            int width  = dataList[0].toInt();
            int height = dataList[1].toInt();     

            resData.clear();
            
            for (int k = 2; k < dataList.size(); ++k) {
               resData.append(dataList[k].toInt());
            }               

            //
            const uchar *p = (const uchar *)resData.constData();
           
            ColoredImgDataItem images;
            images.path    = targetDir;
            images.name    = targetName;
            images.width   = width;
            images.height  = height;
            images.content = p;
            images.alpha   = 0;
          
            writeColoredImgData(images);
            return true;
         }
         break;

         case Resource::LumAlpha: {
            // replace .luma with .png

            // convert file, throw out any line starting with #            
            QString data = resData;
            
            QRegExp comment("#.*\\n"); 
            comment.setMinimal(true);

            data.replace(comment, "");

            QRegExp blanks("\\s+"); 
            QStringList dataList = data.split(blanks);

            int width  = dataList[0].toInt();
            int height = dataList[1].toInt();         

            resData.clear();
            
            for (int k = 2; k < dataList.size(); ++k) {
               resData.append(dataList[k].toInt());
            }           
            
            //       
            const uchar *p = (const uchar *)resData.constData();
          
            ColoredImgDataItem images;
            images.path    = targetDir;
            images.name    = targetName;
            images.width   = width;
            images.height  = height;
            images.content = p;
            images.alpha   = p + (width * height);
        
            writeColoredImgData(images);
            return true;
         }
         break;

         case Resource::CSS: {
            QFile f(outputName);

            if (f.open(QIODevice::WriteOnly)) {
              
               QTextStream t(&f);
               resData = replaceColorMarkers(resData);

               if (fName == "navtree.css") {

                  QByteArray temp = QByteArray().setNum(Config::getInt("treeview-width")) + "px";
                  t << substitute(resData, "$width", temp);         

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

QByteArray ResourceMgr::getAsString(const QString &fName) const
{
   QByteArray retval;

   QString resourceFileName = ":/resources/" + fName; 
   QFile resource(resourceFileName);

   if (resource.open(QIODevice::ReadOnly)) {
      retval = resource.readAll();            
   }
              
   return retval;
}

