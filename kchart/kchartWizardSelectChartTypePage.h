#ifndef _kchartWIZARDSELECTCHARTTYPEPAGE_H
#define _kchartWIZARDSELECTCHARTTYPEPAGE_H

#include <qlayout.h>
#include <qvbox.h>
#include <qwidget.h>

#include "kchart_params.h"

class QFrame;
class QPushButton;
class QButtonGroup;

namespace KChart
{

class KChartPart;

// Contains a button with a text label below it
//
// FIXME: Move to kchartWizardSelectChartTypePage.cc
class KChartButton : public QVBox
{
  Q_OBJECT
  public:
    KChartButton(QWidget* parent, const QString &, const QPixmap &);
    ~KChartButton();

    QPushButton *button() const { return m_button;}
  private:
    QPushButton *m_button;
};

class KChartWizardSelectChartTypePage : public QWidget
{
  Q_OBJECT

public:
  KChartWizardSelectChartTypePage( QWidget* parent, KChartPart* chart );

public slots:
  void  apply();

private slots:
  void  chartTypeSelected( int type );

signals:
  void  chartChange(int);

private:
  void  addButton(const QString &name, const QString &icon_name, int type);
  void  incPos();

private:
  KChartPart    *m_chart;

  QButtonGroup  *m_typeBG;
  QGridLayout   *m_layout;

  // column and row position (used by addButton() to know where to put
  // the new button)
  int            m_colPos;
  int            m_rowPos;

  KDChartParams::ChartType  m_type;

};

}  //namespace KChart

#endif
