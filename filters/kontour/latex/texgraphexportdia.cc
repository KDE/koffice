/* This file is part of the KDE project
   Copyright (C) 2001 Robert JACOLIN <rjacolin@ifrance.com>

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

/*
   This file is based on the file :
    koffice/filters/kword/html/htmlexportdia.cc
	Copyright (C) 2001 Nicolas Goutte <nicog@snafu.de>

   which was based on the old file:
    /home/kde/koffice/filters/kspread/csv/csvfilterdia.cc

   The old file was copyrighted by
    Copyright (C) 1999 David Faure <faure@kde.org>

   The old file was licensed under the terms of the GNU Library General Public
   License version 2.
*/

#include <kapp.h>
#include <texgraphexportdia.h>
#include <texgraphexportdia.moc>
#include "kdebug.h"

TEXGRAPHExportDia::TEXGRAPHExportDia(QWidget *parent, const char *name) :
			KDialogBase(parent, name, true,
			i18n("Latex export filter parameters"), Ok|Cancel)
{
	kapp->restoreOverrideCursor();
	createDialog();
}

TEXGRAPHExportDia::TEXGRAPHExportDia(const KoStore& in, QWidget *parent, const char *name) :
			KDialogBase(parent, name, true,
			i18n("Latex export filter parameters"), Ok|Cancel)
{
	kapp->restoreOverrideCursor();
	_in = new KoStore(in);
	createDialog();
}

void TEXGRAPHExportDia::createDialog()
{
	resize(size());
	QWidget *page = new QWidget( this ); 
	setMainWidget(page);
	QBoxLayout *mainLayout = new QVBoxLayout(page, 0, spacingHint());
	/*styleBox = new QVButtonGroup(i18n("Document Style"), this);
	mainLayout->addWidget(styleBox);*/

	/* First part */
	/*QBoxLayout *styleLayout = new QVBoxLayout(styleBox);

	latexStyleRBtn = new QRadioButton(i18n("pstricks"), styleBox);
	styleLayout->addWidget(latexStyleRBtn);

	kwordStyleRBtn = new QRadioButton(i18n("KWord style"), styleBox);
	styleLayout->addWidget(kwordStyleRBtn);*/

	/*typeLabel  = new ...
	styleLayout->addWidget(typeLabel);
	typeCBox = new ...
	styleLayout->addWidget(typeCBox);
	*/
	/*styleBox->setExclusive(true);
	styleBox->setButton(0);*/			/* LATEX STYLE IS DEFAULT */
	//styleLayout->activate();

	/* Second part */
	langBox = new QVButtonGroup(i18n("Language"), page);
	mainLayout->addWidget(langBox);

	QBoxLayout *langLayout = new QVBoxLayout(langBox);

	unicodeRBtn = new QRadioButton(i18n("Unicode"), langBox);
	langLayout->addWidget(unicodeRBtn);

	latin1RBtn = new QRadioButton(i18n("latin1"), langBox);
	langLayout->addWidget(latin1RBtn);

	/*babelCheckBox = new ...
	langLayout->addWidget(babelCheckBox);

	babelCBox = new ...
	langLayout->addWidget(babelCBox);
	*/
	langBox->setExclusive(true);
	langBox->setButton(1);			/* LATIN1 IS THE DEFAULT. */
	langLayout->activate();

	/* Third part */
	docBox = new QVButtonGroup(i18n("Document type"), page);
	mainLayout->addWidget(docBox);

	QBoxLayout *docLayout = new QVBoxLayout(docBox);

	newDocRBtn = new QRadioButton(i18n("New document"), docBox);
	docLayout->addWidget(newDocRBtn);

	embededRBtn = new QRadioButton(i18n("Embeded document"), docBox);
	docLayout->addWidget(embededRBtn);

	docBox->setExclusive(true);
	docBox->setButton(0);			/* NEW DOC IS THE DEFAULT. */
	docLayout->activate();

	/* Display the main layout */
	mainLayout->addStretch(5);
	mainLayout->activate();
}

QString TEXGRAPHExportDia::state()
{
	QString result;

	if(newDocRBtn == docBox->selected())
		result += "DOC";
	else if(embededRBtn == docBox->selected())
		result += "EMBEDED";

	result += '-';

	if(unicodeRBtn == langBox->selected())
		result += "UNICODE";
	else if(latin1RBtn == langBox->selected())
		result += "LATIN1";
	result += '-';
	
	/*if(latexStyleRBtn == styleBox->selected())
		result += "PSTRICKS";
	else if(kwordStyleRBtn == styleBox->selected())
		result += "KWORD";*/
	return result;
}

void TEXGRAPHExportDia::slotOk()
{
	hide();
	kdDebug() << "config : " << state() << endl;

	kdDebug() << "TEXGRAPH FILTER --> BEGIN" << endl;
	Document TEXGRAPHParser(*_in, _fileOut, state());
	TEXGRAPHParser.analyse();
	kdDebug() << "---------- generate file -------------" << endl;
	TEXGRAPHParser.generate();
	kdDebug() << "TEXGRAPH FILTER --> END" << endl;

	reject();
}
