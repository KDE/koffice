/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 2004 Nicolas GOUTTE <goutte@kde.org>

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

#include <csvexportdialog.h>
#include <exportdialogui.h>

#include <kspread_map.h>
#include <kspread_sheet.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qradiobutton.h>
#include <qtextstream.h>
#include <qtabwidget.h>
#include <qtextcodec.h>
#include <qvalidator.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kcharsets.h>

using namespace KSpread;

CSVExportDialog::CSVExportDialog( QWidget * parent )
  : KDialogBase( parent, 0, true, QString::null, Ok | Cancel, No, true ),
    m_dialog( new ExportDialogUI( this ) ),
    m_delimiter( "," ),
    m_textquote('"')
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

  setButtonBoxOrientation ( Vertical );

  setMainWidget(m_dialog);

  // Invalid 'Other' delimiters
  // - Quotes
  // - CR,LF,Vetical-tab,Formfeed,ASCII bel
  QRegExp rx( "^[^\"'\r\n\v\f\a]{0,1}$" );
  m_delimiterValidator = new QRegExpValidator( rx, m_dialog->m_delimiterBox );
  m_dialog->m_delimiterEdit->setValidator( m_delimiterValidator );

  connect( m_dialog->m_delimiterBox, SIGNAL( clicked(int) ),
           this, SLOT( delimiterClicked( int ) ) );
  connect( m_dialog->m_delimiterEdit, SIGNAL( returnPressed() ),
           this, SLOT( returnPressed() ) );
  connect( m_dialog->m_delimiterEdit, SIGNAL( textChanged ( const QString & ) ),
           this, SLOT(textChanged ( const QString & ) ) );
  connect( m_dialog->m_comboQuote, SIGNAL( activated( const QString & ) ),
           this, SLOT( textquoteSelected( const QString & ) ) );
  connect( m_dialog->m_selectionOnly, SIGNAL( toggled( bool ) ),
           this, SLOT( selectionOnlyChanged( bool ) ) );

  loadSettings();
}

CSVExportDialog::~CSVExportDialog()
{
  saveSettings();
  kapp->setOverrideCursor(Qt::waitCursor);
  delete m_delimiterValidator;
}

void CSVExportDialog::loadSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("CSVDialog Settings");
    m_textquote = config->readEntry("textquote", "\"")[0];
    m_delimiter = config->readEntry("delimiter", ",");
    const QString codecText = config->readEntry("codec", "");
    bool selectionOnly = config->readBoolEntry("selectionOnly", false);
    const QString sheetDelim = config->readEntry("sheetDelimiter", m_dialog->m_sheetDelimiter->text());
    bool delimAbove = config->readBoolEntry("sheetDelimiterAbove", false);
    const QString eol = config->readEntry("eol", "\r\n");

    // update widgets
    if (!codecText.isEmpty()) {
      m_dialog->comboBoxEncoding->setCurrentText(codecText);
    }
    if (m_delimiter == ",") m_dialog->m_radioComma->setChecked(true);
    else if (m_delimiter == "\t") m_dialog->m_radioTab->setChecked(true);
    else if (m_delimiter == " ") m_dialog->m_radioSpace->setChecked(true);
    else if (m_delimiter == ";") m_dialog->m_radioSemicolon->setChecked(true);
    else {
        m_dialog->m_radioOther->setChecked(true);
        m_dialog->m_delimiterEdit->setText(m_delimiter);
    }
    m_dialog->m_comboQuote->setCurrentItem(m_textquote == '\'' ? 1
        : m_textquote == '"' ? 0 : 2);
    m_dialog->m_selectionOnly->setChecked(selectionOnly);
    m_dialog->m_sheetDelimiter->setText(sheetDelim);
    m_dialog->m_delimiterAboveAll->setChecked(delimAbove);
    if (eol == "\r\n") m_dialog->radioEndOfLineCRLF->setChecked(true);
    else if (eol == "\r") m_dialog->radioEndOfLineCR->setChecked(true);
    else m_dialog->radioEndOfLineLF->setChecked(true);
}

void CSVExportDialog::saveSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("CSVDialog Settings");
    QString q = m_textquote;
    config->writeEntry("textquote", q);
    config->writeEntry("delimiter", m_delimiter);
    config->writeEntry("codec", m_dialog->comboBoxEncoding->currentText());
    config->writeEntry("selectionOnly", exportSelectionOnly());
    config->writeEntry("sheetDelimiter", getSheetDelimiter());
    config->writeEntry("sheetDelimiterAbove", printAlwaysSheetDelimiter());
    config->writeEntry("eol", getEndOfLine());
    config->sync();
}

void CSVExportDialog::fillSheet( Map * map )
{
  m_dialog->m_sheetList->clear();
  QCheckListItem * item;

  QPtrListIterator<Sheet> it( map->sheetList() );
  for( ; it.current(); ++it )
  {
    item = new QCheckListItem( m_dialog->m_sheetList,
                               it.current()->sheetName(),
                               QCheckListItem::CheckBox );
    item->setOn(true);
    m_dialog->m_sheetList->insertItem( item );
  }

  m_dialog->m_sheetList->setSorting(0, true);
  m_dialog->m_sheetList->sort();
  m_dialog->m_sheetList->setSorting( -1 );
}

QChar CSVExportDialog::getDelimiter() const
{
  return m_delimiter[0];
}

QChar CSVExportDialog::getTextQuote() const
{
  return m_textquote;
}

bool CSVExportDialog::printAlwaysSheetDelimiter() const
{
  return m_dialog->m_delimiterAboveAll->isChecked();
}

QString CSVExportDialog::getSheetDelimiter() const
{
  return m_dialog->m_sheetDelimiter->text();
}

bool CSVExportDialog::exportSheet(QString const & sheetName) const
{
  for (QListViewItem * item = m_dialog->m_sheetList->firstChild(); item; item = item->nextSibling())
  {
    if (((QCheckListItem * ) item)->isOn())
    {
      if ( ((QCheckListItem * ) item)->text() == sheetName )
        return true;
    }
  }
  return false;
}

void CSVExportDialog::slotOk()
{
  accept();
}

void CSVExportDialog::slotCancel()
{
  reject();
}

void CSVExportDialog::returnPressed()
{
  if ( m_dialog->m_delimiterBox->id( m_dialog->m_delimiterBox->selected() ) != 4 )
    return;

  m_delimiter = m_dialog->m_delimiterEdit->text();
}

void CSVExportDialog::textChanged ( const QString & )
{

  if ( m_dialog->m_delimiterEdit->text().isEmpty() )
  {
    enableButtonOK( ! m_dialog->m_radioOther->isChecked() );
    return;
  }

  m_dialog->m_radioOther->setChecked ( true );
  delimiterClicked(4);
}

void CSVExportDialog::delimiterClicked( int id )
{
  enableButtonOK( true );

  //Erase "Other Delimiter" text box if the user has selected one of 
  //the standard options instead (comma, semicolon, tab or space)
  if (id != 4)
  	m_dialog->m_delimiterEdit->setText("");
  
  switch (id)
  {
    case 0: // comma
      m_delimiter = ",";
      break;
    case 1: // semicolon
      m_delimiter = ";";
      break;
    case 2: // tab
      m_delimiter = "\t";
      break;
    case 3: // space
      m_delimiter = " ";
      break;
    case 4: // other
      enableButtonOK( ! m_dialog->m_delimiterEdit->text().isEmpty() );
      m_delimiter = m_dialog->m_delimiterEdit->text();
      break;
  }
}

void CSVExportDialog::textquoteSelected( const QString & mark )
{
  m_textquote = mark[0];
}

void CSVExportDialog::selectionOnlyChanged( bool on )
{
  m_dialog->m_sheetList->setEnabled( !on );
  m_dialog->m_delimiterLineBox->setEnabled( !on );

  if ( on )
    m_dialog->m_tabWidget->setCurrentPage( 1 );
}

bool CSVExportDialog::exportSelectionOnly() const
{
  return m_dialog->m_selectionOnly->isChecked();
}

QTextCodec* CSVExportDialog::getCodec(void) const
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

QString CSVExportDialog::getEndOfLine(void) const
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

#include "csvexportdialog.moc"

