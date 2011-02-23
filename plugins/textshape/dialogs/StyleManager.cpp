/* This file is part of the KDE project
 * Copyright (C) 2007-2011 Thomas Zander <zander@kde.org>
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

#include "StyleManager.h"

#include "StylesWidget.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

#include <KDebug>

StyleManager::StyleManager(QWidget *parent)
        : QWidget(parent),
        m_styleManager(0),
        m_shadowStyleManager(0),
        m_selectedParagStyle(0),
        m_selectedCharStyle(0),
        m_blockSignals(false)
{
    widget.setupUi(this);
    layout()->setMargin(0);
    widget.styleTypeContainer->setVisible(false);

    connect(widget.styles, SIGNAL(paragraphStyleSelected(KoParagraphStyle *, bool)),
        this, SLOT(setParagraphStyle(KoParagraphStyle*,bool)));
    connect(widget.styles, SIGNAL(characterStyleSelected(KoCharacterStyle *, bool)),
        this, SLOT(setCharacterStyle(KoCharacterStyle*,bool)));

    connect(widget.bNew, SIGNAL(pressed()), this, SLOT(buttonNewPressed()));
    connect(widget.bDelete, SIGNAL(pressed()), this, SLOT(buttonDeletePressed()));

    connect(widget.createPage, SIGNAL(newParagraphStyle(KoParagraphStyle*)),
        this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(widget.createPage, SIGNAL(newCharacterStyle(KoCharacterStyle*)),
        this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(widget.createPage, SIGNAL(cancelled()),
        this, SLOT(toStartupScreen()));
    connect(widget.paragButton, SIGNAL(clicked(bool)),
        this, SLOT(switchStyle(bool)));
    connect(widget.charButton, SIGNAL(clicked(bool)),
        this, SLOT(switchStyle(bool)));
}

void StyleManager::toStartupScreen()
{
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
}

StyleManager::~StyleManager()
{
}

void StyleManager::setStyleManager(KoStyleManager *sm)
{
    Q_ASSERT(sm);
    if (m_styleManager == sm)
        return;
    if (m_shadowStyleManager) {
        m_alteredStyles.clear();
        delete m_shadowStyleManager;
    }
    m_styleManager = sm;
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
    connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));

    // don't operate on the origs since user might cancel later.
    m_shadowStyleManager = new KoStyleManager(this);
    QHash<KoParagraphStyle*, KoParagraphStyle*> cloneMapper; // orig -> clone
    QSet<int> seenCharStyles;

    // the default style is a bit tricky since we can't set it on the manager;
    KoParagraphStyle *defParag = m_styleManager->defaultParagraphStyle();
    KoParagraphStyle *defShadowParag = m_shadowStyleManager->defaultParagraphStyle();
    const int defId = defShadowParag->styleId();
    const int defCharId = defShadowParag->characterStyle()->styleId();
    defShadowParag->copyProperties(defParag);
    defShadowParag->setStyleId(defId); // restore id that styleManager knows it as
    defShadowParag->characterStyle()->setStyleId(defCharId); // restore
    cloneMapper.insert(defParag, defShadowParag);
    seenCharStyles << defParag->characterStyle()->styleId();

    foreach (KoParagraphStyle *ps, m_styleManager->paragraphStyles()) {
        KoParagraphStyle *cloned = ps->clone();
        m_shadowParagraphStyles.insert(cloned, ps->styleId());
        m_shadowCharacterStyles.insert(cloned->characterStyle(), ps->characterStyle()->styleId());
        seenCharStyles << ps->characterStyle()->styleId();
        if (ps == defParag) {
            cloned->setName(i18n("Document defaults"));
            widget.paragraphStylePage->setDefaultParagraphStyle(cloned);
            continue;
        }
        cloneMapper.insert(ps, cloned);
    }

    // now; need to redirect the parents
    foreach (KoParagraphStyle *ps, m_shadowParagraphStyles.keys()) {
        if (ps->parentStyle() == 0)
            continue; // the default style shows up twice in our copy so the user can edit it;
        Q_ASSERT(cloneMapper.value(ps->parentStyle()));
        ps->setParentStyle(cloneMapper.value(ps->parentStyle()));
    }
    foreach (KoParagraphStyle *ps, m_shadowParagraphStyles.keys())
        m_shadowStyleManager->add(ps);
    // last; after ids are set by the stylemanager need to redirect the 'next' (which is an int)
    foreach (KoParagraphStyle *ps, m_shadowParagraphStyles.keys()) {
        KoParagraphStyle *orig = m_styleManager->paragraphStyle(ps->nextStyle());
        if (orig) {
            KoParagraphStyle *clone = cloneMapper.value(orig);
            if (clone)
                ps->setNextStyle(clone->styleId());
        }
    }

    // character styles too!
    foreach (KoCharacterStyle *cs, m_styleManager->characterStyles()) {
        const int id = cs->styleId();
        if (seenCharStyles.contains(id))
            continue;
        KoCharacterStyle *clone = cs->clone();
        m_shadowStyleManager->add(clone);
        m_shadowCharacterStyles.insert(clone, id);
    }

    widget.styles->setStyleManager(m_shadowStyleManager);
    widget.paragraphStylePage->setParagraphStyles(m_shadowStyleManager->paragraphStyles());

    // do this at the end so the tree list can be expanded
    widget.styles->setEmbedded(true);
}

// this method deals with a style in m_styleManager
void StyleManager::setParagraphStyle(KoParagraphStyle *style)
{
    Q_ASSERT(style);
    Q_ASSERT(style->styleId());
    const int styleId = style->styleId();
    QHash<KoParagraphStyle*, int>::ConstIterator iter;
    iter = m_shadowParagraphStyles.constBegin();
    while (iter != m_shadowParagraphStyles.constEnd()) {
        if (styleId == iter.value()) {
            setParagraphStyle(iter.key(), false);
            break;
        }
        ++iter;
    }
}

void StyleManager::setParagraphStyle(KoParagraphStyle *style, bool canDelete)
{
    if (widget.charButton->isChecked()) { // show char style instead
        setCharacterStyle(style->characterStyle(), false, true);
        widget.styleTypeContainer->setVisible(true);
        m_selectedParagStyle = style;
        return;
    }

    m_selectedCharStyle = 0;
    m_selectedParagStyle = style;
    widget.characterStylePage->save();
    widget.characterStylePage->setStyle(0);
    widget.paragraphStylePage->save();

    m_alteredStyles << m_shadowParagraphStyles.value(style);
    widget.paragraphStylePage->setStyle(style);
    widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    widget.styleTypeContainer->setVisible(true);
    if (m_shadowParagraphStyles.value(style) == m_styleManager->defaultParagraphStyle()->styleId())
        canDelete = false;
    widget.bDelete->setEnabled(canDelete);
}

void StyleManager::setCharacterStyle(KoCharacterStyle *style, bool canDelete)
{
    setCharacterStyle(style, canDelete, false);
}

void StyleManager::setCharacterStyle(KoCharacterStyle *style, bool canDelete, bool partOfParag)
{
    m_selectedParagStyle = 0;
    m_selectedCharStyle = style;
    widget.paragraphStylePage->save();
    widget.paragraphStylePage->setStyle(0);
    widget.characterStylePage->save();

    m_alteredStyles << m_shadowCharacterStyles.value(style);
    widget.characterStylePage->setStyle(style);
    widget.stackedWidget->setCurrentWidget(widget.characterStylePage);
    widget.styleTypeContainer->setVisible(false);
    widget.bDelete->setEnabled(canDelete);
    widget.characterStylePage->setStyleNameVisible(!partOfParag);
}

void StyleManager::setUnit(const KoUnit &unit)
{
    widget.paragraphStylePage->setUnit(unit);
    widget.characterStylePage->setUnit(unit);
}

void StyleManager::save()
{
    m_blockSignals = true;
    widget.paragraphStylePage->save();
    widget.characterStylePage->save();

    QMap<int, int> cloneMapper; // orig to clone
    foreach (KoParagraphStyle *clone, m_shadowParagraphStyles.keys())
        cloneMapper.insert(m_shadowParagraphStyles.value(clone), clone->styleId());
    foreach (KoCharacterStyle *clone, m_shadowCharacterStyles.keys())
        cloneMapper.insert(m_shadowCharacterStyles.value(clone), clone->styleId());

    foreach (int styleId, m_alteredStyles) {
        KoParagraphStyle *orig = m_styleManager->paragraphStyle(styleId);
        if (orig) {
            KoParagraphStyle *clone = m_shadowStyleManager->paragraphStyle(cloneMapper.value(styleId));
            Q_ASSERT(clone);
            KoCharacterStyle * const oldCharStyle = orig->characterStyle();
            orig->copyProperties(clone);
            orig->setStyleId(styleId);
            // correct 'next' and 'parent'
            KoParagraphStyle *next = m_shadowStyleManager->paragraphStyle(clone->nextStyle());
            Q_ASSERT(next);
            orig->setNextStyle(m_shadowParagraphStyles.value(next));
            if (orig == m_styleManager->defaultParagraphStyle()) {
                orig->setParentStyle(0);
            } else {
                int parentId = m_shadowParagraphStyles.value(clone->parentStyle());
                if (parentId == 0) // defaultStyle
                    orig->setParentStyle(m_styleManager->defaultParagraphStyle());
                else
                    orig->setParentStyle(m_styleManager->paragraphStyle(parentId));
            }
            orig->setCharacterStyle(oldCharStyle);
            m_styleManager->alteredStyle(orig);
        } else {
            KoCharacterStyle *origc = m_styleManager->characterStyle(styleId);
            if (origc) {
                KoCharacterStyle *clone = m_shadowStyleManager->characterStyle(cloneMapper.value(styleId));
                Q_ASSERT(clone);
                origc->copyProperties(clone);
                origc->setStyleId(styleId);
                m_styleManager->alteredStyle(origc);
            }
        }
    }
    m_alteredStyles.clear();

    m_blockSignals = false;
}

void StyleManager::buttonNewPressed()
{
    widget.stackedWidget->setCurrentWidget(widget.createPage);
    widget.createPage->setFocus();
    // that widget will emit a new style which we will add using addParagraphStyle or addCharacterStyle
    widget.styleTypeContainer->setVisible(false);
}

void StyleManager::addParagraphStyle(KoParagraphStyle *style)
{
    widget.paragraphStylePage->save();
    widget.characterStylePage->save();
    widget.characterStylePage->setStyle(0);
    widget.paragraphStylePage->setStyle(0);

    if (m_blockSignals) return;

    KoCharacterStyle *cs = style->characterStyle();
    if (cs->name().isEmpty())
        cs->setName(style->name());
    addCharacterStyle(cs);

    m_shadowStyleManager->add(style);
    widget.paragraphStylePage->setParagraphStyles(m_shadowStyleManager->paragraphStyles());
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
    widget.styleTypeContainer->setVisible(false);
    setParagraphStyle(style, true);
}

void StyleManager::addCharacterStyle(KoCharacterStyle *style)
{
    if (m_blockSignals) return;

    m_shadowStyleManager->add(style);
    widget.paragraphStylePage->setParagraphStyles(m_shadowStyleManager->paragraphStyles());
    widget.stackedWidget->setCurrentWidget(widget.welcomePage);
    setCharacterStyle(style, true, false);
}

void StyleManager::buttonDeletePressed()
{
    widget.styles->deleteStyleClicked();
}

void StyleManager::removeParagraphStyle(KoParagraphStyle* style)
{
    const int id = style->styleId();
    m_alteredStyles.remove(id);

    QHash<KoParagraphStyle*, int>::ConstIterator iter = m_shadowParagraphStyles.constBegin();
    while (iter != m_shadowParagraphStyles.constEnd()) {
        if (iter.value() == id) {
            KoParagraphStyle *deletedStyle = iter.key();
            m_shadowParagraphStyles.remove(deletedStyle);
            m_shadowStyleManager->remove(deletedStyle);
            delete deletedStyle;
            break;
        }
        ++iter;
    }
    widget.paragraphStylePage->setParagraphStyles(m_shadowStyleManager->paragraphStyles());
}

void StyleManager::removeCharacterStyle(KoCharacterStyle* style)
{
    const int id = style->styleId();
    m_alteredStyles.remove(id);

    QHash<KoCharacterStyle*, int>::ConstIterator iter = m_shadowCharacterStyles.constBegin();
    while (iter != m_shadowCharacterStyles.constEnd()) {
        if (iter.value() == id) {
            KoCharacterStyle *deletedStyle = iter.key();
            m_shadowCharacterStyles.remove(deletedStyle);
            m_shadowStyleManager->remove(deletedStyle);
            delete deletedStyle;
            break;
        }
        ++iter;
    }
}

void StyleManager::switchStyle(bool on)
{
    if (!on) return;
    if (m_selectedParagStyle) {
        setParagraphStyle(m_selectedParagStyle, widget.bDelete->isEnabled());
    }
}

void StyleManager::hideSelector()
{
    widget.selectorPane->setVisible(false);
}

/* TODO
    Add a connection to the same 'name' text field when I press enter it should press the create button.
    on 'new' use the currently selected style as a template
*/

#include <StyleManager.moc>
