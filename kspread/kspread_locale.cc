#include "kspread_locale.h"

#include <qdom.h>

KSpreadLocale::KSpreadLocale()
	: KLocale("kspread")
{
	insertCatalogue("koffice");
}

void KSpreadLocale::load( const QDomElement& element )
{
    if ( element.hasAttribute( "weekStartsMonday" ) )
    {
	QString c = element.attribute( "weekStartsMonday" );
	setWeekStartsMonday( c != "False");
    }
    if ( element.hasAttribute( "decimalSymbol" ) )
	setDecimalSymbol( element.attribute( "decimalSymbol" ) );
    if ( element.hasAttribute( "thousandsSeparator" ) )
	setThousandsSeparator( element.attribute( "thousandsSeparator" ) );
    if ( element.hasAttribute( "currencySymbol" ) )
	setCurrencySymbol( element.attribute( "currencySymbol" ) );
    if ( element.hasAttribute( "monetaryDecimalSymbol" ) )
	setMonetaryDecimalSymbol( element.attribute( "monetaryDecimalSymbol" ) );
    if ( element.hasAttribute( "monetaryThousandsSeparator" ) )
	setMonetaryThousandsSeparator( element.attribute( "monetaryThousandsSeparator" ) );
    if ( element.hasAttribute( "positiveSign" ) )
	setPositiveSign( element.attribute( "positiveSign" ) );
    if ( element.hasAttribute( "negativeSign" ) )
	setNegativeSign( element.attribute( "negativeSign" ) );
    if ( element.hasAttribute( "fracDigits" ) )
	setFracDigits( element.attribute( "fracDigits" ).toInt() );
    if ( element.hasAttribute( "positivePrefixCurrencySymbol" ) )
    {
	QString c = element.attribute( "positivePrefixCurrencySymbol" );
	setPositivePrefixCurrencySymbol( c == "True" );
    }
    if ( element.hasAttribute( "negativePrefixCurrencySymbol" ) )
    {
	QString c = element.attribute( "negativePrefixCurrencySymbol" );
	setNegativePrefixCurrencySymbol( c == "True" );
    }
    if ( element.hasAttribute( "positiveMonetarySignPosition" ) )
	setPositiveMonetarySignPosition( (SignPosition)element.attribute( "positiveMonetarySignPosition" ).toInt() );
    if ( element.hasAttribute( "negativeMonetarySignPosition" ) )
	setNegativeMonetarySignPosition( (SignPosition)element.attribute( "negativeMonetarySignPosition" ).toInt() );
    if ( element.hasAttribute( "timeFormat" ) )
	setTimeFormat( element.attribute( "timeFormat" ) );
    if ( element.hasAttribute( "dateFormat" ) )
	setDateFormat( element.attribute( "dateFormat" ) );
    if ( element.hasAttribute( "dateFormatShort" ) )
	setDateFormatShort( element.attribute( "dateFormatShort" ) );
}

QDomElement KSpreadLocale::save( QDomDocument& doc ) const
{
    QDomElement element = doc.createElement( "locale" );

    element.setAttribute( "weekStartsMonday", weekStartsMonday() ? "True" : "False" );
    element.setAttribute( "decimalSymbol", decimalSymbol() );
    element.setAttribute( "thousandsSeparator", thousandsSeparator() );
    element.setAttribute( "currencySymbol", currencySymbol() );
    element.setAttribute( "monetaryDecimalSymbol", monetaryDecimalSymbol() );
    element.setAttribute( "monetaryThousandsSeparator", monetaryThousandsSeparator() );
    element.setAttribute( "positiveSign", positiveSign() );
    element.setAttribute( "negativeSign", negativeSign() );
    element.setAttribute( "fracDigits", fracDigits() );
    element.setAttribute( "positivePrefixCurrencySymbol", positivePrefixCurrencySymbol() ? "True" : "False");
    element.setAttribute( "negativePrefixCurrencySymbol", negativePrefixCurrencySymbol() ? "True" : "False");
    element.setAttribute( "positiveMonetarySignPosition", (int)positiveMonetarySignPosition() );
    element.setAttribute( "negativeMonetarySignPosition", (int)negativeMonetarySignPosition() );
    element.setAttribute( "timeFormat", timeFormat() );
    element.setAttribute( "dateFormat", dateFormat() );
    element.setAttribute( "dateFormatShort", dateFormatShort() );

    return element;
}
