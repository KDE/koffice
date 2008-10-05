/* This file is part of the KDE project
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Sebastian Sauer <mail@dipe.org>
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

#ifndef KWPAGESTYLE_H
#define KWPAGESTYLE_H

#include "frames/KWTextFrameSet.h"
#include "KWord.h"
#include "kword_export.h"

#include <KoPageLayout.h>

#include <QSharedDataPointer>

class KWPageStylePrivate;

/**
 * A container for all information for the page wide style.
 *
 * For documents that have a main text auto generated we have a lot of little options
 * to do that. This class wraps all these options.
 *
 * \note that the margins are stored in a \a KoPageLayout instance.
 *
 * \note once you created an instance of \a KWPageStyle you may like to use the
 * KWPageManager::addPageStyle() method to let KWord handle the ownership else
 * you are responsible for deleting the instance and taking care that no \a KWPage
 * instance or something else still keeps a (then dangling) pointer to it.
 */
class KWORD_TEST_EXPORT KWPageStyle
{
public:
    /**
     * Creates an empty, invalid page style.
     */
    KWPageStyle();
    /**
     * constructor, initializing the data to some default values.
     *
     * \p masterPageName The name of this page style.
     */
    KWPageStyle(const QString& mastername);
    KWPageStyle(const KWPageStyle &ps);
    KWPageStyle &operator=(const KWPageStyle &ps);
    /// destructor
    ~KWPageStyle();

    /// returns true if the KWPageStyle is valid
    bool isValid() const;

    /**
     * Return the current columns settings.
     */
    const KoColumns &columns() const;
    /**
     * Set the new columns settings
     */
    void setColumns(const KoColumns &columns);

    /// Return the type of header the pages will get.
    KWord::HeaderFooterType headers() const;
    /// set the type of header the pages will get.
    void setHeaderPolicy(KWord::HeaderFooterType p);

    /// Return the type of footers the pages will get.
    KWord::HeaderFooterType footers() const;
    /// Set the type of footers the pages will get.
    void setFooterPolicy(KWord::HeaderFooterType p);

    /**
     * This is the main toggle for all automatically generated frames.
     * The generation and placing of the main text frame, as well as headers, footers,
     * end notes and footnotes for the main text flow is enabled as soon as this is on.
     * Turn it off and all the other settings on this class will be ignored.
     * @param on the big switch for auto-generated frames.
     */
    void setMainTextFrame(bool on);
    /**
     * Return if the main text frame, but also the headers/footers etc should be autogenerated.
     */
    bool hasMainTextFrame() const;

    /// return the distance between the main text and the header
    qreal headerDistance() const;
    /**
     * Set the distance between the main text and the header
     * @param distance the distance
     */
    void setHeaderDistance(qreal distance);

    /// return the distance between the footer and the frame directly above that (footnote or main)
    qreal footerDistance() const;
    /**
     * Set the distance between the footer and the frame directly above that (footnote or main)
     * @param distance the distance
     */
    void setFooterDistance(qreal distance);

    /// return the distance between the footnote and the main frame.
    qreal footnoteDistance() const;
    /**
     * Set the distance between the footnote and the main frame.
     * @param distance the distance
     */
    void setFootnoteDistance(qreal distance);
    /// return the distance between the main text frame and the end notes frame.
    qreal endNoteDistance() const;
    /**
     * Set the distance between the main text frame and the end notes frame.
     * @param distance the distance
     */
    void setEndNoteDistance(qreal distance);

    /// return the line length of the foot note separator line, in percent of the pagewidth
    int footNoteSeparatorLineLength() const;
    /// set the line length of the foot note separator line, in percent of the pagewidth
    void setFootNoteSeparatorLineLength(int length);

    /// return the thickness of the line (in pt) drawn above the foot notes
    qreal footNoteSeparatorLineWidth() const;
    /// set the thickness of the line (in pt) drawn above the foot notes
    void setFootNoteSeparatorLineWidth(qreal width);

    /// return the pen style used to draw the foot note separator line
    Qt::PenStyle footNoteSeparatorLineType() const;
    /// set the pen style used to draw the foot note separator line
    void setFootNoteSeparatorLineType(Qt::PenStyle type);

    /// return the position on the page for the foot note separator line
    KWord::FootNoteSeparatorLinePos footNoteSeparatorLinePosition() const;
    /// set the position on the page for the foot note separator line
    void setFootNoteSeparatorLinePosition(KWord::FootNoteSeparatorLinePos position);

    /// initialize to default settings
    void clear();

    /// return the pageLayout applied for these pages
    const KoPageLayout pageLayout() const;

    /// set the pageLayout applied for these pages
    void setPageLayout(const KoPageLayout &layout);

    /**
     * Get a frameset that is stored in this page style.
     * Example of framesets stored : the headers, footers...
     * @param hfType the type of the frameset that must be returned
     * @returns the required frameSet, 0 if none found.
     */
    KWTextFrameSet *getFrameSet(KWord::TextFrameSetType hfType);

    /**
     * Add a frameset in this page style.
     * Example of framesets stored : the headers, footers...
     * @param hfType the type of the frameset
     * @param fSet the frameset
     * This frameset will be destroyed when the page style is destroyed.
     */
    void addFrameSet(KWord::TextFrameSetType hfType, KWTextFrameSet *fSet);

    /// get the master page name for this page style.
    QString name() const;

    bool operator==(const KWPageStyle &other) const;

private:
    QExplicitlySharedDataPointer<KWPageStylePrivate> d;
};

#endif
