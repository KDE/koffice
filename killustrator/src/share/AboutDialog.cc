/* -*- C++ -*-

  $Id$

  This file is part of KIllustrator.
  Copyright (C) 1998 Kai-Uwe Sattler (kus@iti.cs.uni-magdeburg.de)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Library General Public License as
  published by  
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU Library General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include "AboutDialog.h"
#include "AboutDialog.moc"

#include <qlabel.h>
#include <qpushbt.h>
#include <klocale.h>
#include <kapp.h>
#include "version.h"

#define ABOUT_INFO "Version "APP_VERSION"\n\nCopyright (C) 1998\n\
Kai-Uwe Sattler (kus@iti.cs-uni-magdeburg.de)\n\
Steffen Thorhauer (sth@iti.cs.uni-magdeburg.de)\n\n\
WMF support\n\
Copyright (C) 1998\
\nStefan Taferner (taferner@salzburg.co.at)"

AboutDialog::AboutDialog (QWidget* parent) : 
    QDialog (parent, "About", true) {
    QString info (ABOUT_INFO);

    setCaption (i18n ("About KIllustrator"));
    resize (400, 300);
    setFixedSize (size ());

    QLabel *label = new QLabel ("KIllustrator", this);
    label->setFont (QFont ("helvetica", 18, QFont::Bold));
    label->setGeometry (50, 30, 100, 25);

    label = new QLabel (info.data (), this);
    label->setAlignment (AlignLeft|WordBreak);
    label->setGeometry (50, 60, 290, 180);

    QFrame* frame = new QFrame (this);
    frame->setLineWidth (1);
    frame->setFrameStyle (QFrame::HLine|QFrame::Sunken);
    frame->setGeometry (5, 235, 390, 5);

    QPushButton* button = new QPushButton ("Ok", this);
    button->setGeometry (width () / 2 - 50, height () - 45, 75, 32);
    button->setDefault (true);
    connect (button, SIGNAL(released ()), this, SLOT(accept ()));
    button->setFocus ();
}
