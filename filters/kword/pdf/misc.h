/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef MISC_H
#define MISC_H

#include <qcolor.h>
#include <qfont.h>
#include <qdom.h>


double toPoint(double mm);
bool equal(double d1, double d2, double delta = 0.1);

//-----------------------------------------------------------------------------
class KoFilterChain;

class FilterData
{
 public:
    FilterData(KoFilterChain *, QSize pageSize, KoFormat, KoPageOrientation,
               uint nbPages);

    QDomElement createElement(const QString &name)
        { return _document.createElement(name); }
    QDomElement createParagraph(const QString &text,
                                const QValueVector<QDomElement> &layouts,
                                const QValueVector<QDomElement> &formats);

    KoFilterChain *chain() const { return _chain; }
    QDomDocument document() const { return _document; }
    uint imageIndex() const { return _imageIndex; }
    QDomElement bookmarks() const { return _bookmarks; }
    QDomElement pictures() const { return _pictures; }

    QDomElement createPictureFrameset(double left, double right,
                                      double top, double bottom)
        { return createFrameset(Picture, left, right, top, bottom); }
    void newPage();

 private:
    KoFilterChain *_chain;
    QDomDocument   _document;
    uint           _imageIndex, _textIndex;
    bool           _needNewTextFrameset;
    double         _pageHeight;
    QDomElement    _mainElement, _framesets, _pictures, _bookmarks;
    QDomElement    _textFrameset, _mainTextFrameset, _lastMainLayout;

    enum FramesetType { Text, Picture };
    QDomElement createFrameset(FramesetType, double left, double right,
                               double top, double bottom);
};

//-----------------------------------------------------------------------------
class FilterFont
{
 public:
    FilterFont(const QString &name = "Times", uint size = 12,
               const QColor &color = Qt::black);

    bool operator ==(const FilterFont &) const;
    bool format(QDomDocument &, QDomElement &format, uint pos, uint len,
                bool all = false) const;
    const QFont &font() const { return _font; }

    static FilterFont *defaultFont;

 private:
    QFont  _font;
    QColor _color;
};

//-----------------------------------------------------------------------------
class LinkAction;
class Catalog;

class FilterLink
{
 public:
    FilterLink(double x1, double x2, double y1, double y2,
               LinkAction &, Catalog &);

    bool inside(double xMin, double xMax, double yMin, double yMax) const;
    void format(QDomDocument &, QDomElement &format,
                uint pos, const QString &text) const;

 private:
    double  _xMin, _yMin, _xMax, _yMax;
    QString _href;
};

#endif
