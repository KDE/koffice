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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef CSVIMPORTDIALOG_H
#define CSVIMPORTDIALOG_H

#include <qstringlist.h>

#include <kdialogbase.h>

class DialogUI;

class CSVImportDialog : public KDialogBase
{
    Q_OBJECT
public:
    enum Header
    {
        TEXT,       ///< Normal text
        NUMBER,     ///< Number (either like locale or like C)
        DATE,       ///< Date \todo What type exactly?
        CURRENCY,   ///< Currency
        COMMANUMBER,///< Number, which decimal symbol is a comma
        POINTNUMBER ///< Number, which decimal symbol is a point/dot
    };

    CSVImportDialog(QWidget* parent, QByteArray& fileArray);
    ~CSVImportDialog();

    bool     firstRowContainHeaders();
    bool     firstColContainHeaders();
    int      rows();
    int      cols();
    int      headerType(int col);
    QString  text(int row, int col);

private:
    void fillTable();
    void fillComboBox();
    void setText(int row, int col, const QString& text);
    void adjustRows(int iRows);
    void adjustCols(int iCols);
    bool checkUpdateRange();
    QTextCodec* getCodec(void) const;

    // The real contents of the dialog
    DialogUI  *m_dialog;

    bool       m_adjustRows;
    bool       m_adjustCols;
    int        m_startRow;
    int        m_startCol;
    int        m_endRow;
    int        m_endCol;
    QChar      m_textquote;
    QString    m_delimiter;
    bool       m_ignoreDups;
    QByteArray m_fileArray;
    QTextCodec *m_codec;
    QStringList m_formatList; ///< List of the column formats

private slots:
    void returnPressed();
    void formatChanged( const QString& );
    void delimiterClicked(int id);
    void textquoteSelected(const QString& mark);
    void currentCellChanged(int, int col);
    void textChanged ( const QString & );
    void ignoreDuplicatesChanged( int );
    void updateClicked();
    void encodingChanged ( const QString & );
};

#endif
