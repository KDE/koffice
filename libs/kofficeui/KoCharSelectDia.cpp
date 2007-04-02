/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "KoCharSelectDia.h"
#include "KoCharSelectDia.moc"

#include <QLayout>
#include <QGridLayout>

#include <klocale.h>
#include <kcharselect.h>
#include <kdebug.h>
#include <KStandardGuiItem>

/******************************************************************/
/* class KoCharSelectDia                                           */
/******************************************************************/

KoCharSelectDia::KoCharSelectDia( QWidget *parent, const char *name, const QChar &_chr, const QString &_font, bool _enableFont , bool _modal)
    : KDialog( parent )
{
    setCaption( i18n("Select Character") );
    setModal( _modal );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setObjectName( name );

    initDialog(_chr,_font,_enableFont);

    KGuiItem okItem = KStandardGuiItem::ok(); // start from std item to keep the OK icon...
    okItem.setText( i18n("&Insert") );
    okItem.setWhatsThis( i18n("Insert the selected character in the text") );
    setButtonGuiItem( KDialog::Ok, okItem );
}

KoCharSelectDia::KoCharSelectDia( QWidget *parent, const char *name, const QString &_font, const QChar &_chr, bool _modal )
    : KDialog( parent )
{
    setCaption( i18n("Select Character") );
    setModal( _modal );
    setButtons( User1 | Close );
    setDefaultButton( User1 );
    setObjectName( name );

    initDialog(_chr,_font,true);

    setButtonText( User1, i18n("&Insert") );
    setButtonToolTip( User1, i18n("Insert the selected character in the text") );
    connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
}

void KoCharSelectDia::initDialog(const QChar &_chr, const QString &_font, bool /*_enableFont*/)
{
    QWidget *page = mainWidget()/*plainPage()*/;

    grid = new QGridLayout( page );
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    charSelect = new KCharSelect( page );
    charSelect->setCurrentChar( _chr );
    charSelect->setCurrentFont( QFont(_font) );
    connect(charSelect, SIGNAL(charSelected()),this, SLOT(slotDoubleClicked()));
    charSelect->resize( charSelect->sizeHint() );
//     charSelect->enableFontCombo( true );
    grid->addWidget( charSelect, 0, 0 );

    grid->addItem( new QSpacerItem( charSelect->width(), 0 ), 0, 0 );
    grid->addItem( new QSpacerItem( 0, charSelect->height() ), 0, 0 );
    grid->setRowStretch( 0, 0 );
    charSelect->setFocus();
}

KoCharSelectDia::~KoCharSelectDia()
{
}

void KoCharSelectDia::closeDialog()
{
    KDialog::close();
}

bool KoCharSelectDia::selectChar( QString &_font, QChar &_chr, bool _enableFont, QWidget* parent, const char* name )
{
    bool res = false;

    KoCharSelectDia *dlg = new KoCharSelectDia( parent, name, _chr, _font, _enableFont );
    dlg->setFocus();
    if ( dlg->exec() == Accepted )
    {
        _font = dlg->font();
        _chr = dlg->chr();
        res = true;
    }

    delete dlg;

    return res;
}

QChar KoCharSelectDia::chr() const
{
    return charSelect->currentChar();
}

QString KoCharSelectDia::font() const
{
    return charSelect->font().family();
}

void KoCharSelectDia::slotUser1()
{
    emit insertChar(chr(),font());
}

void KoCharSelectDia::slotDoubleClicked()
{
    emit insertChar(chr(),font());
}
