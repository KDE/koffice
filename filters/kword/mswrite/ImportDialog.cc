// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#include <qtextcodec.h>

#include <klocale.h>
#include <kcharsets.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kapplication.h>

#include <ImportDialogUI.h>
#include <ImportDialog.h>

MSWriteImportDialog :: MSWriteImportDialog(QWidget* parent)
    : KDialogBase(parent, 0, true,  i18n("KWord's MS Write Import Filter"), Ok|Cancel, No, true),
      m_dialog(new ImportDialogUI(this))
{
    kapp->restoreOverrideCursor();

   m_dialog->comboBoxEncoding->insertStringList(KGlobal::charsets()->availableEncodingNames());
    //m_dialog->comboBoxEncoding->insertStringList(KGlobal::charsets()->descriptiveEncodingNames());

    resize(size()); // Is this right?

    setMainWidget(m_dialog);

    connect(m_dialog->comboBoxEncoding, SIGNAL(activated(int)), this,
        SLOT(comboBoxEncodingActivated(int)));
}

MSWriteImportDialog :: ~MSWriteImportDialog(void)
{
    kapp->setOverrideCursor(Qt::waitCursor);
}

QTextCodec* MSWriteImportDialog::getCodec(void) const
{
    QTextCodec* codec=NULL;

    if (m_dialog->radioEncodingDefault==m_dialog->buttonGroupEncoding->selected())
    {
        kdDebug(30509) << "Encoding: CP 1252" << endl;
        codec=QTextCodec::codecForName("CP 1252");
    }
    /*else if (m_dialog->radioEncodingLocal==m_dialog->buttonGroupEncoding->selected())
    {
        kdDebug(30503) << "Encoding: Locale" << endl;
        codec=QTextCodec::codecForLocale();
    }*/
    else if (m_dialog->radioEncodingOther==m_dialog->buttonGroupEncoding->selected())
    {
        QString strCodec=m_dialog->comboBoxEncoding->currentText();
        kdDebug(30509) << "Encoding: " << strCodec << endl;
        if (strCodec.isEmpty())
        {
            codec=QTextCodec::codecForLocale();
        }
        else
        {
            // We do not use QTextCodec::codecForName here
            //   because we fear subtle problems
            codec=KGlobal::charsets()->codecForName(strCodec);
        }
    }

    if (!codec)
    {
        // Default: UTF-8
        kdWarning(30509) << "No codec set, assuming UTF-8" << endl;
        codec=QTextCodec::codecForName("UTF-8");
    }

    return codec;
}

bool MSWriteImportDialog::getSimulateLinespacing (void) const
{
	 return (m_dialog->checkBoxLinespacing->isChecked ());
}

bool MSWriteImportDialog::getSimulateImageOffset (void) const
{
	 return (m_dialog->checkBoxImageOffset->isChecked ());
}

void MSWriteImportDialog::comboBoxEncodingActivated(int)
{
	 m_dialog->buttonGroupEncoding->setButton(1); // Select the "Other Encoding" button
}


#include <ImportDialog.moc>
