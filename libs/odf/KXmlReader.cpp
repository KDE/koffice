/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KXmlReader.h"
#include "KOdfXmlNS.h"

/*
  This is a memory-efficient DOM implementation for KOffice. See the API
  documentation for details.

  IMPORTANT !

  * When you change this stuff, make sure it DOES NOT BREAK the test suite.
    Build tests/koxmlreadertest.cpp and verify it. Many sleepless nights
    have been sacrificed for this piece of code, do not let those precious
    hours wasted!

  * Run koxmlreadertest.cpp WITH Valgrind and make sure NO illegal
    memory read/write and any type of leak occurs. If you are not familiar
    with Valgrind then RTFM first and come back again later on.

  * The public API shall remain as compatible as QDom.

  * All QDom-compatible methods should behave the same. All QDom-compatible
    functions should return the same result. In case of doubt, run
    koxmlreadertest.cpp but uncomment KOXML_USE_QDOM in koxmlreader.h
    so that the tests are performed with standard QDom.

  Some differences compared to QDom:

  - DOM tree in KXmlDocument is read-only, you can not modify it. This is
    sufficient for KOffice since the tree is only accessed when loading
    a document to the application. For saving the document to XML file,
    use KXmlWriter.

  - Because the dynamic loading and unloading, you have to use the
    nodes (and therefore also elements) carefully since the whole API
 (just like QDom) is reference-based, not pointer-based. If the
 parent node is unloaded from memory, the reference is not valid
 anymore and may give unpredictable result.
 The easiest way: use the node/element in very short time only.

  - Comment node (like QDomComment) is not implemented as comments are
    simply ignored.

  - DTD, entity and entity reference are not handled. Thus, the associated
    nodes (like QDomDocumentType, QDomEntity, QDomEntityReference) are also
    not implemented.

  - Attribute mapping node is not implemented. But of course, functions to
    query attributes of an element are available.


 */

#include <QTextCodec>
#include <QTextDecoder>

#ifndef KOXML_USE_QDOM

#include <qxml.h>
#include <qdom.h>
#include <QXmlStreamReader>
#include <QXmlStreamEntityResolver>

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QHash>
#include <QPair>
#include <QStringList>
#include <QVector>

/*
 Use more compact representation of in-memory nodes.

 Advantages: faster iteration, can facilitate real-time compression.
 Disadvantages: still buggy, eat slightly more memory.
*/
#define KOXML_COMPACT

/*
 Use real-time compression. Only works in conjuction with KOXML_COMPACT
 above because otherwise the non-compact layout will slow down everything.
*/
#define KOXML_COMPRESS


// prevent mistake, see above
#ifdef KOXML_COMPRESS
#ifndef KOXML_COMPACT
#error Please enable also KOXML_COMPACT
#endif
#endif

// this is used to quickly get namespaced attribute(s)
typedef QPair<QString, QString> KoXmlStringPair;

class KOdfQName {
public:
    QString nsURI;
    QString name;

    explicit KOdfQName(const QString& nsURI_, const QString& name_)
        : nsURI(nsURI_), name(name_) {}
    bool operator==(const KOdfQName& qname) const {
        // local name is more likely to differ, so compare that first
        return name == qname.name && nsURI == qname.nsURI;
    }
};

uint qHash(const KOdfQName& qname)
{
    // possibly add a faster hash function that only includes some trailing
    // part of the nsURI

    // in case of doubt, use this:
    // return qHash(qname.nsURI)^qHash(qname.name);
    return qHash(qname.nsURI)^qHash(qname.name);
}

// this simplistic hash is rather fast-and-furious. it works because
// likely there is very few namespaced attributes per element
static inline uint qHash(const KoXmlStringPair &p)
{
    return qHash(p.second[0].unicode()) ^ 0x1477;

    // in case of doubt, use this:
    // return qHash(p.first)^qHash(p.second);
}

static inline bool operator==(const KoXmlStringPair &a, const KoXmlStringPair &b)
{
    return a.second == b.second && a.first == b.first;
}

// Older versions of OpenOffice.org used different namespaces. This function
// does translate the old namespaces into the new ones.
static QString fixNamespace(const QString &nsURI)
{
    static QString office = QString::fromLatin1("http://openoffice.org/2000/office");
    static QString text = QString::fromLatin1("http://openoffice.org/2000/text");
    static QString style = QString::fromLatin1("http://openoffice.org/2000/style");
    static QString fo = QString::fromLatin1("http://www.w3.org/1999/XSL/Format");
    static QString table = QString::fromLatin1("http://openoffice.org/2000/table");
    static QString drawing = QString::fromLatin1("http://openoffice.org/2000/drawing");
    static QString datastyle = QString::fromLatin1("http://openoffice.org/2000/datastyle");
    static QString svg = QString::fromLatin1("http://www.w3.org/2000/svg");
    static QString chart = QString::fromLatin1("http://openoffice.org/2000/chart");
    static QString dr3d = QString::fromLatin1("http://openoffice.org/2000/dr3d");
    static QString form = QString::fromLatin1("http://openoffice.org/2000/form");
    static QString script = QString::fromLatin1("http://openoffice.org/2000/script");
    static QString meta = QString::fromLatin1("http://openoffice.org/2000/meta");
    static QString config = QString::fromLatin1("http://openoffice.org/2001/config");
    static QString pres = QString::fromLatin1("http://openoffice.org/2000/presentation");
    static QString manifest = QString::fromLatin1("http://openoffice.org/2001/manifest");
    if (nsURI == text)
        return KOdfXmlNS::text;
    if (nsURI == style)
        return KOdfXmlNS::style;
    if (nsURI == office)
        return KOdfXmlNS::office;
    if (nsURI == fo)
        return KOdfXmlNS::fo;
    if (nsURI == table)
        return KOdfXmlNS::table;
    if (nsURI == drawing)
        return KOdfXmlNS::draw;
    if (nsURI == datastyle)
        return KOdfXmlNS::number;
    if (nsURI == svg)
        return KOdfXmlNS::svg;
    if (nsURI == chart)
        return KOdfXmlNS::chart;
    if (nsURI == dr3d)
        return KOdfXmlNS::dr3d;
    if (nsURI == form)
        return KOdfXmlNS::form;
    if (nsURI == script)
        return KOdfXmlNS::script;
    if (nsURI == meta)
        return KOdfXmlNS::meta;
    if (nsURI == config)
        return KOdfXmlNS::config;
    if (nsURI == pres)
        return KOdfXmlNS::presentation;
    if (nsURI == manifest)
        return KOdfXmlNS::manifest;
    return nsURI;
}

// ==================================================================
//
//         KXmlPackedItem
//
// ==================================================================

// 12 bytes on most system 32 bit systems, 16 bytes on 64 bit systems
class KXmlPackedItem
{
public:
bool attr: 1;
KXmlNode::NodeType type: 3;

#ifdef KOXML_COMPACT
quint32 childStart: 28;
#else
unsigned depth: 28;
#endif

    unsigned qnameIndex;
    QString value;

    // it is important NOT to have a copy constructor, so that growth is optimal
    // see http://doc.trolltech.com/4.2/containers.html#growth-strategies
#if 0
    KXmlPackedItem(): attr(false), type(KXmlNode::NullNode), childStart(0), depth(0) {}
#endif
};

Q_DECLARE_TYPEINFO(KXmlPackedItem, Q_MOVABLE_TYPE);

#ifdef KOXML_COMPRESS
static QDataStream& operator<<(QDataStream& s, const KXmlPackedItem& item)
{
    quint8 flag = item.attr ? 1 : 0;

    s << flag;
    s << (quint8) item.type;
    s << item.childStart;
    s << item.qnameIndex;
    s << item.value;

    return s;
}

static QDataStream& operator>>(QDataStream& s, KXmlPackedItem& item)
{
    quint8 flag;
    quint8 type;
    quint32 child;
    QString value;

    s >> flag;
    s >> type;
    s >> child;
    s >> item.qnameIndex;
    s >> value;

    item.attr = (flag != 0);
    item.type = (KXmlNode::NodeType) type;
    item.childStart = child;
    item.value = value;

    return s;
}
#endif

#ifdef KOXML_COMPRESS

// ==================================================================
//
//         KXmlVector
//
// ==================================================================


// similar to QVector, but using LZF compression to save memory space
// this class is however not reentrant

// comment it to test this class without the compression
#define KOXMLVECTOR_USE_LZF

// when number of buffered items reach this, compression will start
// small value will give better memory usage at the cost of speed
// bigger value will be better in term of speed, but use more memory
#define ITEMS_FULL  (1*256)

// LZF stuff is wrapper in KLZF
#ifdef KOXML_COMPRESS
#ifdef KOXMLVECTOR_USE_LZF

#define HASH_LOG  12
#define HASH_SIZE (1<< HASH_LOG)
#define HASH_MASK  (HASH_SIZE-1)

#define UPDATE_HASH(v,p) { v = *((quint16*)p); v ^= *((quint16*)(p+1))^(v>>(16-HASH_LOG)); }

#define MAX_COPY       32
#define MAX_LEN       264  /* 256 + 8 */
#define MAX_DISTANCE 8192

// Lossless compression using LZF algorithm, this is faster on modern CPU than
// the original implementation in http://liblzf.plan9.de/
static int lzff_compress(const void* input, int length, void* output, int maxout)
{
    Q_UNUSED(maxout);

    const quint8* ip = (const quint8*) input;
    const quint8* ip_limit = ip + length - MAX_COPY - 4;
    quint8* op = (quint8*) output;

    const quint8* htab[HASH_SIZE];
    const quint8** hslot;
    quint32 hval;

    quint8* ref;
    qint32 copy;
    qint32 len;
    qint32 distance;
    quint8* anchor;

    /* initializes hash table */
    for (hslot = htab; hslot < htab + HASH_SIZE; hslot++)
        *hslot = ip;

    /* we start with literal copy */
    copy = 0;
    *op++ = MAX_COPY - 1;

    /* main loop */
    while (ip < ip_limit) {
        /* find potential match */
        UPDATE_HASH(hval, ip);
        hslot = htab + (hval & HASH_MASK);
        ref = (quint8*) * hslot;

        /* update hash table */
        *hslot = ip;

        /* find itself? then it's no match */
        if (ip == ref)
            goto literal;

        /* is this a match? check the first 2 bytes */
        if (*((quint16*)ref) != *((quint16*)ip))
            goto literal;

        /* now check the 3rd byte */
        if (ref[2] != ip[2])
            goto literal;

        /* calculate distance to the match */
        distance = ip - ref;

        /* skip if too far away */
        if (distance >= MAX_DISTANCE)
            goto literal;

        /* here we have 3-byte matches */
        anchor = (quint8*)ip;
        len = 3;
        ref += 3;
        ip += 3;

        /* now we have to check how long the match is */
        if (ip < ip_limit - MAX_LEN) {
            while (len < MAX_LEN - 8) {
                /* unroll 8 times */
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                if (*ref++ != *ip++) break;
                len += 8;
            }
            ip--;
        }
        len = ip - anchor;

        /* just before the last non-matching byte */
        ip = anchor + len;

        /* if we have copied something, adjust the copy count */
        if (copy) {
            /* copy is biased, '0' means 1 byte copy */
            anchor = anchor - copy - 1;
            *(op - copy - 1) = copy - 1;
            copy = 0;
        } else
            /* back, to overwrite the copy count */
            op--;

        /* length is biased, '1' means a match of 3 bytes */
        len -= 2;

        /* distance is also biased */
        distance--;

        /* encode the match */
        if (len < 7)
            *op++ = (len << 5) + (distance >> 8);
        else {
            *op++ = (7 << 5) + (distance >> 8);
            *op++ = len - 7;
        }
        *op++ = (distance & 255);

        /* assuming next will be literal copy */
        *op++ = MAX_COPY - 1;

        /* update the hash at match boundary */
        --ip;
        UPDATE_HASH(hval, ip);
        htab[hval & HASH_MASK] = ip;
        ip++;

        continue;

    literal:
        *op++ = *ip++;
        copy++;
        if (copy >= MAX_COPY) {
            copy = 0;
            *op++ = MAX_COPY - 1;
        }
    }

    /* left-over as literal copy */
    ip_limit = (const quint8*)input + length;
    while (ip < ip_limit) {
        *op++ = *ip++;
        copy++;
        if (copy == MAX_COPY) {
            copy = 0;
            *op++ = MAX_COPY - 1;
        }
    }

    /* if we have copied something, adjust the copy length */
    if (copy)
        *(op - copy - 1) = copy - 1;
    else
        op--;

    return op - (quint8*)output;
}

static int lzff_decompress(const void* input, int length, void* output, int maxout)
{
    const quint8* ip = (const quint8*) input;
    const quint8* ip_limit  = ip + length - 1;
    quint8* op = (quint8*) output;
    quint8* op_limit = op + maxout;
    quint8* ref;

    while (ip < ip_limit) {
        quint32 ctrl = (*ip) + 1;
        quint32 ofs = ((*ip) & 31) << 8;
        quint32 len = (*ip++) >> 5;

        if (ctrl < 33) {
            /* literal copy */
            if (op + ctrl > op_limit)
                return 0;

            /* crazy unrolling */
            if (ctrl) {
                *op++ = *ip++;
                ctrl--;

                if (ctrl) {
                    *op++ = *ip++;
                    ctrl--;

                    if (ctrl) {
                        *op++ = *ip++;
                        ctrl--;

                        for (;ctrl; ctrl--)
                            *op++ = *ip++;
                    }
                }
            }
        } else {
            /* back reference */
            len--;
            ref = op - ofs;
            ref--;

            if (len == 7 - 1)
                len += *ip++;

            ref -= *ip++;

            if (op + len + 3 > op_limit)
                return 0;

            if (ref < (quint8 *)output)
                return 0;

            *op++ = *ref++;
            *op++ = *ref++;
            *op++ = *ref++;
            if (len)
                for (; len; --len)
                    *op++ = *ref++;
        }
    }

    return op - (quint8*)output;
}

class KLZF
{
public:
    static QByteArray compress(const QByteArray&);
    static void decompress(const QByteArray&, QByteArray&);
};

QByteArray KLZF::compress(const QByteArray& input)
{
    const void* const in_data = (const void*) input.constData();
    unsigned int in_len = (unsigned int)input.size();

    QByteArray output;
    output.resize(in_len + 4 + 1);

    // we use 4 bytes to store uncompressed length
    // and 1 extra byte as flag (0=uncompressed, 1=compressed)
    output[0] = in_len & 255;
    output[1] = (in_len >> 8) & 255;
    output[2] = (in_len >> 16) & 255;
    output[3] = (in_len >> 24) & 255;
    output[4] = 1;

    unsigned int out_len = in_len - 1;
    unsigned char* out_data = (unsigned char*) output.data() + 5;

    unsigned int len = lzff_compress(in_data, in_len, out_data, out_len);
    out_len = len;

    if ((len > out_len) || (len == 0)) {
        // output buffer is too small, likely because the data can't
        // be compressed. so here just copy without compression
        out_len = in_len;
        output.insert(5, input);

        // flag must indicate "uncompressed block"
        output[4] = 0;
    }

    // minimize memory
    output.resize(out_len + 4 + 1);
    output.squeeze();

    return output;
}

// will not squeeze output
void KLZF::decompress(const QByteArray& input, QByteArray& output)
{
    // read out first how big is the uncompressed size
    unsigned int unpack_size = 0;
    unpack_size |= ((quint8)input[0]);
    unpack_size |= ((quint8)input[1]) << 8;
    unpack_size |= ((quint8)input[2]) << 16;
    unpack_size |= ((quint8)input[3]) << 24;

    // prepare the output
    output.reserve(unpack_size);

    // compression flag
    quint8 flag = (quint8)input[4];

    // prepare for lzf
    const void* const in_data = (const void*)(input.constData() + 5);
    unsigned int in_len = (unsigned int)input.size() - 5;
    unsigned char* out_data = (unsigned char*) output.data();
    unsigned int out_len = (unsigned int)unpack_size;

    if (flag == 0)
        memcpy(output.data(), in_data, in_len);
    else {
        unsigned int len = lzff_decompress(in_data, in_len, out_data, out_len);
        output.resize(len);
        output.squeeze();
    }
}


#endif
#endif

template <typename T>
class KXmlVector
{
private:
    unsigned totalItems;
    QVector<unsigned> startIndex;
    QVector<QByteArray> blocks;

    unsigned bufferStartIndex;
    QVector<T> bufferItems;
    QByteArray bufferData;

protected:
    // fetch given item index to the buffer
    // will INVALIDATE all references to the buffer
    void fetchItem(unsigned index) {
        // already in the buffer ?
        if (index >= bufferStartIndex)
            if (index - bufferStartIndex < (unsigned)bufferItems.count())
                return;

        // search in the stored blocks
        // TODO: binary search to speed up
        int loc = startIndex.count() - 1;
        for (int c = 0; c < startIndex.count() - 1; c++)
            if (index >= startIndex[c])
                if (index < startIndex[c+1]) {
                    loc = c;
                    break;
                }

        bufferStartIndex = startIndex[loc];
#ifdef KOXMLVECTOR_USE_LZF
        KLZF::decompress(blocks[loc], bufferData);
#else
        bufferData = blocks[loc];
#endif
        QBuffer buffer(&bufferData);
        buffer.open(QIODevice::ReadOnly);
        QDataStream in(&buffer);
        bufferItems.clear();
        in >> bufferItems;
    }

    // store data in the buffer to main blocks
    void storeBuffer() {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        QDataStream out(&buffer);
        out << bufferItems;

        startIndex.append(bufferStartIndex);
#ifdef KOXMLVECTOR_USE_LZF
        blocks.append(KLZF::compress(buffer.data()));
#else
        blocks.append(buffer.data());
#endif

        bufferStartIndex += bufferItems.count();
        bufferItems.clear();
    }

public:
    inline KXmlVector(): totalItems(0), bufferStartIndex(0) {};

    void clear() {
        totalItems = 0;
        startIndex.clear();
        blocks.clear();

        bufferStartIndex = 0;
        bufferItems.clear();
        bufferData.reserve(1024*1024);
    }

    inline int count() const {
        return (int)totalItems;
    }
    inline int size() const {
        return (int)totalItems;
    }
    inline bool isEmpty() const {
        return totalItems == 0;
    }

    // append a new item
    // WARNING: use the return value as soon as possible
    // it may be invalid if another function is invoked
    T& newItem() {
        // buffer full?
        if (bufferItems.count() >= ITEMS_FULL - 1)
            storeBuffer();

        totalItems++;
        bufferItems.resize(bufferItems.count() + 1);
        return bufferItems[bufferItems.count()-1];
    }

    // WARNING: use the return value as soon as possible
    // it may be invalid if another function is invoked
    const T &operator[](int i) const {
        ((KXmlVector*)this)->fetchItem((unsigned)i);
        return bufferItems[i - bufferStartIndex];
    }

    // optimize memory usage
    // will INVALIDATE all references to the buffer
    void squeeze() {
        storeBuffer();
    }

};

#endif

// ==================================================================
//
//         KXmlPackedDocument
//
// ==================================================================

#ifdef KOXML_COMPRESS
typedef KXmlVector<KXmlPackedItem> KoXmlPackedGroup;
#else
typedef QVector<KXmlPackedItem> KoXmlPackedGroup;
#endif

// growth strategy: increase every GROUP_GROW_SIZE items
// this will override standard QVector's growth strategy
#define GROUP_GROW_SHIFT 3
#define GROUP_GROW_SIZE (1 << GROUP_GROW_SHIFT)

class KXmlPackedDocument
{
public:
    bool processNamespace;
#ifdef KOXML_COMPACT
    // map given depth to the list of items
    QHash<int, KoXmlPackedGroup> groups;
#else
    QVector<KXmlPackedItem> items;
#endif

    QList<KOdfQName> qnameList;
    QString docType;

private:
    QHash<KOdfQName, unsigned> qnameHash;

    unsigned cacheQName(const QString& name, const QString& nsURI) {
        KOdfQName qname(nsURI, name);

        const unsigned ii = qnameHash.value(qname, (unsigned)-1);
        if (ii != (unsigned)-1)
            return ii;

        // not yet declared, so we add it
        unsigned i = qnameList.count();
        qnameList.append(qname);
        qnameHash.insert(qname, i);

        return i;
    }

    QHash<QString, unsigned> valueHash;
    QStringList valueList;

    QString cacheValue(const QString& value) {
        if (value.isEmpty())
            return 0;

        const unsigned& ii = valueHash[value];
        if (ii > 0)
            return valueList[ii];

        // not yet declared, so we add it
        unsigned i = valueList.count();
        valueList.append(value);
        valueHash.insert(value, i);

        return valueList[i];
    }

#ifdef KOXML_COMPACT
public:
    const KXmlPackedItem& itemAt(unsigned depth, unsigned index) {
        const KoXmlPackedGroup& group = groups[depth];
        return group[index];
    }

    unsigned itemCount(unsigned depth) {
        const KoXmlPackedGroup& group = groups[depth];
        return group.count();
    }

    /*
       NOTE:
          Function clear, newItem, addElement, addAttribute, addText,
          addCData, addProcessing are all related. These are all necessary
          for stateful manipulation of the document. See also the calls
          to these function from parseDocument().

          The state itself is defined by the member variables
          currentDepth and the groups (see above).
     */

    unsigned currentDepth;

    KXmlPackedItem& newItem(unsigned depth) {
        KoXmlPackedGroup& group = groups[depth];

#ifdef KOXML_COMPRESS
        KXmlPackedItem& item = group.newItem();
#else
        // reserve up front
        if ((groups.size() % GROUP_GROW_SIZE) == 0)
            group.reserve(GROUP_GROW_SIZE * (1 + (groups.size() >> GROUP_GROW_SHIFT)));
        group.resize(group.count() + 1);

        KXmlPackedItem& item = group[group.count()-1];
#endif

        // this is necessary, because intentionally we don't want to have
        // a constructor for KXmlPackedItem
        item.attr = false;
        item.type = KXmlNode::NullNode;
        item.qnameIndex = 0;
        item.childStart = itemCount(depth + 1);
        item.value.clear();

        return item;
    }

    void clear() {
        currentDepth = 0;
        qnameHash.clear();
        qnameList.clear();
        valueHash.clear();
        valueList.clear();
        groups.clear();
        docType.clear();

        // first node is root
        KXmlPackedItem& rootItem = newItem(0);
        rootItem.type = KXmlNode::DocumentNode;
    }

    void finish() {
        // won't be needed anymore
        qnameHash.clear();
        valueHash.clear();
        valueList.clear();

        // optimize, see documentation on QVector::squeeze
        for (int d = 0; d < groups.count(); d++) {
            KoXmlPackedGroup& group = groups[d];
            group.squeeze();
        }
    }

    // in case namespace processing, 'name' contains the prefix already
    void addElement(const QString& name, const QString& nsURI) {
        KXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KXmlNode::ElementNode;
        item.qnameIndex = cacheQName(name, nsURI);

        currentDepth++;
    }

    void closeElement() {
        currentDepth--;
    }

    void addDTD(const QString& dt) {
        docType = dt;
    }

    void addAttribute(const QString& name, const QString& nsURI, const QString& value) {
        KXmlPackedItem& item = newItem(currentDepth + 1);
        item.attr = true;
        item.qnameIndex = cacheQName(name, nsURI);
        //item.value = cacheValue( value );
        item.value = value;
    }

    void addText(const QString& text) {
        KXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KXmlNode::TextNode;
        item.value = text;
    }

    void addCData(const QString& text) {
        KXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KXmlNode::CDATASectionNode;
        item.value = text;
    }

    void addProcessingInstruction() {
        KXmlPackedItem& item = newItem(currentDepth + 1);
        item.type = KXmlNode::ProcessingInstructionNode;
    }

public:
    KXmlPackedDocument(): processNamespace(false), currentDepth(0) {
        clear();
    }

#else

private:
    unsigned elementDepth;

public:

    KXmlPackedItem& newItem() {
        unsigned count = items.count() + 512;
        count = 1024 * (count >> 10);
        items.reserve(count);

        items.resize(items.count() + 1);

        // this is necessary, because intentionally we don't want to have
        // a constructor for KXmlPackedItem
        KXmlPackedItem& item = items[items.count()-1];
        item.attr = false;
        item.type = KXmlNode::NullNode;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.depth = 0;

        return item;
    }

    void addElement(const QString& name, const QString& nsURI) {
        // we are going one level deeper
        elementDepth++;

        KXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KXmlNode::ElementNode;
        item.depth = elementDepth;
        item.nameIndex = cacheString(name);
        item.nsURIIndex = cacheString(nsURI);
    }

    void closeElement() {
        // we are going up one level
        elementDepth--;
    }

    void addAttribute(const QString& name, const QString& nsURI, const QString& value) {
        KXmlPackedItem& item = newItem();

        item.attr = true;
        item.type = KXmlNode::NullNode;
        item.depth = elementDepth;
        item.nameIndex = cacheString(name);
        item.nsURIIndex = cacheString(nsURI);
        //item.value = cacheValue( value );
        item.value = value;
    }

    void addText(const QString& str) {
        KXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KXmlNode::TextNode;
        item.depth = elementDepth + 1;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.value = str;
    }

    void addCData(const QString& str) {
        KXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KXmlNode::CDATASectionNode;
        item.depth = elementDepth + 1;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.value = str;
    }

    void addProcessingInstruction() {
        KXmlPackedItem& item = newItem();

        item.attr = false;
        item.type = KXmlNode::ProcessingInstructionNode;
        item.depth = elementDepth + 1;
        item.nameIndex = 0;
        item.nsURIIndex = 0;
        item.value.clear();
    }

    void clear() {
        stringList.clear();
        stringHash.clear();
        valueHash.clear();
        valueList.clear();
        items.clear();
        elementDepth = 0;

        // reserve index #0
        cacheString(".");

        KXmlPackedItem& rootItem = newItem();
        rootItem.attr = false;
        rootItem.type = KXmlNode::DocumentNode;
        rootItem.depth = 0;
        rootItem.nsURIIndex = 0;
        rootItem.nameIndex = 0;
    }

    void finish() {
        stringHash.clear();
        valueList.clear();
        valueHash.clear();
        items.squeeze();
    }

    KXmlPackedDocument(): processNamespace(false), elementDepth(0) {
    }

#endif

};


namespace {

    class ParseError {
    public:
        QString errorMsg;
        int errorLine;
        int errorColumn;
        bool error;

        ParseError() :errorLine(-1), errorColumn(-1), error(false) {}
    };

    /// @param inText if true this is a subtree under a text:p or text:h
    void parseElement(QXmlStreamReader &xml, KXmlPackedDocument &doc, bool inText);

    // parse one element as if this were a standalone xml document
    ParseError parseDocument(QXmlStreamReader &xml, KXmlPackedDocument &doc) {
        doc.clear();
        ParseError error;
        xml.readNext();
        while (!xml.atEnd() && xml.tokenType() != QXmlStreamReader::EndDocument) {
            switch (xml.tokenType()) {
            case QXmlStreamReader::StartElement:
                parseElement(xml, doc, false);
                break;
            case QXmlStreamReader::DTD:
                doc.addDTD(xml.dtdName().toString());
                break;
            case QXmlStreamReader::StartDocument:
                if (!xml.documentEncoding().isEmpty() || !xml.documentVersion().isEmpty()) {
                    doc.addProcessingInstruction();
                }
                break;
            case QXmlStreamReader::ProcessingInstruction:
                doc.addProcessingInstruction();
                break;
            default:
                break;
            }
            xml.readNext();
        }
        if (xml.hasError()) {
            error.error = true;
            error.errorMsg = xml.errorString();
            error.errorColumn = xml.columnNumber();
            error.errorLine = xml.lineNumber();
        } else {
            doc.finish();
        }
        return error;
    }

    void parseElementContents(QXmlStreamReader &xml, KXmlPackedDocument &doc, bool inText)
    {
        xml.readNext();
        bool appendWhitespace = false;
        while (!xml.atEnd()) {
            switch (xml.tokenType()) {
            case QXmlStreamReader::EndElement:
                // if an element contains only whitespace, put it in the dom
                if (appendWhitespace)
                    doc.addText(QLatin1String(" "));
                return;
            case QXmlStreamReader::StartElement: {
                bool textNode = false;
                if (!inText) {
                    const QStringRef name = xml.name();
                    if (xml.name().size() == 1
                            && (name.at(0).unicode() == 'p' || name.at(0).unicode() == 'h')
                            && KOdfXmlNS::text == xml.namespaceUri().toString()) {
                        // we just stumbled upon a 'text:h' or 'text:p'
                        textNode = true;
                    }
                }
                if (appendWhitespace)
                    doc.addText(QLatin1String(" "));
                appendWhitespace = false;
                parseElement(xml, doc, inText || textNode);
                break;
            }
            case QXmlStreamReader::Characters:
                if (xml.isCDATA()) {
                    doc.addCData(xml.text().toString());
                } else if (!xml.isWhitespace()) {
                    doc.addText(xml.text().toString());
                } else if (inText) {
                    appendWhitespace = true;
                }
                break;
            case QXmlStreamReader::ProcessingInstruction:
                doc.addProcessingInstruction();
                break;
            default:
                break;
            }
            xml.readNext();
        }
    }

    void parseElement(QXmlStreamReader &xml, KXmlPackedDocument &doc, bool inText) {
        // reader.tokenType() is now QXmlStreamReader::StartElement
        doc.addElement(xml.qualifiedName().toString(),
                       fixNamespace(xml.namespaceUri().toString()));
        QXmlStreamAttributes attr = xml.attributes();
        QXmlStreamAttributes::const_iterator a = attr.constBegin();
        while (a != attr.constEnd()) {
            doc.addAttribute(a->qualifiedName().toString(),
                             a->namespaceUri().toString(),
                             a->value().toString());
            ++a;
        }
        parseElementContents(xml, doc, inText);
        // reader.tokenType() is now QXmlStreamReader::EndElement
        doc.closeElement();
    }

}


// ==================================================================
//
//         KXmlNodeData
//
// ==================================================================

class KXmlNodeData
{
public:

    KXmlNodeData();
    ~KXmlNodeData();

    // generic properties
    KXmlNode::NodeType nodeType;
    QString tagName;
    QString namespaceURI;
    QString prefix;
    QString localName;

#ifdef KOXML_COMPACT
    unsigned nodeDepth;
#endif

    // reference counting
    unsigned long count;
    void ref() {
        count++;
    }
    void unref() {
        if (this == &null) return; if (!--count) delete this;
    }

    // type information
    bool emptyDocument;
    QString nodeName() const;

    // for tree and linked-list
    KXmlNodeData* parent;
    KXmlNodeData* prev;
    KXmlNodeData* next;
    KXmlNodeData* first;
    KXmlNodeData* last;

    QString text();

    // node manipulation
    void appendChild(KXmlNodeData* child);
    void clear();

    // attributes
    inline void setAttribute(const QString& name, const QString& value);
    inline QString attribute(const QString& name, const QString& def) const;
    inline bool hasAttribute(const QString& name) const;
    inline void setAttributeNS(const QString& nsURI, const QString& name, const QString& value);
    inline QString attributeNS(const QString& nsURI, const QString& name, const QString& def) const;
    inline bool hasAttributeNS(const QString& nsURI, const QString& name) const;
    inline void clearAttributes();
    inline QStringList attributeNames() const;
    inline QList<QPair<QString, QString> > attributeNSNames() const;

    // for text and CDATA
    QString data() const;
    void setData(const QString& data);

    // reference from within the packed doc
    KXmlPackedDocument* packedDoc;
    unsigned long nodeIndex;

    // for document node
    bool setContent(QXmlStreamReader *reader,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);

    // used when doing on-demand (re)parse
    bool loaded;
    void loadChildren(int depth = 1);
    void unloadChildren();

    void dump();

    static KXmlNodeData null;

    // compatibility
    QDomNode asQDomNode(QDomDocument ownerDoc) const;

private:
    QHash<QString, QString> attr;
    QHash<KoXmlStringPair, QString> attrNS;
    QString textData;
    friend class KXmlElement;
};

KXmlNodeData KXmlNodeData::null;

KXmlNodeData::KXmlNodeData() : nodeType(KXmlNode::NullNode),
#ifdef KOXML_COMPACT
        nodeDepth(0),
#endif
        count(1), emptyDocument(true), parent(0), prev(0), next(0), first(0), last(0),
        packedDoc(0), nodeIndex(0), loaded(false)
{
}

KXmlNodeData::~KXmlNodeData()
{
    clear();
}

void KXmlNodeData::clear()
{
    if (first)
        for (KXmlNodeData* node = first; node ;) {
            KXmlNodeData* next = node->next;
            node->unref();
            node = next;
        }

    // only document can delete these
    // normal nodes don't "own" them
    if (nodeType == KXmlNode::DocumentNode)
        delete packedDoc;

    nodeType = KXmlNode::NullNode;
    tagName.clear();
    prefix.clear();
    namespaceURI.clear();
    textData.clear();
    packedDoc = 0;

    attr.clear();
    attrNS.clear();

    parent = 0;
    prev = next = 0;
    first = last = 0;

    loaded = false;
}

QString KXmlNodeData::text()
{
    QString t;

    loadChildren();

    KXmlNodeData* node = first;
    while (node) {
        switch (node->nodeType) {
        case KXmlNode::ElementNode:
            t += node->text(); break;
        case KXmlNode::TextNode:
            t += node->data(); break;
        case KXmlNode::CDATASectionNode:
            t += node->data(); break;
        default: break;
        }
        node = node->next;
    }

    return t;
}

QString KXmlNodeData::nodeName() const
{
    switch (nodeType) {
    case KXmlNode::ElementNode: {
        QString n(tagName);
        if (!prefix.isEmpty())
            n.prepend(':').prepend(prefix);
        return n;
    }
    break;

    case KXmlNode::TextNode:         return QLatin1String("#text");
    case KXmlNode::CDATASectionNode: return QLatin1String("#cdata-section");
    case KXmlNode::DocumentNode:     return QLatin1String("#document");
    case KXmlNode::DocumentTypeNode: return tagName;

    default: return QString(); break;
    }

    // should not happen
    return QString();
}

void KXmlNodeData::appendChild(KXmlNodeData* node)
{
    node->parent = this;
    if (!last)
        first = last = node;
    else {
        last->next = node;
        node->prev = last;
        node->next = 0;
        last = node;
    }
}

void KXmlNodeData::setAttribute(const QString& name, const QString& value)
{
    attr[ name ] = value;
}

QString KXmlNodeData::attribute(const QString& name, const QString& def) const
{
    if (attr.contains(name))
        return attr[ name ];
    else
        return def;
}

bool KXmlNodeData::hasAttribute(const QString& name) const
{
    return attr.contains(name);
}

void KXmlNodeData::setAttributeNS(const QString& nsURI,
                                   const QString& name, const QString& value)
{
    int i = name.indexOf(':');
    if (i != -1) {
        QString localName(name.mid(i + 1));
        KoXmlStringPair key(nsURI, localName);
        attrNS.insert(key, value);
    }
}

QString KXmlNodeData::attributeNS(const QString& nsURI, const QString& name,
                                   const QString& def) const
{
    KoXmlStringPair key(nsURI, name);
    return attrNS.value(key, def);
}

bool KXmlNodeData::hasAttributeNS(const QString& nsURI, const QString& name) const
{
    KoXmlStringPair key(nsURI, name);
    return attrNS.contains(key);
}

void KXmlNodeData::clearAttributes()
{
    attr.clear();
    attrNS.clear();
}

// FIXME how about namespaced attributes ?
QStringList KXmlNodeData::attributeNames() const
{
    QStringList result;
    result = attr.keys();

    return result;
}

QList<QPair<QString, QString> > KXmlNodeData::attributeNSNames() const
{
    QList<QPair<QString, QString> > result;
    result = attrNS.keys();

    return result;
}

QString KXmlNodeData::data() const
{
    return textData;
}

void KXmlNodeData::setData(const QString& d)
{
    textData = d;
}

bool KXmlNodeData::setContent(QXmlStreamReader* reader, QString* errorMsg, int* errorLine, int* errorColumn)
{
    if (nodeType != KXmlNode::DocumentNode)
        return false;

    clear();
    nodeType = KXmlNode::DocumentNode;

    // sanity checks
    if (!reader) return false;

    delete packedDoc;
    packedDoc = new KXmlPackedDocument;
    packedDoc->processNamespace = reader->namespaceProcessing();

    ParseError error = parseDocument(*reader, *packedDoc);
    if (error.error) {
        // parsing error has occurred
        if (errorMsg) *errorMsg = error.errorMsg;
        if (errorLine) *errorLine = error.errorLine;
        if (errorColumn)  *errorColumn = error.errorColumn;
        return false;
    }

    // initially load
    loadChildren();

    return true;
}

#ifdef KOXML_COMPACT

void KXmlNodeData::loadChildren(int depth)
{
    // sanity check
    if (!packedDoc) return;

    // already loaded ?
    if (loaded && (depth <= 1)) return;

    // in case depth is different
    unloadChildren();


    KXmlNodeData* lastDat = 0;

    unsigned childStop = 0;
    if (nodeIndex == packedDoc->itemCount(nodeDepth) - 1)
        childStop = packedDoc->itemCount(nodeDepth + 1);
    else {
        const KXmlPackedItem& next = packedDoc->itemAt(nodeDepth, nodeIndex + 1);
        childStop = next.childStart;
    }

    const KXmlPackedItem& self = packedDoc->itemAt(nodeDepth, nodeIndex);

    for (unsigned i = self.childStart; i < childStop; i++) {
        const KXmlPackedItem& item = packedDoc->itemAt(nodeDepth + 1, i);
        bool textItem = (item.type == KXmlNode::TextNode);
        textItem |= (item.type == KXmlNode::CDATASectionNode);

        // attribute belongs to this node
        if (item.attr) {
            KOdfQName qname = packedDoc->qnameList[item.qnameIndex];
            QString value = item.value;

            QString prefix;

            QString qName; // with prefix
            QString localName;  // without prefix, i.e. local name

            localName = qName = qname.name;
            int i = qName.indexOf(':');
            if (i != -1) prefix = qName.left(i);
            if (i != -1) localName = qName.mid(i + 1);

            if (packedDoc->processNamespace) {
                setAttributeNS(qname.nsURI, qName, value);
                setAttribute(localName, value);
            } else
                setAttribute(qName, value);
        } else {
            KOdfQName qname = packedDoc->qnameList[item.qnameIndex];
            QString value = item.value;

            QString nodeName = qname.name;
            QString localName;
            QString prefix;

            if (packedDoc->processNamespace) {
                localName = qname.name;
                int di = qname.name.indexOf(':');
                if (di != -1) {
                    localName = qname.name.mid(di + 1);
                    prefix = qname.name.left(di);
                }
                nodeName = localName;
            }

            // make a node out of this item
            KXmlNodeData* dat = new KXmlNodeData;
            dat->nodeIndex = i;
            dat->packedDoc = packedDoc;
            dat->nodeDepth = nodeDepth + 1;
            dat->nodeType = item.type;
            dat->tagName = nodeName;
            dat->localName = localName;
            dat->prefix = prefix;
            dat->namespaceURI = qname.nsURI;
            dat->count = 1;
            dat->parent = this;
            dat->prev = lastDat;
            dat->next = 0;
            dat->first = 0;
            dat->last = 0;
            dat->loaded = false;
            dat->textData = (textItem) ? value : QString();

            // adjust our linked-list
            first = (first) ? first : dat;
            last = dat;
            if (lastDat)
                lastDat->next = dat;
            lastDat = dat;

            // recursive
            if (depth > 1)
                dat->loadChildren(depth - 1);
        }
    }

    loaded = true;
}

#else

void KXmlNodeData::loadChildren(int depth)
{
    // sanity check
    if (!packedDoc) return;

    // already loaded ?
    if (loaded && (depth <= 1)) return;

    // cause we don't know how deep this node's children already loaded are
    unloadChildren();

    KXmlNodeData* lastDat = 0;
    int nodeDepth = packedDoc->items[nodeIndex].depth;

    for (int i = nodeIndex + 1; i < packedDoc->items.count(); i++) {
        KXmlPackedItem& item = packedDoc->items[i];
        bool textItem = (item.type == KXmlNode::TextNode);
        textItem |= (item.type == KXmlNode::CDATASectionNode);

        // element already outside our depth
        if (!item.attr && (item.type == KXmlNode::ElementNode))
            if (item.depth <= (unsigned)nodeDepth)
                break;

        // attribute belongs to this node
        if (item.attr && (item.depth == (unsigned)nodeDepth)) {
            QString name = packedDoc->stringList[item.nameIndex];
            QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);
            QString value = item.value;

            QString prefix;

            QString qName; // with prefix
            QString localName;  // without prefix, i.e. local name

            localName = qName = name;
            int i = qName.indexOf(':');
            if (i != -1) prefix = qName.left(i);
            if (i != -1) localName = qName.mid(i + 1);

            if (packedDoc->processNamespace) {
                setAttributeNS(nsURI, qName, value);
                setAttribute(localName, value);
            } else
                setAttribute(name, value);
        }

        // the child node
        if (!item.attr) {
            bool instruction = (item.type == KXmlNode::ProcessingInstructionNode);
            bool ok = (textItem || instruction)  ? (item.depth == (unsigned)nodeDepth) : (item.depth == (unsigned)nodeDepth + 1);

            ok = (item.depth == (unsigned)nodeDepth + 1);

            if (ok) {
                QString name = packedDoc->stringList[item.nameIndex];
                QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);
                QString value = item.value;

                QString nodeName = name;
                QString localName;
                QString prefix;

                if (packedDoc->processNamespace) {
                    localName = name;
                    int di = name.indexOf(':');
                    if (di != -1) {
                        localName = name.mid(di + 1);
                        prefix = name.left(di);
                    }
                    nodeName = localName;
                }

                // make a node out of this item
                KXmlNodeData* dat = new KXmlNodeData;
                dat->nodeIndex = i;
                dat->packedDoc = packedDoc;
                dat->nodeType = item.type;
                dat->tagName = nodeName;
                dat->localName = localName;
                dat->prefix = prefix;
                dat->namespaceURI = nsURI;
                dat->count = 1;
                dat->parent = this;
                dat->prev = lastDat;
                dat->next = 0;
                dat->first = 0;
                dat->last = 0;
                dat->loaded = false;
                dat->textData = (textItem) ? value : QString();

                // adjust our linked-list
                first = (first) ? first : dat;
                last = dat;
                if (lastDat)
                    lastDat->next = dat;
                lastDat = dat;

                // recursive
                if (depth > 1)
                    dat->loadChildren(depth - 1);
            }
        }
    }

    loaded = true;
}
#endif

void KXmlNodeData::unloadChildren()
{
    // sanity check
    if (!packedDoc) return;

    if (!loaded) return;

    if (first)
        for (KXmlNodeData* node = first; node ;) {
            KXmlNodeData* next = node->next;
            node->unloadChildren();
            node->unref();
            node = next;
        }

    clearAttributes();
    loaded = false;
    first = last = 0;
}

#ifdef KOXML_COMPACT


static QDomNode itemAsQDomNode(QDomDocument ownerDoc, KXmlPackedDocument* packedDoc,
                               unsigned nodeDepth, unsigned nodeIndex)
{
    // sanity check
    if (!packedDoc)
        return QDomNode();

    const KXmlPackedItem& self = packedDoc->itemAt(nodeDepth, nodeIndex);

    unsigned childStop = 0;
    if (nodeIndex == packedDoc->itemCount(nodeDepth) - 1)
        childStop = packedDoc->itemCount(nodeDepth + 1);
    else {
        const KXmlPackedItem& next = packedDoc->itemAt(nodeDepth, nodeIndex + 1);
        childStop = next.childStart;
    }

    // nothing to do here
    if (self.type == KXmlNode::NullNode)
        return QDomNode();

    // create the element properly
    if (self.type == KXmlNode::ElementNode) {
        QDomElement element;

        KOdfQName qname = packedDoc->qnameList[self.qnameIndex];
        qname.nsURI = fixNamespace(qname.nsURI);

        if (packedDoc->processNamespace)
            element = ownerDoc.createElementNS(qname.nsURI, qname.name);
        else
            element = ownerDoc.createElement(qname.name);

        // check all subnodes for attributes
        for (unsigned i = self.childStart; i < childStop; i++) {
            const KXmlPackedItem& item = packedDoc->itemAt(nodeDepth + 1, i);
            bool textItem = (item.type == KXmlNode::TextNode);
            textItem |= (item.type == KXmlNode::CDATASectionNode);

            // attribute belongs to this node
            if (item.attr) {
                KOdfQName qname = packedDoc->qnameList[item.qnameIndex];
                qname.nsURI = fixNamespace(qname.nsURI );
                QString value = item.value;

                QString prefix;

                QString qName; // with prefix
                QString localName;  // without prefix, i.e. local name

                localName = qName = qname.name;
                int i = qName.indexOf(':');
                if (i != -1) prefix = qName.left(i);
                if (i != -1) localName = qName.mid(i + 1);

                if (packedDoc->processNamespace) {
                    element.setAttributeNS(qname.nsURI, qName, value);
                    element.setAttribute(localName, value);
                } else
                    element.setAttribute(qname.name, value);
            } else {
                // add it recursively
                QDomNode childNode = itemAsQDomNode(ownerDoc, packedDoc, nodeDepth + 1, i);
                element.appendChild(childNode);
            }
        }

        return element;
    }

    // create the text node
    if (self.type == KXmlNode::TextNode) {
        QString text = self.value;

        // FIXME: choose CDATA when the value contains special characters
        QDomText textNode = ownerDoc.createTextNode(text);
        return textNode;
    }

    // nothing matches? strange...
    return QDomNode();
}

QDomNode KXmlNodeData::asQDomNode(QDomDocument ownerDoc) const
{
    return itemAsQDomNode(ownerDoc, packedDoc, nodeDepth, nodeIndex);
}

#else

static QDomNode itemAsQDomNode(QDomDocument ownerDoc, KXmlPackedDocument* packedDoc,
                               unsigned nodeIndex)
{
    // sanity check
    if (!packedDoc)
        return QDomNode();

    KXmlPackedItem& item = packedDoc->items[nodeIndex];

    // nothing to do here
    if (item.type == KXmlNode::NullNode)
        return QDomNode();

    // create the element properly
    if (item.type == KXmlNode::ElementNode) {
        QDomElement element;

        QString name = packedDoc->stringList[item.nameIndex];
        QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);

        if (packedDoc->processNamespace)
            element = ownerDoc.createElementNS(nsURI, name);
        else
            element = ownerDoc.createElement(name);

        // check all subnodes for attributes
        int nodeDepth = item.depth;
        for (int i = nodeIndex + 1; i < packedDoc->items.count(); i++) {
            KXmlPackedItem& item = packedDoc->items[i];
            bool textItem = (item.type == KXmlNode::TextNode);
            textItem |= (item.type == KXmlNode::CDATASectionNode);

            // element already outside our depth
            if (!item.attr && (item.type == KXmlNode::ElementNode))
                if (item.depth <= (unsigned)nodeDepth)
                    break;

            // attribute belongs to this node
            if (item.attr && (item.depth == (unsigned)nodeDepth)) {
                QString name = packedDoc->stringList[item.nameIndex];
                QString nsURI = fixNamespace(packedDoc->stringList[item.nsURIIndex]);
                QString value = item.value;
                QString prefix;

                QString qName; // with prefix
                QString localName;  // without prefix, i.e. local name

                localName = qName = name;
                int i = qName.indexOf(':');
                if (i != -1) prefix = qName.left(i);
                if (i != -1) localName = qName.mid(i + 1);

                if (packedDoc->processNamespace) {
                    element.setAttributeNS(nsURI, qName, value);
                    element.setAttribute(localName, value);
                } else
                    element.setAttribute(name, value);
            }

            // direct child of this node
            if (!item.attr && (item.depth == (unsigned)nodeDepth + 1)) {
                // add it recursively
                QDomNode childNode = itemAsQDomNode(ownerDoc, packedDoc, i);
                element.appendChild(childNode);
            }
        }

        return element;
    }

    // create the text node
    if (item.type == KXmlNode::TextNode) {
        QString text = item.value;
        // FIXME: choose CDATA when the value contains special characters
        QDomText textNode = ownerDoc.createTextNode(text);
        return textNode;
    }

    // nothing matches? strange...
    return QDomNode();
}

QDomNode KXmlNodeData::asQDomNode(QDomDocument ownerDoc) const
{
    return itemAsQDomNode(ownerDoc, packedDoc, nodeIndex);
}

#endif

void KXmlNodeData::dump()
{
    printf("NodeData %p\n", (void*)this);

    printf("  nodeIndex: %d\n", (int)nodeIndex);
    printf("  packedDoc: %p\n", (void*)packedDoc);

    printf("  nodeType : %d\n", (int)nodeType);
    printf("  tagName: %s\n", qPrintable(tagName));
    printf("  namespaceURI: %s\n", qPrintable(namespaceURI));
    printf("  prefix: %s\n", qPrintable(prefix));
    printf("  localName: %s\n", qPrintable(localName));

    printf("  parent : %p\n", (void*)parent);
    printf("  prev : %p\n", (void*)prev);
    printf("  next : %p\n", (void*)next);
    printf("  first : %p\n", (void*)first);
    printf("  last : %p\n", (void*)last);

    printf("  count: %ld\n", count);

    if (loaded)
        printf("  loaded: TRUE\n");
    else
        printf("  loaded: FALSE\n");
}

// ==================================================================
//
//         KXmlNode
//
// ==================================================================

// Creates a null node
KXmlNode::KXmlNode()
{
    d = &KXmlNodeData::null;
}

// Destroys this node
KXmlNode::~KXmlNode()
{
    if (d)
        if (d != &KXmlNodeData::null)
            d->unref();

    d = 0;
}

// Creates a copy of another node
KXmlNode::KXmlNode(const KXmlNode& node)
{
    d = node.d;
    d->ref();
}

// Creates a node for specific implementation
KXmlNode::KXmlNode(KXmlNodeData* data)
{
    d = data;
    data->ref();
}

// Creates a shallow copy of another node
KXmlNode& KXmlNode::operator=(const KXmlNode & node)
{
    d->unref();
    d = node.d;
    d->ref();
    return *this;
}

// Note: two null nodes are always equal
bool KXmlNode::operator==(const KXmlNode& node) const
{
    if (isNull() && node.isNull()) return true;
    return(d == node.d);
}

// Note: two null nodes are always equal
bool KXmlNode::operator!=(const KXmlNode& node) const
{
    if (isNull() && !node.isNull()) return true;
    if (!isNull() && node.isNull()) return true;
    if (isNull() && node.isNull()) return false;
    return(d != node.d);
}

KXmlNode::NodeType KXmlNode::nodeType() const
{
    return d->nodeType;
}

bool KXmlNode::isNull() const
{
    return d->nodeType == NullNode;
}

bool KXmlNode::isElement() const
{
    return d->nodeType == ElementNode;
}

bool KXmlNode::isText() const
{
    return (d->nodeType == TextNode) || isCDATASection();
}

bool KXmlNode::isCDATASection() const
{
    return d->nodeType == CDATASectionNode;
}

bool KXmlNode::isDocument() const
{
    return d->nodeType == DocumentNode;
}

bool KXmlNode::isDocumentType() const
{
    return d->nodeType == DocumentTypeNode;
}

void KXmlNode::clear()
{
    d->unref();
    d = new KXmlNodeData;
}

QString KXmlNode::nodeName() const
{
    return d->nodeName();
}

QString KXmlNode::prefix() const
{
    return isElement() ? d->prefix : QString();
}

QString KXmlNode::namespaceURI() const
{
    return isElement() ? d->namespaceURI : QString();
}

QString KXmlNode::localName() const
{
    return isElement() ? d->localName : QString();
}

KXmlDocument KXmlNode::ownerDocument() const
{
    KXmlNodeData* node = d;
    while (node->parent) node = node->parent;

    return KXmlDocument(node);
}

KXmlNode KXmlNode::parentNode() const
{
    return d->parent ? KXmlNode(d->parent) : KXmlNode();
}

bool KXmlNode::hasChildNodes() const
{
    if (isText())
        return false;

    if (!d->loaded)
        d->loadChildren();

    return d->first != 0 ;
}

int KXmlNode::childNodesCount() const
{
    if (isText())
        return 0;

    if (!d->loaded)
        d->loadChildren();

    KXmlNodeData* node = d->first;
    int count = 0;
    while (node) {
        count++;
        node = node->next;
    }

    return count;
}

QStringList KXmlNode::attributeNames() const
{
    if (!d->loaded)
        d->loadChildren();

    return d->attributeNames();
}

QList<QPair<QString, QString> > KXmlNode::attributeNSNames() const
{
    if (!d->loaded)
        d->loadChildren();
    
    return d->attributeNSNames();
}

KXmlNode KXmlNode::firstChild() const
{
    if (!d->loaded)
        d->loadChildren();
    return d->first ? KXmlNode(d->first) : KXmlNode();
}

KXmlNode KXmlNode::lastChild() const
{
    if (!d->loaded)
        d->loadChildren();
    return d->last ? KXmlNode(d->last) : KXmlNode();
}

KXmlNode KXmlNode::nextSibling() const
{
    return d->next ? KXmlNode(d->next) : KXmlNode();
}

KXmlNode KXmlNode::previousSibling() const
{
    return d->prev ? KXmlNode(d->prev) : KXmlNode();
}

KXmlNode KXmlNode::namedItem(const QString& name) const
{
    if (!d->loaded)
        d->loadChildren();

    KXmlNodeData* node = d->first;
    while (node) {
        if (node->nodeName() == name)
            return KXmlNode(node);
        node = node->next;
    }

    // not found
    return KXmlNode();
}

KXmlNode KXmlNode::namedItemNS(const QString& nsURI, const QString& name) const
{
    if (!d->loaded)
        d->loadChildren();

    KXmlNodeData* node = d->first;
    while (node) {
        if (node->nodeType == KXmlNode::ElementNode
                 && node->namespaceURI == nsURI
                 && node->localName == name) {
            return KXmlNode(node);
        }
        node = node->next;
    }

    // not found
    return KXmlNode();
}

KXmlElement KXmlNode::toElement() const
{
    return isElement() ? KXmlElement(d) : KXmlElement();
}

KoXmlText KXmlNode::toText() const
{
    return isText() ? KoXmlText(d) : KoXmlText();
}

KoXmlCDATASection KXmlNode::toCDATASection() const
{
    return isCDATASection() ? KoXmlCDATASection(d) : KoXmlCDATASection();
}

KXmlDocument KXmlNode::toDocument() const
{
    if (isDocument())
        return KXmlDocument(d);

    KXmlDocument newDocument;
    newDocument.d->emptyDocument = false;
    return newDocument;
}

void KXmlNode::load(int depth)
{
    d->loadChildren(depth);
}

void KXmlNode::unload()
{
    d->unloadChildren();
}

QDomNode KXmlNode::asQDomNode(QDomDocument ownerDoc) const
{
    return d->asQDomNode(ownerDoc);
}

// ==================================================================
//
//         KXmlElement
//
// ==================================================================

// Creates an empty element
KXmlElement::KXmlElement(): KXmlNode(new KXmlNodeData)
{
    // because referenced also once in KXmlNode constructor
    d->unref();
}

KXmlElement::~KXmlElement()
{
    if (d)
        if (d != &KXmlNodeData::null)
            d->unref();

    d = 0;
}

// Creates a shallow copy of another element
KXmlElement::KXmlElement(const KXmlElement& element): KXmlNode(element.d)
{
}

KXmlElement::KXmlElement(KXmlNodeData* data): KXmlNode(data)
{
}

// Copies another element
KXmlElement& KXmlElement::operator=(const KXmlElement & element)
{
    KXmlNode::operator=(element);
    return *this;
}

bool KXmlElement::operator== (const KXmlElement& element) const
{
    if (isNull() || element.isNull()) return false;
    return (d == element.d);
}

bool KXmlElement::operator!= (const KXmlElement& element) const
{
    if (isNull() && element.isNull()) return false;
    if (isNull() || element.isNull()) return true;
    return (d != element.d);
}

QString KXmlElement::tagName() const
{
    return isElement() ? ((KXmlNodeData*)d)->tagName : QString();
}

QString KXmlElement::text() const
{
    return d->text();
}

QString KXmlElement::attribute(const QString& name,
                                const QString& defaultValue) const
{
    if (!isElement())
        return defaultValue;

    if (!d->loaded)
        d->loadChildren();

    return d->attribute(name, defaultValue);
}

QString KXmlElement::attributeNS(const QString& namespaceURI,
                                  const QString& localName, const QString& defaultValue) const
{
    if (!isElement())
        return defaultValue;

    if (!d->loaded)
        d->loadChildren();

    KoXmlStringPair key(namespaceURI, localName);
    return d->attrNS.value(key, defaultValue);

//  return d->attributeNS( namespaceURI, localName, defaultValue );
}

bool KXmlElement::hasAttribute(const QString& name) const
{
    if (!d->loaded)
        d->loadChildren();

    return isElement() ? d->hasAttribute(name) : false;
}

bool KXmlElement::hasAttributeNS(const QString& namespaceURI,
                                  const QString& localName) const
{
    if (!d->loaded)
        d->loadChildren();

    return isElement() ? d->hasAttributeNS(namespaceURI, localName) : false;
}

// ==================================================================
//
//         KoXmlText
//
// ==================================================================

KoXmlText::KoXmlText(): KXmlNode(new KXmlNodeData)
{
    // because referenced also once in KXmlNode constructor
    d->unref();
}

KoXmlText::~KoXmlText()
{
    if (d)
        if (d != &KXmlNodeData::null)
            d->unref();

    d = 0;
}

KoXmlText::KoXmlText(const KoXmlText& text): KXmlNode(text.d)
{
}

KoXmlText::KoXmlText(KXmlNodeData* data): KXmlNode(data)
{
}

bool KoXmlText::isText() const
{
    return true;
}

QString KoXmlText::data() const
{
    return d->data();
}

KoXmlText& KoXmlText::operator=(const KoXmlText & element)
{
    KXmlNode::operator=(element);
    return *this;
}

// ==================================================================
//
//         KoXmlCDATASection
//
// ==================================================================

KoXmlCDATASection::KoXmlCDATASection(): KoXmlText()
{
    d->nodeType = KXmlNode::CDATASectionNode;
}

KoXmlCDATASection::KoXmlCDATASection(const KoXmlCDATASection& cdata)
        : KoXmlText(cdata)
{
    *this = cdata;
}

KoXmlCDATASection::~KoXmlCDATASection()
{
    d->unref();
    d = 0;
}

KoXmlCDATASection::KoXmlCDATASection(KXmlNodeData* cdata):
        KoXmlText(cdata)
{
}

bool KoXmlCDATASection::isCDATASection() const
{
    return true;
}

KoXmlCDATASection& KoXmlCDATASection::operator=(const KoXmlCDATASection & cdata)
{
    KXmlNode::operator=(cdata);
    return *this;
}

// ==================================================================
//
//         KoXmlDocumentType
//
// ==================================================================

KoXmlDocumentType::KoXmlDocumentType(): KXmlNode(new KXmlNodeData)
{
    // because referenced also once in KXmlNode constructor
    d->unref();
}

KoXmlDocumentType::~KoXmlDocumentType()
{
    d->unref();
    d = 0;
}

KoXmlDocumentType::KoXmlDocumentType(const KoXmlDocumentType& dt):
        KXmlNode(dt.d)
{
}

QString KoXmlDocumentType::name() const
{
    return nodeName();
}

KoXmlDocumentType::KoXmlDocumentType(KXmlNodeData* dt): KXmlNode(dt)
{
}

KoXmlDocumentType& KoXmlDocumentType::operator=(const KoXmlDocumentType & dt)
{
    KXmlNode::operator=(dt);
    return *this;
}

// ==================================================================
//
//         KXmlDocument
//
// ==================================================================

KXmlDocument::KXmlDocument(): KXmlNode()
{
    d->emptyDocument = false;
}

KXmlDocument::~KXmlDocument()
{
    if (d)
        if (d != &KXmlNodeData::null)
            d->unref();

    d = 0;
}

KXmlDocument::KXmlDocument(KXmlNodeData* data): KXmlNode(data)
{
    d->emptyDocument = true;
}

// Creates a copy of another document
KXmlDocument::KXmlDocument(const KXmlDocument& doc): KXmlNode(doc.d)
{
}

// Creates a shallow copy of another document
KXmlDocument& KXmlDocument::operator=(const KXmlDocument & doc)
{
    KXmlNode::operator=(doc);
    return *this;
}

// Checks if this document and doc are equals
bool KXmlDocument::operator==(const KXmlDocument& doc) const
{
    return(d == doc.d);
}

// Checks if this document and doc are not equals
bool KXmlDocument::operator!=(const KXmlDocument& doc) const
{
    return(d != doc.d);
}

KXmlElement KXmlDocument::documentElement() const
{
    d->loadChildren();

    for (KXmlNodeData* node = d->first; node;) {
        if (node->nodeType == KXmlNode::ElementNode)
            return KXmlElement(node);
        else node = node->next;
    }

    return KXmlElement();
}

KoXmlDocumentType KXmlDocument::doctype() const
{
    return dt;
}

QString KXmlDocument::nodeName() const
{
    if (d->emptyDocument)
        return QLatin1String("#document");
    else
        return QString();
}

void KXmlDocument::clear()
{
    KXmlNode::clear();
    d->emptyDocument = false;
}

namespace {
    /* Use an entity resolver that ignores undefined entities and simply
       returns an empty string for them.
       */
    class DumbEntityResolver : public QXmlStreamEntityResolver {
    public:
        QString resolveUndeclaredEntity ( const QString &) { return ""; }
    };

}

bool KXmlDocument::setContent(QXmlStreamReader *reader,
                               QString* errorMsg, int* errorLine, int* errorColumn)
{
    if (d->nodeType != KXmlNode::DocumentNode) {
        d->unref();
        d = new KXmlNodeData;
        d->nodeType = KXmlNode::DocumentNode;
    }

    dt = KoXmlDocumentType();
    bool result = d->setContent(reader, errorMsg, errorLine, errorColumn);
    if (result && !isNull()) {
        dt.d->nodeType = KXmlNode::DocumentTypeNode;
        dt.d->tagName = d->packedDoc->docType;
        dt.d->parent = d;
    }

    return result;
}

// no namespace processing
bool KXmlDocument::setContent(QIODevice* device, QString* errorMsg,
                               int* errorLine, int* errorColumn)
{
    return setContent(device, false, errorMsg, errorLine, errorColumn);
}

bool KXmlDocument::setContent(QIODevice* device, bool namespaceProcessing,
                               QString* errorMsg, int* errorLine, int* errorColumn)
{
    if (d->nodeType != KXmlNode::DocumentNode) {
        d->unref();
        d = new KXmlNodeData;
        d->nodeType = KXmlNode::DocumentNode;
    }

    device->open(QIODevice::ReadOnly);
    QXmlStreamReader reader(device);
    reader.setNamespaceProcessing(namespaceProcessing);
    DumbEntityResolver entityResolver;
    reader.setEntityResolver(&entityResolver);

    dt = KoXmlDocumentType();
    bool result = d->setContent(&reader, errorMsg, errorLine, errorColumn);
    {
        dt.d->nodeType = KXmlNode::DocumentTypeNode;
        dt.d->tagName = d->packedDoc->docType;
        dt.d->parent = d;
    }

    return result;
}

bool KXmlDocument::setContent(const QByteArray& text, bool namespaceProcessing,
                               QString *errorMsg, int *errorLine, int *errorColumn)
{
    QBuffer buffer;
    buffer.setData(text);
    return setContent(&buffer, namespaceProcessing, errorMsg, errorLine, errorColumn);
}

bool KXmlDocument::setContent(const QString& text, bool namespaceProcessing,
                               QString *errorMsg, int *errorLine, int *errorColumn)
{
    if (d->nodeType != KXmlNode::DocumentNode) {
        d->unref();
        d = new KXmlNodeData;
        d->nodeType = KXmlNode::DocumentNode;
    }

    QXmlStreamReader reader(text);
    reader.setNamespaceProcessing(namespaceProcessing);
    DumbEntityResolver entityResolver;
    reader.setEntityResolver(&entityResolver);

    dt = KoXmlDocumentType();
    bool result = d->setContent(&reader, errorMsg, errorLine, errorColumn);
    if (result && !isNull()) {
        dt.d->nodeType = KXmlNode::DocumentTypeNode;
        dt.d->tagName = d->packedDoc->docType;
        dt.d->parent = d;
    }

    return result;
}

bool KXmlDocument::setContent(const QString& text,
                               QString *errorMsg, int *errorLine, int *errorColumn)
{
    return setContent(text, false, errorMsg, errorLine, errorColumn);
}

#endif

// ==================================================================
//
//         functions in KoXml namespace
//
// ==================================================================

KXmlElement KoXml::namedItemNS(const KXmlNode& node, const QString& nsURI,
                                const QString& localName)
{
#ifdef KOXML_USE_QDOM
    // David's solution for namedItemNS, only for QDom stuff
    KXmlNode n = node.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement() && n.localName() == localName &&
                n.namespaceURI() == nsURI)
            return n.toElement();
    }
    return KXmlElement();
#else
    return node.namedItemNS(nsURI, localName).toElement();
#endif
}

void KoXml::load(KXmlNode& node, int depth)
{
#ifdef KOXML_USE_QDOM
    // do nothing, QDom has no on-demand loading
    Q_UNUSED(node);
    Q_UNUSED(depth);
#else
    node.load(depth);
#endif
}


void KoXml::unload(KXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    // do nothing, QDom has no on-demand unloading
    Q_UNUSED(node);
#else
    node.unload();
#endif
}

int KoXml::childNodesCount(const KXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    return node.childNodes().count();
#else
    // compatibility function, because no need to implement
    // a class like QDomNodeList
    return node.childNodesCount();
#endif
}

QStringList KoXml::attributeNames(const KXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    QStringList result;

    QDomNamedNodeMap attrMap = node.attributes();
    for (int i = 0; i < attrMap.count(); i++)
        result += attrMap.item(i).toAttr().name();

    return result;
#else
    // compatibility function, because no need to implement
    // a class like QDomNamedNodeMap
    return node.attributeNames();
#endif
}

QDomNode KoXml::asQDomNode(QDomDocument ownerDoc, const KXmlNode& node)
{
#ifdef KOXML_USE_QDOM
    Q_UNUSED(ownerDoc);
    return node;
#else
    return node.asQDomNode(ownerDoc);
#endif
}

QDomElement KoXml::asQDomElement(QDomDocument ownerDoc, const KXmlElement& element)
{
    return KoXml::asQDomNode(ownerDoc, element).toElement();
}

QDomDocument KoXml::asQDomDocument(QDomDocument ownerDoc, const KXmlDocument& document)
{
    return KoXml::asQDomNode(ownerDoc, document).toDocument();
}

bool KoXml::setDocument(KXmlDocument& doc, QIODevice* device,
                        bool namespaceProcessing, QString* errorMsg, int* errorLine,
                        int* errorColumn)
{
    QXmlStreamReader reader(device);
    reader.setNamespaceProcessing(namespaceProcessing);
    bool result = doc.setContent(&reader, errorMsg, errorLine, errorColumn);
    return result;
}
