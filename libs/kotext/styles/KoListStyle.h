/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#ifndef KOLISTSTYLE_H
#define KOLISTSTYLE_H

#include <QTextFormat>
#include <QMap>
#include <QPointer>
#include <QTextList>
#include <kotext_export.h>
class StylePrivate;


/**
 * This class groups all styling-options for lists.
 * See KoParagraphStyle::setListStyle()
 * This class represents one list-level which can span several paragraphs, but
 * typically just one pargraph-style since that style can be reused on various
 * paragraphs.
 */
class  KOTEXT_EXPORT KoListStyle {
public:
    /// This list is used to specify what kind of list-style to use
    enum Style {
        /// Draw a square
        SquareItem = QTextListFormat::ListSquare,
        /// Draw a disc (filled circle)  (aka bullet)
        DiscItem = QTextListFormat::ListDisc,
        /// Draw a disc (non-filled disk)
        CircleItem = QTextListFormat::ListCircle,
        /// use arabic numbering (1, 2, 3, ...)
        DecimalItem = QTextListFormat::ListDecimal,
        /// use alpha numbering (a, b, c, ... aa, ab, ...)
        AlphaLowerItem = QTextListFormat::ListLowerAlpha,
        /// use alpha numbering (A, B, C, ... AA, AB, ...)
        UpperAlphaItem = QTextListFormat::ListUpperAlpha,
        /// don't draw a list item, just the prefix/suffix
        NoItem = 1,
        /// use lower roman counting.  (i, ii, iii, iv, ...)
        RomanLowerItem,
        /// use lower roman counting.  (I, II, III, IV, ...)
        UpperRomanItem,
        /// draw a box
        BoxItem,
        /// use an unicode char for the numbering
        CustomCharItem,
        Bengali,    ///< Bengali characters for normal 10-base counting
        Gujarati,   ///< Gujarati characters for normal 10-base counting
        Gurumukhi,  ///< Gurumukhi characters for normal 10-base counting
        Kannada,    ///< Kannada characters for normal 10-base counting
        Malayalam,  ///< Malayalam characters for normal 10-base counting
        Oriya,      ///< Oriya characters for normal 10-base counting
        Tamil,      ///< Tamil characters for normal 10-base counting
        Telugu,     ///< Telugu characters for normal 10-base counting
        Tibetan,    ///< Tibetan characters for normal 10-base counting
        Thai,       ///< Thai characters for normal 10-base counting
        Abjad,      ///< Abjad sequence.
        AbjadMinor, ///< A lesser known version of the Abjad sequence.
        ArabicAlphabet,

         // TODO look at css 3 for things like hebrew counters
         // TODO allow a bitmap 'bullet'

        Foo
    };

    /// further properties
    enum Property {
        ListItemPrefix = QTextFormat::UserProperty+1000, ///< The text to be printed before the listItem
        ListItemSuffix, ///< The text to be printed after the listItem
        StartValue,     ///< First value to use
        Level,          ///< list nesting level, is 1 or higher, or zero when implied
        DisplayLevel,   ///< show this many levels. Is always lower than the (implied) level.
        CharacterStyleId,///< CharacterStyle used for markup of the counter
        BulletCharacter,///< an int with the unicode value of the character (for CustomCharItem)
        BulletSize,     ///< size in percent relative to the height of the text
        Alignment,      ///< Alignment of the counter
        MinimumWidth    ///< The minimum width, in pt, of the listItem including the prefix/suffix.
    };

    /**
     * Constructor
     * Create a new list style which uses bulletted listitems.
     */
    KoListStyle();
    /// copy constructor
    KoListStyle(const KoListStyle &orig);

    // destuctor;
    ~KoListStyle();

    /// set the style to be used for this list.
    void setStyle(Style style);
    /// return the used style
    Style style() const;
    void setListItemPrefix(const QString &prefix);
    QString listItemPrefix() const;
    void setListItemSuffix(const QString &suffix);
    QString listItemSuffix() const;
    void setStartValue(int value);
    int startValue() const;
    void setLevel(int level);
    int level() const;
    void setDisplayLevel(int level);
    int displayLevel() const;
    void setCharacterStyleId(int id);
    int characterStyleId() const;
    void setBulletCharacter(QChar character);
    QChar bulletCharacter() const;
    void setRelativeBulletSize(int percent);
    int relativeBulletSize() const;
    void setAlignment(Qt::Alignment align);
    Qt::Alignment alignment() const;
    void setMinimumWidth(double width);
    double minimumWidth();

    /// return the name of the style.
    QString name() const;

    /// set a user-visible name on the style.
    void setName(const QString &name);

    /**
     * Apply this style to a blockFormat by copying all properties from this style
     * to the target block format.
     */
    void applyStyle(const QTextBlock &block);

    bool operator==(const KoListStyle &other) const;

    QTextList *textList(const QTextDocument *doc);

    static KoListStyle* fromTextList(QTextList *list);

protected:
    friend class KoParagraphStyle;
    void addUser();
    void removeUser();
    int userCount() const;

    void apply(const KoListStyle &other);

private:
    void setProperty(int key, const QVariant &value);
    int propertyInt(int key) const;
    bool propertyBoolean(int key) const;
    double propertyDouble(int key) const;
    QString propertyString(int key) const;

    class Private;
    Private * const d;
};

#endif
