#include "koOasisStyles.h"
#include <kdebug.h>
#include <qdom.h>

KoOasisStyles::KoOasisStyles()
{

}

KoOasisStyles::~KoOasisStyles()
{

}

void KoOasisStyles::createStyleMap( const QDomDocument& doc )
{
    QDomElement docElement  = doc.documentElement();
    QDomNode docStyles   = docElement.namedItem( "office:document-styles" );
    // We used to have the office:version check here, but better let the apps do that
    QDomNode fontStyles = docElement.namedItem( "office:font-decls" );

    if ( !fontStyles.isNull() ) {
        //kdDebug(30003) << "Starting reading in font-decl..." << endl;

        insertStyles( fontStyles.toElement() );
    }// else
    //   kdDebug(30003) << "No items found" << endl;

    //kdDebug(30003) << "Starting reading in office:automatic-styles" << endl;

    QDomNode autoStyles = docElement.namedItem( "office:automatic-styles" );
    if ( !autoStyles.isNull() ) {
        insertStyles( autoStyles.toElement() );
    }// else
    //    kdDebug(30003) << "No items found" << endl;


    //kdDebug(30003) << "Reading in master styles" << endl;

    QDomNode masterStyles = docElement.namedItem( "office:master-styles" );

    if ( !masterStyles.isNull() ) {
        QDomElement master = masterStyles.firstChild().toElement();
        for ( ; !master.isNull() ; master = master.nextSibling().toElement() ) {
            if ( master.tagName() ==  "style:master-page" ) {
                QString name = master.attribute( "style:name" );
                //kdDebug(30003) << "Master style: '" << name << "' loaded " << endl;
                m_masterPages.insert( name, new QDomElement( master ) );
            } else
                kdWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles" << endl;
        }
    }


    //kdDebug(30003) << "Starting reading in office:styles" << endl;

    QDomElement officeStyle = docElement.namedItem( "office:styles" ).toElement();

    if ( !officeStyle.isNull() ) {
        m_officeStyle = officeStyle;
        insertStyles( m_officeStyle );
    }

    //kdDebug(30003) << "Styles read in." << endl;
}

QValueVector<QDomElement> KoOasisStyles::userStyles() const
{
    QValueVector<QDomElement> vec;
    // Collect user styles
    QDomNodeList children = m_officeStyle.elementsByTagName( "style:style" );
    vec.resize( children.length() );
    for ( unsigned int i = 0 ; i < children.length(); ++i )
        vec[i] = children.item( i ).toElement();
    return vec;
}

void KoOasisStyles::insertStyles( const QDomElement& styles )
{
    //kdDebug(30003) << "Inserting styles from " << styles.tagName() << endl;
    for ( QDomNode n = styles.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement e = n.toElement();
        QString tagName = e.tagName();

        QString name = e.attribute( "style:name" );
        if ( tagName == "style:style"
             || tagName == "style:page-master"
             || tagName == "style:font-decl" )
        {
            QDomElement* ep = new QDomElement( e );
            m_styles.insert( name, ep );
            //kdDebug(30003) << "Style: '" << name << "' loaded " << endl;
        } else if ( tagName == "style:default-style" ) {
            m_defaultStyle = e;
        } else if ( tagName == "text:list-style" ) {
            QDomElement* ep = new QDomElement( e );
            m_listStyles.insert( name, ep );
            //kdDebug(30003) << "List style: '" << name << "' loaded " << endl;
        }
        // TODO: number-style, date-style and time-style here
        // The rest (*-configuration and outline-style) is to be done by the apps.
    }
}
