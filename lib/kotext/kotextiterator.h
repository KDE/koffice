/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KOTEXTITERATOR_H
#define KOTEXTITERATOR_H

#include <qvaluelist.h>
#include <qstring.h>
#include <qpair.h>
class KoTextParag;
class KoTextObject;
class KoTextView;

/**
 * A convenient way to iterate over paragraphs, possibly in multiple textobjects,
 * with many options (from cursor, backwards, in selection).
 * @short General purpose paragraph iterator
 */
class KoTextIterator
{
public:
    /**
     * @param options see KFindDialog
     */
    KoTextIterator( const QValueList<KoTextObject *> & lstObjects, KoTextView* textView, int options ) {
        init( lstObjects, textView, options );
    }
    void init( const QValueList<KoTextObject *> & lstObjects, KoTextView* textView, int options );

    /**
     * Restart from the beginning - assumes same parameters given to init
     */
    void restart();

    /**
     * Change options during iteration. ## Not sure how if all cases will be handled :}
     * At least this is useful for the "Replace All" button during replacing,
     * and for switching to "FindBackwards" temporarily for "find previous".
     */
    void setOptions( int options );

    /**
     * Go to next paragraph that we must iterate over
     */
    void operator++();

    /**
     * @return true if we have iterated over all paragraphs
     */
    bool atEnd() const;

    /**
     * @return true if currentText() isn't empty. The implementation is
     * faster than calling currentText().isEmpty() though.
     */
    bool hasText() const;

    /**
     * @return the string at the current position of the iterator
     */
    QString currentText() const;

    /**
     * @return the string at the current position of the iterator
     */
    KoTextParag* currentParag() const { return m_currentParag; }

    /**
     * @return the text object in which @ref currentParag() is.
     */
    KoTextObject* currentTextObject() const { return *m_currentTextObj; }

    /**
     * Where in @ref currentParag() does @ref currentText() start?
     */
    int currentStartIndex() const;

    /**
     * @return the string at the current position of the iterator,
     * as well as the index in the current paragraph.
     * Use this instead of separate calls to currentText and currentStartIndex,
     * for performance reasons.
     */
    QPair<int, QString> currentTextAndIndex() const;

private:
    // The reason we use a QValueList of pointers instead of QPtrList
    // is that having a QPtrListIterator member var can't work, one can't
    // initialize it afterwards.
    QValueList<KoTextObject *> m_lstObjects;
    int m_options;

    // This is relative to the first textobject in m_lstObjects
    // We always start from this paragraph (even when going backwards)
    KoTextParag* m_firstParag;
    int m_firstIndex;

    // This is relative to the last textobject in m_lstObjects
    // We're done when we hit this paragraph
    KoTextParag* m_lastParag;
    int m_lastIndex;

    // Our current position
    QValueList<KoTextObject *>::Iterator m_currentTextObj;
    KoTextParag* m_currentParag;
};

#endif
