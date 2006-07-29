/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_GRID_MANAGER_H
#define KIS_GRID_MANAGER_H

#include <qobject.h>

#include "kis_types.h"

class KisView;
class KActionCollection;
class KToggleAction;
class KAction;

class KisGridManager : public QObject
{
    Q_OBJECT
    public:
        KisGridManager(KisView * parent);
        ~KisGridManager();
    public:
        void setup(KActionCollection * collection);
        void drawGrid(QRect wr, QPainter *p, bool openGL = false);
    public slots:
        void updateGUI();
    private slots:
        void toggleGrid();
        void fastConfig1x1();
        void fastConfig2x2();
        void fastConfig5x5();
        void fastConfig10x10();
        void fastConfig20x20();
        void fastConfig40x40();
    private:
        KisView* m_view;
        KToggleAction* m_toggleGrid;
        KAction* m_gridConfig;
        KAction* m_gridFastConfig1x1;
        KAction* m_gridFastConfig2x2;
        KAction* m_gridFastConfig5x5;
        KAction* m_gridFastConfig10x10;
        KAction* m_gridFastConfig20x20;
        KAction* m_gridFastConfig40x40;
};

#endif
