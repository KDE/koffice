/* This file is part of the KDE libraries
    Copyright (C) 1999 Ilya Baran (ibaran@mit.edu)

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

#include "MatrixDialog.h"
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <klocale.h>

KFORMULA_NAMESPACE_BEGIN

const int DEFAULT_SIZE = 3;
const int MAX_SIZE = 200;

MatrixDialog::MatrixDialog( QWidget *parent, int _width, int _height )
        : KDialogBase(parent, "Matrix Dialog", true,i18n("Add Matrix"),Ok|Cancel)
{
    w = _width;
    h = _height;

    QLabel *rows, *columns;
    QWidget *page = new QWidget( this );
    setMainWidget(page);
    Q3GridLayout *grid = new Q3GridLayout(page, 4, 2, 10);

    rows = new QLabel(i18n("Rows:"), page);
    columns = new QLabel(i18n("Columns:"), page);

    grid->addWidget(rows, 0, 0);
    grid->addWidget(columns, 0, 1);

    QSpinBox *width, *height;

    height = new QSpinBox(1, MAX_SIZE, 1, page);
    grid->addWidget(height, 1, 0);
    height->setValue(h);
    connect(height, SIGNAL(valueChanged(int)), SLOT(setHeight(int)));

    width = new QSpinBox(1, MAX_SIZE, 1, page);
    grid->addWidget(width, 1, 1);
    width->setValue(w);
    connect(width, SIGNAL(valueChanged(int)), SLOT(setWidth(int)));
    height->setFocus();
}

void MatrixDialog::setHeight(int value)
{
    h = value;
}

void MatrixDialog::setWidth(int value)
{
    w = value;
}

KFORMULA_NAMESPACE_END

using namespace KFormula;
#include "MatrixDialog.moc"
