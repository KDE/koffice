/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#ifndef __VCDLGROUNDRECT_H__
#define __VCDLGROUNDRECT_H__

#include <kdialog.h>

class QLineEdit;

class VCDlgRoundRect : public KDialog
{
	Q_OBJECT
public:
	VCDlgRoundRect();

	double valueWidth() const;
	double valueHeight() const;
	double valueRound() const;
	void setValueWidth( const double value );
	void setValueHeight( const double value );
	void setValueRound( const double value );

private:
	QLineEdit* m_width;
	QLineEdit* m_height;
	QLineEdit* m_round;
};

#endif
