/*
 * $Id$
 *
 * Copyright 1999-2000 by Matthias Kalle Dalheimer <kalle@kde.org>,
 *   released under Artistic License
 */

#ifndef __KCHARTAREASUBTYPECHARTPAGE_H__
#define __KCHARTAREASUBTYPECHARTPAGE_H__

#include "kchartSubTypeChartPage.h"

class QLabel;
class QRadioButton;
class KChartParams;

class KChartAreaSubTypeChartPage : public KChartSubTypeChartPage
{
    Q_OBJECT

public:
    KChartAreaSubTypeChartPage( KChartParameters* params,QWidget* parent );
    virtual void init();
    virtual void apply();

private slots:
	  void slotChangeSubType( int type );

private:
    QRadioButton *depth;
    QRadioButton *beside;
	QLabel* exampleLA;
};

#endif
