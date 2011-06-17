/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "ListItemsHelper.h"

#include <KTextBlockData.h>
#include <KListStyle.h>
#include <KParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoList.h>

#include <KDebug>
#include <KLocale>
#include <QTextList>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
using namespace Lists;

QString Lists::intToRoman(int n)
{
    static const QByteArray RNUnits[] = {"", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"};
    static const QByteArray RNTens[] = {"", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc"};
    static const QByteArray RNHundreds[] = {"", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm"};
    static const QByteArray RNThousands[] = {"", "m", "mm", "mmm"};

    if (n <= 0) {
        kWarning(32500) << "intToRoman called with negative number: n=" << n;
        return QString::number(n);
    }
    return QString::fromLatin1(RNThousands[(n / 1000)] + RNHundreds[(n / 100) % 10 ] +
                               RNTens[(n / 10) % 10 ] + RNUnits[(n) % 10 ]);
}

QString Lists::intToAlpha(int n, Capitalisation caps, bool letterSynchronization)
{
    const char offset = caps == Uppercase ? 'A' : 'a';
    QString answer;
    if (letterSynchronization) {
        int digits = 1;
        for (; n > 26; n -= 26)
            digits += 1;
        for (int i = 0; i < digits; i++)
            answer.prepend(QChar(offset + n - 1));
        return answer;
    } else {
        char bottomDigit;
        while (n > 26) {
            bottomDigit = (n - 1) % 26;
            n = (n - 1) / 26;
            answer.prepend(QChar(offset + bottomDigit));
        }
    }
    answer.prepend(QChar(offset + n - 1));
    return answer;
}

QString Lists::intToScript(int n, KListStyle::Style type)
{
    // 10-base
    static const int bengali = 0x9e6;
    static const int gujarati = 0xae6;
    static const int gurumukhi = 0xa66;
    static const int kannada = 0xce6;
    static const int malayalam = 0xd66;
    static const int oriya = 0xb66;
    static const int tamil = 0x0be6;
    static const int telugu = 0xc66;
    static const int tibetan = 0xf20;
    static const int thai = 0xe50;

    int offset;
    switch (type) {
    case KListStyle::Bengali:
        offset = bengali;
        break;
    case KListStyle::Gujarati:
        offset = gujarati;
        break;
    case KListStyle::Gurumukhi:
        offset = gurumukhi;
        break;
    case KListStyle::Kannada:
        offset = kannada;
        break;
    case KListStyle::Malayalam:
        offset = malayalam;
        break;
    case KListStyle::Oriya:
        offset = oriya;
        break;
    case KListStyle::Tamil:
        offset = tamil;
        break;
    case KListStyle::Telugu:
        offset = telugu;
        break;
    case KListStyle::Tibetan:
        offset = tibetan;
        break;
    case KListStyle::Thai:
        offset = thai;
        break;
    default:
        return QString::number(n);
    }
    QString answer;
    while (n > 0) {
        answer.prepend(QChar(offset + n % 10));
        n = n / 10;
    }
    return answer;
}

QString Lists::intToScriptList(int n, KListStyle::Style type)
{
    // 1 time Sequences
    // note; the leading X is to make these 1 based.
    static const char* Abjad[] = { "أ", "ب", "ج", "د", "ﻫ", "و", "ز", "ح", "ط", "ي", "ك", "ل", "م",
                                   "ن", "س", "ع", "ف", "ص", "ق", "ر", "ش", "ت", "ث", "خ", "ذ", "ض", "ظ", "غ"
                                 };
    static const char* Abjad2[] = { "ﺃ", "ﺏ", "ﺝ", "ﺩ", "ﻫ", "ﻭ", "ﺯ", "ﺡ", "ﻁ", "ﻱ", "ﻙ", "ﻝ", "ﻡ",
                                    "ﻥ", "ﺹ", "ﻉ", "ﻑ", "ﺽ", "ﻕ", "ﺭ", "ﺱ", "ﺕ", "ﺙ", "ﺥ", "ﺫ", "ﻅ", "ﻍ", "ﺵ"
                                  };
    static const char* ArabicAlphabet[] = {"ا", "ب", "ت", "ث", "ج", "ح", "خ", "د", "ذ", "ر", "ز",
                                           "س", "ش", "ص", "ض", "ط", "ظ", "ع", "غ", "ف", "ق", "ك", "ل", "م", "ن", "ه", "و", "ي"
                                          };

    /*
    // see this page for the 10, 100, 1000 etc http://en.wikipedia.org/wiki/Chinese_numerals
    static const char* chinese1[] = { '零','壹','貳','叄','肆','伍','陸','柒','捌','玖' };
    static const char* chinese2[] = { '〇','一','二','三','四','五','六','七','八','九' };

    TODO: http://en.wikipedia.org/wiki/Korean_numerals
    http://en.wikipedia.org/wiki/Japanese_numerals
    'http://en.wikipedia.org/wiki/Hebrew_numerals'
    'http://en.wikipedia.org/wiki/Armenian_numerals'
    'http://en.wikipedia.org/wiki/Greek_numerals'
    'http://en.wikipedia.org/wiki/Cyrillic_numerals'
    'http://en.wikipedia.org/wiki/Sanskrit_numerals'
    'http://en.wikipedia.org/wiki/Ge%27ez_alphabet#Numerals'
    'http://en.wikipedia.org/wiki/Abjad_numerals'
    */

    switch (type) {
    case KListStyle::Abjad:
        if (n > 22) return "*";
        return QString::fromUtf8(Abjad[n-1]);
    case KListStyle::AbjadMinor:
        if (n > 22) return "*";
        return QString::fromUtf8(Abjad2[n-1]);
    case KListStyle::ArabicAlphabet:
        if (n > 28) return "*";
        return QString::fromUtf8(ArabicAlphabet[n-1]);
    default:
        return QString::number(n);
    }
}

QList<ListStyleItem> Lists::genericListStyleItems()
{
    QList<ListStyleItem> answer;
    answer.append(ListStyleItem(i18nc("Text list-style", "None"), KListStyle::None));
    answer.append(ListStyleItem(i18n("Arabic"), KListStyle::DecimalItem));
    answer.append(ListStyleItem(i18n("Lower Alphabetical"), KListStyle::AlphaLowerItem));
    answer.append(ListStyleItem(i18n("Upper Alphabetical"), KListStyle::UpperAlphaItem));
    answer.append(ListStyleItem(i18n("Lower Roman"), KListStyle::RomanLowerItem));
    answer.append(ListStyleItem(i18n("Upper Roman"), KListStyle::UpperRomanItem));
    answer.append(ListStyleItem(i18n("Disc Bullet"), KListStyle::DiscItem));
    answer.append(ListStyleItem(i18n("Square Bullet"), KListStyle::SquareItem));
    answer.append(ListStyleItem(i18n("Box Bullet"), KListStyle::BoxItem));
    answer.append(ListStyleItem(i18n("Rhombus Bullet"), KListStyle::RhombusItem));
    answer.append(ListStyleItem(i18n("Circle Bullet"), KListStyle::CircleItem));
    answer.append(ListStyleItem(i18n("Check Mark Bullet"), KListStyle::HeavyCheckMarkItem));
    answer.append(ListStyleItem(i18n("Ballot X Bullet"), KListStyle::BallotXItem));
    answer.append(ListStyleItem(i18n("Rightwards Arrow Bullet"), KListStyle::RightArrowItem));
    answer.append(ListStyleItem(i18n("Rightwards Arrow Head Bullet"), KListStyle::RightArrowHeadItem));
    return answer;
}

QList<ListStyleItem> Lists::otherListStyleItems()
{
    QList<ListStyleItem> answer;
    answer.append(ListStyleItem(i18n("Bengali"), KListStyle::Bengali));
    answer.append(ListStyleItem(i18n("Gujarati"), KListStyle::Gujarati));
    answer.append(ListStyleItem(i18n("Gurumukhi"), KListStyle::Gurumukhi));
    answer.append(ListStyleItem(i18n("Kannada"), KListStyle::Kannada));
    answer.append(ListStyleItem(i18n("Malayalam"), KListStyle::Malayalam));
    answer.append(ListStyleItem(i18n("Oriya"), KListStyle::Oriya));
    answer.append(ListStyleItem(i18n("Tamil"), KListStyle::Tamil));
    answer.append(ListStyleItem(i18n("Telugu"), KListStyle::Telugu));
    answer.append(ListStyleItem(i18n("Tibetan"), KListStyle::Tibetan));
    answer.append(ListStyleItem(i18n("Thai"), KListStyle::Thai));
    answer.append(ListStyleItem(i18n("Abjad"), KListStyle::Abjad));
    answer.append(ListStyleItem(i18n("AbjadMinor"), KListStyle::AbjadMinor));
    answer.append(ListStyleItem(i18n("ArabicAlphabet"), KListStyle::ArabicAlphabet));
    return answer;
}

// ------------------- ListItemsHelper ------------
/// \internal helper class for calculating text-lists prefixes and indents
ListItemsHelper::ListItemsHelper(QTextList *textList, const QFont &font)
        : m_textList(textList),
        m_fm(font, textList->document()->documentLayout()->paintDevice()),
        m_displayFont(font)
{
}

void ListItemsHelper::recalculate()
{
    //kDebug(32500);
    const QTextListFormat format = m_textList->format();
    const KListStyle::Style listStyle = static_cast<KListStyle::Style>(m_textList->format().style());

    const QString prefix = format.stringProperty(KListStyle::ListItemPrefix);
    const QString suffix = format.stringProperty(KListStyle::ListItemSuffix);
    const int level = format.intProperty(KListStyle::Level);
    int dp = format.intProperty(KListStyle::DisplayLevel);
    if (dp > level)
        dp = level;
    const int displayLevel = dp ? dp : 1;

    int startValue = 1;
    if (format.hasProperty(KListStyle::StartValue))
        startValue = format.intProperty(KListStyle::StartValue);
    if (format.boolProperty(KListStyle::ContinueNumbering)) {
        // Look for the index of a previous list of the same numbering style and level
        for (QTextBlock tb = m_textList->item(0).previous(); tb.isValid(); tb = tb.previous()) {
            if (!tb.textList() || tb.textList() == m_textList)
                continue; // no list here or it's the same list; keep looking

            QTextListFormat otherFormat = tb.textList()->format();
            if (otherFormat.intProperty(KListStyle::Level) != level)
                break; // found a different list but of a different level

            if (otherFormat.style() == format.style()) {
                if (KTextBlockData *data = dynamic_cast<KTextBlockData *>(tb.userData()))
                    startValue = data->counterIndex() + 1; // Start from previous list value + 1
            }

            break;
        }
    }

    int index = startValue;
    QList<QTextList*> sublistsToRecalculate;
    qreal width = format.doubleProperty(KListStyle::MinimumWidth);
    for (int i = 0; i < m_textList->count(); i++) {
        QTextBlock tb = m_textList->item(i);
        //kDebug(32500) <<" *" << tb.text();
        KTextBlockData *data = dynamic_cast<KTextBlockData*>(tb.userData());
        if (!data) {
            data = new KTextBlockData();
            tb.setUserData(data);
        }
        QTextBlockFormat blockFormat = tb.blockFormat();

        if (blockFormat.boolProperty(KParagraphStyle::UnnumberedListItem)
            || blockFormat.boolProperty(KParagraphStyle::IsListHeader)) {
            data->setCounterText(QString());
            data->setPartialCounterText(QString());
            continue;
        }

        if (blockFormat.boolProperty(KParagraphStyle::RestartListNumbering))
            index = format.intProperty(KListStyle::StartValue);
        const int paragIndex = blockFormat.intProperty(KParagraphStyle::ListStartValue);
        if (paragIndex > 0)
            index = paragIndex;

        //check if this is the first of this level meaning we should start from startvalue
        QTextBlock b = tb.previous();
        for (;b.isValid(); b = b.previous()) {
            if (b.textList() == m_textList)
                break; // all fine
            if (b.textList() == 0)
                continue;
            QTextListFormat otherFormat = b.textList()->format();
            if (otherFormat.style() != format.style())
                continue; // uninteresting for us
            if (b.textList()->format().intProperty(KListStyle::Level) < level) {
                index = startValue;
                break;
            }
        }

        QString item;
        if (displayLevel > 1) {
            int checkLevel = level;
            int tmpDisplayLevel = displayLevel;
            for (QTextBlock b = tb.previous(); tmpDisplayLevel > 1 && b.isValid(); b = b.previous()) {
                if (b.textList() == 0)
                    continue;
                QTextListFormat lf = b.textList()->format();
                if (lf.style() != format.style())
                    continue; // uninteresting for us
                const int otherLevel  = lf.intProperty(KListStyle::Level);
                if (checkLevel <= otherLevel)
                    continue;
                /*if(needsRecalc(b->textList())) {
                      TODO
                  } */
                KTextBlockData *otherData = dynamic_cast<KTextBlockData*>(b.userData());
                if (! otherData) {
                    kWarning(32500) << "Missing KTextBlockData, Skipping textblock";
                    continue;
                }
                if (tmpDisplayLevel - 1 < otherLevel) { // can't just copy it fully since we are
                    // displaying less then the full counter
                    item += otherData->partialCounterText();
                    tmpDisplayLevel--;
                    checkLevel--;
                    for (int i = otherLevel + 1; i < level; i++) {
                        tmpDisplayLevel--;
                        item += ".1"; // add missing counters.
                    }
                } else { // just copy previous counter as prefix
                    QString otherPrefix = lf.stringProperty(KListStyle::ListItemPrefix);
                    QString otherSuffix = lf.stringProperty(KListStyle::ListItemSuffix);
                    QString pureCounter = otherData->counterText().mid(otherPrefix.size());
                    pureCounter = pureCounter.left(pureCounter.size() - otherSuffix.size());
                    item += pureCounter;
                    for (int i = otherLevel + 1; i < level; i++)
                        item += ".1"; // add missing counters.
                    tmpDisplayLevel = 0;
                    break;
                }
            }
            for (int i = 1; i < tmpDisplayLevel; i++)
                item = "1." + item; // add missing counters.
        }

        if ((listStyle == KListStyle::DecimalItem || listStyle == KListStyle::AlphaLowerItem ||
                listStyle == KListStyle::UpperAlphaItem ||
                listStyle == KListStyle::RomanLowerItem ||
                listStyle == KListStyle::UpperRomanItem) &&
                !(item.isEmpty() || item.endsWith('.') || item.endsWith(' '))) {
            item += '.';
        }
        bool calcWidth = true;
        QString partialCounterText;
        switch (listStyle) {
        case KListStyle::DecimalItem:
            partialCounterText = QString::number(index);
            break;
        case KListStyle::AlphaLowerItem:
            partialCounterText = intToAlpha(index, Lowercase,
                                            m_textList->format().boolProperty(KListStyle::LetterSynchronization));
            break;
        case KListStyle::UpperAlphaItem:
            partialCounterText = intToAlpha(index, Uppercase,
                                            m_textList->format().boolProperty(KListStyle::LetterSynchronization));
            break;
        case KListStyle::RomanLowerItem:
            partialCounterText = intToRoman(index);
            break;
        case KListStyle::UpperRomanItem:
            partialCounterText = intToRoman(index).toUpper();
            break;
        case KListStyle::SquareItem:
        case KListStyle::DiscItem:
        case KListStyle::CircleItem:
        case KListStyle::HeavyCheckMarkItem:
        case KListStyle::BallotXItem:
        case KListStyle::RightArrowItem:
        case KListStyle::RightArrowHeadItem:
        case KListStyle::RhombusItem:
        case KListStyle::BoxItem: {
            calcWidth = false;
            item = ' ';
            width = m_displayFont.pointSizeF();
            int percent = format.intProperty(KListStyle::BulletSize);
            if (percent > 0)
                width = width * (percent / 100.0);
            break;
        }
        case KListStyle::CustomCharItem:
            calcWidth = false;
            if (format.intProperty(KListStyle::BulletCharacter))
                item = QString(QChar(format.intProperty(KListStyle::BulletCharacter)));
            width = m_fm.width(item);
            break;
        case KListStyle::None:
            calcWidth = false;
            width =  0.0;
            break;
        case KListStyle::Bengali:
        case KListStyle::Gujarati:
        case KListStyle::Gurumukhi:
        case KListStyle::Kannada:
        case KListStyle::Malayalam:
        case KListStyle::Oriya:
        case KListStyle::Tamil:
        case KListStyle::Telugu:
        case KListStyle::Tibetan:
        case KListStyle::Thai:
            partialCounterText = intToScript(index, listStyle);
            break;
        case KListStyle::Abjad:
        case KListStyle::ArabicAlphabet:
        case KListStyle::AbjadMinor:
            partialCounterText = intToScriptList(index, listStyle);
            break;
        case KListStyle::ImageItem:
            calcWidth = false;
            width = qMax(format.doubleProperty(KListStyle::Width), (qreal)1.0);
            break;
        default:  // others we ignore.
            calcWidth = false;
        }

        data->setCounterIsImage(listStyle == KListStyle::ImageItem);
        data->setPartialCounterText(partialCounterText);
        data->setCounterIndex(index);
        item += partialCounterText;
        if (calcWidth)
            width = qMax(width, m_fm.width(item));
        data->setCounterText(prefix + item + suffix);
        index++;

        // have to recalculate any sublists under this element too
        QTextBlock nb = tb.next();
        while (nb.isValid() && nb.textList() == 0)
            nb = nb.next();
        if (nb.isValid()) {
            QTextListFormat lf = nb.textList()->format();
            if ((lf.style() == format.style())
              && nb.textList()->format().intProperty(KListStyle::Level) > level) {
                // this is a sublist
                // have to remember to recalculate this list after the current level is done
                // cant do it right away since the sublist's prefix text is dependant on this level
                sublistsToRecalculate.append(nb.textList());
            }
        }
    }

    for (int i = 0; i < sublistsToRecalculate.count(); i++) {
        ListItemsHelper lih(sublistsToRecalculate.at(i), m_displayFont);
        lih.recalculate();
    }

    qreal counterSpacing = 0;
    if (listStyle != KListStyle::None)
        counterSpacing = qMax(format.doubleProperty(KListStyle::MinimumDistance), m_fm.width(' '));
    width += m_fm.width(prefix + suffix); // same for all
    if (listStyle != KListStyle::None)
        width = qMax(format.doubleProperty(KListStyle::MinimumWidth), width);
    for (int i = 0; i < m_textList->count(); i++) {
        QTextBlock tb = m_textList->item(i);
        KTextBlockData *data = dynamic_cast<KTextBlockData*>(tb.userData());
        Q_ASSERT(data);
        data->setCounterWidth(width);
        data->setCounterSpacing(counterSpacing);
        //kDebug(32500) << data->counterText() <<"" << tb.text();
        //kDebug(32500) <<"    setCounterWidth:" << width;
    }
    //kDebug(32500);
}

// static
bool ListItemsHelper::needsRecalc(QTextList *textList)
{
    Q_ASSERT(textList);
    QTextBlock tb = textList->item(0);
    KTextBlockData *data = dynamic_cast<KTextBlockData*>(tb.userData());
    if (data == 0)
        return true;
    return !data->hasCounterData();
}
