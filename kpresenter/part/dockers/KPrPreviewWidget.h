/* This file is part of the KDE project
   Copyright (C) 2008 Sven Langkamp <sven.langkamp@gmail.com>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPRPREVIEWWIDGET_H
#define KPRPREVIEWWIDGET_H

#include <QWidget>
#include <KoShape.h>
#include <QTimeLine>

class KPrPage;
class KPrPageEffect;
class KPrPageEffectRunner;

/**
 * A widget for page effect preview. It shows a transition from a black page to the current page
 */
class KPrPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    KPrPreviewWidget( QWidget* parent = 0 );
    ~KPrPreviewWidget();

    /**
     * Set a page effect
     *
     * @param pageEffect The effect which will be previewed. If 0 the preview will be x
     * @param page The current page used in the preview. If 0 the preview will be x
     */
    void setPageEffect( KPrPageEffect* pageEffect, KPrPage* page );

    /**
     * Run the current page effect. Does nothing if no page effect was set.
     */
    void runPreview();

protected:
    void paintEvent( QPaintEvent* event );

protected slots:
    void animate();

private:
    QTimeLine m_timeLine;

    KPrPageEffect* m_pageEffect;
    KPrPageEffectRunner* m_pageEffectRunner;
};

#endif /* KPRPREVIEWWIDGET_H */
