/* This file is part of the KDE project
 * Copyright (C)  2006-2011 Thomas Zander <zander@kde.org>
 * Copyright (C)  2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#include "KoText.h"
#include "styles/KoStyleManager.h"
#include "styles/KoStyleManager.h"
#include "changetracker/KoChangeTracker.h"
#include "KoTextShapeData.h"
#include "KoTextDocumentLayout.h"
#include "KoInlineTextObjectManager.h"

#include <KoStore.h>
#include <KoOdfReadStore.h>
#include <KoOdfReadStore.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoTextSharedLoadingData.h>
#include <KoOdfLoadingContext.h>

#include <QTextDocument>

#include <klocale.h>
#include <kdebug.h>

using namespace KoText;

QStringList KoText::underlineTypeList()
{
    QStringList lst;
    lst << i18nc("Underline Style", "None");
    lst << i18nc("Underline Style", "Single");
    lst << i18nc("Underline Style", "Double");
    return lst;
}

QStringList KoText::underlineStyleList()
{
    QStringList lst;
    lst << "_________";  // solid
    lst << "___ ___ __"; // dash
    lst << "_ _ _ _ _ _"; // dot
    lst << "___ _ ___ _"; // dash_dot
    lst << "___ _ _ ___"; // dash_dot_dot
    lst << "~~~~~~~"; // wavy lines
    return lst;
}

KoText::Tab::Tab()
        : position(0.),
        type(QTextOption::LeftTab),
        leaderType(KoCharacterStyle::NoLineType),
        leaderStyle(KoCharacterStyle::NoLineStyle),
        leaderWeight(KoCharacterStyle::AutoLineWeight),
        leaderWidth(0)
{
}

bool KoText::Tab::operator==(const Tab &other) const
{
    return other.position == position &&
           other.type == type &&
           other.delimiter == delimiter &&
           other.leaderStyle == leaderStyle &&
           other.leaderColor == leaderColor &&
           other.leaderText == leaderText ;
}

Qt::Alignment KoText::alignmentFromString(const QString &align)
{
    Qt::Alignment alignment = Qt::AlignLeft;
    if (align == "left")
        alignment = Qt::AlignLeft | Qt::AlignAbsolute;
    else if (align == "right")
        alignment = Qt::AlignRight | Qt::AlignAbsolute;
    else if (align == "start")
        alignment = Qt::AlignLeading;
    else if (align == "end")
        alignment = Qt::AlignTrailing;
    else if (align == "center")
        alignment = Qt::AlignHCenter;
    else if (align == "justify")
        alignment = Qt::AlignJustify;
    else if (align == "margins") // in tables this is effectively the same as justify
        alignment = Qt::AlignJustify;
    return alignment;
}

QString KoText::alignmentToString(Qt::Alignment alignment)
{
    QString align;
    if (alignment == (Qt::AlignLeft | Qt::AlignAbsolute))
        align = "left";
    else if (alignment == (Qt::AlignRight | Qt::AlignAbsolute))
        align = "right";
    else if (alignment == Qt::AlignLeading)
        align = "start";
    else if (alignment == Qt::AlignTrailing)
        align = "end";
    else if (alignment == Qt::AlignHCenter)
        align = "center";
    else if (alignment == Qt::AlignJustify)
        align = "justify";
    return align;
}

Qt::Alignment KoText::valignmentFromString(const QString &align)
{
    Qt::Alignment alignment = Qt::AlignTop;
    if (align == "top")
        alignment = Qt::AlignTop;
    else if (align == "center")
        alignment = Qt::AlignVCenter;
    else if (align == "bottom")
        alignment = Qt::AlignBottom;
    return alignment;
}

QString KoText::valignmentToString(Qt::Alignment alignment)
{
    QString align;
    if (alignment == (Qt::AlignTop))
        align = "top";
    else if (alignment == Qt::AlignVCenter)
        align = "center";
    else if (alignment == Qt::AlignBottom)
        align = "bottom";
    return align;
}

KoText::Direction KoText::directionFromString(const QString &writingMode)
{
    // LTR is lr-tb. RTL is rl-tb
    if (writingMode == "lr" || writingMode == "lr-tb")
        return KoText::LeftRightTopBottom;
    if (writingMode == "rl" || writingMode == "rl-tb")
        return KoText::RightLeftTopBottom;
    if (writingMode == "tb" || writingMode == "tb-rl")
        return KoText::TopBottomRightLeft;
    if (writingMode == "page")
        return KoText::InheritDirection;
    return KoText::AutoDirection;
}

QTextDocument *KoText::loadOpenDocument(const QString &filename, QTextDocument *document)
{
    KoStore *readStore = KoStore::createStore(filename, KoStore::Read, "", KoStore::Zip);
    KoOdfReadStore odfReadStore(readStore);
    QString error;
    if (!odfReadStore.loadAndParse(error)) {
        kWarning(32500) << "Parsing error : " << error;
        return 0;
    }

    KoXmlElement content = odfReadStore.contentDoc().documentElement();
    KoXmlElement realBody(KoXml::namedItemNS(content, KoXmlNS::office, "body"));
    KoXmlElement body = KoXml::namedItemNS(realBody, KoXmlNS::office, "text");

    KoStyleManager *styleManager = new KoStyleManager;
    KoChangeTracker *changeTracker = new KoChangeTracker;

    KoOdfLoadingContext odfLoadingContext(odfReadStore.styles(), odfReadStore.store());
    KoShapeLoadingContext shapeLoadingContext(odfLoadingContext, 0);
    KoTextSharedLoadingData *textSharedLoadingData = new KoTextSharedLoadingData;
    textSharedLoadingData->loadOdfStyles(shapeLoadingContext, styleManager);
    shapeLoadingContext.addSharedData(KOTEXT_SHARED_LOADING_ID, textSharedLoadingData);

    KoTextShapeData *textShapeData = new KoTextShapeData;
    if (document == 0)
        document = new QTextDocument;
    textShapeData->setDocument(document, false /* ownership */);
    KoTextDocumentLayout *layout = new KoTextDocumentLayout(textShapeData->document());
    layout->setInlineTextObjectManager(new KoInlineTextObjectManager(layout)); // required while saving
    KoTextDocument(document).setStyleManager(styleManager);
    textShapeData->document()->setDocumentLayout(layout);
    KoTextDocument(document).setChangeTracker(changeTracker);

    if (!textShapeData->loadOdf(body, shapeLoadingContext)) {
        qDebug() << "KoTextShapeData failed to load ODT";
    }

    delete readStore;
    delete textShapeData;
    return document;
}
