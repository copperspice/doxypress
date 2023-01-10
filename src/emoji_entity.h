/************************************************************************
*
* Copyright (c) 2014-2023 Barbara Geller & Ansel Sermersheim
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

#ifndef EMOJI_ENTITY_H
#define EMOJI_ENTITY_H

#include <QHash>
#include <QFile>
#include <QTextStream>

class EmojiEntityMapper
{
  public:
    static EmojiEntityMapper *instance();
    static void deleteInstance();

    QString name(int index) const;
    QString unicode(int index) const;
    void writeEmojiFile(QFile &file);
    int symbol2index(const QString &symName) const;

  private:
    EmojiEntityMapper();
   ~EmojiEntityMapper();

    static EmojiEntityMapper *s_instance;
    QHash<QString, int>       m_name2sym;
};

#endif
