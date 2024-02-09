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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <stdlib.h>

#include <formula.h>
#include <config.h>
#include <doxy_globals.h>
#include <image.h>
#include <index.h>
#include <message.h>
#include <portable.h>
#include <util.h>

Formula::Formula(const QString &text)
{
   static int count = 0;

   number = count++;
   form   = text;
}

int Formula::getId()
{
   return number;
}

void FormulaList::generateBitmaps(const QString &path)
{
   int x1;
   int y1;
   int x2;
   int y2;

   // store the original directory
   QDir d(path);

   if (! d.exists()) {
      err("Output dir %s does not exist\n", csPrintable(path));
      Doxy_Work::stopDoxyPress();
   }

   QString oldDir = QDir::currentPath();

   // move to the image directory
   QDir::setCurrent(d.absolutePath());
   QDir thisDir = QDir::current();

   // generate a latex file containing one formula per page
   QString texName = "_formulas.tex";
   QList<int> pagesToGenerate;

   QFile f(texName);
   bool formulaError = false;

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);

      if (Config::getBool("latex-batch-mode")) {
         t << "\\batchmode" << endl;
      }

      t << "\\documentclass{article}\n";
      t << "\\usepackage{ifthen}\n";
      t << "\\usepackage{epsfig}\n";             // include images
      t << "\\usepackage[utf8]{inputenc}\n";     // some older distributions with newunicode package 1.1 need this option

      writeExtraLatexPackages(t);
      writeLatexSpecialFormulaChars(t);

      t << "\\pagestyle{empty}\n";
      t << "\\begin{document}\n";

      int page = 0;

      for (auto &formula : *this) {
         QString resultName = QString("form_%1.png").formatArg(formula.getId());

         // only formulas for which no image exists are generated
         QFileInfo fi(resultName);

         if (! fi.exists()) {
            // we force a pagebreak after each formula
            t << formula.getFormulaText() << endl << "\\pagebreak\n\n";
            pagesToGenerate.append(page);
         }

         Doxy_Globals::indexList.addImageFile(resultName);
         page++;
      }

      t << "\\end{document}" << endl;
      f.close();
   }

   if (pagesToGenerate.count() > 0) {
      // new formulas
      QString latexCmd = Config::getString("latex-cmd-name");

      if (latexCmd.isEmpty()) {
         latexCmd = "latex";
      }
      portable_sysTimerStart();

      if (portable_system(latexCmd, "_formulas.tex") != 0) {
         err("Unable to run LaTeX, verify your installation, _formulas.tex, and _formulas.log\n");
         formulaError = true;
      }

      portable_sysTimerStop();

      int pageIndex = 1;

      for (auto pageNum : pagesToGenerate) {
         msg("Generating image form_%d.png for formula\n", pageNum);

         QString formBase;
         formBase = QString("_form%1").formatArg(pageNum);

         // run dvips to convert the page with number pageIndex to an encapsulated postscript

         char dviArgs[4096];
         sprintf(dviArgs, "-q -D 600 -E -n 1 -p %d -o %s.eps _formulas.dvi",pageIndex, csPrintable(formBase));

         portable_sysTimerStart();

         if (portable_system("dvips", dviArgs) != 0) {
            err("Unable to run dvips, check your installation\n");
            portable_sysTimerStop();

            // reset the directory to the original location
            QDir::setCurrent(oldDir);

            return;
         }
         portable_sysTimerStop();

         // now we read the generated postscript file to extract the bounding box
         QFileInfo fi(formBase + ".eps");

         if (fi.exists()) {
            QString eps = fileToString(formBase + ".eps");
            int i = eps.indexOf("%%BoundingBox:");

            if (i != -1) {
               sscanf(eps.toUtf8().constData() + i, "%%%%BoundingBox:%d %d %d %d", &x1, &y1, &x2, &y2);

            } else {
               err("Unable to extract bounding box\n");
            }
         }

         // generate a postscript file which contains the eps
         // and displays it in the right colors and the right bounding box
         f.setFileName(formBase + ".ps");

         if (f.open(QIODevice::WriteOnly)) {
            QTextStream t(&f);

            t << "1 1 1 setrgbcolor" << endl;  // anti-alias to white background
            t << "newpath" << endl;
            t << "-1 -1 moveto" << endl;
            t << (x2 - x1 + 2) << " -1 lineto" << endl;
            t << (x2 - x1 + 2) << " " << (y2 - y1 + 2) << " lineto" << endl;
            t << "-1 " << (y2 - y1 + 2) << " lineto" << endl;
            t << "closepath" << endl;
            t << "fill" << endl;
            t << -x1 << " " << -y1 << " translate" << endl;
            t << "0 0 0 setrgbcolor" << endl;
            t << "(" << formBase << ".eps) run" << endl;
            f.close();
         }

         // scale the image so that it is four times larger than needed
         // and the sizes are a multiple of four
         double scaleFactor = 16.0 / 3.0;
         int zoomFactor = Config::getInt("formula-fontsize");

         if (zoomFactor < 8 || zoomFactor > 50) {
            zoomFactor = 10;
         }

         scaleFactor *= zoomFactor / 10.0;
         int gx = (((int)((x2 - x1) * scaleFactor)) + 3) & ~1;
         int gy = (((int)((y2 - y1) * scaleFactor)) + 3) & ~1;

         // run ghostscript to convert the postscript to a pixmap
         // The pixmap is a truecolor image, where only black and white are used

         char gsArgs[4096];
         sprintf(gsArgs, "-q -g%dx%d -r%dx%dx -sDEVICE=ppmraw "
                 "-sOutputFile=%s.pnm -dNOPAUSE -dBATCH -- %s.ps",
                 gx, gy, (int)(scaleFactor * 72), (int)(scaleFactor * 72), csPrintable(formBase), csPrintable(formBase) );

         portable_sysTimerStart();

         QString gsExe = Config::getString("ghostscript");

         if (portable_system(gsExe, gsArgs) != 0) {
            err("Unable to run GhostScript %s %s. Verify your installation\n", csPrintable(gsExe), gsArgs);
            portable_sysTimerStop();

            // reset the directory to the original location
            QDir::setCurrent(oldDir);

            return;
         }

         portable_sysTimerStop();
         f.setFileName(formBase + ".pnm");

         uint imageX = 0;
         uint imageY = 0;

         // read the generated image again, to obtain the pixel data
         if (f.open(QIODevice::ReadOnly)) {
            QTextStream t(&f);
            QString s;

            if (! t.atEnd()) {
               s = t.readLine();
            }

            if (s.length() < 2 || s.left(2) != "P6") {
               err("GhostScript produced an illegal image format");

            } else {
               // assume the size is after the first line that does not start with
               // # excluding the first line of the file.

               while (! t.atEnd()) {
                  s = t.readLine();

                  if (s.isEmpty()) {
                     break;
                  }

                  if (s.at(0) != '#') {
                     break;
                  }
               }

               sscanf(s.toUtf8().constData(), "%u %u", &imageX, &imageY);
            }

            if (imageX > 0 && imageY > 0) {
               // converting image

               // rgb 8:8:8 format
               std::vector<char> tmp(imageX * imageY * 3);

               char *data = &tmp[0];
               uint i, x, y, ix, iy;
               f.read(data, imageX * imageY * 3);

               Image srcImage(imageX, imageY);
               Image filteredImage(imageX, imageY);
               Image dstImage(imageX / 4, imageY / 4);

               uchar *ps = srcImage.getData();

               // convert image to black (1) and white (0) index.
               for (i = 0; i < imageX * imageY; i++) {
                  *ps = (data[i * 3] == 0 ? 1 : 0);
                  ++ps;
               }

               // apply a simple box filter to the image
               static int filterMask[] = {1, 2, 1, 2, 8, 2, 1, 2, 1};
               for (y = 0; y < srcImage.getHeight(); y++) {

                  for (x = 0; x < srcImage.getWidth(); x++) {
                     int color = 0;

                     for (iy = 0; iy < 2; iy++) {
                        for (ix = 0; ix < 2; ix++) {
                           color += srcImage.getPixel(x + ix - 1, y + iy - 1) * filterMask[iy * 3 + ix];
                        }
                     }

                     filteredImage.setPixel(x, y, color);
                  }
               }

               // down-sample the image to 1/16th of the area using 16 gray scale colors.

               for (y = 0; y < dstImage.getHeight(); y++) {
                  for (x = 0; x < dstImage.getWidth(); x++) {
                     int xp = x << 2;
                     int yp = y << 2;

                     int c = filteredImage.getPixel(xp + 0, yp + 0) +
                             filteredImage.getPixel(xp + 1, yp + 0) +
                             filteredImage.getPixel(xp + 2, yp + 0) +
                             filteredImage.getPixel(xp + 3, yp + 0) +
                             filteredImage.getPixel(xp + 0, yp + 1) +
                             filteredImage.getPixel(xp + 1, yp + 1) +
                             filteredImage.getPixel(xp + 2, yp + 1) +
                             filteredImage.getPixel(xp + 3, yp + 1) +
                             filteredImage.getPixel(xp + 0, yp + 2) +
                             filteredImage.getPixel(xp + 1, yp + 2) +
                             filteredImage.getPixel(xp + 2, yp + 2) +
                             filteredImage.getPixel(xp + 3, yp + 2) +
                             filteredImage.getPixel(xp + 0, yp + 3) +
                             filteredImage.getPixel(xp + 1, yp + 3) +
                             filteredImage.getPixel(xp + 2, yp + 3) +
                             filteredImage.getPixel(xp + 3, yp + 3);

                     // here we scale and clip the color value so the
                     // resulting image has a reasonable contrast
                     dstImage.setPixel(x, y, qMin(15, (c * 15) / (16 * 10)));
                  }
               }

               // save the result as a bitmap
               QString fileName = QString("form_%1.png").formatArg(pageNum);

               QFile pngFile(fileName);

               if (pngFile.open(QIODevice::WriteOnly)) {

                  // parameter 1 is used as a temporary to select the right color palette
                  QByteArray buffer = dstImage.convert(1);

                  pngFile.write(buffer);
                  pngFile.close();

               } else {
                  err("Unable to open file %s for writing, OS Error #: %d\n", csPrintable(fileName), pngFile.error());

               }
            }

            f.close();
         }

         // remove intermediate image files
         thisDir.remove(formBase + ".eps");
         thisDir.remove(formBase + ".pnm");
         thisDir.remove(formBase + ".ps");

         ++pageIndex;
      }

      // remove intermediate files produced by latex
      thisDir.remove("_formulas.dvi");

      if (! formulaError) {
         thisDir.remove("_formulas.log");   // keep file in case of errors
      }
      thisDir.remove("_formulas.aux");
   }

   // remove the latex file itself
   if (! formulaError) {
      thisDir.remove("_formulas.tex");
   }

   // write/update the formula repository so we know what text the
   // generated images represent (we use this next time to avoid regeneration
   // of the images, and to avoid forcing the user to delete all images in order
   // to let a browser refresh the images).

   f.setFileName("formula.repository");

   if (f.open(QIODevice::WriteOnly)) {
      QTextStream t(&f);

      for (auto &formula : *this) {
         t << "\\form#" << formula.getId() << ":" << formula.getFormulaText() << endl;
      }
      f.close();
   }

   // reset the directory to the original location
   QDir::setCurrent(oldDir);
}


#ifdef FORMULA_TEST
int main()
{
   FormulaList fl;
   fl.append(new Formula("$x^2$"));
   fl.append(new Formula("$y^2$"));
   fl.append(new Formula("$\\sqrt{x_0^2+x_1^2+x_2^2}$"));
   fl.generateBitmaps("dest");

   return 0;
}
#endif
