/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.1.0                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: Pixmap (header)                                        */
/******************************************************************/

#ifndef kppixmap_h
#define kppixmap_h

#include <qpixmap.h>
#include <qstring.h>
#include <ksize.h>

#include "global.h"

/******************************************************************/
/* Class: KPPixmap                                                */
/******************************************************************/

class KPPixmap
{
public:
    KPPixmap( QString _filename, KSize _size );
    KPPixmap( QString _filename, QString _data, KSize _size );
    KPPixmap( QString _filename, QString _data, QPixmap *_pixmap, KSize _size );
    virtual ~KPPixmap()
    {}
    
    virtual QString getFilename()
    { return filename; }
    virtual QString getPixData()
    { return data; }
    virtual QString getPixDataNative()
    { return string_to_native_string( data ); }
    virtual QPixmap* getPixmap()
    { return &pixmap; }
    virtual KSize getSize()
    { return pixmap.size(); }

    virtual QPixmap* getOrigPixmap()
    { return &orig_pixmap; }
    virtual KSize getOrigSize()
    { return orig_pixmap.size(); }

    virtual void addRef();
    virtual bool removeRef();

    static QString string_to_native_string( const char* _pixmap );

protected:
    KPPixmap()
    {; }

    QString load_pixmap_native_format( const char *_file );
    QPixmap native_string_to_pixmap( const char *_pixmap );

    QString filename;
    QPixmap pixmap;
    QPixmap orig_pixmap;
    QString data;

    int refCount;

};

#endif
