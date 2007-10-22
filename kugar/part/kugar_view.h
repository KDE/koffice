// Copyright (c) 2000 Phil Thompson <phil@river-bank.demon.co.uk>
//
// This file contains the definition of the interface to the Kugar KPart.


#ifndef _KUGAR_VIEW_H
#define _KUGAR_VIEW_H

#include "mreportviewer.h"

#include <KoView.h>

class KugarPart;

class KugarView: public KoView
{
    Q_OBJECT

public:
    KugarView( KugarPart *part, QWidget *parent);
    virtual ~KugarView();

    void setForcedUserTemplate( const QString &name );

    void updateReadWrite( bool )
    {
        ;
    }

    bool renderReport();

    bool setReportTemplate( const QString &data )
    {
        return view -> setReportTemplate( data );
    }


    virtual void setupPrinter( QPrinter &printer, QPrintDialog &printDialog );
    virtual void print( QPrinter &printer, QPrintDialog &printDialog );



    //protected:
    //  virtual bool openFile();
    //  virtual bool closeUrl();


private:
    Kugar::MReportViewer *view;
    QString m_forcedUserTemplate;
};

#endif
