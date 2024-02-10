/************************************************************************
*
* Copyright (c) 2014-2024 Barbara Geller & Ansel Sermersheim
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

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

#include <resourcemgr.h>

#include <config.h>
#include <doxy_build_info.h>
#include <message.h>
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
      ResourceMgr::Type type =  ResourceMgr::Verbatim;

      if (fName.endsWith(".lum")) {
         type =  ResourceMgr::Luminance;

      } else if (fName.endsWith(".luma")) {
         type =  ResourceMgr::LumAlpha;

      } else if (fName.endsWith(".css")) {
         type =  ResourceMgr::CSS;

      }

      switch (type) {

         case ResourceMgr::Verbatim: {
            QFile f(outputName);

            if (f.open(QIODevice::WriteOnly))  {

               if (f.write(resData) == resData.size() ) {
                  return true;
               }
            }
         }

         break;

         case ResourceMgr::Luminance: {
            // replace .lum with .png

            // convert file, throw out any line starting with #
            QRegularExpression regExp_comment("#.*?\r?\n", QPatternOption::MultilineOption);

            QString data = QString::fromUtf8(resData);
            data.replace(regExp_comment, "");

            QRegularExpression regExp_blanks("\\s+", QPatternOption::MultilineOption);
            QStringList dataList = data.split(regExp_blanks);

            int width  = dataList[0].toInteger<int>();
            int height = dataList[1].toInteger<int>();

            resData.clear();

            for (int k = 2; k < dataList.size(); ++k) {
               resData.append(dataList[k].toInteger<int>());
            }

            //
            const uchar *p = (const uchar *)resData.constData();

            ColoredImgDataItem images;
            images.path    = targetDir;
            images.name    = targetName;
            images.width   = width;
            images.height  = height;
            images.content = p;
            images.alpha   = nullptr;

            writeColoredImgData(images);
            return true;
         }
         break;

         case ResourceMgr::LumAlpha: {
            // convert .luma data to outputName.png

            // convert file, throw out any line starting with #
            QRegularExpression regExp_comment("#.*?\r?\n", QPatternOption::MultilineOption);

            QString data = QString::fromUtf8(resData);
            data.replace(regExp_comment, "");

            QRegularExpression regExp_blanks("\\s+", QPatternOption::MultilineOption);
            QStringList dataList = data.split(regExp_blanks);

            int width  = dataList[0].toInteger<int>();
            int height = dataList[1].toInteger<int>();

            resData.clear();

            for (int k = 2; k < dataList.size(); ++k) {
               resData.append(dataList[k].toInteger<int>());
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

         case ResourceMgr::CSS: {
            QFile f(outputName);

            if (f.open(QIODevice::WriteOnly)) {

               QTextStream t(&f);
               QString data = replaceColorMarkers(resData);

               if (fName.endsWith("navtree.css")) {
                  QString temp = QString::number(Config::getInt("treeview-width")) + "px";
                  t << substitute(data, "$width", temp);

               } else {
                  t << substitute(data, "$doxypressversion", versionString);
                  t << substitute(data, "$doxygenversion",   versionString);        // compatibility

               }

               return true;
            }
         }
         break;
      }

   } else {
      err("Requested resource '%s' is unknown\n", csPrintable(fName));

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
