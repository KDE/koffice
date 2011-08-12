/* This file is part of the KDE project
   Copyright 2007 Montel Laurent <montel@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef PICTURE_TOOL
#define PICTURE_TOOL

#include <KToolBase.h>
#include <KJob>

class PictureShape;

class PictureTool : public KToolBase
{
    Q_OBJECT
public:
    explicit PictureTool(KCanvasBase* canvas);

    /// reimplemented from KToolBase
    virtual void paint(QPainter&, const KViewConverter&) {}

    /// reimplemented from KToolBase
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    /// reimplemented from KToolBase
    virtual void deactivate();

protected:
    /// reimplemented from KToolBase
    virtual QWidget *createOptionWidget();
    /// reimplemented from superclass
    virtual void mouseDoubleClickEvent(KPointerEvent *event);

private slots:
    void changeUrlPressed();
    void setImageData(KJob *job);

private:
    PictureShape *m_pictureshape;
};

#endif
