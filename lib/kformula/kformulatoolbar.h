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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef KFORMULATOOLBAR_H_INCLUDED
#define KFORMULATOOLBAR_H_INCLUDED

#include "kformulaedit.h"
#include <ktoolbar.h>

/**
 * Toolbar for formula editing functions.
 *
 * Just make one of these and call connectToFormula passing
 * your KFormulaEdit.
 * @author Ilya Baran <ibaran@mit.edu>
 */
class KFormulaToolBar : public KToolBar
{
  Q_OBJECT
public:
  KFormulaToolBar(QWidget *parent=0L, const char *name=0L, int _item_size = -1);

  void connectToFormula(KFormulaEdit *formula);
};


#endif //KFORMULATOOLBAR_H_INCLUDED








