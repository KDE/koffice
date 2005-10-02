/*
   This file is part of the KDE project
   Copyright 2001, 2003, 2004 Nicolas GOUTTE <goutte@kde.org>

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

#include <qtextcodec.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcharsets.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kmessagebox.h>

#include <ExportDialogUI.h>
#include <ExportDialog.h>

AsciiExportDialog :: AsciiExportDialog(QWidget* parent)
    : KDialogBase(parent, 0, true, i18n("KWord's Plain Text Export Filter"), Ok|Cancel, No, true),
      m_dialog(new ExportDialogUI(this))
{

    kapp->restoreOverrideCursor();

    QStringList encodings;
    encodings << i18n( "Descriptive encoding name", "Recommended ( %1 )" ).arg( "UTF-8" );
    encodings << i18n( "Descriptive encoding name", "Locale ( %1 )" ).arg( QTextCodec::codecForLocale()->name() );
    encodings += KGlobal::charsets()->descriptiveEncodingNames();
    // Add a few non-standard encodings, which might be useful for text files
    const QString description(i18n("Descriptive encoding name","Other ( %1 )"));
    encodings << description.arg("Apple Roman"); // Apple 
    encodings << description.arg("IBM 850") << description.arg("IBM 866"); // MS DOS
    encodings << description.arg("CP 1258"); // Windows

    m_dialog->comboBoxEncoding->insertStringList(encodings);

    setMainWidget(m_dialog);

}

AsciiExportDialog :: ~AsciiExportDialog(void)
{
    kapp->setOverrideCursor(Qt::waitCursor);
}

QTextCodec* AsciiExportDialog::getCodec(void) const
{
    const QString strCodec( KGlobal::charsets()->encodingForName( m_dialog->comboBoxEncoding->currentText() ) );
    kdDebug(30502) << "Encoding: " << strCodec << endl;

    bool ok = false;
    QTextCodec* codec = QTextCodec::codecForName( strCodec.utf8() );

    // If QTextCodec has not found a valid encoding, so try with KCharsets.
    if ( codec )
    {
        ok = true;
    }
    else
    {
        codec = KGlobal::charsets()->codecForName( strCodec, ok );
    }

    // Still nothing?
    if ( !codec || !ok )
    {
        // Default: UTF-8
        kdWarning(30502) << "Cannot find encoding:" << strCodec << endl;
        // ### TODO: what parent to use?
        KMessageBox::error( 0, i18n("Cannot find encoding: %1").arg( strCodec ) );
        return 0;
    }

    return codec;
}

QString AsciiExportDialog::getEndOfLine(void) const
{
    QString strReturn;
    if (m_dialog->radioEndOfLineLF==m_dialog->buttonGroupEndOfLine->selected())
        strReturn="\n";
    else if (m_dialog->radioEndOfLineCRLF==m_dialog->buttonGroupEndOfLine->selected())
        strReturn="\r\n";
    else if (m_dialog->radioEndOfLineCR==m_dialog->buttonGroupEndOfLine->selected())
        strReturn="\r";
    else
        strReturn="\n";

    return strReturn;
}

#include <ExportDialog.moc>
