/* This file is part of the KDE project
   Copyright (C) 1999 Werner Trobin <wtrobin@carinthia.com>

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

#include <stylesheet.h>

StyleSheet::StyleSheet(const myFile &t, const FIB * const f) : table(t),
                       fib(f), lastSti(0xffff) {

    stdOffset=fib->fcStshf+read16(table.data+fib->fcStshf);
    cstd=read16(table.data+fib->fcStshf+2);
    stdBaseLength=read16(table.data+fib->fcStshf+4);

    unsigned long offset=stdOffset, limit=fib->fcStshf+fib->lcbStshf, tmpOffset;
    unsigned short istd=0, len, sti, tmp;
    StyleSheet::STD mySTD;

    do {
        len=read16(table.data+offset);              // length of this STD
        offset+=2;                                  // set offset to beginning of STD
        if(len!=0) {
            sti=read16(table.data+offset) & 0x0fff; // read the sti (lower 12 bits)
            tmp=read16(table.data+offset+2);
            unsigned short sgc=tmp & 0x000f;        // read the sgc
            mySTD.istdBase=(tmp & 0xfff0) >> 4;     // and the istdBase
            tmp=read16(table.data+offset+4);
            unsigned short cupx=tmp & 0x000f;       // we also need the cupx
            tmpOffset=offset+stdBaseLength;         // use it as temporary offset :)
            align2(tmpOffset);
            tmp=read16(table.data+tmpOffset);       // the length
            tmpOffset+=2;                           // length field is a short
            tmp<<=1;                                // we use 2-byte chars!
            mySTD.name="";                          // clear name
            for(unsigned int i=0; i<tmp; i=i+2)
                mySTD.name+=QChar(char2uni(read16(table.data+tmpOffset+i)));
            tmpOffset+=tmp+2;
            if(sgc==1 && cupx==2) {
                mySTD.style.paragStyle=true;
                tmp=read16(table.data+tmpOffset);   // read the length of the UPX
                tmpOffset+=2;                       // Adjust offset (2 bytes length info)
                mySTD.style.fcPAPX=tmpOffset+2;
                mySTD.style.lenPAPX=tmp;
                tmpOffset+=tmp;
                tmp=read16(table.data+tmpOffset);
                tmpOffset+=2;
                mySTD.style.fcCHPX=tmpOffset;
                mySTD.style.lenCHPX=tmp;
            }
            else if(sgc==2 && cupx==1) {
                mySTD.style.paragStyle=false;
                tmp=read16(table.data+tmpOffset);
                tmpOffset+=2;
                mySTD.style.fcCHPX=tmpOffset;
                mySTD.style.lenCHPX=tmp;
                mySTD.style.fcPAPX=0;
                mySTD.style.lenPAPX=0;
            }
            else
                kdebug(KDEBUG_ERROR, 31000, "StyleSheet::chain_rec(): Error: Don't know this stylesheet-format!");
            offset+=len;                            // next STD
            mySTD.istd=istd-1;
            styleMap.insert(sti, mySTD);
        }
        ++istd;
    } while(offset<limit && istd<=cstd);
}

StyleSheet::~StyleSheet() {
    _chain.clear();
}

const QValueList<STYLE> StyleSheet::chain(const unsigned short &sti) {

    if(lastSti==sti)
        return _chain;

    _chain.clear();
    QMap<unsigned long, StyleSheet::STD>::Iterator it=styleMap.find(sti);
    if(it!=styleMap.end() && chain_rec(it.data().istd))
        lastSti=sti;
    else {
        _chain.clear();
        lastSti=0xffff;
    }
    return _chain;
}

const bool StyleSheet::chain_rec(const unsigned short &istd) {

    QMap<unsigned long, StyleSheet::STD>::Iterator it=styleMap.begin();
    bool success=false;
    bool found=false;

    while(it!=styleMap.end() && !found) {
        if(it.data().istd==istd)
            found=true;
        else
            ++it;
    }
    if(found) {
        success=true;
        if(it.data().istdBase!=0x0fff)
            success=chain_rec(it.data().istdBase);
        _chain.append(it.data().style);
    }
    else {
        QString d="StyleSheet::chain_rec(): ERROR: Didn't find istd(";
        d+=QString::number((long)istd);
        d+=")!";
        kdebug(KDEBUG_ERROR, 31000, static_cast<const char*>(d));
    }
    return success;
}

const QString StyleSheet::styleName(const unsigned short &sti) {

    QMap<unsigned long, StyleSheet::STD>::Iterator it=styleMap.find(sti);
    if(it!=styleMap.end())
        return it.data().name;
    else
        return QString("");
}
