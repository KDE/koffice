/* This file is part of the KDE project
   Original file (serialletter.h): Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

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

#ifndef _SERIALLETTER_INTERFACE_H_
#define _SERIALLETTER_INTERFACE_H_
#include <qmap.h>
#include <qcstring.h>
#include <qdom.h>
#include <kinstance.h>
#include <qmap.h>
#include <qobject.h>
#include <dcopobject.h>

typedef QMap<QString,QString> DbRecord;
#define KWSLUnspecified		0
#define KWSLEdit		1
#define KWSLCreate		2
#define KWSLOpen		3
#define KWSLMergePreview	4
#define KWSLMergeDocument	5

#define KWSLCreate_text		"create"
#define KWSLOpen_text		"open"
/******************************************************************
 *
 * Class: KWSerialLetterDataSource
 *
 ******************************************************************/

class KWSerialLetterDataSource: public QObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP
    public:
    KWSerialLetterDataSource(KInstance* inst,QObject *parent);
    virtual ~KWSerialLetterDataSource(){;}
    virtual class QString getValue( const class QString &name, int record = -1 ) const=0;

    const QMap< QString, QString > &getRecordEntries() const;

    KInstance *KWInstance(){return m_instance;}
    virtual  bool showConfigDialog(class QWidget*,int) =0;

    virtual void save(QDomDocument&, QDomElement&)=0;
    virtual void load( class QDomElement& elem )=0;

    virtual void refresh(bool force)=0;
    QByteArray info;
    protected:
    DbRecord sampleRecord;
    private:
    KInstance *m_instance;
k_dcop:
    virtual  int getNumRecords() const =0;
};




#endif
