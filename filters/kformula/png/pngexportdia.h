/* This file is part of the KDE project
   Copyright (C) 2002 Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef PNGEXPORTDIA_H
#define PNGEXPORTDIA_H

class QCheckBox;
class QString;
class KIntNumInput;
class KDoubleNumInput;

namespace KFormula {
    class Container;
    class Document;
}

#include <qdom.h>

#include <kdialogbase.h>


class PNGExportDia : public KDialogBase
{
    Q_OBJECT

public:
    PNGExportDia( const QDomDocument &dom, const QString &outFile, QWidget *parent=0L, const char *name=0L );
    ~PNGExportDia();

public slots:
    void slotOk();

protected slots:

    void widthChanged( int  );
    void heightChanged( int );
    void percentWidthChanged( double );
    void percentHeightChanged( double );

    void proportionalClicked();

private:

    void connectAll();
    void disconnectAll();

    void setupGUI();

    int realWidth;
    int realHeight;

    QString _fileOut;
    QByteArray _arrayOut;

    KFormula::Document* doc;
    KFormula::Container* formula;

    QCheckBox* proportional;
    KIntNumInput* widthEdit;
    KIntNumInput* heightEdit;
    KDoubleNumInput* percWidthEdit;
    KDoubleNumInput* percHeightEdit;
};

#endif // PNGEXPORTDIA_H
