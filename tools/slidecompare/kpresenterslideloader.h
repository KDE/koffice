/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <jos.van.den.oever@kogmbh.com>

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
#ifndef KPRESENTERSLIDELOADER_H
#define KPRESENTERSLIDELOADER_H

#include "slideloader.h"

class KoPADocument;

class KPresenterSlideLoader : public SlideLoader {
private:
    const KoPADocument* m_doc;
    int version;

    void closeDocument();
public:
    KPresenterSlideLoader(QObject* parent = 0);
    ~KPresenterSlideLoader();
    int numberOfSlides();
    QSize slideSize();
    int slideVersion(int /*position*/) {
        // version is independent of position for this loader
        return version;
    }
    QPixmap loadSlide(int number, const QSize& maxsize);
    void open(const QString& path);
};

#endif
