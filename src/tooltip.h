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

#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QHash>
#include <QSharedPointer>
#include <QString>

class Definition;
class CodeGenerator;

class TooltipManager
{
 public:
   static TooltipManager *instance();
   void clearTooltips();
   void addTooltip(QSharedPointer<Definition> d);
   void writeTooltips(CodeGenerator &ol);

 private:
   QHash<QString, QSharedPointer<Definition>> m_tooltipInfo;

   TooltipManager();
   ~TooltipManager();

   static TooltipManager *s_theInstance;
};

#endif
