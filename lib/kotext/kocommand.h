/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#ifndef kocommand_h
#define kocommand_h

#include <kcommand.h>
#include <qrichtext_p.h>
class KoTextObject;
class KoTextDocument;
#include <koparaglayout.h>

/**
 * Wraps a KoTextDocCommand into a KCommand, for the UI
 * In fact the KoTextDocCommand isn't even known from here.
 * When the UI orders execute or unexecute, we simply call undo/redo
 * on the KoTextObject. Since one KCommand is created for each
 * command there, the two simply map.
 */
class KoTextCommand : public KNamedCommand
{
public:
    KoTextCommand( KoTextObject * textobj, const QString & name ) :
        KNamedCommand( name ), m_textobj(textobj) {}
    ~KoTextCommand() {}

    virtual void execute();
    virtual void unexecute();

protected:
    KoTextObject * m_textobj;
};

/**
 * Command created when deleting some text
 */
class KoTextDeleteCommand : public KoTextDocDeleteCommand
{
public:
    KoTextDeleteCommand( KoTextDocument *d, int i, int idx, const QMemArray<KoTextStringChar> &str,
                         const CustomItemsMap & customItemsMap,
                         const QValueList<KoParagLayout> & oldParagLayouts );
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );
protected:
    QValueList<KoParagLayout> m_oldParagLayouts;
    CustomItemsMap m_customItemsMap;
};

/**
 * Command created when inserting some text
 */
class KoTextInsertCommand : public KoTextDeleteCommand
{
public:
    KoTextInsertCommand( KoTextDocument *d, int i, int idx, const QMemArray<KoTextStringChar> &str,
                         const CustomItemsMap & customItemsMap,
                         const QValueList<KoParagLayout> &oldParagLayouts )
        : KoTextDeleteCommand( d, i, idx, str, customItemsMap, oldParagLayouts ) {}
    Commands type() const { return Insert; };
    KoTextCursor *execute( KoTextCursor *c ) { return KoTextDeleteCommand::unexecute( c ); }
    KoTextCursor *unexecute( KoTextCursor *c ) { return KoTextDeleteCommand::execute( c ); }
};

/**
 * Command created when changing paragraph attributes
 */
class KoTextParagCommand : public KoTextDocCommand
{
public:
    KoTextParagCommand( KoTextDocument *d, int fParag, int lParag,
                        const QValueList<KoParagLayout> &oldParagLayouts,
                        KoParagLayout newParagLayout,
                        int /*KoParagLayout::Flags*/ flags,
                        QStyleSheetItem::Margin margin = (QStyleSheetItem::Margin)-1, bool borderOutline=false );
                        // margin is only meaningful if flags==Margins only. -1 means all.
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );
protected:
    int firstParag, lastParag;
    QValueList<KoParagLayout> m_oldParagLayouts;
    KoParagLayout m_newParagLayout;
    int m_flags;
    int m_margin;
    bool m_borderOutline;
};

/**
 * Command created when changing the default format of paragraphs.
 * This is ONLY used for counters and bullet's formatting.
 * See KoTextFormatCommand for the command used when changing the formatting of any set of characters.
 */
class KoParagFormatCommand : public KoTextDocCommand
{
public:
    KoParagFormatCommand( KoTextDocument *d, int fParag, int lParag,
                          const QValueList<KoTextFormat *> &oldFormats,
                          KoTextFormat * newFormat );
    ~KoParagFormatCommand();
    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );
protected:
    int firstParag, lastParag;
    QValueList<KoTextFormat *> m_oldFormats;
    KoTextFormat * m_newFormat;
};

/**
 * Command created when changing formatted text
 */
class KoTextFormatCommand : public KoTextDocFormatCommand
{
public:
    KoTextFormatCommand( KoTextDocument *d, int sid, int sidx, int eid, int eidx, const QMemArray<KoTextStringChar> &old, KoTextFormat *f, int fl );
    virtual ~KoTextFormatCommand();

    KoTextCursor *execute( KoTextCursor *c );
    KoTextCursor *unexecute( KoTextCursor *c );
    void resizeCustomItems();
};

#endif
