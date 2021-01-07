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

#ifndef PLANTUML_H
#define PLANTUML_H

class QByteArray;

/** Plant UML output image formats */
enum PlantUMLOutputFormat { PUML_BITMAP, PUML_EPS, PUML_SVG };

/** Write a PlantUML compatible file.
 *  @param[in] outDir   the output directory to write the file to.
 *  @param[in] fileName the name of the file. If empty a name will be chosen automatically.
 *  @param[in] content  the contents of the PlantUML file.
 *  @returns The name of the generated file.
 */
QString writePlantUMLSource(const QString &outDir, const QString &fileName, const QString &content);

/** Convert a PlantUML file to an image.
 *  @param[in] baseName the name of the generated file (as returned by writePlantUMLSource())
 *  @param[in] outDir   the directory to write the resulting image into.
 *  @param[in] format   the image format to generate.
 */
void generatePlantUMLOutput(const QString &baseName, const QString &outDir, PlantUMLOutputFormat format);

#endif

