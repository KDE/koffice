/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#ifndef KWLOADINGINFO_H
#define KWLOADINGINFO_H

/// Temporary information used only during loading
class KWLoadingInfo
{
public:
    KWLoadingInfo() {}
    ~KWLoadingInfo() {}

    // Bookmarks (kword-1.3 XML: they need all framesets to be loaded first)
    struct BookMark
    {
        QString bookname;
        int paragStartIndex;
        int paragEndIndex;
        QString frameSetName;
        int cursorStartIndex;
        int cursorEndIndex;
    };

    typedef QValueList<BookMark> BookMarkList;
    BookMarkList bookMarkList;

    // Bookmarks (OASIS XML). Only need to store bookmark starts, until hitting bookmark ends
    struct BookmarkStart {
        BookmarkStart() {} // for stupid QValueList
        BookmarkStart( KWTextDocument* _doc, KoTextParag* par, int ind )
            : doc( _doc ), parag( par ), pos( ind ) {}
        KWTextDocument* doc;
        KoTextParag* parag;
        int pos;
    };
    typedef QMap<QString, BookmarkStart> BookmarkStartsMap;
    BookmarkStartsMap m_bookmarkStarts;

    // Text frame chains; see KWTextFrameSet::loadOasisText

    void storeNextFrame( KWFrame* thisFrame, const QString& chainNextName ) {
        m_nextFrameDict.insert( chainNextName, thisFrame );
    }
    KWFrame* chainPrevFrame( const QString& frameName ) const {
        return m_nextFrameDict[frameName]; // returns 0 if not found
    }

    void storeFrameName( KWFrame* frame, const QString& name ) {
        m_frameNameDict.insert( name, frame );
    }
    KWFrame* frameByName( const QString& name ) const {
        return m_frameNameDict[name]; // returns 0 if not found
    }

private:
    QDict<KWFrame> m_nextFrameDict;
    QDict<KWFrame> m_frameNameDict;
};

#endif /* KWLOADINGINFO_H */

