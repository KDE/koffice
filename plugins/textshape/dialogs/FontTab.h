/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __kofonttab_h__
#define __kofonttab_h__

#include <QFont>
#include <QWidget>

#include <kfontchooser.h>

class KoCharacterStyle;

class FontTab : public QWidget
{
    Q_OBJECT

public:
    explicit FontTab(bool uniqueFormat, QWidget* parent = 0);
    ~FontTab() {}

    void setDisplay(const KoCharacterStyle *displayStyle);
    void save(KoCharacterStyle *style) const;

signals:
    void fontChanged(const QFont &font);

private:
    KFontChooser *m_fontChooser;
    bool m_uniqueFormat;
};

#endif
