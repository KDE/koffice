/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <KoBorder.h>
#include <KoZoomHandler.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <QPainter>
#include <QWidget>
#include <q3grid.h>
//Added by qt3to4:
#include <QPaintEvent>

class MyWidget : public QWidget
{
public:
    MyWidget(KoZoomHandler* _zh, QWidget* parent)
            : QWidget(parent), m_zh(_zh) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    QSize sizeHint() const {
        return QSize(150, 150);
    }
    QSize minimumSizeHint() const {
        return sizeHint();
    }

    KoBorder m_leftBorder;
    KoBorder m_rightBorder;
    KoBorder m_topBorder;
    KoBorder m_bottomBorder;

protected:
    virtual void paintEvent(QPaintEvent*) {
        QPainter p(this);
        QRect rect(10, 10, 100, 100);
        KoBorder::drawBorders(p, m_zh, rect, m_leftBorder,
                              m_rightBorder, m_topBorder, m_bottomBorder,
                              0, QPen());
        // Show the corners of the rect - they must apppear _inside_ the borders.
        p.setPen(Qt::black);
        p.drawPoint(rect.topLeft());
        p.drawPoint(rect.topRight());
        p.drawPoint(rect.bottomRight());
        p.drawPoint(rect.bottomLeft());
    }
private:
    KoZoomHandler* m_zh;
};

int main(int argc, char ** argv)
{
    KCmdLineArgs::init(argc, argv, "kobordertest", 0, KLocalizedString(), 0, KLocalizedString());
    KApplication app;

    KoZoomHandler* zh = new KoZoomHandler();

    Q3Grid* grid = new Q3Grid(2, Qt::Horizontal, 0L); { // 2 columns
        // First square
        MyWidget* w = new MyWidget(zh, grid);
        w->m_leftBorder.setPenWidth(6);
        w->m_leftBorder.color = Qt::red;
        w->m_rightBorder.setPenWidth(9);
        w->m_rightBorder.color = Qt::red;
        w->m_topBorder.setPenWidth(11);
        w->m_topBorder.color = Qt::blue;
        w->m_bottomBorder.setPenWidth(13);
        w->m_bottomBorder.color = Qt::green;
    }
    {
        // Second square, with opposite (odd/even-wise) widths
        MyWidget* w = new MyWidget(zh, grid);
        w->m_leftBorder.setPenWidth(7);
        w->m_leftBorder.color = Qt::red;
        w->m_rightBorder.setPenWidth(8);
        w->m_rightBorder.color = Qt::red;
        w->m_topBorder.setPenWidth(10);
        w->m_topBorder.color = Qt::blue;
        w->m_bottomBorder.setPenWidth(12);
        w->m_bottomBorder.color = Qt::green;
    }
    {
        // Third square, with double borders
        MyWidget* w2 = new MyWidget(zh, grid);
        w2->m_leftBorder.setPenWidth(2);
        w2->m_leftBorder.setStyle(KoBorder::DOUBLE_LINE);
        w2->m_rightBorder.setPenWidth(6);
        w2->m_rightBorder.color = Qt::red;
        w2->m_rightBorder.setStyle(KoBorder::DOUBLE_LINE);
        w2->m_topBorder.setPenWidth(4);
        w2->m_topBorder.color = Qt::blue;
        w2->m_topBorder.setStyle(KoBorder::DOUBLE_LINE);
        w2->m_bottomBorder.setPenWidth(6);
        w2->m_bottomBorder.color = Qt::green;
        w2->m_bottomBorder.setStyle(KoBorder::DOUBLE_LINE);
    }
    {
        // Fourth square, with double borders
        MyWidget* w2 = new MyWidget(zh, grid);
        w2->m_leftBorder.setPenWidth(1);
        w2->m_leftBorder.setStyle(KoBorder::DOUBLE_LINE);
        w2->m_rightBorder.setPenWidth(5);
        w2->m_rightBorder.color = Qt::red;
        w2->m_rightBorder.setStyle(KoBorder::DOUBLE_LINE);
        w2->m_topBorder.setPenWidth(3);
        w2->m_topBorder.color = Qt::blue;
        w2->m_topBorder.setStyle(KoBorder::DOUBLE_LINE);
        w2->m_bottomBorder.setPenWidth(5);
        w2->m_bottomBorder.color = Qt::green;
        w2->m_bottomBorder.setStyle(KoBorder::DOUBLE_LINE);
    }
    grid->show();
    return app.exec();
}
