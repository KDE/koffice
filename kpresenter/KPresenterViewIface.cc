/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "KPresenterViewIface.h"

#include "kpresenter_view.h"

#include <kapplication.h>
#include <dcopclient.h>

KPresenterViewIface::KPresenterViewIface( KPresenterView *view_ )
    : KoViewIface( view_ )
{
    view = view_;
}

void KPresenterViewIface::editCut()
{
    view->editCut();
}

void KPresenterViewIface::editCopy()
{
    view->editCopy();
}

void KPresenterViewIface::editPaste()
{
    view->editPaste();
}

void KPresenterViewIface::editDelete()
{
    view->editDelete();
}

void KPresenterViewIface::editSelectAll()
{
    view->editSelectAll();
}

void KPresenterViewIface::editDuplicatePage()
{
    view->editDuplicatePage();
}

void KPresenterViewIface::editDelPage()
{
    view->editDelPage();
}

void KPresenterViewIface::editFind()
{
    view->editFind();
}

void KPresenterViewIface::editReplace()
{
    view->editReplace();
}


void KPresenterViewIface::newView()
{
    view->newView();
}

void KPresenterViewIface::insertPage()
{
    view->insertPage();
}

void KPresenterViewIface::insertPicture()
{
    view->insertPicture();
}

void KPresenterViewIface::insertClipart()
{
    view->insertClipart();
}

void KPresenterViewIface::toolsMouse()
{
    view->toolsMouse();
}

void KPresenterViewIface::toolsLine()
{
    view->toolsLine();
}

void KPresenterViewIface::toolsRectangle()
{
    view->toolsRectangle();
}

void KPresenterViewIface::toolsCircleOrEllipse()
{
    view->toolsCircleOrEllipse();
}

void KPresenterViewIface::toolsPie()
{
    view->toolsPie();
}

void KPresenterViewIface::toolsText()
{
    view->toolsText();
}

void KPresenterViewIface::toolsAutoform()
{
    view->toolsAutoform();
}

void KPresenterViewIface::toolsDiagramm()
{
    view->toolsDiagramm();
}

void KPresenterViewIface::toolsTable()
{
    view->toolsTable();
}

void KPresenterViewIface::toolsFormula()
{
    view->toolsFormula();
}

void KPresenterViewIface::toolsObject()
{
    view->toolsObject();
}

void KPresenterViewIface::toolsFreehand()
{
    view->toolsFreehand();
}

void KPresenterViewIface::toolsPolyline()
{
    view->toolsPolyline();
}

void KPresenterViewIface::toolsQuadricBezierCurve()
{
    view->toolsQuadricBezierCurve();
}

void KPresenterViewIface::toolsCubicBezierCurve()
{
    view->toolsCubicBezierCurve();
}

void KPresenterViewIface::toolsConvexOrConcavePolygon()
{
    view->toolsConvexOrConcavePolygon();
}

void KPresenterViewIface::extraPenBrush()
{
    view->extraPenBrush();
}

void KPresenterViewIface::extraConfigPie()
{
    view->extraConfigPie();
}

void KPresenterViewIface::extraConfigRect()
{
    view->extraConfigRect();
}

void KPresenterViewIface::extraConfigPolygon()
{
    view->extraConfigPolygon();
}

void KPresenterViewIface::extraRaise()
{
    view->extraRaise();
}

void KPresenterViewIface::extraLower()
{
    view->extraLower();
}

void KPresenterViewIface::extraRotate()
{
    view->extraRotate();
}

void KPresenterViewIface::extraShadow()
{
    view->extraShadow();
}

void KPresenterViewIface::extraBackground()
{
    view->extraBackground();
}

void KPresenterViewIface::extraLayout()
{
    view->extraLayout();
}

void KPresenterViewIface::extraConfigure()
{
    view->extraConfigure();
}

void KPresenterViewIface::extraLineBegin()
{
    view->extraLineBegin();
}

void KPresenterViewIface::extraLineEnd()
{
    view->extraLineEnd();
}

void KPresenterViewIface::extraWebPres()
{
    view->extraWebPres();
}

void KPresenterViewIface::extraAlignObjLeft()
{
    view->extraAlignObjLeft();
}

void KPresenterViewIface::extraAlignObjCenterH()
{
    view->extraAlignObjCenterH();
}

void KPresenterViewIface::extraAlignObjRight()
{
    view->extraAlignObjRight();
}

void KPresenterViewIface::extraAlignObjTop()
{
    view->extraAlignObjTop();
}

void KPresenterViewIface::extraAlignObjCenterV()
{
    view->extraAlignObjCenterV();
}

void KPresenterViewIface::extraAlignObjBottom()
{
    view->extraAlignObjBottom();
}

void KPresenterViewIface::extraAlignObjs()
{
    view->extraAlignObjs();
}

void KPresenterViewIface::extraGroup()
{
    view->extraGroup();
}

void KPresenterViewIface::extraUnGroup()
{
    view->extraUnGroup();
}

void KPresenterViewIface::extraPenStyle()
{
    view->extraPenStyle();
}

void KPresenterViewIface::extraPenWidth()
{
    view->extraPenWidth();
}

void KPresenterViewIface::screenConfigPages()
{
    view->screenConfigPages();
}

void KPresenterViewIface::screenPresStructView()
{
    view->screenPresStructView();
}

void KPresenterViewIface::screenAssignEffect()
{
    view->screenAssignEffect();
}

void KPresenterViewIface::screenStart()
{
    view->screenStart();
}

void KPresenterViewIface::screenStop()
{
    view->screenStop();
}

void KPresenterViewIface::screenPause()
{
    view->screenPause();
}

void KPresenterViewIface::screenFirst()
{
    view->screenFirst();
}

void KPresenterViewIface::screenPrev()
{
    view->screenPrev();
}

void KPresenterViewIface::screenNext()
{
    view->screenNext();
}

void KPresenterViewIface::screenLast()
{
    view->screenLast();
}

void KPresenterViewIface::screenSkip()
{
    view->screenSkip();
}

void KPresenterViewIface::screenPenColor()
{
    view->screenPenColor();
}

void KPresenterViewIface::screenPenWidth( const QString &w )
{
    view->screenPenWidth( w );
}

void KPresenterViewIface::screenViewPage()
{
    view->screenViewPage();
}

void KPresenterViewIface::sizeSelected( int size )
{
    view->sizeSelected( size );
}

void KPresenterViewIface::fontSelected( const QString &fontFamily )
{
    view->fontSelected( fontFamily );
}

void KPresenterViewIface::textBold()
{
    view->textBold();
}

void KPresenterViewIface::textItalic()
{
    view->textItalic();
}

void KPresenterViewIface::textUnderline()
{
    view->textUnderline();
}

void KPresenterViewIface::textColor()
{
    view->textColor();
}

void KPresenterViewIface::textAlignLeft()
{
    view->textAlignLeft();
}

void KPresenterViewIface::textAlignCenter()
{
    view->textAlignCenter();
}

void KPresenterViewIface::textAlignRight()
{
    view->textAlignRight();
}

void KPresenterViewIface::mtextFont()
{
    view->mtextFont();
}

void KPresenterViewIface::textDepthPlus()
{
    view->textDepthPlus();
}

void KPresenterViewIface::textDepthMinus()
{
    view->textDepthMinus();
}

void KPresenterViewIface::textContentsToHeight()
{
    view->textContentsToHeight();
}

void KPresenterViewIface::textObjectToContents()
{
    view->textObjectToContents();
}

void KPresenterViewIface::penChosen()
{
    view->penChosen();
}

void KPresenterViewIface::brushChosen()
{
    view->brushChosen();
}

int KPresenterViewIface::getCurrentPresPage()
{
    return view->getCurrentPresPage();
}

int KPresenterViewIface::getCurrentPresStep()
{
    return view->getCurrentPresStep();
}

int KPresenterViewIface::getPresStepsOfPage()
{
    return view->getPresStepsOfPage();
}


int KPresenterViewIface::getNumPresPages()
{
    return view->getNumPresPages();
}

bool KPresenterViewIface::gotoPresPage( int pg )
{
    return view->gotoPresPage( pg );
}

int KPresenterViewIface::getCurrentPageNum()
{
    return view->getCurrPgNum();
}

double KPresenterViewIface::getCurrentFaktor()
{
    return (double)view->getCurrentFaktor();
}

void KPresenterViewIface::insertSpecialChar()
{
    view->insertSpecialChar();
}

void KPresenterViewIface::insertLink()
{
    view->insertLink();
}

void KPresenterViewIface::editCustomVars()
{
    view->editCustomVars();
}

void KPresenterViewIface::insertVariable()
{
    view->insertVariable();
}

void KPresenterViewIface::insertCustomVariable()
{
    view->insertCustomVariable();
}

void KPresenterViewIface::insertNewCustomVariable()
{
    view->insertNewCustomVariable();
}

void KPresenterViewIface::changeCaseOfText()
{
    view->changeCaseOfText();
}

void KPresenterViewIface::viewShowSideBar()
{
    view->viewShowSideBar();
}

void KPresenterViewIface::viewShowNoteBar()
{
    view->viewShowNoteBar();
}

void KPresenterViewIface::formatParagraph()
{
    view->formatParagraph();
}

void KPresenterViewIface::extraAutoFormat()
{
    view->extraAutoFormat();
}

void KPresenterViewIface::refreshAllVariable()
{
    view->extraAutoFormat();
}

void KPresenterViewIface::skipToPage( int _num )
{
    //don't try to skip to a not exist page
    if(_num>(int)view->kPresenterDoc()->getPageNums()-1)
        return;
    view->skipToPage(_num);
}


void KPresenterViewIface::toolsRotate()
{
    view->toolsRotate();
}

void KPresenterViewIface::toolsZoom()
{
    view->toolsZoom();
}
