/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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
#include <kdialogbase.h>
#include <klocale.h>

#include <qlayout.h>
#include <qvbox.h>
#include <kbuttonbox.h>

#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "kwchangecasedia.h"
#include "kwview.h"
#include "kwtextframeset.h"


KWChangeCaseDia::KWChangeCaseDia( QWidget *parent, const char *name )
    : KDialogBase( parent, name , true, "", Ok|Cancel, Ok, true )
{

    setCaption( i18n("Change case") );
    QVBox *page = makeVBoxMainWidget();

    QButtonGroup *grp = new QButtonGroup( 1, QGroupBox::Horizontal, i18n( "Case:" ),page );
    grp->setRadioButtonExclusive( TRUE );
    grp->layout();
    m_upperCase=new QRadioButton( i18n("&Uppercase"), grp );
    m_lowerCase=new QRadioButton( i18n("&Lowercase"), grp );

    m_titleCase=new QRadioButton( i18n("T&itle Case"), grp );

    m_toggleCase=new QRadioButton( i18n("&Toggle Case"), grp );

    m_upperCase->setChecked(true);
    grp->setRadioButtonExclusive( TRUE );

}

int KWChangeCaseDia::getTypeOfCase()
{
    KWTextFrameSet::TypeOfCase type=KWTextFrameSet::UpperCase;
    if( m_upperCase->isChecked())
        type=KWTextFrameSet::UpperCase;
    else if(m_lowerCase->isChecked())
        type=KWTextFrameSet::LowerCase;
    else if(m_titleCase->isChecked())
        type=KWTextFrameSet::TitleCase;
    else if( m_toggleCase->isChecked())
        type=KWTextFrameSet::ToggleCase;
    return type;
}

#include "kwchangecasedia.moc"
