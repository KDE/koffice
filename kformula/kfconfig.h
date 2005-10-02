/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef kfconfig_h
#define kfconfig_h

#include <kdialogbase.h>

#include <kformulaconfigpage.h>

class KFormulaPartView;


/**
 * The big config dialog.
 */
class KFConfig : public KDialogBase
{
    Q_OBJECT
public:
    KFConfig( KFormulaPartView* parent );

public slots:
    void slotApply();
    void slotDefault();

private:
    KFormula::ConfigurePage* _page;
//     KFormula::MathFontsConfigurePage* _mathFontPage;
};

#endif
