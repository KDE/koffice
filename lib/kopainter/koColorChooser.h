/* This file is part of the KDE project
  Copyright (c) 1999 Matthias Elter <me@kde.org>
  Copyright (c) 2001 Igor Janssen <rm@linux.ru.net>

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

#ifndef __ko_ColorChooser_h__
#define __ko_ColorChooser_h__

#include <qwidget.h>
#include "koColor.h"

class KoFrameButton;
class QGridLayout;
class RGBWidget;
class HSVWidget;
class CMYKWidget;
class LABWidget;
class GreyWidget;
class KoColor;
class KoColorSlider;
class QLabel;
class QSpinBox;
class KHSSelector;
class KColorPatch;

class KoColorChooser : public QWidget
{
  Q_OBJECT
public:
  KoColorChooser(QWidget *parent = 0L, const char *name = 0L);

  const KoColor &color() const {return mColor; }

public slots:
  void slotShowRGB();
  void slotShowHSV();
  void slotShowCMYK();
  void slotShowLAB();
  void slotShowGrey();

signals:
  void colorChanged(const KoColor &c);

protected slots:
  void slotChangeXY(int h, int s);
  void slotChangeColor(const KoColor &c);
  void slotChangeColor(const QColor &c);

private:
  KoColor           mColor;

  QGridLayout      *mGrid;
  KoFrameButton    *btnRGB;
  KoFrameButton    *btnHSV;
  KoFrameButton    *btnCMYK;
  KoFrameButton    *btnLAB;
  KoFrameButton    *btnGrey;
  RGBWidget        *mRGBWidget;
  HSVWidget        *mHSVWidget;
  CMYKWidget       *mCMYKWidget;
  LABWidget        *mLABWidget;
  GreyWidget       *mGreyWidget;
  KHSSelector      *mColorSelector;
  KColorPatch      *mColorPatch;
};

class RGBWidget : public QWidget
{
  Q_OBJECT
public:
  RGBWidget(KoColorChooser *aCC, QWidget *parent = 0L);

public slots:
  void slotChangeColor();

protected slots:
  void slotRSliderChanged(int r);
  void slotGSliderChanged(int g);
  void slotBSliderChanged(int b);

  void slotRInChanged(int r);
  void slotGInChanged(int g);
  void slotBInChanged(int b);

signals:
  void colorChanged(const KoColor &c);

private:
  KoColorChooser *mCC;
  KoColorSlider *mRSlider;
  KoColorSlider *mGSlider;
  KoColorSlider *mBSlider;
  QLabel *mRLabel;
  QLabel *mGLabel;
  QLabel *mBLabel;
  QSpinBox *mRIn;
  QSpinBox *mGIn;
  QSpinBox *mBIn;
};

class HSVWidget : public QWidget
{
  Q_OBJECT
public:
  HSVWidget(KoColorChooser *aCC, QWidget *parent = 0L);

public slots:
  void slotChangeColor();

protected slots:
  void slotHSliderChanged(int h);
  void slotSSliderChanged(int s);
  void slotVSliderChanged(int v);

  void slotHInChanged(int h);
  void slotSInChanged(int s);
  void slotVInChanged(int v);

signals:
  void colorChanged(const KoColor &c);

private:
  KoColorChooser   *mCC;
  KoColorSlider    *mHSlider;
  KoColorSlider    *mSSlider;
  KoColorSlider    *mVSlider;
  QLabel           *mHLabel;
  QLabel           *mSLabel;
  QLabel           *mVLabel;
  QSpinBox         *mHIn;
  QSpinBox         *mSIn;
  QSpinBox         *mVIn;
};

class GreyWidget : public QWidget
{
  Q_OBJECT
public:
  GreyWidget(KoColorChooser *aCC, QWidget *parent = 0L);

public slots:
  void slotChangeColor();

protected slots:
  void slotVSliderChanged(int v);
  void slotVInChanged(int v);

signals:
  void colorChanged(const KoColor &c);

protected:
  KoColorChooser *mCC;
  KoColorSlider *mVSlider;
  QLabel      *mVLabel;
  QSpinBox    *mVIn;
};

#endif
