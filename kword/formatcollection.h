/******************************************************************/
/* KWord - (c) by Reginald Stadlbauer 1997-1998                   */
/* based on Torben Weis' KWord                                    */
/* Version: 0.0.1                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Format Collection (header)                             */
/******************************************************************/

#ifndef formatcollection_h
#define formatcollection_h

#include "format.h"

#include <qdict.h>
#include <qfont.h>
#include <qstring.h>
#include <qcolor.h>

class KWordDocument_impl;

/******************************************************************/
/* Class: KWFormatCollection                                      */
/******************************************************************/

class KWFormatCollection
{
public:
  KWFormatCollection(KWordDocument_impl *_doc);
  ~KWFormatCollection();

  KWFormat *getFormat(const KWFormat &_format);
  void removeFormat(KWFormat *_format);

  QString generateKey(KWFormat *_format)
    { return generateKey(*_format); }
  
protected:
  QString generateKey(const KWFormat &_format);
  KWFormat *findFormat(QString _key);
  KWFormat *insertFormat(QString _key,const KWFormat &_format);

  QDict<KWFormat> formats;
  KWordDocument_impl *doc;
};

#endif
