/* This file is part of the KDE project
   Made by Tomislav Lukman (tomislav.lukman@ck.tel.hr)
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VTRANSFORMDLG_H__
#define __VTRANSFORMDLG_H__

#include <qdockwindow.h>

class QCheckBox;
class QTabWidget;
class QVButtonGroup;

class KoMainWindow;
class KoView;

class KarbonPart;
class TKUFloatSpinBox;

enum ButtonChoice { tr_Duplicate, tr_Apply };

// Translate widget for this dialog...
class VTranslateWidget : public QWidget
{
	Q_OBJECT
public:
	VTranslateWidget( QWidget* parent );
private:
	QCheckBox* mRelative;
	QVButtonGroup* mButtonGroup;
	TKUFloatSpinBox* mHSpinBox;
	TKUFloatSpinBox* mVSpinBox;
};

// Rotation widget for this dialog...
class VRotateWidget : public QWidget
{
	Q_OBJECT
public:
	VRotateWidget( QWidget* parent );
private:
	QCheckBox* mRelative;
	QVButtonGroup* mButtonGroup;
	TKUFloatSpinBox* mAngle;
	TKUFloatSpinBox* mHSpinBox;
	TKUFloatSpinBox* mVSpinBox;
};

// Shear widget for this dialog...
class VShearWidget : public QWidget
{
	Q_OBJECT
public:
	VShearWidget( QWidget* parent );
private:
	QVButtonGroup* mButtonGroup;
	TKUFloatSpinBox* mHSpinBox;
	TKUFloatSpinBox* mVSpinBox;
};

// Scale widget for this dialog...
class VScaleWidget : public QWidget
{
	Q_OBJECT
public:
	VScaleWidget( QWidget* parent );
private:
	QCheckBox* mRelative;
	QVButtonGroup* mButtonGroup;
	TKUFloatSpinBox* mHSpinBox;
	TKUFloatSpinBox* mVSpinBox;
};

// The dialog ...
class VTransformDlg : public QDockWindow
{
	Q_OBJECT

public:
	enum TabChoice {
		Translate,
		Rotate,
		Shear,
		Scale
	}; //<--- For quickjumping to a certain tab - useful for quick shortcuts
	
	VTransformDlg( KarbonPart* part, KoView* parent = 0L, const char* name = 0L );
  
private:
	QTabWidget* mTabWidget;
	VTranslateWidget* mTranslateWidget;
	VRotateWidget* mRotateWidget;
	VShearWidget* mShearWidget;
	VScaleWidget* mScaleWidget;
	KarbonPart *m_part;
};

#endif

