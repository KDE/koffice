/*
   This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
                 2000 Werner Trobin <trobin@kde.org>

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

#ifndef koTemplateCreateDia_h
#define koTemplateCreateDia_h

#include <kdialogbase.h>

class QString;
class QPixmap;
class QWidget;
class KInstance;
class KLineEdit;
class KoTemplateCreateDiaPrivate;

/****************************************************************************
 *
 * Class: koTemplateCreateDia
 *
 ****************************************************************************/

class KoTemplateCreateDia : public KDialogBase
{
    Q_OBJECT

public:
    KoTemplateCreateDia( const QCString &templateType, KInstance *instance,
                         const QString &file, const QPixmap &pix, QWidget *parent=0L );
    ~KoTemplateCreateDia();

    static void createTemplate( const QCString &templateType, KInstance *instance,
                                const QString &file, const QPixmap &pix, QWidget *parent=0L );

protected:
    void slotOk();

private slots:
    void slotDefault();
    void slotCustom();
    void slotSelect();
    void slotNameChanged(const QString &name);

    void slotAddGroup();
    void slotRemove();

private:
    void updatePixmap();
    void fillGroupTree();

    QString m_file;
    QPixmap m_pixmap;
    KoTemplateCreateDiaPrivate *d;
};


class KoNewGroupDia : public KDialogBase {

    Q_OBJECT
public:
    KoNewGroupDia(QWidget *parent);
    ~KoNewGroupDia() {}

    // If the name is empty, Cancel was pressed :)
    static QString newGroupName(QWidget *parent);
    QString name() const;

private slots:
    void slotTextChanged(const QString &name);

private:
    KLineEdit *m_name;
};
#endif
