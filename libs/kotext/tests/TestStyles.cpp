/* This file is part of the KOffice project
 * Copyright (C) 2006-2011 Thomas Zander <zander@kde.org>
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
#include "TestStyles.h"

#include <styles/KParagraphStyle.h>
#include <styles/KoStyleManager.h>
#include <styles/KoStyleManager_p.h>
#include <styles/KCharacterStyle.h>
#include <KDebug>
#include <QTextDocument>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>

void TestStyles::testStyleInheritance()
{
    KParagraphStyle style1;
    style1.setTopMargin(10.0);
    QCOMPARE(style1.topMargin(), 10.0);

    KParagraphStyle style2;
    style2.setParentStyle(&style1);

    QCOMPARE(style2.topMargin(), 10.0);
    style2.setTopMargin(20.0);
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style1.topMargin(), 10.0);

    style1.setTopMargin(15.0);
    QCOMPARE(style2.topMargin(), 20.0);
    QCOMPARE(style1.topMargin(), 15.0);

    style2.setTopMargin(15.0); // the same, resetting the difference.
    QCOMPARE(style2.topMargin(), 15.0);
    QCOMPARE(style1.topMargin(), 15.0);

    style1.setTopMargin(12.0); // parent, so both are affected
    QCOMPARE(style2.topMargin(), 12.0);
    QCOMPARE(style1.topMargin(), 12.0);
}

void TestStyles::testChangeParent()
{
    KParagraphStyle style1;
    style1.setTopMargin(10);

    KParagraphStyle style2;
    style2.setTopMargin(20);

    style2.setParentStyle(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style2.topMargin(), 20.0);

    KParagraphStyle style3;
    style3.setParentStyle(&style1);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 10.0);

    // test that separating will leave the child with exactly the same dataset
    // as it had before the inheritance
    style3.setParentStyle(0);
    QCOMPARE(style1.topMargin(), 10.0);
    QCOMPARE(style3.topMargin(), 0.0); // we hadn't explicitly set the margin on style3

    // test adding it to another will not destroy any data
    style3.setParentStyle(&style1);
    QCOMPARE(style1.topMargin(), 10.0); // from style1
    QCOMPARE(style2.topMargin(), 20.0); // from style2
    QCOMPARE(style3.topMargin(), 10.0); // inherited from style1

    // Check that style3 now starts following the parent since it does not have
    // the property set
    style3.setParentStyle(&style2);
    QCOMPARE(style3.topMargin(), 20.0); // inherited from style2
}

void TestStyles::testTabsStorage()
{
    KParagraphStyle paragStyle;

    QList<KoText::Tab> tabs;
    paragStyle.setTabPositions(tabs);
    QCOMPARE(paragStyle.tabPositions().count(), 0);

    KoText::Tab tab;
    tabs.append(tab);
    KoText::Tab tab2;
    tab2.position = 10;
    tabs.append(tab2);

    paragStyle.setTabPositions(tabs);
    QCOMPARE(paragStyle.tabPositions().count(), 2);
    QCOMPARE(paragStyle.tabPositions()[0], tab);
    QCOMPARE(paragStyle.tabPositions()[1], tab2);
}

void TestStyles::testApplyParagraphStyle()
{
    KParagraphStyle style;
    style.setStyleId(1001);

    QTextBlockFormat format;
    QCOMPARE(format.properties().count(), 0);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 1); // the styleId

    style.setName("name");
    style.setAlignment(Qt::AlignRight);
    style.applyStyle(format);
    QCOMPARE(format.properties().count(), 2);
    QCOMPARE(format.alignment(), Qt::AlignRight);
}

void TestStyles::testApplyParagraphStyleWithParent()
{
    KParagraphStyle style1;
    style1.setStyleId(1002);
    KParagraphStyle style2;
    style2.setStyleId(1003);
    KParagraphStyle style3;
    style3.setStyleId(1004);

    style3.setParentStyle(&style2);
    style2.setParentStyle(&style1);

    style1.setAlignment(Qt::AlignRight);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignRight);
    QCOMPARE(style3.alignment(), Qt::AlignRight);

    style2.setAlignment(Qt::AlignCenter);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignCenter);
    QCOMPARE(style3.alignment(), Qt::AlignCenter);

    style3.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(style1.alignment(), Qt::AlignRight);
    QCOMPARE(style2.alignment(), Qt::AlignCenter);
    QCOMPARE(style3.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);

    style1.setLeftMargin(10.);
    QCOMPARE(style1.leftMargin(), 10.);
    QCOMPARE(style2.leftMargin(), 10.);
    QCOMPARE(style3.leftMargin(), 10.);
    style2.setRightMargin(20.);
    QCOMPARE(style1.rightMargin(), 0.);
    QCOMPARE(style2.rightMargin(), 20.);
    QCOMPARE(style3.rightMargin(), 20.);

    // now actually apply it.
    QTextBlockFormat format;
    style1.applyStyle(format);
    QCOMPARE(format.properties().count(), 3);
    QCOMPARE(format.alignment(), Qt::AlignRight);
    QCOMPARE(format.property(KParagraphStyle::StyleId).toInt(), 1002);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 0.);

    format = QTextBlockFormat(); // clear
    style2.applyStyle(format);
    QCOMPARE(format.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignCenter);
    QCOMPARE(format.property(KParagraphStyle::StyleId).toInt(), 1003);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 20.);

    format = QTextBlockFormat(); // clear
    style3.applyStyle(format);
    QCOMPARE(format.properties().count(), 4);
    QCOMPARE(format.alignment(), Qt::AlignLeft | Qt::AlignAbsolute);
    QCOMPARE(format.property(KParagraphStyle::StyleId).toInt(), 1004);
    QCOMPARE(format.leftMargin(), 10.);
    QCOMPARE(format.rightMargin(), 20.);
}

void TestStyles::testCopyParagraphStyle()
{
    KParagraphStyle style1;
    KParagraphStyle style2;
    style2.setParentStyle(&style1);

    style1.setLeftMargin(10.);
    style1.setRightMargin(30.);
    style2.setRightMargin(20.);

    KParagraphStyle newStyle;
    newStyle.copyProperties(&style2);
    QCOMPARE(newStyle.leftMargin(), 10.);
    QCOMPARE(newStyle.rightMargin(), 20.);
}

void TestStyles::testUnapplyStyle()
{
    // in this test we should avoid testing any of the hardcodedDefaultProperties; see KCharacterStyle for details!
    KParagraphStyle headers;
    headers.characterStyle()->setFontOverline(true);
    headers.characterStyle()->setFontWeight(QFont::Bold);
    headers.setAlignment(Qt::AlignCenter);
    KParagraphStyle head1;
    head1.setParentStyle(&headers);
    head1.setLeftMargin(40);

    QTextDocument doc;
    doc.setPlainText("abc");
    QTextBlock block = doc.begin();
    head1.applyStyle(block);

    QTextCursor cursor(block);
    QTextBlockFormat bf = cursor.blockFormat();
    QCOMPARE(bf.alignment(), Qt::AlignCenter);
    QCOMPARE(bf.leftMargin(), 40.);
    QTextCharFormat cf = cursor.charFormat();
    QCOMPARE(cf.fontOverline(), true);

    head1.unapplyStyle(block);
    bf = cursor.blockFormat();
    QCOMPARE(bf.hasProperty(QTextFormat::BlockAlignment), false);
    QCOMPARE(bf.hasProperty(QTextFormat::BlockLeftMargin), false);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);

    doc.clear();
    block = doc.begin();
    head1.applyStyle(block);
    bf = cursor.blockFormat();
    QCOMPARE(bf.alignment(), Qt::AlignCenter);
    QCOMPARE(bf.leftMargin(), 40.);
    cf = cursor.charFormat();
    QCOMPARE(cf.fontOverline(), true);

    head1.unapplyStyle(block);
    bf = cursor.blockFormat();
    QCOMPARE(bf.hasProperty(QTextFormat::BlockAlignment), false);
    QCOMPARE(bf.hasProperty(QTextFormat::BlockLeftMargin), false);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);

    doc.setHtml("bla bla<i>italic</i>enzo");
    block = doc.begin();
    head1.applyStyle(block);
    bf = cursor.blockFormat();
    QCOMPARE(bf.alignment(), Qt::AlignCenter);
    QCOMPARE(bf.leftMargin(), 40.);
    cf = cursor.charFormat();
    QCOMPARE(cf.fontOverline(), true);

    head1.unapplyStyle(block);
    cursor.setPosition(0);
    bf = cursor.blockFormat();
    QCOMPARE(bf.hasProperty(QTextFormat::BlockAlignment), false);
    QCOMPARE(bf.hasProperty(QTextFormat::BlockLeftMargin), false);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);

    cursor.setPosition(8);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.fontItalic(), true);
    cursor.setPosition(13);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.fontItalic(), true);

    cursor.setPosition(14);
    cf = cursor.charFormat();
    QCOMPARE(cf.hasProperty(QTextFormat::FontOverline), false);
    QCOMPARE(cf.hasProperty(QTextFormat::FontWeight), false);
    QCOMPARE(cf.hasProperty(QTextFormat::FontItalic), false);
}

void TestStyles::testChangeManagedStyle()
{
    KoStyleManager manager;
    KParagraphStyle *p1 = new KParagraphStyle(&manager);
    p1->setName("Headers");

    KParagraphStyle *p2 = new KParagraphStyle(&manager);
    p2->setName("Head1");
    p2->setParentStyle(p1);
    KCharacterStyle *charStyle = p1->characterStyle();
    charStyle->setUnderlineColor(Qt::red);

    manager.add(p2);

    QVERIFY(manager.paragraphStyles().contains(p1));
    QVERIFY(manager.paragraphStyles().contains(p2));

    // register a qtextdocument and insert a parag following the second style.
    QTextDocument myDoc;
    manager.add(&myDoc);
    QTextCursor cursor(&myDoc);
    cursor.insertText("some text\n");
    QTextBlock block = myDoc.begin();
    p2->applyStyle(block);

    cursor.setPosition(6);
    QTextCharFormat fmt = cursor.charFormat();
    QVERIFY(fmt.hasProperty(QTextFormat::TextUnderlineColor));

    // remove a property (like underline color) on the parent style and call alteredStyle()
    // on the manager. Then check if the property has been removed on the doc.
    charStyle->clearForeground();
    manager.alteredStyle(charStyle);
    manager.priv()->updateAlteredStyles();

    fmt = cursor.charFormat();

    QVERIFY(! fmt.hasProperty(QTextFormat::TextUnderlineColor));
}

void TestStyles::testModifiedParag()
{
    // if the user modified some paragraph properties on a block and the style is later
    // changed I expect the modified properties on the block to not be lost.

    KoStyleManager manager;
    KParagraphStyle *defaultStyle = manager.defaultParagraphStyle();
    defaultStyle->setLineHeightPercent(140);

    KParagraphStyle *myStyle = new KParagraphStyle();
    myStyle->setName("myStyle");
    manager.add(myStyle);
    QCOMPARE(myStyle->parentStyle(), defaultStyle);

    QTextDocument doc;
    doc.setPlainText("abc");
    QTextBlock block = doc.begin();
    myStyle->applyStyle(block);
    manager.add(&doc);

    QTextBlockFormat format = block.blockFormat();
    QCOMPARE(format.property(KParagraphStyle::StyleId).toInt(), myStyle->styleId());

    // Make the block have a different line spacing compared to the styles.
    QTextBlockFormat lineSpacing;
    lineSpacing.setProperty(KParagraphStyle::PercentLineHeight, 200);
    QTextCursor cursor(&doc);
    cursor.mergeBlockFormat(lineSpacing);

    format = block.blockFormat();
    QCOMPARE(format.property(KParagraphStyle::StyleId).toInt(), myStyle->styleId());
    QCOMPARE(format.property(KParagraphStyle::PercentLineHeight).toInt(), 200);

    // change something like top-margin on child style and make sure the parag gets updatd.
    myStyle->setTopMargin(33);
    manager.alteredStyle(myStyle); // mark as dirty, but change is delayed with singleshot
    manager.priv()->updateAlteredStyles(); // don't wait for the singleshot.

    // check that the block still has the old linespacing we set on the block.
    format = block.blockFormat();
    QCOMPARE(format.property(KParagraphStyle::StyleId).toInt(), myStyle->styleId());
    QCOMPARE(format.property(KParagraphStyle::PercentLineHeight).toInt(), 200);
    // check that the block got the new top-margin too.
    QCOMPARE(format.property(QTextFormat::BlockTopMargin).toReal(), 33.);
}

QTEST_KDEMAIN(TestStyles, GUI)
#include <TestStyles.moc>
