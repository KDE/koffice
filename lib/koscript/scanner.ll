/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

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

%{

#define YY_NO_UNPUT
#define YY_USER_INIT BEGIN(PLAIN);

#include <stdlib.h>
#include <ctype.h>
#include <qstring.h>

#include "koscript_parsenode.h"
#include "koscript_types.h"
#include "koscript_synext.h"
#include "koscript_locale.h"

#ifndef KDE_USE_FINAL
#include "yacc.cc.h"
#endif

extern int idl_line_no;

static bool s_kspread;
KLocale* s_koscript_locale = 0;
static KLocale* s_defaultLocale = 0;

static KScript::Long ascii_to_longlong( long base, const char *s )
{
  KScript::Long ll = 0;
  while( *s != '\0' ) {
    char c = *s++;
    if( c >= 'a' )
      c -= 'a' - 'A';
    c -= '0';
    if( c > 9 )
      c -= 'A' - '0' - 10;
    ll = ll * base + c;
  }
  return ll;
}

static KScript::Double ascii_to_longdouble( const char *s )
{
  KScript::Double d;
#ifdef HAVE_SCANF_LF
  sscanf (s, "%Lf", &d);
#else
  /*
   * this is only an approximation and will probably break fixed<>
   * parameter calculation on systems where
   * sizeof(double) < sizeof(long double). but fortunately all
   * systems where scanf("%Lf") is known to be broken (Linux/Alpha
   * and HPUX) have sizeof(double) == sizeof(long double).
   */
  d = strtod (s, NULL);
#endif
  return d;
}

static ushort translate_char( const char *s )
{
  char c = *s++;
  char buf[5];

  if( c != '\\' )
    return c;
  c = *s++;
  switch( c ) {
  case 'n':
    return '\n';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case 'b':
    return '\b';
  case 'r':
    return '\r';
  case 'f':
    return '\f';
  case 'a':
    return '\a';
  case 'u':        /* FIXME: This is really brittle */
    buf[0]=*s++; buf[1]=*s++;
    buf[2]=*s++; buf[3]=*s++;
    buf[4]='\0';
    return ascii_to_longlong(16, buf);
  case '\\':
    return '\\';
  case '?':
    return '\?';
  case '\'':
    return '\'';
  case '"':
    return '"';
  case 'x':
  case 'X':
    return (char) ascii_to_longlong( 16, s );
  default:
    // Gotta be an octal
    return (char) ascii_to_longlong( 8, s );
  }
}

static QChar translate_unichar( const QChar *s )
{
  QChar c = *s++;

  if( c != '\\' )
    return c;
  c = *s++;
  switch( c.latin1() ) {
  case 'n':
    return '\n';
  case 't':
    return '\t';
  case 'v':
    return '\v';
  case 'b':
    return '\b';
  case 'r':
    return '\r';
  case 'f':
    return '\f';
  case 'a':
    return '\a';
  case '\\':
    return '\\';
  case '?':
    return '\?';
  case '\'':
    return '\'';
  case '"':
    return '"';
  default:
    return c;
  }
}

static void translate_string( QString& str )
{
        int pos = 0;
        while( ( pos = str.find( '\\', pos ) ) != -1 )
        {
                QChar ch = translate_unichar( str.unicode() + pos );
                str.replace( pos, 2, &ch, 1 );
        }
}

%}

%option noyywrap

/* allow localized input of integer/fp literals, send  T_COMMA for ';' (Werner) */
%s KSPREAD

/* standard ASCII stuff, C-like fp literals, T_COMMA for - yay! - ',' (Werner) */
%s PLAIN

/*--------------------------------------------------------------------------*/
/* Note: All the "special" shortcuts are prefixed with "Plain_" or "KSpread_" */

Sep                     [ .,]
Sep2                    [.,]
Digit                   [0-9]
Digits                  {Digit}+
Oct_Digit               [0-7]
Hex_Digit               [a-fA-F0-9]
Plain_Int_Literal       [1-9][0-9]*
Oct_Literal             0{Oct_Digit}*
Hex_Literal             (0x|0X){Hex_Digit}*
Esc_Sequence1           "\\"[ntvbrfau\\\?\'\"]
Esc_Sequence2           "\\"{Oct_Digit}{1,3}
Esc_Sequence3           "\\"(x|X){Hex_Digit}{1,2}
Esc_Sequence            ({Esc_Sequence1}|{Esc_Sequence2}|{Esc_Sequence3})
Char                    ([^\n\t\"\'\\]|{Esc_Sequence})
Char_Literal            "'"({Char}|\")"'"
String_Literal          \"({Char}|"'")*\"
Table_Name              [A-Z a-z0-9\x80-\xff]+

Plain_Float_Literal1    {Digits}"."{Digits}(e|E)("+"|"-")?{Digits}
Plain_Float_Literal2    {Digits}(e|E)("+"|"-")?{Digits}
Plain_Float_Literal3    {Digits}"."{Digits}
Plain_Float_Literal4    "."{Digits}
Plain_Float_Literal5    "."{Digits}(e|E)("+"|"-")?{Digits}

KSpread_Float_Literal1  {Digits}({Sep}{Digit}{Digit}{Digit})*{Sep2}{Digits}(e|E)("+"|"-")?{Digits}
KSpread_Float_Literal2  {Digits}({Sep}{Digit}{Digit}{Digit})*(e|E)("+"|"-")?{Digits}
KSpread_Float_Literal3  {Digits}({Sep}{Digit}{Digit}{Digit})*{Sep2}{Digits}
KSpread_Float_Literal4  {Sep2}{Digits}
KSpread_Float_Literal5  {Sep2}{Digits}(e|E)("+"|"-")?{Digits}

KSpread_Int_Or_Float_Literal2   [1-9][0-9]*({Sep}{Digit}{Digit}{Digit})+
KSpread_Int_Literal1    [1-9][0-9]*

/*--------------------------------------------------------------------------*/

KScript_Identifier      [_a-zA-Z][a-zA-Z0-9_]*

/*--------------------------------------------------------------------------*/


%%

[ \t]                   ;
[\n]                    { idl_line_no++; }
"//"[^\n]*              ;
"#!"[^\n]*              ;
"#"[^\n]*               ;

"=~"" "*"s/"(\\.|[^\\/])*"/"(\\.|[^\\/])*"/"    {
                                                  const char *c = yytext + 2;
                                                  while( isspace( *c ) ) ++c;
                                                  ++c; ++c;
                                                  yylval._str = new QString( c );
                                                  yylval._str->truncate( yylval._str->length() - 1 );
                                                  return T_SUBST;
                                                }
"=~"" "*"/"(\\.|[^\\/])*"/"                     {
                                                  const char *c = yytext + 2;
                                                  while( isspace( *c ) ) ++c;
                                                  ++c;
                                                  yylval._str = new QString( c );
                                                  yylval._str->truncate( yylval._str->length() - 1 );
                                                  return T_MATCH;
                                                }
"/"(\\.|[^\\/\n])*"/"/" "*[^ A-Za-z0-9_.(]        {
                                                  yylval._str = new QString( yytext + 1 );
                                                  yylval._str->truncate( yylval._str->length() - 1 );
                                                  return T_MATCH_LINE;
                                                }

"$"?[A-Za-z]+"$"?{Digits} {
                          if ( !s_kspread )
                          {
                                yylval.ident = new QString( yytext );
                                return T_IDENTIFIER;
                          }
                          yylval._str = new QString( yytext );
                          return T_CELL;
                       };

{Table_Name}"!""$"?[A-Za-z]+"$"?{Digits} {
                          if ( !s_kspread )
                          {
                                yylval.ident = new QString( yytext );
                                return T_IDENTIFIER;
                          }
                          QString s = QString::fromUtf8( yytext );
                          yylval._str = new QString( s );
                          return T_CELL;
                       };

"$"?[A-Za-z]+"$"?{Digits}":""$"?[A-Za-z]+"$"?{Digits} {
                          if ( !s_kspread )
                          {
                                yylval.ident = new QString( yytext );
                                return T_IDENTIFIER;
                          }
                          yylval._str = new QString( yytext );
                          return T_RANGE;
                       };

{Table_Name}"!""$"?[A-Za-z]+"$"?{Digits}":""$"?[A-Za-z]+"$"?{Digits} {
                          if ( !s_kspread )
                          {
                                yylval.ident = new QString( yytext );
                                return T_IDENTIFIER;
                          }
                          QString s = QString::fromUtf8( yytext );
                          yylval._str = new QString( s );
                          return T_RANGE;
                       };

<KSPREAD>"'"[A-Za-z\x80-\xff]+"'" {
                          if ( !s_kspread )
                          {
                                yylval.ident = new QString( yytext );
                                return T_IDENTIFIER;
                          }
                          QString s = QString::fromUtf8( yytext );
                          yylval._str = new QString( s );
                          return T_RANGE;
                       };

"{"                     return T_LEFT_CURLY_BRACKET;
"}"                     return T_RIGHT_CURLY_BRACKET;
"["                     return T_LEFT_SQUARE_BRACKET;
"]"                     return T_RIGHT_SQUARE_BRACKET;
"("                     return T_LEFT_PARANTHESIS;
")"                     return T_RIGHT_PARANTHESIS;
":"                     return T_COLON;
<PLAIN>","              return T_COMMA;
<PLAIN>";"              return T_SEMICOLON;
<KSPREAD>";"            return T_COMMA;
"=="                    return T_EQUAL;
"!="                    return T_NOTEQUAL;
"!"                     return T_NOT;
"="                     return T_ASSIGN;
"<>"                    return T_INPUT;
">>"                    return T_SHIFTRIGHT;
"<<"                    return T_SHIFTLEFT;
"+"                     return T_PLUS_SIGN;
"-"                     return T_MINUS_SIGN;
"*"                     return T_ASTERISK;
"/"                     return T_SOLIDUS;
"%"                     return T_PERCENT_SIGN;
"~"                     return T_TILDE;
"||"                    return T_OR;
"|"                     return T_VERTICAL_LINE;
"^"                     return T_CIRCUMFLEX;
"&"                     return T_AMPERSAND;
"&&"                    return T_AND;
"<="                    return T_LESS_OR_EQUAL;
">="                    return T_GREATER_OR_EQUAL;
"<"                     return T_LESS_THAN_SIGN;
">"                     return T_GREATER_THAN_SIGN;
"."                     return T_MEMBER;
"+="                    return T_PLUS_ASSIGN;
"-="                    return T_MINUS_ASSIGN;
"$_"                    return T_LINE;
"$"                     return T_DOLLAR;

const                   return T_CONST;
FALSE                   return T_FALSE;
TRUE                    return T_TRUE;
false                   return T_FALSE;
true                    return T_TRUE;
struct                  return T_STRUCT;
switch                  return T_SWITCH;
case                    return T_CASE;
default                 return T_DEFAULT;
enum                    return T_ENUM;
in                      return T_IN;
out                     return T_OUT;
while                   return T_WHILE;
do                      return T_DO;
for                     return T_FOR;
if                      {
                                if ( !s_kspread )
                                        return T_IF;
                                yylval.ident = new QString( yytext );
                                return T_IDENTIFIER;
                        }
else                    return T_ELSE;
main                    return T_MAIN;
foreach                 return T_FOREACH;
return                  return T_RETURN;
import                  return T_IMPORT;
var                     return T_VAR;
inout                   return T_INOUT;
try                     return T_TRY;
catch                   return T_CATCH;
raise                   return T_RAISE;
from                    return T_FROM;

"++"                    return T_INCR;
"--"                    return T_DECR;
"::"                    return T_SCOPE;

{KScript_Identifier}    {
                          yylval.ident = new QString( yytext );
                          return T_IDENTIFIER;
                        }

<KSPREAD>{KSpread_Int_Or_Float_Literal2} |
<KSPREAD>{KSpread_Float_Literal1}        |
<KSPREAD>{KSpread_Float_Literal2}        |
<KSPREAD>{KSpread_Float_Literal3}        |
<KSPREAD>{KSpread_Float_Literal4}        |
<KSPREAD>{KSpread_Float_Literal5}        {
                          QString s( yytext );
                          bool ok = TRUE;
                          double f = s_koscript_locale->readNumber( s, &ok );
                          // if ( !ok )
                          //    f = s.toDouble( &ok );
                          if ( !ok )
                                return T_UNKNOWN;
                          int i = (int)f;
                          if ( i == f )
                          {
                              yylval._int = i;
                              return T_INTEGER_LITERAL;
                          }
                          yylval._float = f;
                          return T_FLOATING_PT_LITERAL;
                        }
<KSPREAD>{KSpread_Int_Literal1} {
                          yylval._int = ascii_to_longlong( 10, yytext );
                          return T_INTEGER_LITERAL;
                        }

<PLAIN>{Plain_Float_Literal1} |
<PLAIN>{Plain_Float_Literal2} |
<PLAIN>{Plain_Float_Literal3} |
<PLAIN>{Plain_Float_Literal4} |
<PLAIN>{Plain_Float_Literal5} {
                          yylval._float = ascii_to_longdouble( yytext );
                          return T_FLOATING_PT_LITERAL;
                        }
<PLAIN>{Plain_Int_Literal} {
                          yylval._int = ascii_to_longlong( 10, yytext );
                          return T_INTEGER_LITERAL;
                        }

{Oct_Literal}           {
                          yylval._int = ascii_to_longlong( 8, yytext );
                          return T_INTEGER_LITERAL;
                        }
{Hex_Literal}           {
                          yylval._int = ascii_to_longlong( 16, yytext + 2 );
                          return T_INTEGER_LITERAL;
                        }
{Char_Literal}          {
                          QCString s( yytext );
                          s = s.mid( 1, s.length() - 2 );
                          yylval._char = translate_char( s );
                          return T_CHARACTER_LITERAL;
                        }
{String_Literal}        {
                          QString s=QString::fromUtf8( yytext );
                          yylval._str = new QString( s.mid( 1, s.length() - 2 ) );
                          if ( yylval._str->isNull() )
                                *(yylval._str) = "";
                          translate_string( *(yylval._str) );
                          return T_STRING_LITERAL;
                        }

.                       return T_UNKNOWN;

%%

void kscriptInitFlex( const char *_code, int extension, KLocale* locale )
{
   s_koscript_locale = locale;
   if ( !s_koscript_locale )
   {
        if ( !s_defaultLocale )
                s_defaultLocale = new KSLocale;
       s_koscript_locale = s_defaultLocale;
   }
   if ( extension == KSCRIPT_EXTENSION_KSPREAD )
        s_kspread = TRUE;
   else
        s_kspread = FALSE;
   yy_switch_to_buffer( yy_scan_string( _code ) );
}

void kscriptInitFlex( int extension, KLocale* locale )
{
   s_koscript_locale = locale;
   if ( !s_koscript_locale )
   {
        if ( !s_defaultLocale )
                s_defaultLocale = new KSLocale;
       s_koscript_locale = s_defaultLocale;
   }
   if ( extension == KSCRIPT_EXTENSION_KSPREAD )
        s_kspread = TRUE;
   else
        s_kspread = FALSE;
}

void kspread_mode()
{
    BEGIN( KSPREAD );
}
