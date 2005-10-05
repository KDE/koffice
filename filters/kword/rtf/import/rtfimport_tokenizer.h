/*
   This file is part of the KDE project
   Copyright (C) 2001 Ewald Snel <ewald@rambo.its.tudelft.nl>
   Copyright (C) 2001 Tomasz Grobelny <grotk@poczta.onet.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef __RTFIMPORT_TOKENIZER_H__
#define __RTFIMPORT_TOKENIZER_H__

#include <qcstring.h>
#include <qfile.h>


// This class represents the tokenizer and the token
class RTFTokenizer
{
public:
    enum TokenType { OpenGroup, CloseGroup, ControlWord, PlainText };

    RTFTokenizer();

    void open( QFile *in );
    void next();

    // token data
    char *text;		// plain text or control word/symbol
    TokenType type;
    int value;		// numeric parameter
    bool hasParam;	// token has a (numeric) parameter

    // tokenizer (private) data
private:
    int nextChar();

    QFile *infile;
    QByteArray fileBuffer;
    QCString tokenText;
    uchar *fileBufferPtr;
    uchar *fileBufferEnd;
};

#endif
