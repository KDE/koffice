/* This file is part of the KDE project
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Jim Courtiau <jeremy.courtiau@gmail.com>
 * Copyright (C) 2009 Alexia Allanic <alexia_allanic@yahoo.fr>
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
 * Copyright (C) 2009 Jérémy Lugagne <jejewindsurf@hotmail.com>
 * Copyright (C) 2009 Johann Hingue <yoan1703@hotmail.fr>
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
#ifndef SCPRESENTATIONTOOL_H
#define SCPRESENTATIONTOOL_H

#include <KToolBase.h>

#include <QSet>

class QFrame;
class KEventAction;
class SCViewModePresentation;
class SCPresentationToolWidget;
class SCPresentationStrategyBase;
class SCPresentationToolAdaptor;

/// The tool used for presentations
class SCPresentationTool : public KToolBase
{
    Q_OBJECT
public:
    explicit SCPresentationTool(SCViewModePresentation &viewMode);
    ~SCPresentationTool();

    void paint(QPainter &painter, const KViewConverter &converter);

    SCPresentationStrategyBase *strategy();
    SCViewModePresentation &viewModePresentation();

    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    virtual void deactivate();


    // TODO
    /*  Showcase and libKoPA did something weird here; the canvas forwards all
        events to the view mode which then calls the tool.
        The proper way is for the canvas to use the proxy *after* having done any
        viewmode conversions.

        This bug only got in due to the following event handlers being public, while
        flake has them protected.  Lets review the design, it would be good to make
        these protected again and to make showcase use the toolproxy too.
     */
    virtual void mousePressEvent(KPointerEvent *event);
    virtual void mouseDoubleClickEvent(KPointerEvent *event);
    virtual void mouseMoveEvent(KPointerEvent *event);
    virtual void mouseReleaseEvent(KPointerEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void wheelEvent(KPointerEvent * event);

public slots:
    void highlightPresentation();
    void drawOnPresentation();
    void blackPresentation();
    void normalPresentation();

private:
    void finishEventActions();
    void switchStrategy(SCPresentationStrategyBase * strategy);
    bool eventFilter(QObject * obj, QEvent * event);

    /**
     * Returns true if shape is a TextShape and event->point is over hyperlink
     * @param event the mouse event
     * @param shape the shape fhich is searched for hyperlink
     * @param hyperLink the string which is filled with hyperlink url
     */
    bool checkHyperlink(KPointerEvent *event, KShape * shape, QString &hyperLink);

    /**
     * Runs url string defined inside hyperlink
     * @param hyperLink the hyperlink string
     */
    void runHyperlink(QString hyperLink);

    SCViewModePresentation &m_viewMode;
    QSet<KEventAction *> m_eventActions;

    SCPresentationToolWidget * m_presentationToolWidget;
    QFrame * m_frame;
    SCPresentationStrategyBase * m_strategy;
    SCPresentationToolAdaptor *m_bus;
    friend class SCPresentationStrategyBase;
};

#endif /* SCPRESENTATIONTOOL_H */
