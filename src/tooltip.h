/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
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

class Definition;
class CodeOutputInterface;

class TooltipManager
{
 public:
   static TooltipManager *instance();
   void clearTooltips();
   void addTooltip(QSharedPointer<Definition> d);
   void writeTooltips(CodeOutputInterface &ol);

 private:
   QHash<QString, QSharedPointer<Definition>> m_tooltipInfo;

   TooltipManager();
   ~TooltipManager();

   static TooltipManager *s_theInstance;
};

#endif
