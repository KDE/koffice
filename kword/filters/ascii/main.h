/******************************************************************/
/* KWord - (c) by Reginald Stadlbauer and Torben Weis 1997-1998   */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer, Torben Weis                       */
/* E-Mail: reggie@kde.org, weis@kde.org                           */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Filter (header)                                        */
/******************************************************************/

#ifndef __main_h__
#define __main_h__

#include <kom.h>
#include <komComponent.h>
#include <koffice.h>
#include <koApplication.h>

#include <qstring.h>

/******************************************************************/
/* MyApplication                                                  */
/******************************************************************/

class MyApplication : public KoApplication
{
  Q_OBJECT
public:
  MyApplication(int &argc,char **argv);

  void start();
};

/******************************************************************/
/* Factory                                                        */
/******************************************************************/

class Factory : virtual public KOffice::FilterFactory_skel
{
public:
  Factory(const CORBA::ORB::ObjectTag &_tag);
  Factory(CORBA::Object_ptr _obj);

  KOffice::Filter_ptr create();

};

/******************************************************************/
/* Filter                                                         */
/******************************************************************/

class Filter : virtual public KOMComponent,
 	       virtual public KOffice::Filter_skel
{
public:
  Filter();

  void filter(KOffice::Filter::Data& data,const QCString &_from,const QCString &_to);

};

#endif
