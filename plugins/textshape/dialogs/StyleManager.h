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

#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <ui_StyleManager.h>

#include <QWidget>

class KoStyleManager;
class KParagraphStyle;
class KCharacterStyle;

class StyleManager : public QWidget
{
    Q_OBJECT
public:
    StyleManager(QWidget *parent = 0);
    ~StyleManager();

    void setStyleManager(KoStyleManager *sm);

    void setUnit(const KUnit &unit);

    // use the widget without the style selector (and new/delete buttons)
    void hideSelector();

    /// handle opening a style from the passed in styleManager
    void setParagraphStyle(KParagraphStyle *style);

public slots:
    void save();

private slots:
    void buttonNewPressed();
    void buttonDeletePressed();
    void addParagraphStyle(KParagraphStyle*);
    void addCharacterStyle(KCharacterStyle*);
    void removeParagraphStyle(KParagraphStyle*);
    void removeCharacterStyle(KCharacterStyle*);
    /// handle opening a 'cloned' style
    void setParagraphStyle(KParagraphStyle *style, bool canDelete);
    /// handle opening a 'cloned' style
    void setCharacterStyle(KCharacterStyle *style, bool canDelete);
    // switches between paragraph and character styles
    void switchStyle(bool on);
    void toStartupScreen();

private:
    void setCharacterStyle(KCharacterStyle *style, bool canDelete, bool partOfParag);

    Ui::StyleManager widget;
    KoStyleManager *m_styleManager;
    KoStyleManager *m_shadowStyleManager;

    QHash<KParagraphStyle*, int> m_shadowParagraphStyles; // shadow to orig Id
    QHash<KCharacterStyle*, int> m_shadowCharacterStyles; // shadow to orig Id
    QSet<int> m_alteredStyles;

    KParagraphStyle *m_selectedParagStyle;
    KCharacterStyle *m_selectedCharStyle;

    bool m_blockSignals;
};

#endif
