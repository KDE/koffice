/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KSPREAD_VALUE_H
#define KSPREAD_VALUE_H

#include <qdatetime.h>
#include <qstring.h>
#include <qtextstream.h>

class KSpreadValueData;

/**
 * Provides a wrapper for cell value.
 *
 * Each cell in a worksheet must hold a value, either as enterred by user
 * or as a result of formula evaluation. Default cell holds empty value.
 *
 * KSpreadValue uses implicit data sharing to reduce memory usage.
 */

class KSpreadValue
{

  public:

    typedef enum {
      Empty,
      Boolean,
      Integer,
      Float,
      String,
      CellRange, // not used yet
      Array,     // not used yet
      Error
    } Type;

    /**
     * Creates an empty value, i.e holds nothing.
     */
    KSpreadValue();

    /** 
     * Creates a value of certain type.
     */
    KSpreadValue( Type _type );

    /** 
     * Destroys the value.
     */
    virtual ~KSpreadValue();

    /** 
     * Creates a copy from another value.
     */
    KSpreadValue( const KSpreadValue& _value );

    /** 
     * Assigns from another value.
     *
     * Because the data is implicitly shared, such assignment is very fast and
     * doesn't consume additional memory.
     */
    KSpreadValue& operator= ( const KSpreadValue& _value );

    /** 
     * Assigns from another value. Same as above.
     */
    KSpreadValue& assign( const KSpreadValue& _value );

    /**
     * Creates a boolean value.
     */
    KSpreadValue( bool b );

    /**
     * Creates an integer value.
     */
    KSpreadValue( long i );

    /**
     * Creates an integer value.
     */
    KSpreadValue( int i );

    /**
     * Create a floating-point value.
     */
    KSpreadValue( double f );

    /** 
     * Create a string value.
     */
    KSpreadValue( const QString& s );

    /**
     * Create a floating-point value from date/time.
     *
     * Internally date/time is represented as serial-number, i.e number of
     * elapsed day since reference date. Day 61 is defined as March 1, 1900.
     */
    KSpreadValue( const QDateTime& dt );

    /**
     * Create a floating-point value from date.
     * See also note above.
     */
    KSpreadValue( const QTime& time );

    /**
     * Create a floating-point value from time.
     * See also note above.
     */
    KSpreadValue( const QDate& date );

    /**
     * Returns the type of the value.
     */
    Type type() const;

    /**
     * Returns true if empty.
     */
    bool isEmpty() const { return type() == Empty; }

    /**
     * Returns true if the type of this value is Boolean.
     */
    bool isBoolean() const { return type() == Boolean; }

    /**
     * Returns true if the type of this value is integer.
     */
    bool isInteger() const { return type() == Integer; }

    /**
     * Returns true if the type of this value is floating-point.
     */
    bool isFloat() const { return type() == Float; }

    /**
     * Returns true if the type of this value is either 
     * integer or floating-point.
     */
    bool isNumber() const { return (type() == Integer) || (type() == Float); }

    /**
     * Returns true if the type of this value is string.
     */
    bool isString() const { return type() == String; }

    /**
     * Returns true if this value holds error information.
     */
    bool isError() const { return type() == Error; }

    /**
     * Sets this value to another value.
     */
    void setValue( const KSpreadValue& v );

    /**
     * Sets this value to boolean value.
     */
    void setValue( bool b );

    /**
     * Sets this value to integer value.
     */
    void setValue( long i );

    /**
     * Sets this value to integer value.
     */
    void setValue( int i );
    
    /**
     * Sets this value to floating-point value.
     */
    void setValue( double f );

    /**
     * Sets this value to string value.
     */
    void setValue( const QString& s );

    /**
     * Sets this value to hold error message.
     */
    void setError( const QString& msg );

    /**
     * Sets this value to floating-point number representing the date/time.
     */
    void setValue( const QDateTime& dt );

    /**
     * Sets this value to floating-point number representing the date.
     */
    void setValue( const QTime& dt );

    /**
     * Sets this value to floating-point number representing the time.
     */
    void setValue( const QDate& dt );

    /**
     * Returns the boolean value of this value.
     *
     * Call this function only if isBoolean() returns true.
     */
    bool asBoolean() const;

    /**
     * Returns the integer value of this value.
     *
     * Call this function only if isNumber() returns true.
     */
    long asInteger() const;

    /**
     * Returns the floating-point value of this value.
     *
     * Call this function only if isNumber() returns true.
     */
    double asFloat() const;

    /**
     * Returns the string value of this value.
     *
     * Call this function only if isString() returns true.
     */
    QString asString() const;

    /**
     * Returns the date/time representation of this value.
     */
    QDateTime asDateTime() const;

    /**
     * Returns the date representation of this value.
     */
    QDate asDate() const;

    /**
     * Returns the time representation of this value.
     */
    QTime asTime() const;

    /**
     * Returns error message associated with this value.
     *
     * Call this function only if isError() returns true.
     */
    QString errorMessage() const;

    /**
     * Detaches itself from shared value data, i.e make a private, deep copy
     * of the data. Usually this function is called automatically so you
     * don't have to care about it.
     */
    void detach();

    /**
     * Returns constant reference to empty value.
     */
    static const KSpreadValue& empty();

    /**
     * Returns constant reference to #DIV/0! error.
     *
     * This is used to indicate that a formula divides by 0 (zero).
     */
    static const KSpreadValue& errorDIV0();

    /**
     * Returns constant reference to #N/A error.
     *
     * This is to indicate that  a value is not available to a function.
     */
    static const KSpreadValue& errorNA();

    /**
     * Returns constant reference to #NAME? error.
     *
     * This is to indicate that certain text inside formula is not 
     * recognized, possibly a misspelled name or name that 
     * does not exist.
     */
    static const KSpreadValue& errorNAME();

    /**
     * Returns constant reference to #NUM! error.
     *
     * This is to indicate a problem with a number in a formula.
     */
    static const KSpreadValue& errorNUM();

    /**
     * Returns constant reference to #NULL! error.
     *
     * This is to indicate that two area do not intersect.
     */
    static const KSpreadValue& errorNULL();

    /**
     * Returns constant reference to #REF! error.
     *
     * This is used to indicate an invalid cell reference.
     */
    static const KSpreadValue& errorREF();

    /**
     * Returns constant reference to #VALUE! error.
     *
     * This is to indicate that wrong type of argument or operand
     * is used, usually within a function call, e.g SIN("some text").
     */
    static const KSpreadValue& errorVALUE();

    /**
     * Returns true if it is OK to compare this value with v.
     * If this function returns false, then return value of compare is undefined.
     */
    bool allowComparison( const KSpreadValue& v ) const;
    
    /**
     * Returns -1, 0, 1, depends whether this value is less than, equal to, or
     * greater than v.
     */
    int compare( const KSpreadValue& v ) const;
    
    /**
     * Returns true if this value is equal to v.
     */
    bool equal( const KSpreadValue& v ) const;

    /**
     * Returns true if this value is less than v.
     */
    bool less( const KSpreadValue& v ) const;

    /**
     * Returns true if this value is greater than v.
     */
    bool greater( const KSpreadValue& v ) const;
    
    static int compare( double v1, double v2 );
    
    bool isZero() const;
    
    static bool isZero( double v );
    
    /**
     * Adds two given values, return the result. This only works if both are
     * numbers. 
     *
     * The result is a floating-point number, except when both values 
     * are integers and the result is still within integer limits.
     */    
     static KSpreadValue add( const KSpreadValue& v1, const KSpreadValue& v2 );
     
     KSpreadValue& add( const KSpreadValue& v );

    /**
     * Subtracts two given values, return the result. This only works if both are
     * numbers. 
     *
     * The result is a floating-point number, except when both values 
     * are integers and the result is still within integer limits.
     */    
     static KSpreadValue sub( const KSpreadValue& v1, const KSpreadValue& v2 );

     KSpreadValue& sub( const KSpreadValue& v );

    /**
     * Multiplies two given values, return the result. This only works if both are
     * numbers. If any of them is not a number, it will be converted first
     * into number. When conversion fails, this function will return #VALUE.
     *
     * The result is a floating-point number, except when both values 
     * are integers and the result is still within integer limits.
     */    
     static KSpreadValue mul( const KSpreadValue& v1, const KSpreadValue& v2 );

     KSpreadValue& mul( const KSpreadValue& v );
     
    /**
     * Divides two given values, return the result. This only works if both are
     * numbers. If any of them is not a number, it will be converted first
     * into number. When conversion fails, this function will return #VALUE.
     *
     * The result is a floating-point number, except when the result is still
     * integer and within integer limits when both values are integers.
     */    
     static KSpreadValue div( const KSpreadValue& v1, const KSpreadValue& v2 );

     KSpreadValue& div( const KSpreadValue& v );
     
    /**
     * Calculates one value raised to the power of another value. This only 
     * works if both are numbers. If any of them is not a number, it will be 
     * converted first into number. When conversion fails, this function will 
     * return #VALUE.
     *
     * The result is a floating-point number, except when the result is still
     * integer and within integer limits when both values are integers.
     */    
     static KSpreadValue pow( const KSpreadValue& v1, const KSpreadValue& v2 );
     
     KSpreadValue& pow( const KSpreadValue& v );

  protected:

    KSpreadValueData* d; // can't never be 0
};

QTextStream& operator<< ( QTextStream& ts, KSpreadValue::Type type );


#endif // KSPREAD_VALUE_H
