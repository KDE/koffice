/*
 * $Id$
 *
 * Copyright 2000 by Laurent Montel, released under Artistic License.
 */

#ifndef __KCHARTPARAMETERPIECONFIGPAGE_H__
#define __KCHARTPARAMETERPIECONFIGPAGE_H__

#include <qwidget.h>

class QCheckBox;
class QRadioButton;
class QLineEdit;
class QSpinBox;

class KChartParams;

class KChartParameterPieConfigPage : public QWidget
{
    Q_OBJECT

public:
    KChartParameterPieConfigPage( KChartParams* params,QWidget* parent );
    void init();
    void apply();

public slots:
    void changeState(bool );	
private:
    KChartParams* _params;
    QRadioButton *_typeNone;
    QRadioButton *_typeAbove;
    QRadioButton *_typeBelow;
    QRadioButton *_typeRight;
    QRadioButton *_typeLeft;
    QCheckBox *lineLabel;
    QLineEdit *title;
    QSpinBox *dist;  
};

#endif
