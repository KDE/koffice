/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>, Torben Weis <weis@kde.org>

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

#ifndef __char_h__
#define __char_h__

#include "format.h"
#include "searchdia.h"
#include "autoformat.h"
#include "image.h"
#include "variable.h"
#include "footnote.h"

#include <qstring.h>
#include <qregexp.h>
#include <qcstring.h>

#include <iostream>

class KWordDocument;
class KWTextFrameSet;

enum ClassIDs { ID_KWCharNone = 0, ID_KWCharFormat = 1, ID_KWCharImage = 2, ID_KWCharTab = 3,
		ID_KWCharVariable = 4, ID_KWCharFootNote = 5, ID_KWCharAnchor = 6};

/******************************************************************/
/* Class: KWCharAttribute                                         */
/******************************************************************/

class KWCharAttribute
{
public:
    KWCharAttribute() { classId = ID_KWCharNone; }
    virtual ~KWCharAttribute() {}

    int getClassId()
    { return classId; }
    virtual bool operator==( const KWCharAttribute &_attrib ) const
    { return classId == static_cast<KWCharAttribute>( _attrib ).classId; }

    virtual void save( ostream & ) {}

protected:
    int classId;

};

/******************************************************************/
/* Class: KWCharFormat                                            */
/******************************************************************/

class KWCharFormat : public KWCharAttribute
{
public:
    KWCharFormat() : KWCharAttribute() { classId = ID_KWCharFormat; format = 0L; }
    KWCharFormat( KWFormat* _format ) : KWCharAttribute() { classId = ID_KWCharFormat; format = _format; }
    ~KWCharFormat() { if ( format ) format->decRef(); format = 0L; }

    virtual KWFormat* getFormat()
    { return format; }
    virtual void setFormat( KWFormat *_format )
    { format = _format; }
    virtual bool operator==( const KWCharFormat &_attrib ) {
        return ( classId == _attrib.classId &&
                 ( format ) && *format == *_attrib.format );
    }

    virtual void save( ostream &out )
    { format->save( out ); }

protected:
    KWFormat *format;

};

/******************************************************************/
/* Class: KWCharImage                                             */
/******************************************************************/

class KWCharImage : public KWCharAttribute
{
public:
    KWCharImage() { classId = ID_KWCharImage; image = 0L; }
    KWCharImage( KWImage *_image ) : KWCharAttribute() { classId = ID_KWCharImage; image = _image; }
    ~KWCharImage() { image->decRef(); image = 0L; }

    virtual KWImage* getImage()
    { return image; }
    virtual void setImage( KWImage *_image )
    { image = _image; }
    virtual void save( ostream &out )
    { image->save( out ); }

protected:
    KWImage *image;

};

/******************************************************************/
/* Class: KWCharTab                                               */
/******************************************************************/

class KWCharTab : public KWCharAttribute
{
public:
    KWCharTab() { classId = ID_KWCharTab; }

};

/******************************************************************/
/* Class: KWCharVariable                                          */
/******************************************************************/

class KWCharVariable : public KWCharFormat
{
public:
    KWCharVariable( KWVariable *_var ) : KWCharFormat() { classId = ID_KWCharVariable; var = _var; }
    ~KWCharVariable() { if ( var ) delete var; if ( format ) format->decRef(); format = 0L; }

    QString getText() { return var->getText(); }

    KWVariable *getVar() { return var; }

    virtual void save( ostream &out ) {
        var->save( out );
        out << otag << "<FRMAT>" << endl;
        KWCharFormat::save( out );
        out << etag << "</FRMAT>" << endl;
    }

protected:
    KWVariable *var;

};

/******************************************************************/
/* Class: KWCharFootNote                                         */
/******************************************************************/

class KWCharFootNote : public KWCharFormat
{
public:
    KWCharFootNote( KWFootNote *_fn ) : KWCharFormat() { fn = _fn; classId = ID_KWCharFootNote; }
    ~KWCharFootNote() { if ( fn ) delete fn; if ( format ) format->decRef(); format = 0L; }

    QString getText() { return fn->getText(); }

    KWFootNote *getFootNote() { return fn; }

    virtual void save( ostream &out ) {
        fn->save( out );
        out << otag << "<FRMAT>" << endl;
        KWCharFormat::save( out );
        out << etag << "</FRMAT>" << endl;
    }

protected:
    KWFootNote *fn;

};

/******************************************************************/
/* Class: KWCharAnchor                                            */
/*                                                                */
/* An instance of this class acts as the anchor for a floating    */
/* object. In practice, the object is derived from this class in  */
/* order to provide implementations for the pure virtual logic    */
/* required.                                                      */
/*                                                                */
/* The anchor is enabled by a call to setAnchored().              */
/******************************************************************/

class KWCharAnchor : public KWCharAttribute
{
public:
    KWCharAnchor();
    virtual ~KWCharAnchor() {}

    // Is the anchoring logic enabled at this time?
    bool isAnchored() { return anchored; }
    void setAnchored( bool _anchored );

    // Set or change the location of the anchor.
    QPoint &getOrigin() { return origin; }
    void setOrigin( QPoint _origin );

    // This base class function saves the anchor itself; the
    // deriving class will normally implement its own save() function
    // to save itself.
    void save( ostream &out );

    // Override this function with logic to move the anchored object
    // when the origin of the anchor is changed.
    virtual void moveBy( unsigned int dx, unsigned int dy ) = 0;

    // Override this function to return the text required to identify
    // the anchored object.
    virtual QString anchorFor() = 0;

protected:
    QPoint origin;
    bool anchored;
};

// Torben: Be prepared for unicode
// Reggie: Now we support unicode :- ) )
#define kwchar QChar

struct KWChar
{
    KWChar() : c(), autoformat( 0L ), attrib( 0L )
    {}

    kwchar c;
    KWAutoFormat::AutoformatInfo *autoformat;
    KWCharAttribute* attrib;
};

/******************************************************************/
/* Class: KWString                                                */
/******************************************************************/

class KWString
{
public:
    KWString( KWordDocument *_doc )
    { _max_ = 0; _len_ = 0; _data_ = 0L; doc = _doc; allowRemoveFn = FALSE; }
    KWString( QString _text, KWordDocument *_doc );
    KWString( const KWString &_string );
    ~KWString()
    { free( _data_, _len_ ); delete [] _data_; }

    KWString &operator=( const KWString &_string );

    unsigned int size()
    { return _len_; }
    unsigned int max()
    { return _max_; }
    void append( KWChar *_text, unsigned int _len );
    void append( KWChar _c );
    void insert( unsigned int _pos, QString _text );
    void insert( unsigned int _pos, KWString *_text );
    void insert( unsigned int _pos, const char _c );
    void insert( unsigned int _pos, KWCharImage *_image );
    void insert( unsigned int _pos, KWCharTab *_tab );
    void insert( unsigned int _pos, KWCharVariable *_var );
    void insert( unsigned int _pos, KWCharFootNote *_fn );
    void insert( unsigned int _pos, KWCharAnchor *_anchor );
    void resize( unsigned int _size, bool del = true );
    bool remove( unsigned int _pos, unsigned int _len = 1 );
    KWChar* split( unsigned int _pos );

    KWChar* data()
    { return _data_; }

    QString toString( bool cached = FALSE );
    QString toString( unsigned int _pos, unsigned int _len, bool cached = FALSE );
    void saveFormat( ostream &out );
    void loadFormat( KOMLParser &parser, vector<KOMLAttrib> &lst, KWordDocument *_doc, KWTextFrameSet *_frameset );

    int find( QString _expr, KWSearchDia::KWSearchEntry *_format, int _index, bool _cs, bool _whole );
    int find( QRegExp _regexp, KWSearchDia::KWSearchEntry *_format, int _index, int &_len, bool _cs, bool _wildcard = false );
    int findRev( QString _expr, KWSearchDia::KWSearchEntry *_format, int _index, bool _cs, bool _whole );
    int findRev( QRegExp _regexp, KWSearchDia::KWSearchEntry *_format, int _index, int &_len, bool _cs, bool _wildcard = false );

    KWordDocument *getDocument() { return doc; }

    QString decoded();
    QCString utf8( bool _decoded = true );

    void clear();

protected:
    KWChar* alloc( unsigned int _size );
    void free( KWChar* _data, unsigned int _len );
    KWChar* copy( KWChar *_data, unsigned int _len );
    KWChar& copy( KWChar _c );

    unsigned int _len_;
    unsigned int _max_;
    KWChar* _data_;
    KWordDocument *doc;
    QString cache;
    bool allowRemoveFn;
    
};

void freeChar( KWChar& _char, KWordDocument *_doc, bool allowRemoveFn = FALSE );
ostream& operator<<( ostream &out, KWString &_string );

#endif
