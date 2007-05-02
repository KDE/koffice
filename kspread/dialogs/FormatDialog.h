/* This file is part of the KDE project
   Copyright (C) 2002-2003 Norbert Andres <nandres@web.de>
             (C) 2002 Ariya Hidayat <ariya@kde.org>
             (C) 2002 John Dailey <dailey@vt.edu>
             (C) 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
             (C) 2000-2002 Laurent Montel <montel@kde.org>
             (C) 2000 Werner Trobin <trobin@kde.org>
             (C) 1998-2000 Torben Weis <weis@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KSPREAD_DLG_FORMAT
#define KSPREAD_DLG_FORMAT

#include <kdialog.h>
#include <QString>
#include <QList>
#include <QColor>
#include <QPen>
#include <QFont>
//Added by qt3to4:
#include <QLabel>
#include <KoXmlReader.h>

class QComboBox;
class QLabel;
class QDomDocument;

namespace KSpread
{
class Sheet;
class Style;
class View;

/**
 * Dialog for the "AutoFormat..." action
 */
class FormatDialog : public KDialog
{
    Q_OBJECT
public:
  explicit FormatDialog( View* view, const char* name = 0 );
  ~FormatDialog();

private slots:
    void slotActivated( int index );
    void slotOk();

private:
    bool parseXML( const KoXmlDocument& doc );

    QComboBox* m_combo;
    QLabel* m_label;
    View* m_view;

    struct Entry
    {
	QString xml;
	QString image;
	QString config;
	QString name;
    };
    QList<Entry> m_entries;

    QList<Style*> m_styles;
};

} // namespace KSpread

#endif
