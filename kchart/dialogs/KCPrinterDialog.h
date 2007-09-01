/*
*
* Copyright (C) 2005  Fredrik Edemar   <f_edemar@linux.se>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*
*/

#ifndef KCPRINTERDIALOG
#define KCPRINTERDIALOG

#include <QLabel>

#include <kdeprint/kprintdialogpage.h>
#include <knuminput.h>

namespace KChart
{

class KCPrinterDialog : public KPrintDialogPage
{
  Q_OBJECT
  public:
	/// Getting the common arguments.
    explicit KCPrinterDialog( QWidget *parent = 0, const char *name = 0 );

	/// Reimplemented.
    void getOptions( QMap<QString, QString>& opts, bool include_def = false );
	/// Reimplemented
    void setOptions( const QMap<QString, QString>& opts );
	/// The check box for the option.
  
  private:
    KIntNumInput *txtSizex;
    KIntNumInput *txtSizey;
};

}  //namespace KChart


#endif // KCPRINTERDIALOG
