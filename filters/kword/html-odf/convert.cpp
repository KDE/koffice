/* This file is part of the KOffice project
   Copyright (C) 2010 Pramod S G <pramod.xyle@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the Library GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "convert.h"

#include <QtCore/QFile>
#include <QtCore/QString>

#include <QtCore/QByteArray>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <KOdfStorageDevice.h>
#include <KDE/KStandardDirs>
#include <QtXmlPatterns/QXmlQuery>

void Conversion::convert(QFile *cont1)
{

    QByteArray contall("<?xml version='1.0' encoding='UTF-8'?>");
    contall.append("<office:document xmlns:office='urn:oasis:names:tc:opendocument:xmlns:office:1.0'>");

    QByteArray cont;
    QByteArray sty;
    QByteArray met;

    /*The following line of code contains a parameter "path_of_the_file", which needs to be fixed.
    Here, the url of the file that is opened or the path of the temp file should be substituted.*/

    KOdfStore* storecont=KOdfStore::createStore("path_of_the_file",KOdfStore::Read);
    storecont->extractFile("meta.xml",met);
    met.remove(0,38);
    contall.append(met);

    storecont->extractFile("styles.xml",sty);
    sty.remove(0,38);
    contall.append(sty);

    storecont->extractFile("content.xml",cont);
    cont.remove(0,38);
    contall.append(cont);

    contall.append("</office:document>");

    QFile temp1(KStandardDirs::locate("data","kword/html-odf/converter.xsl"));
    temp1.open(QIODevice::ReadOnly);


    QXmlQuery myQuery(QXmlQuery::XSLT20);
    myQuery.setFocus(contall);
    myQuery.setQuery(temp1.readAll());
    myQuery.evaluateTo(cont1);



    temp1.close();
    contall.clear();
    met.clear();
    sty.clear();
    cont.clear();

}
