/* This file is part of the KDE project
   Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>

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

#include "sheet_properties.h"
#include "sheet_properties_base.h"

#include <qcheckbox.h>
#include <qvbox.h>

#include <kdialogbase.h>
#include <klocale.h>

SheetPropertiesDialog::SheetPropertiesDialog( QWidget* parent ):
  KDialogBase( parent, "sheetPropertiesDialog", true, 
  i18n("Sheet Properties"), 
  KDialogBase::Ok|KDialogBase::Cancel|KDialogBase::Default )
{
  QVBox* mainWidget = makeVBoxMainWidget();
  d = new SheetPropertiesBase( mainWidget );
}

SheetPropertiesDialog::~SheetPropertiesDialog()
{
  delete d;
}

void SheetPropertiesDialog::slotDefault()
{
  setAutoCalc( true );
  setShowGrid( true );
  setShowFormula( false );
  setHideZero( false );
  setShowFormulaIndicator( true );
  setColumnAsNumber( false );
  setLcMode( false );
  setCapitalizeFirstLetter( false );
}

bool SheetPropertiesDialog::autoCalc() const
{
  return d->autoCalcCheckBox->isChecked();
}

void SheetPropertiesDialog::setAutoCalc( bool b )
{
  d->autoCalcCheckBox->setChecked( b );
}

bool SheetPropertiesDialog::showGrid() const
{
  return d->showGridCheckBox->isChecked();
}

void SheetPropertiesDialog::setShowGrid( bool b )
{
  d->showGridCheckBox->setChecked( b );
}

bool SheetPropertiesDialog::showPageBorders() const
{
  return d->showPageBordersCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setShowPageBorders( bool b )
{
  d->showPageBordersCheckBox->setChecked( b );
}   

bool SheetPropertiesDialog::showFormula() const
{
  return d->showFormulaCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setShowFormula( bool b )
{
  d->showFormulaCheckBox->setChecked( b );
}
    
bool SheetPropertiesDialog::hideZero() const
{
  return d->hideZeroCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setHideZero( bool b )
{
  d->hideZeroCheckBox->setChecked( b );
}
    
bool SheetPropertiesDialog::showFormulaIndicator() const
{
  return d->showFormulaIndicatorCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setShowFormulaIndicator( bool b )
{
  d->showFormulaIndicatorCheckBox->setChecked( b );
}
    
bool SheetPropertiesDialog::columnAsNumber() const
{
  return d->showColumnAsNumbersCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setColumnAsNumber( bool b )
{
  d->showColumnAsNumbersCheckBox->setChecked( b );
}
    
bool SheetPropertiesDialog::lcMode() const
{
  return d->useLCModeCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setLcMode( bool b )
{
  d->useLCModeCheckBox->setChecked( b );
}    
    
bool SheetPropertiesDialog::capitalizeFirstLetter() const
{
  return d->capitalizeFirstLetterCheckBox->isChecked();
}
    
void SheetPropertiesDialog::setCapitalizeFirstLetter( bool b )
{
  d->capitalizeFirstLetterCheckBox->setChecked( b );
}    
    
#include "sheet_properties.moc"
