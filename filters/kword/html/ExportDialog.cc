// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kapp.h>
#include <klocale.h>

#include <qstring.h>

#include <ExportDialog.h>

HtmlExportDialog :: HtmlExportDialog(QWidget* parent)
    : KDialogBase(parent, 0, true, QString::null, Ok|Cancel, No, true),
      m_widget(parent,"HTML Export Dialog")
{

    kapp->restoreOverrideCursor();

    resize( 619, 371 );  // FIXME: provisory!

    setMainWidget(&m_widget);
    QBoxLayout *ml=new QVBoxLayout(&m_widget);
    box1=new QVButtonGroup(i18n("Document Type"),this);
    ml->addWidget(box1);
    QBoxLayout *bl1=new QVBoxLayout(box1);
    r1=new QRadioButton(i18n("HTML 4.01"), box1);
    bl1->addWidget(r1);
    r2=new QRadioButton(i18n("XHTML 1.0"), box1);
    bl1->addWidget(r2);
    box1->setExclusive(true);
    box1->setButton(1); // XHTML 1.0 is default.
    bl1->activate();
    //ml->addStretch(5);
    box2=new QVButtonGroup(i18n("Mode"),this);
    ml->addWidget(box2);
    QBoxLayout *bl2=new QVBoxLayout(box2);
    ra1=new QRadioButton(i18n("Spartan (Only document structure, no formatting!)"), box2);
    bl2->addWidget(ra1);
    ra2=new QRadioButton(i18n("Direct (Formatting coded using HTML, no style sheets)"), box2);
    bl2->addWidget(ra2);
    ra3=new QRadioButton(i18n("Style (Formatting coded using CSS2, no style sheets)"), box2);
    bl2->addWidget(ra3);
    box2->setExclusive(true);
    box2->setButton(1); // "Direct" mode is default.
    bl2->activate();
    ml->addStretch(5);
    ml->activate();

}

HtmlExportDialog :: ~HtmlExportDialog(void)
{
    kapp->setOverrideCursor(Qt::waitCursor);
}

QString HtmlExportDialog::getState(void)
{
    QString result;

    if(r1==box1->selected())
        result += "HTML";
    else if(r2==box1->selected())
        result += "XHTML";
    else
        result += "HTML";

    result += '-';

    if(ra1==box2->selected())
        result += "SPARTAN";
    else if(ra2==box2->selected())
        result += "TRANSITIONAL";
    else if(ra3==box2->selected())
        result += "STYLE";
    else
        result += "STYLE"; // TODO: best is CSS2 in fact!

    return result;
}

#include <ExportDialog.moc>