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
#include <kapp.h>

#include <ExportDialogUI.h>
#include <ExportDialog.h>

HtmlExportDialog :: HtmlExportDialog(QWidget* parent)
    : KDialogBase(parent, 0, true, QString::null, Ok|Cancel, No, true),
      m_dialog(new ExportDialogUI(this))
{

    kapp->restoreOverrideCursor();
    
    m_dialog->comboBoxEncoding->insertStringList(KGlobal::charsets()->availableEncodingNames());
    //m_dialog->comboBoxEncoding->insertStringList(KGlobal::charsets()->descriptiveEncodingNames());

    resize(size()); // Is this right?

    setMainWidget(m_dialog);

    connect(m_dialog->comboBoxEncoding, SIGNAL(activated(int)), this,
        SLOT(comboBoxEncodingActivated(int)));
}

HtmlExportDialog :: ~HtmlExportDialog(void)
{
    kapp->setOverrideCursor(Qt::waitCursor);
}

bool HtmlExportDialog::isXHtml(void) const
{
    if(m_dialog->radioDocType1==m_dialog->buttonGroup1->selected())
        return false;
    else if(m_dialog->radioDocType2==m_dialog->buttonGroup1->selected())
        return true;
    return true;
}

QTextCodec* HtmlExportDialog::getCodec(void) const
{
    QTextCodec* codec=NULL;

    if (m_dialog->radioEncodingUTF8==m_dialog->buttonGroupEncoding->selected())
    {
        kdDebug(30503) << "Encoding: UTF-8" << endl;
        codec=QTextCodec::codecForName("UTF-8");
    }
    else if (m_dialog->radioEncodingLocal==m_dialog->buttonGroupEncoding->selected())
    {
        kdDebug(30503) << "Encoding: Locale" << endl;
        codec=QTextCodec::codecForLocale();
    }
    else if (m_dialog->radioEncodingOther==m_dialog->buttonGroupEncoding->selected())
    {
        QString strCodec=m_dialog->comboBoxEncoding->currentText();
        kdDebug(30503) << "Encoding: " << strCodec << endl;
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
        kdWarning(30503) << "No codec set, assuming UTF-8" << endl;
        codec=QTextCodec::codecForName("UTF-8");
    }

    return codec;
}

void HtmlExportDialog::comboBoxEncodingActivated(int)
{
    m_dialog->buttonGroupEncoding->setButton(2); // Select the "other" button
}

int HtmlExportDialog::getMode(void) const
{
    if (m_dialog->radioModeCss==m_dialog->buttonGroupMode->selected())
    {
        return 0;
    }
    else if (m_dialog->radioModeDocStruct==m_dialog->buttonGroupMode->selected())
    {
        return 10;
    }

    return 0;
}


#include <ExportDialog.moc>
