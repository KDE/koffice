/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KPRPAGELAYOUTS_H
#define KPRPAGELAYOUTS_H

#include <QMap>
#include <QObject>
#include <QVariant>

class QRectF;
class SCPageLayout;
class SCPageLayoutWrapper;
class KoPALoadingContext;
class KoPASavingContext;
class QString;

class SCPageLayouts : public QObject
{
public:
    SCPageLayouts(QObject *parent = 0);
    ~SCPageLayouts();

    bool saveOdf(KoPASavingContext & context);

    /**
     * load all not yet loaded styles and add application styles
     */
    bool loadOdf(KoPALoadingContext & context);

    /**
     * Return a pagelayout by name.
     */
    SCPageLayout *pageLayout(const QString &name, KoPALoadingContext &loadingContext, const QRectF &pageRect);

    const QList<SCPageLayout *> layouts() const;

private:
    // this is a simulation of a std::set<SCPageLayout, compareBySCPageLayout>()
    QMap<SCPageLayoutWrapper, SCPageLayout *> m_pageLayouts;
};

Q_DECLARE_METATYPE(SCPageLayouts*)
#endif /* KPRPAGELAYOUTS_H */
