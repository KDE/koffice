/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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
#include <QString>
#include <KoDataCenter.h>

class QRectF;
class KPrPageLayout;
class KPrPageLayoutWrapper;
class KoPALoadingContext;
class KoPASavingContext;

#define PageLayouts "PageLayouts"

class KPrPageLayouts : public KoDataCenter
{
public:
    KPrPageLayouts();
    ~KPrPageLayouts();

    /// reimplemented
    virtual bool completeLoading( KoStore *store );
    /// reimplemented
    virtual bool completeSaving( KoStore *store, KoXmlWriter * manifestWriter, KoShapeSavingContext * context );

    bool saveOdf( KoPASavingContext & context );

    /**
     * load all not yet loaded styles and add application styles
     */
    bool loadOdf( KoPALoadingContext & context );

    /**
     *
     */
    KPrPageLayout * pageLayout( const QString & name, KoPALoadingContext & loadingContext, const QRectF & pageRect );


    const QList<KPrPageLayout *> layouts() const;

private:
    // this is a simulation of a std::set<KPrPageLayout, compareByKPrPageLayout>()
    QMap<KPrPageLayoutWrapper, KPrPageLayout *> m_pageLayouts;
};

#endif /* KPRPAGELAYOUTS_H */
