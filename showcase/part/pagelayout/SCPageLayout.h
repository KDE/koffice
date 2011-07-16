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

#ifndef SCPAGELAYOUT_H
#define SCPAGELAYOUT_H

#include <QList>
#include <QString>

class QRectF;
class QPixmap;
class SCPlaceholder;
class KoPALoadingContext;
class KoPASavingContext;
class KXmlElement;

class SCPageLayout
{
public:
    enum Type {
        Page,
        Handout
    };

    SCPageLayout();
    ~SCPageLayout();

    /**
     * Load the page layout
     *
     * @param
     * @param
     * @return 
     */
    bool loadOdf(const KXmlElement &element, const QRectF &pageRect);

    /**
     * save the page layout to odf
     *
     * @return the style name used for this page layout
     */
    QString saveOdf(KoPASavingContext &context) const;

    /**
     * Get the placeholders of the layout
     */
    QList<SCPlaceholder *> placeholders() const;

    /**
     * get the thumbnail of the layout
     */
    QPixmap thumbnail() const;

    /**
     * Get the type of layout
     */
    Type type() const;

    /**
     * @brief Check if the page layouts match
     *
     * The page layouts match if the placeholder are the same.
     */
    bool operator<(const SCPageLayout &other) const;

    /**
     * @brief Compare layout by content
     *
     * The content is the same when the order of the placholder objects is the same
     */
    static bool compareByContent(const SCPageLayout &pl1, const SCPageLayout &pl2);

private:
    // The display name of the layout
    QString m_name;
    // placeholders used in the layout
    QList<SCPlaceholder *> m_placeholders;
    // the type of the layout.
    // if one placeholder as a presentation:object="handout" then the layout is of tyle Handout
    Type m_layoutType;
};

#endif /* SCPAGELAYOUT_H */
