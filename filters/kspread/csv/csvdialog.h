/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#ifndef CSVDIALOG_H
#define CSVDIALOG_H

#include <kdialogbase.h>

class DialogUI;

class CSVDialog : public KDialogBase
{
    Q_OBJECT
public:
    enum Header { TEXT, NUMBER, DATE, CURRENCY };

    CSVDialog(QWidget* parent, QByteArray& fileArray, const QString seperator);
    ~CSVDialog();

    int getRows();
    int getCols();
    int getHeader(int col);
    QString getText(int row, int col);

private:
    void fillTable();
    void fillComboBox();
    void setText(int row, int col, const QString& text);
    void adjustRows(int iRows);

    int m_adjustRows;
    int m_startline;
    QChar m_textquote;
    QString m_delimiter;
    QByteArray m_fileArray;
    DialogUI *m_dialog;

private slots:
    void returnPressed();
    void formatClicked(int id);
    void delimiterClicked(int id);
    void lineSelected(const QString& line);
    void textquoteSelected(const QString& mark);
    void currentCellChanged(int, int col);
    void textChanged ( const QString & );
};

#endif
