/*
*
* Copyright (C) 2005  Fredrik Edemar
*                     f_edemar@linux.se
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

// Qt includes
#include <qlayout.h>

// KDE includes
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

// local includes
#include "kchartPrinterDlg.h"
namespace KChart
{

 KChartPrinterDlg::KChartPrinterDlg( QWidget *parent, const char *name )
  : KPrintDialogPage( parent, name )
{
  setTitle( i18n( "KChart Options" ) );
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );
  txtSizex = new KIntNumInput(this );
  txtSizex->setSuffix("%");
  txtSizex->setMinValue(1);
  txtSizex->setMaxValue(100);
  txtSizex->setValue(100);
  txtSizey = new KIntNumInput(this );
  txtSizey->setSuffix("%");
  txtSizey->setMinValue(1);
  txtSizey->setMaxValue(100);
  txtSizey->setValue(100);
  
  layout->addWidget( new QLabel(i18n("Print size:"), this) );
  layout->addWidget( new QLabel(i18n("Width: "), this) );
  layout->addWidget( txtSizex );
  layout->addWidget( new QLabel(i18n("Height: "), this) );
  layout->addWidget( txtSizey );
  layout->addStretch( 1 );
}

void KChartPrinterDlg::getOptions( QMap<QString, QString>& opts, bool )
{
  opts["kde-kchart-printsizex"] = QString::number(txtSizex->value());
  opts["kde-kchart-printsizey"] = QString::number(txtSizey->value());
}

void KChartPrinterDlg::setOptions( const QMap<QString, QString>& opts )
{
  if ( opts["kde-kchart-printsizex"].isEmpty() )
    txtSizex->setValue(100);
  else
    txtSizex->setValue((opts["kde-kchart-printsizex"]).toInt());
  if ( opts["kde-kchart-printsizey"].isEmpty() )
    txtSizey->setValue(100);
  else
    txtSizey->setValue((opts["kde-kchart-printsizey"]).toInt());
}

bool KChartPrinterDlg::isValid( const QString& )
{
  return true;
}
}  //namespace KChart
#include "kchartPrinterDlg.moc"
