/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#include "kostyle.h"
#include "kooasiscontext.h"
#include "koparagcounter.h"

#include <koOasisStyles.h>
#include <koGenStyles.h>
#include <koxmlwriter.h>
#include <koxmlns.h>

#include <kdebug.h>
#include <klocale.h>

#include <qdom.h>

KoStyleCollection::KoStyleCollection()
{
    m_styleList.setAutoDelete( false );
    m_deletedStyles.setAutoDelete( true );
    m_lastStyle = 0L;
}

KoStyleCollection::~KoStyleCollection()
{
    clear();
}

void KoStyleCollection::clear()
{
    m_styleList.setAutoDelete( true );
    m_styleList.clear();
    m_styleList.setAutoDelete( false );
    m_deletedStyles.clear();
}

QStringList KoStyleCollection::translatedStyleNames() const
{
    QStringList lst;
    for( QPtrListIterator<KoParagStyle> p( m_styleList ); *p; ++p )
        lst << (*p)->displayName();
    return lst;
}

void KoStyleCollection::loadOasisStyleTemplates( KoOasisContext& context )
{
    QStringList followingStyles;
    QValueVector<QDomElement> userStyles = context.oasisStyles().userStyles();
    uint nStyles = userStyles.count();
    if( nStyles ) { // we are going to import at least one style.
        KoParagStyle *s = findStyle("Standard");
        //kdDebug() << "loadOasisStyleTemplates looking for Standard, to delete it. Found " << s << endl;
        if(s) // delete the standard style.
            removeStyleTemplate(s);
    }
    for (unsigned int item = 0; item < nStyles; item++) {
        QDomElement styleElem = userStyles[item];
	Q_ASSERT( !styleElem.isNull() );

        if ( styleElem.attributeNS( KoXmlNS::style, "family", QString::null ) != "paragraph" )
            continue;

        KoParagStyle *sty = new KoParagStyle( QString::null );
        // Load the style
        sty->loadStyle( styleElem, context );
        // Style created, now let's try to add it
        sty = addStyleTemplate( sty );
        // the real value of followingStyle is set below after loading all styles
        sty->setFollowingStyle( sty );

        kdDebug() << " Loaded style " << sty->name() << endl;

        if(styleList().count() > followingStyles.count() )
        {
            const QString following = styleElem.attributeNS( KoXmlNS::style, "next-style-name", QString::null );
            followingStyles.append( following );
        }
        else
            kdWarning() << "Found duplicate style declaration, overwriting former " << sty->name() << endl;
    }

    if( followingStyles.count() != styleList().count() ) {
        kdDebug() << "Ouch, " << followingStyles.count() << " following-styles, but "
                       << styleList().count() << " styles in styleList" << endl;
    }

    unsigned int i=0;
    for( QValueList<QString>::ConstIterator it = followingStyles.begin(); it != followingStyles.end(); ++it, ++i ) {
        const QString followingStyleName = *it;
	if ( !followingStyleName.isEmpty() ) {
            KoParagStyle * style = findStyle( followingStyleName );
	    if ( style )
                styleAt(i)->setFollowingStyle( style );
	}
    }

    // TODO the same thing for style inheritance (style:parent-style-name) and setParentStyle()

    Q_ASSERT( findStyle( "Standard" ) );
}

QMap<KoParagStyle*, QString> KoStyleCollection::saveOasis( KoGenStyles& styles, int styleType, KoSavingContext& context ) const
{
    // Remember the style names (the automatic names generated by KoGenStyles)
    QMap<KoParagStyle*, QString> autoNames;

    // In order to reduce the bloat, we define that the first style (usually Standard)
    // is the "parent" (reference) for the others.
    // ## This is mostly a hack due to lack of proper style inheritance.
    // Once that's implemented, default to 'styles derive from Standard', but save normally.
    QString refStyleName;

    for( QPtrListIterator<KoParagStyle> p( m_styleList ); *p; ++p ) {
        const QString name = (*p)->saveStyle( styles, styleType, refStyleName, context );
        kdDebug() << k_funcinfo << "Saved style " << (*p)->displayName() << " to OASIS format as " << name << endl;
        autoNames.insert( *p, name );
        if ( refStyleName.isEmpty() ) // i.e. first style
            refStyleName = name;
    }
    // Now edit the kogenstyle and set the next-style-name. This works here
    // because the style's m_name is already unique so there's no risk of
    // "two styles being only different due to their following-style"; the
    // display-name will also be different, and will ensure they get two kogenstyles.
    for( QPtrListIterator<KoParagStyle> p( m_styleList ); *p; ++p ) {
        KoParagStyle* style = *p;
        if ( style->followingStyle() && style->followingStyle() != style ) {
            const QString fsname = autoNames[ style->followingStyle() ];
            KoGenStyle* gs = styles.styleForModification( autoNames[style] );
            Q_ASSERT( gs );
            if ( gs )
                gs->addAttribute( "style:next-style-name", fsname );
        }
    }
    return autoNames;
}


KoParagStyle* KoStyleCollection::findStyle( const QString & _name ) const
{
    // Caching, to speed things up
    if ( m_lastStyle && m_lastStyle->name() == _name )
        return m_lastStyle;

    QPtrListIterator<KoParagStyle> styleIt( m_styleList );
    for ( ; styleIt.current(); ++styleIt )
    {
        if ( styleIt.current()->name() == _name ) {
            m_lastStyle = styleIt.current();
            return m_lastStyle;
        }
    }

    if(_name == "Standard") return m_styleList.getFirst(); // fallback..

    return 0L;
}


KoParagStyle* KoStyleCollection::findTranslatedStyle( const QString & _name ) const
{
    // Caching, to speed things up
    if ( m_lastStyle && m_lastStyle->displayName() == _name )
        return m_lastStyle;

    QPtrListIterator<KoParagStyle> styleIt( m_styleList );
    for ( ; styleIt.current(); ++styleIt )
    {
        if ( styleIt.current()->displayName() == _name ) {
            m_lastStyle = styleIt.current();
            return m_lastStyle;
        }
    }

    if ( ( _name == "Standard" ) || ( _name == i18n( "Style name", "Standard" ) ) )
        return m_styleList.getFirst(); // fallback..

    return 0L;
}


KoParagStyle* KoStyleCollection::addStyleTemplate( KoParagStyle * sty )
{
    // First check for duplicates.
    for ( KoParagStyle* p = m_styleList.first(); p != 0L; p = m_styleList.next() )
    {
        if ( p->name() == sty->name() ) {
            if ( p->displayName() == sty->displayName() ) {
                //kdDebug() << k_funcinfo << "replace existing style " << p->name() << endl;
                // Replace existing style
                if ( sty != p )
                {
                    *p = *sty;
                    delete sty;
                }
                return p;
            } else { // internal name conflict, but it's not the same style as far as the user is concerned
                sty->setName( generateUniqueName() );
            }
        }
    }
    m_styleList.append( sty );

    return sty;
}

void KoStyleCollection::removeStyleTemplate ( KoParagStyle *style ) {
    if( m_styleList.removeRef(style)) {
        if ( m_lastStyle == style )
            m_lastStyle = 0L;
        // Remember to delete this style when deleting the document
        m_deletedStyles.append(style);
    }
}

void KoStyleCollection::updateStyleListOrder( const QStringList &list )
{
    QPtrList<KoParagStyle> orderStyle;
    QStringList lst( list );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
    {
        //kdDebug()<<" style :"<<(*it)<<endl;
        bool found = false;
        QPtrListIterator<KoParagStyle> style( m_styleList );
        for ( ; style.current() ; ++style )
        {
            if ( style.current()->name() == *it)
            {
                orderStyle.append( style.current() );
                found = true;
                //kdDebug()<<" found !!!!!!!!!!!!\n";
                break;
            }
        }
        if ( !found )
            kdDebug() << "style " << *it << " not found" << endl;
    }
    m_styleList.setAutoDelete( false );
    m_styleList.clear();
    m_styleList = orderStyle;
#if 0
    QPtrListIterator<KoParagStyle> style( m_styleList );
    for ( ; style.current() ; ++style )
    {
        kdDebug()<<" style.current()->name() :"<<style.current()->name()<<endl;
    }
#endif
}

void KoStyleCollection::importStyles( const QPtrList<KoParagStyle>& styleList )
{
    QPtrListIterator<KoParagStyle> styleIt( styleList );
    QMap<QString, QString> followStyle;
    for ( ; styleIt.current() ; ++styleIt )
    {
        KoParagStyle* style = new KoParagStyle(*styleIt.current());
        if ( style->followingStyle() ) {
            followStyle.insert( style->name(), style->followingStyle()->name() );
        }
        style = addStyleTemplate( style );
    }

    QMapIterator<QString, QString> itFollow = followStyle.begin();
    for ( ; itFollow != followStyle.end(); ++itFollow )
    {
        KoParagStyle * style = findStyle(itFollow.key());
        const QString followingStyleName = followStyle[ itFollow.key() ];
        KoParagStyle * styleFollow = findStyle(followingStyleName);
        //kdDebug() << "    " << style << "  " << itFollow.key() << ": followed by " << styleFollow << " (" << followingStyleName << ")" << endl;
        Q_ASSERT(styleFollow);
        if ( styleFollow )
            style->setFollowingStyle( styleFollow );
        else
            style->setFollowingStyle( style );
    }
}

void KoStyleCollection::saveOasisOutlineStyles( KoXmlWriter& writer ) const
{
    bool first = true;
    QValueVector<KoParagStyle *> styles = outlineStyles();
    for ( int i = 0 ; i < 10 ; ++i ) {
        if ( styles[i] ) {
            if ( first ) {
                writer.startElement( "text:outline-style" );
                first = false;
            }
            writer.startElement( "text:outline-level-style" );
            styles[i]->paragLayout().counter->saveOasisListLevel( writer, true, true );
            writer.endElement();
        }
    }
    if ( !first )
        writer.endElement(); // text:outline-style
}

QValueVector<KoParagStyle *> KoStyleCollection::outlineStyles() const
{
    QValueVector<KoParagStyle *> lst( 10, 0 );
    for ( int i = 0 ; i < 10 ; ++i ) {
        KoParagStyle* style = outlineStyleForLevel( i );
        if ( style )
            lst[i] = style;
    }
    return lst;
}


KoParagStyle* KoStyleCollection::outlineStyleForLevel( int level ) const
{
    for( QPtrListIterator<KoParagStyle> p( m_styleList ); *p; ++p )
    {
        if ( (*p)->isOutline() && (*p)->paragLayout().counter )
        {
            int styleLevel = (*p)->paragLayout().counter->depth();
            if ( styleLevel == level )
                return *p;
        }
    }
    return 0;
}

KoParagStyle* KoStyleCollection::numberedStyleForLevel( int level ) const
{
    for( QPtrListIterator<KoParagStyle> p( m_styleList ); *p; ++p )
    {
        KoParagCounter* counter = (*p)->paragLayout().counter;
        if ( !(*p)->isOutline() && counter
             && counter->numbering() != KoParagCounter::NUM_NONE
             && !counter->isBullet() )
        {
            int styleLevel = counter->depth();
            if ( styleLevel == level )
                return *p;
        }
    }
    return 0;
}


KoParagStyle* KoStyleCollection::defaultStyle() const
{
    return findStyle( "Standard" ); // includes the fallback to first style
}

QString KoStyleCollection::generateUniqueName() const
{
    int count = 1;
    QString name;
    do {
        name = "new" + QString::number( count++ );
    } while ( findStyle( name ) );
    return name;
}

#ifndef NDEBUG
void KoStyleCollection::printDebug() const
{
    for( QPtrListIterator<KoParagStyle> p( m_styleList ); *p; ++p )
    {
        kdDebug() << *p << "  " << (*p)->name() << "    " << (*p)->displayName() << "  followingStyle=" << (*p)->followingStyle() << endl;
    }
}
#endif

/////////////

KoCharStyle::KoCharStyle( const QString & name )
    : KoUserStyle( name )
{
}

const KoTextFormat & KoCharStyle::format() const
{
    return m_format;
}

KoTextFormat & KoCharStyle::format()
{
    return m_format;
}

///////////////////////////

KoParagStyle::KoParagStyle( const QString & name )
    : KoCharStyle( name )
{
    m_followingStyle = this;

    // This way, KoTextParag::setParagLayout also sets the style pointer, to this style
    m_paragLayout.style = this;
    m_parentStyle = 0L;
    m_inheritedParagLayoutFlag = 0;
    m_inheritedFormatFlag = 0;
    m_bOutline = false;
}

KoParagStyle::KoParagStyle( const KoParagStyle & rhs )
    : KoCharStyle( rhs)
{
    *this = rhs;
}

KoParagStyle::~KoParagStyle()
{
}

void KoParagStyle::operator=( const KoParagStyle &rhs )
{
    KoCharStyle::operator=( rhs );
    m_paragLayout = rhs.m_paragLayout;
    m_followingStyle = rhs.m_followingStyle;
    m_paragLayout.style = this; // must always be "this"
    m_parentStyle = rhs.m_parentStyle;
    m_inheritedParagLayoutFlag = rhs.m_inheritedParagLayoutFlag;
    m_inheritedFormatFlag = rhs.m_inheritedFormatFlag;
    m_bOutline = rhs.m_bOutline;
}

void KoParagStyle::setFollowingStyle( KoParagStyle *fst )
{
  m_followingStyle = fst;
}

void KoParagStyle::saveStyle( QDomElement & parentElem )
{
    m_paragLayout.saveParagLayout( parentElem, m_paragLayout.alignment );

    if ( followingStyle() )
    {
        QDomElement element = parentElem.ownerDocument().createElement( "FOLLOWING" );
        parentElem.appendChild( element );
        element.setAttribute( "name", followingStyle()->displayName() );
    }
    // TODO save parent style, and inherited flags.

    parentElem.setAttribute( "outline", m_bOutline ? "true" : "false" );
}

void KoParagStyle::loadStyle( QDomElement & parentElem, int docVersion )
{
    KoParagLayout layout;
    KoParagLayout::loadParagLayout( layout, parentElem, docVersion );

    // This way, KoTextParag::setParagLayout also sets the style pointer, to this style
    layout.style = this;
    m_paragLayout = layout;

    // Load name
    QDomElement nameElem = parentElem.namedItem("NAME").toElement();
    if ( !nameElem.isNull() ) {
        m_name = nameElem.attribute("value");
        m_displayName = i18n( "Style name", m_name.utf8() );
    } else
        kdWarning() << "No NAME tag in LAYOUT -> no name for this style!" << endl;

    // The followingStyle stuff has to be done after loading all styles.

    m_bOutline = parentElem.attribute( "outline" ) == "true";
}

void KoParagStyle::loadStyle( QDomElement & styleElem, KoOasisContext& context )
{
    // Load name
    m_name = styleElem.attributeNS( KoXmlNS::style, "name", QString::null );
    m_displayName = styleElem.attributeNS( KoXmlNS::style, "display-name", QString::null );
    if ( m_displayName.isEmpty() )
        m_displayName = m_name;

    // OOo hack
    //m_bOutline = m_name.startsWith( "Heading" );
    // real OASIS solution:
    m_bOutline = styleElem.hasAttributeNS( KoXmlNS::style, "default-outline-level" );

    context.styleStack().save();
    context.addStyles( &styleElem ); // Load all parents - only because we don't support inheritance.
    KoParagLayout layout;
    KoParagLayout::loadOasisParagLayout( layout, context );

    // loadOasisParagLayout doesn't load the counter. It's modelled differently for parags and for styles.
    int level = 0;
    bool listOK = false;
    const QString listStyleName = styleElem.attributeNS( KoXmlNS::style, "list-style-name", QString::null );
    if ( m_bOutline ) {
        level = styleElem.attributeNS( KoXmlNS::style, "default-outline-level", QString::null ).toInt(); // 1-based
        listOK = context.pushOutlineListLevelStyle( level );
        // allow overriding the outline numbering, see http://lists.oasis-open.org/archives/office/200310/msg00033.html
        if ( !listStyleName.isEmpty() )
            context.pushListLevelStyle( listStyleName, level );
    }
    else {
        // ######## BIG difference here. In the OOo/OASIS format, one list style has infos for 10 list levels...
        // ###### so we can't know a level at this point...

        // The only solution I can think of, to preserve document content when importing OO but
        // not necessarily the styles used when editing, is:
        // 1) when importing from OOo, convert each non-heading style with numbering
        // into 10 kotext styles (at least those used by the document) [TODO]
        // 2) for KWord's own loading/saving, to add a hack into the file format, say
        // style:default-level.
        // Note that default-level defaults to "1", i.e. works for non-nested OOo lists too.
        level = styleElem.attributeNS( KoXmlNS::style, "default-level", "1" ).toInt(); // 1-based
        listOK = !listStyleName.isEmpty();
        if ( listOK )
            listOK = context.pushListLevelStyle( listStyleName, level );
    }
    if ( listOK ) {
        const QDomElement listStyle = context.listStyleStack().currentListStyle();
        // The tag is either text:list-level-style-number or text:list-level-style-bullet
        const bool ordered = listStyle.localName() == "list-level-style-number";
        Q_ASSERT( !layout.counter );
        layout.counter = new KoParagCounter;
        layout.counter->loadOasis( context, -1, ordered, m_bOutline, level, true );
        context.listStyleStack().pop();
    }

    // This way, KoTextParag::setParagLayout also sets the style pointer, to this style
    layout.style = this;
    m_paragLayout = layout;

    m_format.load( context );

    context.styleStack().restore();
}

QString KoParagStyle::saveStyle( KoGenStyles& genStyles, int styleType, const QString& parentStyleName, KoSavingContext& context ) const
{
    KoGenStyle gs( styleType, "paragraph", parentStyleName );

    gs.addAttribute( "style:display-name", m_displayName );
    if ( m_paragLayout.counter ) {
        if ( m_bOutline )
            gs.addAttribute( "style:default-outline-level", (int)m_paragLayout.counter->depth() + 1 );
        else if ( m_paragLayout.counter->depth() )
            // ### kword-specific attribute, see loadOasis
            gs.addAttribute( "style:default-level", (int)m_paragLayout.counter->depth() + 1 );

        if ( m_paragLayout.counter->numbering() != KoParagCounter::NUM_NONE &&
             m_paragLayout.counter->style() != KoParagCounter::STYLE_NONE )
        {
            KoGenStyle listStyle( KoGenStyle::STYLE_LIST /*, no family*/ );
            m_paragLayout.counter->saveOasis( listStyle, true );
            // This display-name will probably look nicer in OO, but this also means
            // no re-use possible between list styles...
            listStyle.addAttribute( "style:display-name",
                                    i18n( "Numbering Style for %1" ).arg( m_displayName ) );

            QString autoListStyleName = genStyles.lookup( listStyle, "L", true );
            gs.addAttribute( "style:list-style-name", autoListStyleName );
        }
    }

    m_paragLayout.saveOasis( gs, context, true );

    m_format.save( gs, context );

    // try to preserve existing internal name, if it looks adequate (no spaces)
    // ## TODO: check XML-Schemacs NCName conformity
    bool nameIsConform = !m_name.isEmpty() && m_name.find( ' ' ) == -1;
    if ( nameIsConform )
        return genStyles.lookup( gs, m_name, false );
    else
        return genStyles.lookup( gs, "U", true );
}

const KoParagLayout & KoParagStyle::paragLayout() const
{
    return m_paragLayout;
}

KoParagLayout & KoParagStyle::paragLayout()
{
    return m_paragLayout;
}

void KoParagStyle::propagateChanges( int paragLayoutFlag, int /*formatFlag*/ )
{
    if ( !m_parentStyle )
        return;
    if ( !(paragLayoutFlag & KoParagLayout::Alignment) )
        m_paragLayout.alignment = m_parentStyle->paragLayout().alignment;
    if ( !(paragLayoutFlag & KoParagLayout::Margins) )
        for ( int i = 0 ; i < 5 ; ++i )
            m_paragLayout.margins[i] = m_parentStyle->paragLayout().margins[i];
    if ( !(paragLayoutFlag & KoParagLayout::LineSpacing) )
    {
        m_paragLayout.setLineSpacingValue(m_parentStyle->paragLayout().lineSpacingValue());
        m_paragLayout.lineSpacingType = m_parentStyle->paragLayout().lineSpacingType;
    }
    if ( !(paragLayoutFlag & KoParagLayout::Borders) )
    {
        m_paragLayout.leftBorder = m_parentStyle->paragLayout().leftBorder;
        m_paragLayout.rightBorder = m_parentStyle->paragLayout().rightBorder;
        m_paragLayout.topBorder = m_parentStyle->paragLayout().topBorder;
        m_paragLayout.bottomBorder = m_parentStyle->paragLayout().bottomBorder;
        m_paragLayout.joinBorder = m_parentStyle->paragLayout().joinBorder;
    }
    if ( !(paragLayoutFlag & KoParagLayout::BulletNumber) )
        m_paragLayout.counter = m_parentStyle->paragLayout().counter;
    if ( !(paragLayoutFlag & KoParagLayout::Tabulator) )
        m_paragLayout.setTabList(m_parentStyle->paragLayout().tabList());
#if 0
    if ( paragLayoutFlag == KoParagLayout::All )
    {
        setDirection( static_cast<QChar::Direction>(layout.direction) );
        // Don't call applyStyle from here, it would overwrite any paragraph-specific settings
        setStyle( layout.style );
    }
#endif
    // TODO a flag for the "is outline" bool? Otherwise we have no way to inherit
    // that property (and possibly reset it).
}

void KoParagStyle::setOutline( bool b )
{
    m_bOutline = b;
}
