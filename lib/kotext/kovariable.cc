/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kovariable.h"
#include "kovariable.moc"
#include <koDocumentInfo.h>
#include <kozoomhandler.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <qdom.h>
#include <koDocument.h>
#include <kdialogbase.h>
#include <kconfig.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <qstringlist.h>
#include <qcombobox.h>
#include <qvaluelist.h>
#include <qradiobutton.h>
#include "timeformatwidget_impl.h"
#include "dateformatwidget_impl.h"
#include "kocommand.h"
#include "kotextobject.h"

class KoVariableSettings::KoVariableSettingPrivate
{
public:
    KoVariableSettingPrivate() 
    {
        m_lastPrintingDate.setTime_t(0); // Default is 1970-01-01 midnight locale time
    }
    QDateTime m_lastPrintingDate;
    QDateTime m_creationDate;
    QDateTime m_modificationDate;
};


KoVariableSettings::KoVariableSettings()
{
    d = new KoVariableSettingPrivate;
    m_startingPageNumber = 1;
    m_displayLink = true;
    m_displayComment = true;
    m_underlineLink = true;
    m_displayFieldCode = false;
}

KoVariableSettings::~KoVariableSettings()
{
    delete d;
    d = 0;
}

QDateTime KoVariableSettings::lastPrintingDate() const
{
    return d->m_lastPrintingDate;
}

void KoVariableSettings::setLastPrintingDate( const QDateTime & _date)
{
    d->m_lastPrintingDate = _date;
}

QDateTime KoVariableSettings::creationDate() const
{
    return d->m_creationDate;
}

void KoVariableSettings::setCreationDate( const QDateTime & _date)
{
    if ( !d->m_creationDate.isValid() )
        d->m_creationDate = _date;
}

QDateTime KoVariableSettings::modificationDate() const
{
    return d->m_modificationDate;
}

void KoVariableSettings::setModificationDate( const QDateTime & _date)
{
    d->m_modificationDate = _date;
}


void KoVariableSettings::save( QDomElement &parentElem )
{
     QDomElement elem = parentElem.ownerDocument().createElement( "VARIABLESETTINGS" );
     parentElem.appendChild( elem );
    if(m_startingPageNumber!=1)
    {
        elem.setAttribute( "startingPageNumber", m_startingPageNumber );
    }
    elem.setAttribute("displaylink",(int)m_displayLink);
    elem.setAttribute("underlinelink",(int)m_underlineLink);
    elem.setAttribute("displaycomment",(int)m_displayComment);
    elem.setAttribute("displayfieldcode", (int)m_displayFieldCode);

    if ( d->m_lastPrintingDate.isValid())
        elem.setAttribute("lastPrintingDate", d->m_lastPrintingDate.toString(Qt::ISODate));

    if ( d->m_creationDate.isValid())
        elem.setAttribute("creationDate", d->m_creationDate.toString(Qt::ISODate));

    if ( d->m_modificationDate.isValid())
        elem.setAttribute("modificationDate", d->m_modificationDate.toString(Qt::ISODate));
}

void KoVariableSettings::load( QDomElement &elem )
{
    QDomElement e = elem.namedItem( "VARIABLESETTINGS" ).toElement();
    if (!e.isNull())
    {
        if(e.hasAttribute("startingPageNumber"))
            m_startingPageNumber = e.attribute("startingPageNumber").toInt();
        if(e.hasAttribute("displaylink"))
            m_displayLink=(bool)e.attribute("displaylink").toInt();
        if(e.hasAttribute("underlinelink"))
            m_underlineLink=(bool)e.attribute("underlinelink").toInt();
        if(e.hasAttribute("displaycomment"))
            m_displayComment=(bool)e.attribute("displaycomment").toInt();
        if (e.hasAttribute("displayfieldcode"))
            m_displayFieldCode=(bool)e.attribute("displayfieldcode").toInt();

        if (e.hasAttribute("lastPrintingDate"))
            d->m_lastPrintingDate = QDateTime::fromString( e.attribute( "lastPrintingDate" ), Qt::ISODate );
        else
            d->m_lastPrintingDate.setTime_t(0); // 1970-01-01 00:00:00.000 locale time

        if (e.hasAttribute("creationDate"))
            d->m_creationDate = QDateTime::fromString( e.attribute( "creationDate" ), Qt::ISODate );

        if (e.hasAttribute("modificationDate"))
            d->m_modificationDate = QDateTime::fromString( e.attribute( "modificationDate" ), Qt::ISODate );
    }
}

KoVariableDateFormat::KoVariableDateFormat() : KoVariableFormat()
{
}

QString KoVariableDateFormat::convert( const QVariant& data ) const
{
    if ( data.type() != QVariant::Date && data.type() != QVariant::DateTime )
    {
        kdWarning(32500)<<" Error in KoVariableDateFormat::convert. Value is a "
                      << data.typeName() << "(" << data.type() << ")" << endl;
        // dateTime will be invalid, then set to 1970-01-01
    }
    QDateTime dateTime ( data.toDateTime() );
    if ( !dateTime.isValid() )
        return i18n("No date set"); // e.g. old KWord documents

    if (m_strFormat.lower() == "locale" || m_strFormat.isEmpty())
        return KGlobal::locale()->formatDate( dateTime.date(), false );
    else if ( m_strFormat.lower() == "localeshort" )
        return KGlobal::locale()->formatDate( dateTime.date(), true );
    else if ( m_strFormat.lower() == "localedatetime" )
        return KGlobal::locale()->formatDateTime( dateTime, false );
    else if ( m_strFormat.lower() == "localedatetimeshort" )
        return KGlobal::locale()->formatDateTime( dateTime, true );

    QString tmp ( dateTime.toString(m_strFormat) );
    const int month = dateTime.date().month();
    tmp.replace("PPPP", KGlobal::locale()->monthNamePossessive(month, false)); //long possessive month name
    tmp.replace("PPP",  KGlobal::locale()->monthNamePossessive(month, true));  //short possessive month name
    return tmp;
}

QCString KoVariableDateFormat::key() const
{
    return getKey( m_strFormat );
}

QCString KoVariableDateFormat::getKey( const QString& props ) const
{
    return QCString("DATE") + props.utf8();
}

void KoVariableDateFormat::load( const QCString &key )
{
    QCString params( key.mid( 4 ) );
    if ( !params.isEmpty() )
    {
        if (params[0] == '1' || params[0] == '0') // old m_bShort crap
            params = params.mid(1); // skip it
        m_strFormat = QString::fromUtf8( params ); // skip "DATE"
    }
}

// Used by KoVariableFormatCollection::popupActionList(), to apply all formats
// to the current data, in the popup menu.
QStringList KoVariableDateFormat::staticFormatPropsList()
{
    QStringList listDateFormat;
    listDateFormat<<"locale";
    listDateFormat<<"localeshort";
    listDateFormat<<"localedatetime";
    listDateFormat<<"localedatetimeshort";
    listDateFormat<<"dd/MM/yy";
    listDateFormat<<"dd/MM/yyyy";
    listDateFormat<<"MMM dd,yy";
    listDateFormat<<"MMM dd,yyyy";
    listDateFormat<<"dd.MMM.yyyy";
    listDateFormat<<"MMMM dd, yyyy";
    listDateFormat<<"ddd, MMM dd,yy";
    listDateFormat<<"dddd, MMM dd,yy";
    listDateFormat<<"MM-dd";
    listDateFormat<<"yyyy-MM-dd";
    listDateFormat<<"dd/yy";
    listDateFormat<<"MMMM";
    listDateFormat<<"yyyy-MM-dd hh:mm";
    listDateFormat<<"dd.MMM.yyyy hh:mm";
    listDateFormat<<"MMM dd,yyyy h:mm AP";
    listDateFormat<<"yyyy-MM-ddThh:mm:ss"; // ISO 8601
    return listDateFormat;
}

// Used by dateformatwidget_impl
// TODO: shouldn't it apply the formats to the value, like the popupmenu does?
QStringList KoVariableDateFormat::staticTranslatedFormatPropsList()
{
    QStringList listDateFormat;
    listDateFormat<<i18n("Locale date format");
    listDateFormat<<i18n("Short locale date format");
    listDateFormat<<i18n("Locale date & time format");
    listDateFormat<<i18n("Short locale date & time format");
    listDateFormat<<"dd/MM/yy";
    listDateFormat<<"dd/MM/yyyy";
    listDateFormat<<"MMM dd,yy";
    listDateFormat<<"MMM dd,yyyy";
    listDateFormat<<"dd.MMM.yyyy";
    listDateFormat<<"MMMM dd, yyyy";
    listDateFormat<<"ddd, MMM dd,yy";
    listDateFormat<<"dddd, MMM dd,yy";
    listDateFormat<<"MM-dd";
    listDateFormat<<"yyyy-MM-dd";
    listDateFormat<<"dd/yy";
    listDateFormat<<"MMMM";
    listDateFormat<<"yyyy-MM-dd hh:mm";
    listDateFormat<<"dd.MMM.yyyy hh:mm";
    listDateFormat<<"MMM dd,yyyy h:mm AP";
    listDateFormat<<"yyyy-MM-ddThh:mm:ss"; // ISO 8601
    return listDateFormat;
}

////

KoVariableTimeFormat::KoVariableTimeFormat() : KoVariableFormat()
{
}

void KoVariableTimeFormat::load( const QCString &key )
{
    QCString params( key.mid( 4 ) );
    if ( !params.isEmpty() )
	m_strFormat = QString::fromUtf8(params);
}

QString KoVariableTimeFormat::convert( const QVariant & time ) const
{
    if ( time.type() != QVariant::Time )
    {
        kdDebug(32500)<<" Error in KoVariableTimeFormat::convert. Value is a "
                      << time.typeName() << "(" << time.type() << ")" << endl;
        return QString::null;
    }

    if( m_strFormat.lower() == "locale" || m_strFormat.isEmpty() )
	return KGlobal::locale()->formatTime( time.toTime() );
    return time.toTime().toString(m_strFormat);
}

QCString KoVariableTimeFormat::key() const
{
    return getKey( m_strFormat );
}

QCString KoVariableTimeFormat::getKey( const QString& props ) const
{
    return QCString("TIME") + props.utf8();
}

// Used by KoVariableFormatCollection::popupActionList(), to apply all formats
// to the current data, in the popup menu.
QStringList KoVariableTimeFormat::staticFormatPropsList()
{
    QStringList listTimeFormat;
    listTimeFormat<<"locale";
    listTimeFormat<<"hh:mm";
    listTimeFormat<<"hh:mm:ss";
    listTimeFormat<<"hh:mm AP";
    listTimeFormat<<"hh:mm:ss AP";
    listTimeFormat<<"mm:ss.zzz";
    return listTimeFormat;
}

// Used by timeformatwidget_impl
QStringList KoVariableTimeFormat::staticTranslatedFormatPropsList()
{
    QStringList listTimeFormat;
    listTimeFormat<<i18n("Locale format");
    listTimeFormat<<"hh:mm";
    listTimeFormat<<"hh:mm:ss";
    listTimeFormat<<"hh:mm AP";
    listTimeFormat<<"hh:mm:ss AP";
    listTimeFormat<<"mm:ss.zzz";
    return listTimeFormat;
}

////

QString KoVariableStringFormat::convert( const QVariant & string ) const
{
    if ( string.type() != QVariant::String )
    {
        kdDebug(32500)<<" Error in KoVariableStringFormat::convert. Value is a " << string.typeName() << endl;
        return QString::null;
    }

    return string.toString();
}

QCString KoVariableStringFormat::key() const
{
    return getKey( QString::null );
    // TODO prefix & suffix
}

QCString KoVariableStringFormat::getKey( const QString& props ) const
{
    return QCString("STRING") + props.utf8();
}

////

QString KoVariableNumberFormat::convert( const QVariant &value ) const
{
    if ( value.type() != QVariant::Int )
    {
        kdDebug(32500)<<" Error in KoVariableNumberFormat::convert. Value is a " << value.typeName() << endl;
        return QString::null;
    }

    return QString::number( value.toInt() );
}

QCString KoVariableNumberFormat::key() const
{
    return getKey(QString::null);
}

QCString KoVariableNumberFormat::getKey( const QString& props ) const
{
    return QCString("NUMB") + props.utf8();
}

////

KoVariableFormatCollection::KoVariableFormatCollection()
{
    m_dict.setAutoDelete( true );
}

KoVariableFormat * KoVariableFormatCollection::format( const QCString &key )
{
    KoVariableFormat *f = m_dict[ key.data() ];
    if (f)
        return f;
    else
        return createFormat( key );
}

KoVariableFormat * KoVariableFormatCollection::createFormat( const QCString &key )
{
    kdDebug(32500) << "KoVariableFormatCollection: creating format for key=" << key << endl;
    KoVariableFormat * format = 0L;
    // The first 4 chars identify the class
    QCString type = key.left(4);
    if ( type == "DATE" )
        format = new KoVariableDateFormat();
    else if ( type == "TIME" )
        format = new KoVariableTimeFormat();
    else if ( type == "NUMB" ) // this type of programming makes me numb ;)
        format = new KoVariableNumberFormat();
    else if ( type == "STRI" )
        format = new KoVariableStringFormat();

    if ( format )
    {
        format->load( key );
        m_dict.insert( format->key() /* not 'key', it could be incomplete */, format );
    }
    return format;
}

/******************************************************************/
/* Class:       KoVariableCollection                              */
/******************************************************************/
KoVariableCollection::KoVariableCollection(KoVariableSettings *_settings, KoVariableFormatCollection *formatCollection)
{
    m_variableSettings = _settings;
    m_varSelected = 0L;
    m_formatCollection = formatCollection;
}

KoVariableCollection::~KoVariableCollection()
{
    delete m_variableSettings;
}

void KoVariableCollection::registerVariable( KoVariable *var )
{
    if ( !var )
        return;
    variables.append( var );
}

void KoVariableCollection::unregisterVariable( KoVariable *var )
{
    variables.take( variables.findRef( var ) );
}

void KoVariableCollection::recalcVariables(int type)
{
    bool update = false;
    QPtrListIterator<KoVariable> it( variables );
    for ( ; it.current() ; ++it )
    {
        if ( it.current()->isDeleted() )
            continue;
        if ( it.current()->type() == type || type == VT_ALL )
        {
            update = true;
            it.current()->recalc();
            KoTextParag * parag = it.current()->paragraph();
            if ( parag )
            {
                //kdDebug(32500) << "KoDoc::recalcVariables -> invalidating parag " << parag->paragId() << endl;
                parag->invalidate( 0 );
                parag->setChanged( true );
            }
        }
    }
    // TODO pass list of textdocuments as argument
    // Or even better, call emitRepaintChanged on all modified textobjects
    if(update)
        emit repaintVariable();
}


void KoVariableCollection::setVariableValue( const QString &name, const QString &value )
{
    varValues[ name ] = value;
}

QString KoVariableCollection::getVariableValue( const QString &name ) const
{
    if ( !varValues.contains( name ) )
        return i18n( "No value" );
    return varValues[ name ];
}

bool KoVariableCollection::customVariableExist(const QString &varname) const
{
    return varValues.contains( varname );
}

void KoVariableCollection::recalcVariables(KoVariable *var)
{
    if( var )
    {
        var->recalc();
        KoTextParag * parag = var->paragraph();
        if ( parag )
        {
            parag->invalidate( 0 );
            parag->setChanged( true );
        }
        emit repaintVariable();
    }
}

void KoVariableCollection::setVariableSelected(KoVariable * var)
{
    m_varSelected=var;
}

QPtrList<KAction> KoVariableCollection::popupActionList()
{
    QPtrList<KAction> listAction;
    // Insert list of actions that change the subtype
    QStringList list = m_varSelected->subTypeText();
    QStringList::ConstIterator it = list.begin();
    for ( int i = 0; it != list.end() ; ++it, ++i )
    {
        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            // We store the subtype number as the action name
            QCString name; name.setNum(i);
            KToggleAction * act = new KToggleAction( *it, KShortcut(), 0, name );
            connect( act, SIGNAL(activated()), this, SLOT(slotChangeSubType()) );
            if ( i == m_varSelected->subType() )
                act->setChecked( true );
            //m_subTextMap.insert( act, i );
            listAction.append( act );
        }
    }
    // Insert list of actions that change the format properties
    KoVariableFormat* format = m_varSelected->variableFormat();
    QString currentFormat = format->formatProperties();

    list = format->formatPropsList();
    it = list.begin();
    for ( int i = 0; it != list.end() ; ++it, ++i )
    {
        if( i == 0 ) // first item, and list not empty
            listAction.append( new KActionSeparator() );

        if ( !(*it).isEmpty() ) // in case of removed subtypes or placeholders
        {
            format->setFormatProperties( *it ); // temporary change
            QString text = format->convert( m_varSelected->varValue() );
            // We store the raw format as the action name
            KToggleAction * act = new KToggleAction(text, KShortcut(), 0, (*it).utf8());
            connect( act, SIGNAL(activated()), this, SLOT(slotChangeFormat()) );
            if ( (*it) == currentFormat )
                act->setChecked( true );
            listAction.append( act );
        }
    }

    // Restore current format
    format->setFormatProperties( currentFormat );
    return listAction;
}

void KoVariableCollection::slotChangeSubType()
{
    KAction * act = (KAction *)(sender());
    int menuNumber = QCString(act->name()).toInt();
    int newSubType = m_varSelected->variableSubType(menuNumber);
    kdDebug(32500) << "slotChangeSubType: menuNumber=" << menuNumber << " newSubType=" << newSubType << endl;
    if ( m_varSelected->subType() != newSubType )
    {
        KoChangeVariableSubType *cmd=new KoChangeVariableSubType(
            m_varSelected->subType(), newSubType, m_varSelected );
        cmd->execute();
        m_varSelected->textDocument()->emitNewCommand(cmd);
    }
}

void KoVariableCollection::slotChangeFormat()
{
    KAction * act = (KAction *)(sender());
    QString newFormat = QString::fromUtf8(act->name());
    QString oldFormat = m_varSelected->variableFormat()->formatProperties();
    if (oldFormat != newFormat )
    {
        KCommand *cmd=new KoChangeVariableFormatProperties(
            oldFormat, newFormat, m_varSelected );
        cmd->execute();
        m_varSelected->textDocument()->emitNewCommand(cmd);
    }
}

/******************************************************************/
/* Class: KoVariable                                              */
/******************************************************************/
KoVariable::KoVariable( KoTextDocument *textdoc, KoVariableFormat *varFormat, KoVariableCollection *_varColl)
    : KoTextCustomItem( textdoc )
{
    //d = new Private;
    m_varColl=_varColl;
    m_varFormat = varFormat;
    m_varColl->registerVariable( this );
    m_ascent = 0;
}

KoVariable::~KoVariable()
{
    //kdDebug(32500) << "KoVariable::~KoVariable " << this << endl;
    m_varColl->unregisterVariable( this );
    //delete d;
}

QStringList KoVariable::subTypeText()
{
    return QStringList();
}

void KoVariable::resize()
{
    if ( m_deleted )
        return;
    KoTextFormat *fmt = format();
    QFontMetrics fm = fmt->refFontMetrics();
    QString txt = text();

    width = 0;
    for ( int i = 0 ; i < (int)txt.length() ; ++i )
        width += fm.charWidth( txt, i ); // size at 100%
    // zoom to LU
    width = qRound( KoTextZoomHandler::ptToLayoutUnitPt( width ) );
    height = fmt->height();
    m_ascent = fmt->ascent();
    //kdDebug(32500) << "KoVariable::resize text=" << txt << " width=" << width << " height=" << height << " ascent=" << m_ascent << endl;
}

void KoVariable::recalcAndRepaint()
{
    recalc();
    KoTextParag * parag = paragraph();
    if ( parag )
    {
        //kdDebug(32500) << "KoVariable::recalcAndRepaint -> invalidating parag " << parag->paragId() << endl;
        parag->invalidate( 0 );
        parag->setChanged( true );
    }
    textDocument()->emitRepaintChanged();
}

QString KoVariable::fieldCode()
{
    return i18n("Variable");
}

QString KoVariable::text(bool realValue)
{
    KoTextFormat *fmt = format();
    QString str;
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        str = fieldCode();
    else
        str = m_varFormat->convert( m_varValue );

    return fmt->displayedString( str);
}

void KoVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow )
{
    KoTextFormat * fmt = format();
    KoZoomHandler * zh = textDocument()->paintingZoomHandler();
    QFont font( fmt->screenFont( zh ) );
    drawCustomItemHelper( p, x, y, wpix, hpix, ascentpix, cg, selected, offset, fmt, font, fmt->color(), drawingShadow );
}

void KoVariable::drawCustomItemHelper( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, const QColorGroup& cg, bool selected, int offset, KoTextFormat* fmt, const QFont& font, QColor textColor, bool drawingShadow )
{
    // Important: the y value already includes the difference between the parag baseline
    // and the char's own baseline (ascent) (see paintDefault in korichtext.cpp)
    // So we just draw the text there. But we need the baseline for drawFontEffects...
    KoZoomHandler * zh = textDocument()->paintingZoomHandler();

    p->save();

    if ( fmt->textBackgroundColor().isValid() )
        p->fillRect( x, y, wpix, hpix, fmt->textBackgroundColor() );

    if ( drawingShadow ) // Use shadow color if drawing a shadow
    {
        textColor = fmt->shadowColor();
        p->setPen( textColor );
    }
    else if ( selected )
    {
        textColor = cg.color( QColorGroup::HighlightedText );
        p->setPen( QPen( textColor ) );
        p->fillRect( x, y, wpix, hpix, cg.color( QColorGroup::Highlight ) );
    }
    else if ( textDocument() && textDocument()->drawFormattingChars()
              && p->device()->devType() != QInternal::Printer )
    {
        textColor = cg.color( QColorGroup::Highlight );
        p->setPen( QPen ( textColor, 0, Qt::DotLine ) );
        p->drawRect( x, y, wpix, hpix );
    }
    else {
        if ( !textColor.isValid() ) // Resolve the color at this point
            textColor = KoTextFormat::defaultTextColor( p );
        p->setPen( QPen( textColor ) );
    }

    p->setFont( font ); // already done by KoTextCustomItem::draw but someone might
                        // change the font passed to drawCustomItemHelper (e.g. KoLinkVariable)
    QString str = text();
    KoTextParag::drawFontEffects( p, fmt, zh, font, textColor, x, ascentpix, wpix, y, hpix, str[0] );
    int posY = y + ascentpix + offset;
    if ( fmt->vAlign() == KoTextFormat::AlignSubScript )
        posY +=p->fontMetrics().height() / 6;
    if ( fmt->vAlign() != KoTextFormat::AlignSuperScript )
        posY -= fmt->offsetFromBaseLine();
    else if ( fmt->offsetFromBaseLine() < 0 )
        posY -= 2*fmt->offsetFromBaseLine();

    p->drawText( x, posY, str );
    p->restore();
}

void KoVariable::save( QDomElement &parentElem )
{
    //kdDebug(32500) << "KoVariable::save" << endl;
    QDomElement variableElem = parentElem.ownerDocument().createElement( "VARIABLE" );
    parentElem.appendChild( variableElem );
    QDomElement typeElem = parentElem.ownerDocument().createElement( "TYPE" );
    variableElem.appendChild( typeElem );
    typeElem.setAttribute( "type", static_cast<int>( type() ) );
    //// Of course, saving the key is ugly. We'll drop this when switching to the OO format.
    typeElem.setAttribute( "key", m_varFormat->key() );
    typeElem.setAttribute( "text", text(true) );
    if ( correctValue() != 0)
        typeElem.setAttribute( "correct", correctValue() );
    saveVariable( variableElem );
}

void KoVariable::load( QDomElement & )
{
}

KoVariable * KoVariableCollection::createVariable( int type, short int subtype, KoVariableFormatCollection * coll, KoVariableFormat *varFormat,KoTextDocument *textdoc, KoDocument * doc, int _correct, bool _forceDefaultFormat )
{
    QCString string;
    QStringList stringList;
    if ( varFormat == 0L )
    {
        // Get the default format for this variable (this method is only called in the interactive case, not when loading)
        switch ( type ) {
        case VT_DATE:
        case VT_DATE_VAR_KWORD10:  // compatibility with kword 1.0
        {
            if ( _forceDefaultFormat || subtype == KoDateVariable::VST_DATE_LAST_PRINTING || subtype ==KoDateVariable::VST_DATE_CREATE_FILE || subtype ==KoDateVariable::VST_DATE_MODIFY_FILE)
                varFormat = coll->format( KoDateVariable::defaultFormat() );
            else
            {
                QCString result = KoDateVariable::formatStr(_correct);
                if ( result == 0 )//we cancel insert variable
                    return 0L;
                varFormat = coll->format( result );
            }
            break;
        }
        case VT_TIME:
        case VT_TIME_VAR_KWORD10:  // compatibility with kword 1.0
        {
            if ( _forceDefaultFormat )
                varFormat = coll->format( KoTimeVariable::defaultFormat() );
            else
                varFormat = coll->format( KoTimeVariable::formatStr(_correct) );
            break;
        }
        case VT_PGNUM:
            varFormat = coll->format( "NUMBER" );
            break;
        case VT_FIELD:
        case VT_CUSTOM:
        case VT_MAILMERGE:
        case VT_LINK:
        case VT_NOTE:
            varFormat = coll->format( "STRING" );
            break;
        case VT_FOOTNOTE: // this is a KWord-specific variable
            kdError() << "Footnote type not handled in KoVariableCollection: VT_FOOTNOTE" << endl;
            return 0L;
        }
    }
    Q_ASSERT( varFormat );
    if ( varFormat == 0L ) // still 0 ? Impossible!
        return 0L ;

    kdDebug(32500) << "Creating variable. Format=" << varFormat->key() << " type=" << type << endl;
    KoVariable * var = 0L;
    switch ( type ) {
        case VT_DATE:
        case VT_DATE_VAR_KWORD10:  // compatibility with kword 1.0
            var = new KoDateVariable( textdoc, subtype, varFormat, this, _correct );
            break;
        case VT_TIME:
        case VT_TIME_VAR_KWORD10:  // compatibility with kword 1.0
            var = new KoTimeVariable( textdoc, subtype, varFormat, this, _correct );
            break;
        case VT_PGNUM:
            kdError() << "VT_PGNUM must be handled by the application's reimplementation of KoVariableCollection::createVariable" << endl;
            //var = new KoPgNumVariable( textdoc, subtype, varFormat, this );
            break;
        case VT_FIELD:
            var = new KoFieldVariable( textdoc, subtype, varFormat,this,doc );
            break;
        case VT_CUSTOM:
            var = new KoCustomVariable( textdoc, QString::null, varFormat, this);
            break;
        case VT_MAILMERGE:
            var = new KoMailMergeVariable( textdoc, QString::null, varFormat ,this);
            break;
        case VT_LINK:
            var = new KoLinkVariable( textdoc,QString::null, QString::null, varFormat ,this);
            break;
        case VT_NOTE:
            var = new KoNoteVariable( textdoc, QString::null, varFormat ,this);
            break;
    }
    Q_ASSERT( var );
    return var;
}

void KoVariable::setVariableFormat( KoVariableFormat *_varFormat )
{
    // TODO if ( _varFormat ) _varFormat->deref();
    m_varFormat = _varFormat;
    // TODO m_varFormat->ref();
}

/******************************************************************/
/* Class: KoDateVariable                                          */
/******************************************************************/
KoDateVariable::KoDateVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *_varFormat, KoVariableCollection *_varColl, int _correctDate)
    : KoVariable( textdoc, _varFormat,_varColl ), m_subtype( subtype ), m_correctDate( _correctDate)
{
}

QString KoDateVariable::fieldCode()
{
    if ( m_subtype == VST_DATE_FIX )
        return i18n("Date (Fixed)");
    else if ( m_subtype == VST_DATE_CURRENT)
        return i18n("Date");
    else if ( m_subtype == VST_DATE_LAST_PRINTING)
        return i18n("Last Printing");
    else if ( m_subtype == VST_DATE_CREATE_FILE )
        return i18n( "File Creation");
    else if ( m_subtype == VST_DATE_MODIFY_FILE )
        return i18n( "File Modification");
    else
        return i18n("Date");
}

void KoDateVariable::resize()
{
    KoTextFormat * fmt = format();
    QString oldLanguage;
    if ( !fmt->language().isEmpty())
    {
         oldLanguage=KGlobal::locale()->language();
         bool changeLanguage = KGlobal::locale()->setLanguage( fmt->language() );
         KoVariable::resize();
         if ( changeLanguage )
             KGlobal::locale()->setLanguage( oldLanguage );
    }
    else
        KoVariable::resize();
}

void KoDateVariable::recalc()
{
    if ( m_subtype == VST_DATE_CURRENT )
        m_varValue = QDateTime::currentDateTime().addDays(m_correctDate);
    else if ( m_subtype == VST_DATE_LAST_PRINTING )
        m_varValue = m_varColl->variableSetting()->lastPrintingDate();
    else if ( m_subtype == VST_DATE_CREATE_FILE )
        m_varValue = m_varColl->variableSetting()->creationDate();
    else if ( m_subtype == VST_DATE_MODIFY_FILE )
        m_varValue = m_varColl->variableSetting()->modificationDate();
    else
    {
        // Only if never set before (i.e. upon insertion)
        if ( m_varValue.isNull() )
            m_varValue = QDateTime::currentDateTime().addDays(m_correctDate);
    }
    resize();
}

void KoDateVariable::saveVariable( QDomElement& varElem )
{
    QDomElement elem = varElem.ownerDocument().createElement( "DATE" );
    varElem.appendChild( elem );
    QDate date = m_varValue.toDate(); // works with Date and DateTime
    date = date.addDays( -m_correctDate );//remove correctDate value otherwise value stored is bad
    elem.setAttribute( "year", date.year() );
    elem.setAttribute( "month", date.month() );
    elem.setAttribute( "day", date.day() );
    elem.setAttribute( "fix", m_subtype == VST_DATE_FIX ); // for compat
    elem.setAttribute( "correct", m_correctDate);
    elem.setAttribute( "subtype", m_subtype);
    if ( m_varValue.type() == QVariant::DateTime )
    {
        QTime time = m_varValue.toTime();
        elem.setAttribute( "hour", time.hour() );
        elem.setAttribute( "minute", time.minute() );
        elem.setAttribute( "second", time.second() );
    }
}

void KoDateVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "DATE" ).toElement();
    if (!e.isNull())
    {
        const int y = e.attribute("year").toInt();
        const int month = e.attribute("month").toInt();
        const int d = e.attribute("day").toInt();
        const int h = e.attribute("hour").toInt();
        const int min = e.attribute("minute").toInt();
        const int s = e.attribute("second").toInt();
        const int ms = e.attribute("msecond").toInt();
        const bool fix = e.attribute("fix").toInt() == 1;
        if ( e.hasAttribute("correct"))
            m_correctDate = e.attribute("correct").toInt();
        if ( fix )
        {
            QDate date( y, month, d );
            date = date.addDays( m_correctDate );
            const QTime time( h, min, s, ms );
            if (time.isValid())
                m_varValue = QVariant ( QDateTime( date, time ) );
            else
                m_varValue = QVariant( date );
        }
        //old date variable format
        m_subtype = fix ? VST_DATE_FIX : VST_DATE_CURRENT;
        if ( e.hasAttribute( "subtype" ))
            m_subtype = e.attribute( "subtype").toInt();
    }
}

QStringList KoDateVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Current Date (fixed)" );
    lst << i18n( "Current Date (variable)" );
    lst << i18n( "Date of Last Printing" );
    lst << i18n( "Date of File Creation" );
    lst << i18n( "Date of File Modification" );
    return lst;
}

QStringList KoDateVariable::subTypeText()
{
    return KoDateVariable::actionTexts();
}

QCString KoDateVariable::defaultFormat()
{
    return QCString("DATE") + "locale";
}

QCString KoDateVariable::formatStr(int & correct)
{
    QCString string;
    QStringList stringList;
    KDialogBase* dialog=new KDialogBase(0, 0, true, i18n("Date Format"), KDialogBase::Ok|KDialogBase::Cancel);
    DateFormatWidget* widget=new DateFormatWidget(dialog);
    int count=0;
    dialog->setMainWidget(widget);
    KConfig* config = KoGlobal::kofficeConfig();
    if( config->hasGroup("Date format history") )
    {
        KConfigGroupSaver cgs( config, "Date format history");
        const int noe=config->readNumEntry("Number Of Entries", 5);
        for(int i=0;i<noe;i++)
        {
            QString num;
            num.setNum(i);
            const QString tmpString(config->readEntry("Last Used"+num));
            if(tmpString.startsWith("locale"))
                continue;
            else if(stringList.contains(tmpString))
                continue;
            else if(!tmpString.isEmpty())
            {
                stringList.append(tmpString);
                count++;
            }
        }

    }
    if(!stringList.isEmpty())
    {
        widget->combo1->insertItem("---");
        widget->combo1->insertStringList(stringList);
    }
    if(false) { // ### TODO: select the last used item
        QComboBox *combo= widget->combo1;
        combo->setCurrentItem(combo->count() -1);
        widget->updateLabel();
    }

    if(dialog->exec()==QDialog::Accepted)
    {
        string = widget->resultString().utf8();
        correct = widget->correctValue();
    }
    else
    {
        return 0;
    }
    config->setGroup("Date format history");
    stringList.remove(string);
    stringList.prepend(string);
    for(int i=0;i<=count;i++)
    {
        QString num;
        num.setNum(i);
        config->writeEntry("Last Used"+num, stringList[i]);
    }
    config->sync();
    delete dialog;
    return QCString(QCString("DATE") + string );
}

/******************************************************************/
/* Class: KoTimeVariable                                          */
/******************************************************************/
KoTimeVariable::KoTimeVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl, int _correct)
    : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype ), m_correctTime( _correct)
{
}

QString KoTimeVariable::fieldCode()
{
    return (m_subtype == VST_TIME_FIX)?i18n("Time (Fixed)"):i18n("Time");
}


void KoTimeVariable::resize()
{
    KoTextFormat * fmt = format();
    if ( !fmt->language().isEmpty() )
    {
        QString oldLanguage = KGlobal::locale()->language();
        bool changeLanguage = KGlobal::locale()->setLanguage( fmt->language() );
        KoVariable::resize();
        if ( changeLanguage )
            KGlobal::locale()->setLanguage( oldLanguage );
    }
    else
        KoVariable::resize();
}

void KoTimeVariable::recalc()
{
    if ( m_subtype == VST_TIME_CURRENT )
        m_varValue = QVariant( QTime::currentTime().addSecs(60*m_correctTime));
    else
    {
        // Only if never set before (i.e. upon insertion)
        if ( m_varValue.toTime().isNull() )
            m_varValue = QVariant( QTime::currentTime().addSecs(60*m_correctTime));
    }
    resize();
}


void KoTimeVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement elem = parentElem.ownerDocument().createElement( "TIME" );
    parentElem.appendChild( elem );
    QTime time = m_varValue.toTime();
    time = time.addSecs(-60*m_correctTime);
    elem.setAttribute( "hour", time.hour() );
    elem.setAttribute( "minute", time.minute() );
    elem.setAttribute( "second", time.second() );
    elem.setAttribute( "msecond", time.msec() );
    elem.setAttribute( "fix", m_subtype == VST_TIME_FIX );
    elem.setAttribute( "correct", m_correctTime );
}

void KoTimeVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );

    QDomElement e = elem.namedItem( "TIME" ).toElement();
    if (!e.isNull())
    {
        int h = e.attribute("hour").toInt();
        int m = e.attribute("minute").toInt();
        int s = e.attribute("second").toInt();
        int ms = e.attribute("msecond").toInt();
        int correct = 0;
        if ( e.hasAttribute("correct"))
            correct=e.attribute("correct").toInt();
        bool fix = static_cast<bool>( e.attribute("fix").toInt() );
        if ( fix )
        {
            QTime time;
            time.setHMS( h, m, s, ms );
            time = time.addSecs( 60*m_correctTime );
            m_varValue = QVariant( time);

        }
        m_subtype = fix ? VST_TIME_FIX : VST_TIME_CURRENT;
        m_correctTime = correct;
    }
}

QStringList KoTimeVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Current Time (fixed)" );
    lst << i18n( "Current Time (variable)" );
    return lst;
}

QStringList KoTimeVariable::subTypeText()
{
    return KoTimeVariable::actionTexts();
}

QCString KoTimeVariable::formatStr(int & _correct)
{
    QCString string;
    QStringList stringList;
    KDialogBase* dialog=new KDialogBase(0, 0, true, i18n("Time Format"), KDialogBase::Ok|KDialogBase::Cancel);
    TimeFormatWidget* widget=new TimeFormatWidget(dialog);
    dialog->setMainWidget(widget);
    KConfig* config = KoGlobal::kofficeConfig();
    int count=0;
    if( config->hasGroup("Time format history") )
    {
        KConfigGroupSaver cgs( config, "Time format history" );
        const int noe=config->readNumEntry("Number Of Entries", 5);
        for(int i=0;i<noe;i++)
        {
            QString num;
            num.setNum(i);
            QString tmpString(config->readEntry("Last Used"+num));
            if(tmpString.startsWith("locale"))
                continue;
            else if(stringList.contains(tmpString))
                continue;
            else if(!tmpString.isEmpty())
            {
                stringList.append(tmpString);
                count++;
            }
        }
    }
    if(!stringList.isEmpty())
    {
        widget->combo1->insertItem("---");
        widget->combo1->insertStringList(stringList);
    }
    if(false) // ### TODO: select the last used item
    {
        QComboBox *combo= widget->combo1;
        combo->setCurrentItem(combo->count() -1);
    }
    if(dialog->exec()==QDialog::Accepted)
    {
        string = widget->resultString().utf8();
        _correct = widget->correctValue();
    }
    else
    {
        return 0;
    }
    config->setGroup("Time format history");
    stringList.remove(string);
    stringList.prepend(string);
    for(int i=0;i<=count;i++)
    {
        QString num;
        num.setNum(i);
        config->writeEntry("Last Used"+num, stringList[i]);
    }
    config->sync();
    delete dialog;
    return QCString("TIME"+string );
}

QCString KoTimeVariable::defaultFormat()
{
    return QCString(QCString("TIME")+QCString("locale") );
}


/******************************************************************/
/* Class: KoCustomVariable                                        */
/******************************************************************/
KoCustomVariable::KoCustomVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat, KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl )
{
    m_varValue = QVariant( name );
}

QString KoCustomVariable::fieldCode()
{
    return i18n("Custom Variable");
}

QString KoCustomVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        return value();
} // use a format when they are customizable



void KoCustomVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement elem = parentElem.ownerDocument().createElement( "CUSTOM" );
    parentElem.appendChild( elem );
    elem.setAttribute( "name", m_varValue.toString() );
    elem.setAttribute( "value", value() );
}

void KoCustomVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "CUSTOM" ).toElement();
    if (!e.isNull())
    {
        m_varValue = QVariant (e.attribute( "name" ));
        setValue( e.attribute( "value" ) );
    }
}

QString KoCustomVariable::value() const
{
    return m_varColl->getVariableValue( m_varValue.toString() );
}

void KoCustomVariable::setValue( const QString &v )
{
    m_varColl->setVariableValue( m_varValue.toString(), v );
}

QStringList KoCustomVariable::actionTexts()
{
    return QStringList( i18n( "Custom..." ) );
}

void KoCustomVariable::recalc()
{
    resize();
}

/******************************************************************/
/* Class: KoMailMergeVariable                                  */
/******************************************************************/
KoMailMergeVariable::KoMailMergeVariable( KoTextDocument *textdoc, const QString &name, KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat, _varColl )
{
    m_varValue = QVariant ( name );
}

QString KoMailMergeVariable::fieldCode()
{
    return i18n("Mail Merge");
}


void KoMailMergeVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement elem = parentElem.ownerDocument().createElement( "MAILMERGE" );
    parentElem.appendChild( elem );
    elem.setAttribute( "name", m_varValue.toString() );
}

void KoMailMergeVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "MAILMERGE" ).toElement();
    if (!e.isNull())
        m_varValue = QVariant( e.attribute( "name" ) );
}

QString KoMailMergeVariable::value() const
{
    return QString();//m_doc->getMailMergeDataBase()->getValue( m_name );
}

QString KoMailMergeVariable::text(bool /*realValue*/)
{
    // ## should use a format maybe
    QString v = value();
    if ( v == name() )
        return "<" + v + ">";
    return v;
}

QStringList KoMailMergeVariable::actionTexts()
{
    return QStringList( i18n( "&Mail Merge..." ) );
}

/******************************************************************/
/* Class: KoPgNumVariable                                         */
/******************************************************************/
KoPgNumVariable::KoPgNumVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat,KoVariableCollection *_varColl )
        : KoVariable( textdoc, varFormat, _varColl ), m_subtype( subtype )
{
}

QString KoPgNumVariable::fieldCode()
{
    if ( m_subtype == VST_PGNUM_CURRENT )
        return i18n("Page Current Num");
    else if ( m_subtype == VST_PGNUM_TOTAL )
        return i18n("Total Page Num");
    else if ( m_subtype == VST_CURRENT_SECTION )
        return i18n("Current Section");
    else if ( m_subtype == VST_PGNUM_PREVIOUS )
        return i18n("Previous Page Number");
    else if ( m_subtype == VST_PGNUM_NEXT )
        return i18n("Next Page Number");

    else
        return i18n("Current Section");
}


void KoPgNumVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement pgNumElem = parentElem.ownerDocument().createElement( "PGNUM" );
    parentElem.appendChild( pgNumElem );
    pgNumElem.setAttribute( "subtype", m_subtype );
    if ( m_subtype != VST_CURRENT_SECTION )
        pgNumElem.setAttribute( "value", m_varValue.toInt() );
    else
        pgNumElem.setAttribute( "value", m_varValue.toString() );
}

void KoPgNumVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement pgNumElem = elem.namedItem( "PGNUM" ).toElement();
    if (!pgNumElem.isNull())
    {
        m_subtype = pgNumElem.attribute("subtype").toInt();
        // ### This could use the format...
        if ( m_subtype != VST_CURRENT_SECTION )
            m_varValue = QVariant(pgNumElem.attribute("value").toInt());
        else
            m_varValue = QVariant(pgNumElem.attribute("value"));
    }
}

QStringList KoPgNumVariable::actionTexts()
{
    QStringList lst;
    lst << i18n( "Page Number" );
    lst << i18n( "Number of Pages" );
    lst << i18n( "Section Title" );
    lst << i18n( "Previous Page" );
    lst << i18n( "Next Page" );
    return lst;
}

QStringList KoPgNumVariable::subTypeText()
{
    return KoPgNumVariable::actionTexts();
}

void KoPgNumVariable::setVariableSubType( short int type )
{
    m_subtype = type;
    Q_ASSERT( m_varColl );
    KoVariableFormatCollection* fc = m_varColl->formatCollection();
    setVariableFormat((m_subtype == VST_CURRENT_SECTION) ? fc->format("STRING") : fc->format("NUMBER"));
}

/******************************************************************/
/* Class: KoFieldVariable                                         */
/******************************************************************/
KoFieldVariable::KoFieldVariable( KoTextDocument *textdoc, short int subtype, KoVariableFormat *varFormat, KoVariableCollection *_varColl ,KoDocument *_doc )
    : KoVariable( textdoc, varFormat,_varColl ), m_subtype( subtype ), m_doc(_doc)
{
}

QString KoFieldVariable::fieldCode()
{
    switch( m_subtype ) {
    case VST_FILENAME:
        return i18n("Filename");
        break;
    case VST_DIRECTORYNAME:
        return i18n("Directory Name");
        break;
    case VST_PATHFILENAME:
        return i18n("Path Filename");
        break;
    case VST_FILENAMEWITHOUTEXTENSION:
        return i18n("Filename Without Extension");
        break;
    case VST_AUTHORNAME:
        return i18n("Author Name");
        break;
    case VST_EMAIL:
        return i18n("Email");
        break;
    case VST_COMPANYNAME:
        return i18n("Company Name");
        break;
    case VST_TELEPHONE:
        return i18n("Telephone");
        break;
    case VST_FAX:
        return i18n("Fax");
        break;
    case VST_COUNTRY:
        return i18n("Country");
        break;
    case VST_POSTAL_CODE:
        return i18n("Postal Code");
        break;
    case VST_CITY:
        return i18n("City");
        break;
    case VST_STREET:
        return i18n("Street");
        break;
    case VST_AUTHORTITLE:
        return i18n("Author Title");
        break;
    case VST_TITLE:
        return i18n("Title");
        break;
    case VST_ABSTRACT:
        return i18n("Abstract");
        break;
    case VST_INITIAL:
        return i18n("Initials");
        break;
    }
    return i18n("Field");
}

QString KoFieldVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        return value();
} // use a format when they are customizable


void KoFieldVariable::saveVariable( QDomElement& parentElem )
{
    //kdDebug(32500) << "KoFieldVariable::saveVariable" << endl;
    QDomElement elem = parentElem.ownerDocument().createElement( "FIELD" );
    parentElem.appendChild( elem );
    elem.setAttribute( "subtype", m_subtype );
    elem.setAttribute( "value", m_varValue.toString() );
}

void KoFieldVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement e = elem.namedItem( "FIELD" ).toElement();
    if (!e.isNull())
    {
        m_subtype = e.attribute( "subtype" ).toInt();
        if ( m_subtype == VST_NONE )
            kdWarning() << "Field subtype of -1 found in the file !" << endl;
        m_varValue = QVariant( e.attribute( "value" ) );
    } else
        kdWarning() << "FIELD element not found !" << endl;
}

void KoFieldVariable::recalc()
{
    QString value;
    switch( m_subtype ) {
        case VST_NONE:
            kdWarning() << "KoFieldVariable::recalc() called with m_subtype = VST_NONE !" << endl;
            break;
        case VST_FILENAME:
            value = m_doc->url().fileName();
            break;
        case VST_DIRECTORYNAME:
            value = m_doc->url().directory();
            break;
	    case VST_PATHFILENAME:
            value=m_doc->url().path();
            break;
        case VST_FILENAMEWITHOUTEXTENSION:
        {
            QString file=m_doc->url().fileName();
            int pos=file.findRev(".");
            if(pos !=-1)
                value=file.mid(0,pos);
            else
                value=file;
        }
        break;
        case VST_AUTHORNAME:
        case VST_EMAIL:
        case VST_COMPANYNAME:
        case VST_TELEPHONE:
        case VST_FAX:
        case VST_COUNTRY:
        case VST_POSTAL_CODE:
        case VST_CITY:
        case VST_STREET:
        case VST_AUTHORTITLE:
        case VST_INITIAL:
        {
            KoDocumentInfo * info = m_doc->documentInfo();
            KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor *>(info->page( "author" ));
            if ( !authorPage )
                kdWarning() << "Author information not found in documentInfo !" << endl;
            else
            {
                if ( m_subtype == VST_AUTHORNAME )
                    value = authorPage->fullName();
                else if ( m_subtype == VST_EMAIL )
                    value = authorPage->email();
                else if ( m_subtype == VST_COMPANYNAME )
                    value = authorPage->company();
                else if ( m_subtype == VST_TELEPHONE )
                    value = authorPage->telephone();
                else if ( m_subtype == VST_FAX )
                    value = authorPage->fax();
                else if ( m_subtype == VST_COUNTRY )
                    value = authorPage->country();
                else if ( m_subtype == VST_POSTAL_CODE )
                    value = authorPage->postalCode();
                else if ( m_subtype == VST_CITY )
                    value = authorPage->city();
                else if ( m_subtype == VST_STREET )
                    value = authorPage->street();
                else if ( m_subtype == VST_AUTHORTITLE )
                    value = authorPage->title();
                else if ( m_subtype == VST_INITIAL )
                    value = authorPage->initial();
            }
        }
        break;
        case VST_TITLE:
        case VST_ABSTRACT:
        {
            KoDocumentInfo * info = m_doc->documentInfo();
            KoDocumentInfoAbout * aboutPage = static_cast<KoDocumentInfoAbout *>(info->page( "about" ));
            if ( !aboutPage )
                kdWarning() << "'About' page not found in documentInfo !" << endl;
            else
            {
                if ( m_subtype == VST_TITLE )
                    value = aboutPage->title();
                else
                    value = aboutPage->abstract();
            }
        }
        break;
    }

    if (value.isEmpty())        // try the initial value
        value = m_varValue.toString();

    if (value.isEmpty())        // still empty? give up
        value = i18n("<None>");

    m_varValue = QVariant( value );

    resize();
}

QStringList KoFieldVariable::actionTexts()
{
    // NOTE: if you change here, also change fieldSubType()
    QStringList lst;
    lst << i18n( "Author Name" );
    lst << i18n( "Title" );
    lst << i18n( "Company" );
    lst << i18n( "Email" );
    lst << i18n( "Telephone");
    lst << i18n( "Fax");
    lst << i18n( "Street" );
    lst << i18n( "Postal Code" );
    lst << i18n( "City" );
    lst << i18n( "Country");

    lst << i18n( "Document Title" );
    lst << i18n( "Document Abstract" );

    lst << i18n( "File Name" );
    lst << i18n( "File Name without Extension" );
    lst << i18n( "Directory Name" ); // is "Name" necessary ?
    lst << i18n( "Directory && File Name" );
    lst << i18n( "Initials" );
    return lst;
}

short int KoFieldVariable::variableSubType( short int menuNumber )
{
    return fieldSubType(menuNumber);
}

KoFieldVariable::FieldSubType KoFieldVariable::fieldSubType(short int menuNumber)
{
    // NOTE: if you change here, also change actionTexts()
    FieldSubType v;
    switch (menuNumber)
    {
        case 0: v = VST_AUTHORNAME;
                break;
        case 1: v = VST_AUTHORTITLE;
                break;
        case 2: v = VST_COMPANYNAME;
                break;
        case 3: v = VST_EMAIL;
                break;
        case 4: v = VST_TELEPHONE;
                break;
        case 5: v = VST_FAX;
                break;
        case 6: v = VST_STREET;
                break;
        case 7: v = VST_POSTAL_CODE;
                break;
        case 8: v = VST_CITY;
                break;
        case 9: v = VST_COUNTRY;
                break;
        case 10: v = VST_TITLE;
                break;
        case 11: v = VST_ABSTRACT;
                break;
        case 12: v = VST_FILENAME;
                break;
        case 13: v = VST_FILENAMEWITHOUTEXTENSION;
                break;
        case 14: v = VST_DIRECTORYNAME;
                break;
        case 15: v = VST_PATHFILENAME;
                break;
        case 16: v = VST_INITIAL;
                break;
        default:
            v = VST_NONE;
            break;
    }
    return v;
}

QStringList KoFieldVariable::subTypeText()
{
    return KoFieldVariable::actionTexts();
}

/******************************************************************/
/* Class: KoLinkVariable                                          */
/******************************************************************/
KoLinkVariable::KoLinkVariable( KoTextDocument *textdoc, const QString & _linkName, const QString & _ulr,KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl )
    ,m_url(_ulr)
{
    m_varValue = QVariant( _linkName );
}

QString KoLinkVariable::fieldCode()
{
    return i18n("Link");
}

QString KoLinkVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        return value();
}

void KoLinkVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement linkElem = parentElem.ownerDocument().createElement( "LINK" );
    parentElem.appendChild( linkElem );
    linkElem.setAttribute( "linkName", m_varValue.toString() );
    linkElem.setAttribute( "hrefName", m_url );
}

void KoLinkVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement linkElem = elem.namedItem( "LINK" ).toElement();
    if (!linkElem.isNull())
    {
        m_varValue = QVariant(linkElem.attribute("linkName"));
        m_url = linkElem.attribute("hrefName");
    }
}

void KoLinkVariable::recalc()
{
    resize();
}

QStringList KoLinkVariable::actionTexts()
{
    return QStringList( i18n( "Link..." ) );
}


void KoLinkVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int /*cx*/, int /*cy*/, int /*cw*/, int /*ch*/, const QColorGroup& cg, bool selected, int offset, bool drawingShadow )
{
    KoTextFormat * fmt = format();
    KoZoomHandler * zh = textDocument()->paintingZoomHandler();

    bool displayLink = m_varColl->variableSetting()->displayLink();
    QFont font( fmt->screenFont( zh ) );
    if ( m_varColl->variableSetting()->underlineLink() )
        font.setUnderline( true );
    QColor textColor = displayLink ? cg.color( QColorGroup::Link ) : fmt->color();

    drawCustomItemHelper( p, x, y, wpix, hpix, ascentpix, cg, selected, offset, fmt, font, textColor, drawingShadow );
}


/******************************************************************/
/* Class: KoNoteVariable                                          */
/******************************************************************/
KoNoteVariable::KoNoteVariable( KoTextDocument *textdoc, const QString & _note,KoVariableFormat *varFormat,KoVariableCollection *_varColl )
    : KoVariable( textdoc, varFormat,_varColl )
{
    m_varValue = QVariant( _note );
}

QString KoNoteVariable::fieldCode()
{
    return i18n("Note");
}

void KoNoteVariable::saveVariable( QDomElement& parentElem )
{
    QDomElement linkElem = parentElem.ownerDocument().createElement( "NOTE" );
    parentElem.appendChild( linkElem );
    linkElem.setAttribute( "note", m_varValue.toString() );
}

void KoNoteVariable::load( QDomElement& elem )
{
    KoVariable::load( elem );
    QDomElement linkElem = elem.namedItem( "NOTE" ).toElement();
    if (!linkElem.isNull())
    {
        m_varValue = QVariant(linkElem.attribute("note"));
    }
}

void KoNoteVariable::recalc()
{
    resize();
}

QStringList KoNoteVariable::actionTexts()
{
    return QStringList( i18n( "Note..." ) );
}

QString KoNoteVariable::text(bool realValue)
{
    if (m_varColl->variableSetting()->displayComment() &&
        m_varColl->variableSetting()->displayFieldCode()&&!realValue)
        return fieldCode();
    else
        //for a note return just a "space" we can look at
        //note when we "right button"
        return QString(" ");

}

void KoNoteVariable::drawCustomItem( QPainter* p, int x, int y, int wpix, int hpix, int ascentpix, int cx, int cy, int cw, int ch, const QColorGroup& cg, bool selected, int offset, bool drawingShadow )
{
    if ( !m_varColl->variableSetting()->displayComment())
        return;

    KoTextFormat * fmt = format();
    //kdDebug(32500) << "KoNoteVariable::drawCustomItem index=" << index() << " x=" << x << " y=" << y << endl;

    p->save();
    p->setPen( QPen( fmt->color() ) );
    if ( fmt->textBackgroundColor().isValid() )
        p->fillRect( x, y, wpix, hpix, fmt->textBackgroundColor() );
    if ( selected )
    {
        p->setPen( QPen( cg.color( QColorGroup::HighlightedText ) ) );
        p->fillRect( x, y, wpix, hpix, cg.color( QColorGroup::Highlight ) );
    }
    else if ( textDocument() && p->device()->devType() != QInternal::Printer
        && !textDocument()->dontDrawingNoteVariable())
    {
        p->fillRect( x, y, wpix, hpix, Qt::yellow);
        p->setPen( QPen( cg.color( QColorGroup::Highlight ), 0, Qt::DotLine ) );
        p->drawRect( x, y, wpix, hpix );
    }
    //call it for use drawCustomItemHelper just for draw font effect
    KoVariable::drawCustomItem( p, x, y, wpix, hpix, ascentpix, cx, cy, cw, ch, cg, selected, offset, drawingShadow );

    p->restore();
}

void KoPgNumVariable::setSectionTitle( const QString& _title )
{
    QString title( _title );
    if ( title.isEmpty() )
    {
        title = i18n("<None>"); // TODO after msg freeze: <No title>
    }
    m_varValue = QVariant( title );
}

