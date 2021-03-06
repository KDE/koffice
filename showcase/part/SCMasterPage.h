/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCMASTERPAGE_H
#define SCMASTERPAGE_H

#include <KoPAMasterPage.h>

#include "SCPageData.h"

class SCDocument;

class SCMasterPage : public KoPAMasterPage, public SCPageData
{
public:
    explicit SCMasterPage(SCDocument *document);
    virtual ~SCMasterPage();

    /**
     * Get the page type used in the document
     *
     * @return KoPageApp::Slide
     */
    virtual KoPageApp::PageType pageType() const;

    /// reimplemented
    virtual bool loadOdf(const KXmlElement &element, KShapeLoadingContext &context);

protected:
    /// reimplemented
    virtual void loadOdfPageExtra(const KXmlElement &element, KoPALoadingContext &loadingContext);
};

#endif /* SCMASTERPAGE_H */
