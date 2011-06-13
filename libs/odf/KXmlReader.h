/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>

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

#ifndef KOFFICE_XMLREADER
#define KOFFICE_XMLREADER

#include "kodf_export.h"

#include <QtXml/qxml.h>
#include <QtXml/qdom.h>
#include <QtCore/qpair.h>

class QIODevice;
class QTextDecoder;

class QString;
class QXmlStreamReader;

class KXmlNode;
class KoXmlText;
class KoXmlCDATASection;
class KoXmlDocumentType;
class KXmlDocument;
class KXmlNodeData;
class KXmlElement;

/**
* KXmlNode represents a node in a DOM tree.
*
* KXmlNode is a base class for KXmlElement, KoXmlText.
* Often, these subclasses are used for getting the data instead of KXmlNode.
* However, as base class, KXmlNode is very helpful when for example iterating
* all child nodes within one parent node.
*
* KXmlNode implements an explicit sharing, a node shares its data with
* other copies (if exist).
*
* XXX: DO NOT ADD CONVENIENCE API HERE BECAUSE THIS CLASS MUST REMAIN COMPATIBLE WITH QDOMNODE!
*
* @author Ariya Hidayat <ariya@kde.org>
*/
class KODF_EXPORT KXmlNode
{
public:

    enum NodeType {
        NullNode = 0,
        ElementNode,
        TextNode,
        CDATASectionNode,
        ProcessingInstructionNode,
        DocumentNode,
        DocumentTypeNode
    };

    KXmlNode();
    KXmlNode(const KXmlNode& node);
    KXmlNode& operator=(const KXmlNode& node);
    bool operator== (const KXmlNode&) const;
    bool operator!= (const KXmlNode&) const;
    virtual ~KXmlNode();

    virtual KXmlNode::NodeType nodeType() const;
    virtual bool isNull() const;
    virtual bool isElement() const;
    virtual bool isText() const;
    virtual bool isCDATASection() const;
    virtual bool isDocument() const;
    virtual bool isDocumentType() const;

    virtual void clear();
    KXmlElement toElement() const;
    KoXmlText toText() const;
    KoXmlCDATASection toCDATASection() const;
    KXmlDocument toDocument() const;

    virtual QString nodeName() const;
    virtual QString namespaceURI() const;
    virtual QString prefix() const;
    virtual QString localName() const;

    KXmlDocument ownerDocument() const;
    KXmlNode parentNode() const;

    bool hasChildNodes() const;
    KXmlNode firstChild() const;
    KXmlNode lastChild() const;
    KXmlNode nextSibling() const;
    KXmlNode previousSibling() const;

    // equivalent to node.childNodes().count() if node is a QDomNode instance
    int childNodesCount() const;

    // workaround to get and iterate over all attributes
    QStringList attributeNames() const;
    QList<QPair<QString, QString> > attributeNSNames() const;

    KXmlNode namedItem(const QString& name) const;
    KXmlNode namedItemNS(const QString& nsURI, const QString& name) const;

    /**
    * Loads all child nodes (if any) of this node. Normally you do not need
    * to call this function as the child nodes will be automatically
    * loaded when necessary.
    */
    void load(int depth = 1);

    /**
    * Releases all child nodes of this node.
    */
    void unload();

    // compatibility
    QDomNode asQDomNode(QDomDocument ownerDoc) const;

protected:
    KXmlNodeData* d;
    KXmlNode(KXmlNodeData*);
};

/**
* KXmlElement represents a tag element in a DOM tree.
*
* KXmlElement holds information about an XML tag, along with its attributes.
*
* @author Ariya Hidayat <ariya@kde.org>
*/

class KODF_EXPORT KXmlElement: public KXmlNode
{
public:
    KXmlElement();
    KXmlElement(const KXmlElement& element);
    KXmlElement& operator=(const KXmlElement& element);
    virtual ~KXmlElement();
    bool operator== (const KXmlElement&) const;
    bool operator!= (const KXmlElement&) const;

    QString tagName() const;
    QString text() const;

    QString attribute(const QString& name, const QString& defaultValue = QString()) const;
    QString attributeNS(const QString& namespaceURI, const QString& localName,
                        const QString& defaultValue = QString()) const;
    bool hasAttribute(const QString& name) const;
    bool hasAttributeNS(const QString& namespaceURI, const QString& localName) const;

private:
    friend class KXmlNode;
    friend class KXmlDocument;
    KXmlElement(KXmlNodeData*);
};

/**
* KoXmlText represents a text in a DOM tree.
* @author Ariya Hidayat <ariya@kde.org>
*/
class KODF_EXPORT KoXmlText: public KXmlNode
{
public:
    KoXmlText();
    KoXmlText(const KoXmlText& text);
    KoXmlText& operator=(const KoXmlText& text);
    virtual ~KoXmlText();

    QString data() const;
    void setData(const QString& data);
    virtual bool isText() const;

private:
    friend class KXmlNode;
    friend class KoXmlCDATASection;
    friend class KXmlDocument;
    KoXmlText(KXmlNodeData*);
};

/**
* KoXmlCDATASection represents a CDATA section in a DOM tree.
* @author Ariya Hidayat <ariya@kde.org>
*/
class KODF_EXPORT KoXmlCDATASection: public KoXmlText
{
public:
    KoXmlCDATASection();
    KoXmlCDATASection(const KoXmlCDATASection& cdata);
    KoXmlCDATASection& operator=(const KoXmlCDATASection& cdata);
    virtual ~KoXmlCDATASection();

    virtual bool isCDATASection() const;

private:
    friend class KXmlNode;
    friend class KXmlDocument;
    KoXmlCDATASection(KXmlNodeData*);
};

/**
* KoXmlDocumentType represents the DTD of the document. At the moment,
* it can used only to get the document type, i.e. no support for
* entities etc.
*
* @author Ariya Hidayat <ariya@kde.org>
*/

class KODF_EXPORT KoXmlDocumentType: public KXmlNode
{
public:
    KoXmlDocumentType();
    KoXmlDocumentType(const KoXmlDocumentType&);
    KoXmlDocumentType& operator=(const KoXmlDocumentType&);
    virtual ~KoXmlDocumentType();

    QString name() const;

private:
    friend class KXmlNode;
    friend class KXmlDocument;
    KoXmlDocumentType(KXmlNodeData*);
};


/**
* KXmlDocument represents an XML document, structured in a DOM tree.
*
* KXmlDocument is designed to be memory efficient. Unlike QDomDocument from
* Qt's XML module, KXmlDocument does not store all nodes in the DOM tree.
* Some nodes will be loaded and parsed on-demand only.
*
* KXmlDocument is read-only, you can not modify its content.
*
* @author Ariya Hidayat <ariya@kde.org>
*/

class KODF_EXPORT KXmlDocument: public KXmlNode
{
public:
    KXmlDocument();
    KXmlDocument(const KXmlDocument& node);
    KXmlDocument& operator=(const KXmlDocument& node);
    bool operator==(const KXmlDocument&) const;
    bool operator!=(const KXmlDocument&) const;
    virtual ~KXmlDocument();

    KXmlElement documentElement() const;

    KoXmlDocumentType doctype() const;

    virtual QString nodeName() const;
    virtual void clear();

    bool setContent(QIODevice* device, bool namespaceProcessing,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);
    bool setContent(QIODevice* device,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);
    bool setContent(QXmlStreamReader *reader,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);
    bool setContent(const QByteArray& text, bool namespaceProcessing,
                    QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);
    bool setContent(const QString& text, bool namespaceProcessing,
                    QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);

    // no namespace processing
    bool setContent(const QString& text,
                    QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);

private:
    friend class KXmlNode;
    KoXmlDocumentType dt;
    KXmlDocument(KXmlNodeData*);
};

/**
 * This namespace contains a few convenience functions to simplify code using QDom
 * (when loading OASIS documents, in particular).
 *
 * To find the child element with a given name, use KoXml::namedItemNS.
 *
 * To find all child elements with a given name, use
 * QDomElement e;
 * forEachElement( e, parent )
 * {
 *     if ( e.localName() == "..." && e.namespaceURI() == KOdfXmlNS::... )
 *     {
 *         ...
 *     }
 * }
 * Note that this means you don't ever need to use QDomNode nor toElement anymore!
 * Also note that localName is the part without the prefix, this is the whole point
 * of namespace-aware methods.
 *
 * To find the attribute with a given name, use QDomElement::attributeNS.
 *
 * Do not use getElementsByTagNameNS, it's recursive (which is never needed in KOffice).
 * Do not use tagName() or nodeName() or prefix(), since the prefix isn't fixed.
 *
 * @author David Faure <faure@kde.org>
 */
namespace KoXml
{

/**
 * A namespace-aware version of QDomNode::namedItem(),
 * which also takes care of casting to a QDomElement.
 * Use this when a domelement is known to have only *one* child element
 * with a given tagname.
 *
 * Note: do *NOT* use getElementsByTagNameNS, it's recursive!
 */
KODF_EXPORT KXmlElement namedItemNS(const KXmlNode& node,
                                        const QString& nsURI, const QString& localName);

/**
 * Explicitly load child nodes of specified node, up to given depth.
 * This function has no effect if QDom is used.
 */
KODF_EXPORT void load(KXmlNode& node, int depth = 1);

/**
 * Unload child nodes of specified node.
 * This function has no effect if QDom is used.
 */
KODF_EXPORT void unload(KXmlNode& node);

/**
 * Get the number of child nodes of specified node.
 */
KODF_EXPORT int childNodesCount(const KXmlNode& node);

/**
 * Return the name of all attributes of specified node.
 */
KODF_EXPORT QStringList attributeNames(const KXmlNode& node);

/**
 * Convert KoXml classes to the corresponding QDom classes, which has
 * 'ownerDoc' as the owner document (QDomDocument instance).
 */
KODF_EXPORT QDomNode asQDomNode(QDomDocument ownerDoc, const KXmlNode& node);
KODF_EXPORT QDomElement asQDomElement(QDomDocument ownerDoc, const KXmlElement& element);
KODF_EXPORT QDomDocument asQDomDocument(QDomDocument ownerDoc, const KXmlDocument& document);

/*
 * Load an XML document from specified device to a document. You can of
 * course use it with QFile (which inherits QIODevice).
 * This is much more memory efficient than standard QDomDocument::setContent
 * because the data from the device is buffered, unlike
 * QDomDocument::setContent which just loads everything in memory.
 *
 * Note: it is assumed that the XML uses UTF-8 encoding.
 */
KODF_EXPORT bool setDocument(KXmlDocument& doc, QIODevice* device,
                                bool namespaceProcessing, QString* errorMsg = 0,
                                int* errorLine = 0, int* errorColumn = 0);
}

/**
 * \def forEachElement( elem, parent )
 * \brief Loop through all child elements of \parent.
 * This convenience macro is used to implement the forEachElement loop.
 * The \elem parameter is a name of a QDomElement variable and the \parent
 * is the name of the parent element. For example:
 *
 * QDomElement e;
 * forEachElement( e, parent )
 * {
 *     kDebug() << e.localName() << " element found.";
 *     ...
 * }
 */
#define forEachElement( elem, parent ) \
    for ( KXmlNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
        if ( ( elem = _node.toElement() ).isNull() ) {} else


#endif // KOFFICE_XMLREADER
