/* This file is part of the KDE project
   Copyright (C)  2001 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __KWEditPersonnalExpression__
#define __KWEditPersonnalExpression__

#include <kdialogbase.h>
#include <qmap.h>

class QPushButton;
class QComboBox;
class QListBox;

class KWEditPersonnalExpression : public KDialogBase
{
    Q_OBJECT
public:
    KWEditPersonnalExpression( QWidget *parent, const char *name = 0 );

protected:
    void init(const QString& filename );
    void loadFile();
    void initCombobox();
    void saveFile();


protected slots:
    void slotExpressionActivated(const QString &_text );
    void slotAddExpression();
    void slotDelExpression();
    void slotAddGroup();
    void slotDelGroup();
    void slotRenameGroup();
    void slotRenameExpression();
    void slotSelectionChanged();
    virtual void slotOk();

 private:
    typedef QMap<QString,QStringList> list;
    list listExpression;
    QComboBox *m_typeExpression;
    QListBox *m_listOfExpression;
    QPushButton *m_addExpression;
    QPushButton *m_addGroup;
    QPushButton *m_delGroup;
    QPushButton *m_renameGroup;
    QPushButton *m_delExpression;
    QPushButton *m_renameExpression;
    bool m_bChanged;
};

#endif
