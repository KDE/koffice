/* This file is part of the KDE project
   Copyright (C) 2000 Robert JACOLIN <rjacolin@ifrance.com>

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

#ifndef XSLTIMPORTDIA_H
#define XSLTIMPORTDIA_H

#include <QtCore/QByteArray>

#include <KDE/KApplication>
#include <KDE/KFileDialog>
#include <KDE/KConfigGroup>

#include <KOdfStore.h>

#include "ui_xsltdialog.h"

class XSLTImportDia : public QDialog, Ui::XSLTDialog
{
    Q_OBJECT

    static const Qt::ItemDataRole FILEROLE = Qt::UserRole;
    static const Qt::ItemDataRole DIRROLE = (Qt::ItemDataRole)(Qt::UserRole+1);
    
    QString _fileIn;
    QString _fileOut;
    QByteArray _arrayIn;
    KOdfStore* _out;
    /** xslt file current */
    KUrl _currentFile;
    QByteArray _format;
    KConfig* _config;
    KConfigGroup grp;
    /** List of the most recent xslt file used. */
    QStringList _recentList;

    /** Lits use for common xslt files. */
    QStringList _dirsList;
    QStringList _filesList;
    QStringList _namesList;

public:
    XSLTImportDia(KOdfStore*, const QByteArray &format, QWidget* parent = 0, Qt::WFlags fl = 0);
    ~XSLTImportDia();

    void setInputFile(QString file)  {
        _fileIn = file;
    }
    void setOutputFile(QString file) {
        _fileOut = file;
    }

public slots:
    virtual void cancelSlot();
    virtual void chooseSlot();
    virtual void chooseRecentSlot();
    virtual void chooseCommonSlot();
    virtual void okSlot();

};

#endif /* XSLTIMPORTDIA_H */
