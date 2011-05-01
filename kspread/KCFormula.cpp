/* This file is part of the KDE project
   Copyright (C) 2008-2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KCFormula.h"

#include "KCCalculationSettings.h"
#include "KCCell.h"
#include "KCCellStorage.h"
#include "KCFunction.h"
#include "KCFunctionRepository.h"
#include "KCSheet.h"
#include "KCMap.h"
#include "KCNamedAreaManager.h"
#include "KCRegion.h"
#include "KCValue.h"

#include "KCValueCalc.h"
#include "KCValueConverter.h"
#include "ValueParser.h"

#include <limits.h>

#include <QRegExp>
#include <QStack>
#include <QString>
#include <QTextStream>

#include <klocale.h>

#define KSPREAD_UNICODE_OPERATORS

/*
  To understand how this formula engine works, please refer to the documentation
  in file DESIGN.html.

  Useful references:
   - "Principles of Compiler Design", A.V.Aho, J.D.Ullman, Addison Wesley, 1978
   - "Writing Interactive Compilers and Interpreters", P.J. Brown,
     John Wiley and Sons, 1979.
   - "The Theory and Practice of Compiler Writing", J.Tremblay, P.G.Sorenson,
     McGraw-Hill, 1985.
   - "The Java(TM) Virtual Machine Specification", T.Lindholm, F.Yellin,
     Addison-Wesley, 1997.
   - "Java Virtual Machine", J.Meyer, T.Downing, O'Reilly, 1997.

 */


/*
TODO - features:
- handle Intersection
- cell reference is made relative (absolute now)
- shared formula (different owner, same data)
- relative internal representation (independent of owner)
- OASIS support
TODO - optimizations:
- handle initial formula marker = (and +)
- reuse constant already in the pool
- reuse references already in the pool
- expression optimization (e.g. 1+2+A1 becomes 3+A1)
*/

class Opcode
{
public:

    enum { Nop = 0, Load, Ref, KCCell, Range, KCFunction, Add, Sub, Neg, Mul, Div,
           Pow, Concat, Intersect, Not, Equal, Less, Greater, Array, Union
         };

    unsigned type;
    unsigned index;

    Opcode(): type(Nop), index(0) {}
    Opcode(unsigned t): type(t), index(0) {}
    Opcode(unsigned t, unsigned i): type(t), index(i) {}
};

// used when evaluation formulas
struct stackEntry {
    void reset() {
        row1 = col1 = row2 = col2 = -1;
        reg = KCRegion();
        regIsNamedOrLabeled = false;
    }
    KCValue val;
    KCRegion reg;
    bool regIsNamedOrLabeled;
    int row1, col1, row2, col2;
};

class KCFormula::Private : public QSharedData
{
public:
    KCCell cell;
    KCSheet *sheet;
    mutable bool dirty;
    mutable bool valid;
    QString expression;
    mutable QVector<Opcode> codes;
    mutable QVector<KCValue> constants;

    KCValue valueOrElement(FuncExtra &fe, const stackEntry& entry) const;
};

class TokenStack : public QVector<KCToken>
{
public:
    TokenStack();
    bool isEmpty() const;
    unsigned itemCount() const;
    void push(const KCToken& token);
    KCToken pop();
    const KCToken& top();
    const KCToken& top(unsigned index);
private:
    void ensureSpace();
    unsigned topIndex;
};

// for null token
const KCToken KCToken::null;

// static
// helper function: return operator of given token text
// e.g. '*' yields Operator::Asterisk, and so on
KCToken::Op KCToken::matchOperator(const QString& text)
{
    KCToken::Op result = KCToken::InvalidOp;

    if (text.length() == 1) {
        QChar p = text[0];
        switch (p.unicode()) {
        case '+': result = KCToken::Plus; break;
        case '-': result = KCToken::Minus; break;
        case '*': result = KCToken::Asterisk; break;
        case '/': result = KCToken::Slash; break;
        case '^': result = KCToken::Caret; break;
        case ',': result = KCToken::Comma; break;
        case ';': result = KCToken::Semicolon; break;
        case ' ': result = KCToken::Intersect; break;
        case '(': result = KCToken::LeftPar; break;
        case ')': result = KCToken::RightPar; break;
        case '&': result = KCToken::Ampersand; break;
        case '=': result = KCToken::Equal; break;
        case '<': result = KCToken::Less; break;
        case '>': result = KCToken::Greater; break;
        case '%': result = KCToken::Percent; break;
        case '~': result = KCToken::Union; break;
#ifdef KSPREAD_INLINE_ARRAYS
        case '{': result = KCToken::CurlyBra; break;
        case '}': result = KCToken::CurlyKet; break;
        case '|': result = KCToken::Pipe; break;
#endif
#ifdef KSPREAD_UNICODE_OPERATORS
        case 0x2212: result = KCToken::Minus; break;
        case 0x00D7: result = KCToken::Asterisk; break;
        case 0x00F7: result = KCToken::Slash; break;
        case 0x2215: result = KCToken::Slash; break;
#endif
        default : result = KCToken::InvalidOp; break;
        }
    }

    if (text.length() == 2) {
        if (text == "<>") result = KCToken::NotEqual;
        if (text == "!=") result = KCToken::NotEqual;
        if (text == "<=") result = KCToken::LessEqual;
        if (text == ">=") result = KCToken::GreaterEqual;
        if (text == "==") result = KCToken::Equal;
    }

    return result;
}

// helper function: give operator precedence
// e.g. '+' is 1 while '*' is 3
static int opPrecedence(KCToken::Op op)
{
    int prec = -1;
    switch (op) {
    case KCToken::Percent      : prec = 8; break;
    case KCToken::Caret        : prec = 7; break;
    case KCToken::Asterisk     : prec = 5; break;
    case KCToken::Slash        : prec = 6; break;
    case KCToken::Plus         : prec = 3; break;
    case KCToken::Minus        : prec = 3; break;
    case KCToken::Union        : prec = 2; break;
    case KCToken::Ampersand    : prec = 2; break;
    case KCToken::Intersect    : prec = 2; break;
    case KCToken::Equal        : prec = 1; break;
    case KCToken::NotEqual     : prec = 1; break;
    case KCToken::Less         : prec = 1; break;
    case KCToken::Greater      : prec = 1; break;
    case KCToken::LessEqual    : prec = 1; break;
    case KCToken::GreaterEqual : prec = 1; break;
#ifdef KSPREAD_INLINE_ARRAYS
        // FIXME Stefan: I don't know whether zero is right for this case. :-(
    case KCToken::CurlyBra     : prec = 0; break;
    case KCToken::CurlyKet     : prec = 0; break;
    case KCToken::Pipe         : prec = 0; break;
#endif
    case KCToken::Semicolon    : prec = 0; break;
    case KCToken::RightPar     : prec = 0; break;
    case KCToken::LeftPar      : prec = -1; break;
    default: prec = -1; break;
    }
    return prec;
}

// helper function
static KCValue tokenAsValue(const KCToken& token)
{
    KCValue value;
    if (token.isBoolean()) value = KCValue(token.asBoolean());
    else if (token.isInteger()) value = KCValue(token.asInteger());
    else if (token.isFloat()) value = KCValue(token.asFloat());
    else if (token.isString()) value = KCValue(token.asString());
    else if (token.isError()) {
        const QString error = token.asError();
        if (error == KCValue::errorCIRCLE().errorMessage())
            value = KCValue::errorCIRCLE();
        else if (error == KCValue::errorDEPEND().errorMessage())
            value = KCValue::errorDEPEND();
        else if (error == KCValue::errorDIV0().errorMessage())
            value = KCValue::errorDIV0();
        else if (error == KCValue::errorNA().errorMessage())
            value = KCValue::errorNA();
        else if (error == KCValue::errorNAME().errorMessage())
            value = KCValue::errorNAME();
        else if (error == KCValue::errorNUM().errorMessage())
            value = KCValue::errorNUM();
        else if (error == KCValue::errorNULL().errorMessage())
            value = KCValue::errorNULL();
        else if (error == KCValue::errorPARSE().errorMessage())
            value = KCValue::errorPARSE();
        else if (error == KCValue::errorREF().errorMessage())
            value = KCValue::errorREF();
        else if (error == KCValue::errorVALUE().errorMessage())
            value = KCValue::errorVALUE();
        else {
            value = KCValue(KCValue::Error);
            value.setError(error);
        }
    }
    return value;
}

/**********************
    KCToken
 **********************/

// creates a token
KCToken::KCToken(Type type, const QString& text, int pos)
{
    m_type = type;
    m_text = text;
    m_pos = pos;
}

// copy constructor
KCToken::KCToken(const KCToken& token)
{
    m_type = token.m_type;
    m_text = token.m_text;
    m_pos = token.m_pos;
}

// assignment operator
KCToken& KCToken::operator=(const KCToken & token)
{
    m_type = token.m_type;
    m_text = token.m_text;
    m_pos = token.m_pos;
    return *this;
}

bool KCToken::asBoolean() const
{
    if (!isBoolean()) return false;
    return m_text.toLower() == "true";
    // FIXME check also for i18n version
}

qint64 KCToken::asInteger() const
{
    if (isInteger()) return m_text.toLongLong();
    else return 0;
}

double KCToken::asFloat() const
{
    if (isFloat()) return m_text.toDouble();
    else return 0.0;
}

QString KCToken::asString() const
{
    if (isString()) return m_text.mid(1, m_text.length() - 2);
    else return QString();
}

QString KCToken::asError() const
{
    if (isError())
        return m_text;
    else
        return QString();
}

KCToken::Op KCToken::asOperator() const
{
    if (isOperator()) return KCToken::matchOperator(m_text);
    else return InvalidOp;
}

QString KCToken::sheetName() const
{
    if (!isCell() && !isRange()) return QString();
    int i = m_text.indexOf('!');
    if (i < 0) return QString();
    QString sheet = m_text.left(i);
    return sheet;
}

QString KCToken::description() const
{
    QString desc;

    switch (m_type) {
    case  Boolean:    desc = "Boolean"; break;
    case  Integer:    desc = "Integer"; break;
    case  Float:      desc = "Float"; break;
    case  String:     desc = "String"; break;
    case  Identifier: desc = "Identifier"; break;
    case  KCCell:       desc = "KCCell"; break;
    case  Range:      desc = "Range"; break;
    case  Operator:   desc = "Operator"; break;
    case  Error:      desc = "Error"; break;
    default:          desc = "Unknown"; break;
    }

    while (desc.length() < 10) desc.prepend(' ');
    desc.prepend("  ");
    desc.prepend(QString::number(m_pos));
    desc.append(" : ").append(m_text);

    return desc;
}


/**********************
    TokenStack
 **********************/

TokenStack::TokenStack(): QVector<KCToken>()
{
    topIndex = 0;
    ensureSpace();
}

bool TokenStack::isEmpty() const
{
    return topIndex == 0;
}

unsigned TokenStack::itemCount() const
{
    return topIndex;
}

void TokenStack::push(const KCToken& token)
{
    ensureSpace();
    insert(topIndex++, token);
}

KCToken TokenStack::pop()
{
    return (topIndex > 0) ? KCToken(at(--topIndex)) : KCToken();
}

const KCToken& TokenStack::top()
{
    return top(0);
}

const KCToken& TokenStack::top(unsigned index)
{
    if (topIndex > index)
        return at(topIndex - index - 1);
    return KCToken::null;
}

void TokenStack::ensureSpace()
{
    while ((int) topIndex >= size())
        resize(size() + 10);
}

/**********************
    FormulaPrivate
 **********************/

// helper function: return true for valid identifier character
bool KSpread::isIdentifier(const QChar &ch)
{
    return (ch.unicode() == '_') || (ch.unicode() == '$') || (ch.unicode() == '.') || (ch.isLetter());
}




/**********************
    KCFormula
 **********************/

// Constructor

KCFormula::KCFormula(KCSheet *sheet, const KCCell& cell)
        : d(new Private)
{
    d->cell = cell;
    d->sheet = sheet;
    clear();
}

KCFormula::KCFormula(KCSheet *sheet)
        : d(new Private)
{
    d->cell = KCCell();
    d->sheet = sheet;
    clear();
}

KCFormula::KCFormula()
        : d(new Private)
{
    d->cell = KCCell();
    d->sheet = 0;
    clear();
}

KCFormula KCFormula::empty()
{
    static KCFormula f;
    return f;
}

KCFormula::KCFormula(const KCFormula& other)
        : d(other.d)
{
}

// Destructor

KCFormula::~KCFormula()
{
}

const KCCell& KCFormula::cell() const
{
    return d->cell;
}

KCSheet* KCFormula::sheet() const
{
    return d->sheet;
}

// Sets a new expression for this formula.
// note that both the real lex and parse processes will happen later on
// when needed (i.e. "lazy parse"), for example during formula evaluation.

void KCFormula::setExpression(const QString& expr)
{
    d->expression = expr;
    d->dirty = true;
    d->valid = false;
}

// Returns the expression associated with this formula.

QString KCFormula::expression() const
{
    return d->expression;
}

// Returns the validity of the formula.
// note: empty formula is always invalid.

bool KCFormula::isValid() const
{
    if (d->dirty) {
        KLocale* locale = !d->cell.isNull() ? d->cell.locale() : 0;
        if ((!locale) && d->sheet)
            locale = d->sheet->map()->calculationSettings()->locale();
        Tokens tokens = scan(d->expression, locale);
        if (tokens.valid())
            compile(tokens);
        else
            d->valid = false;
    }
    return d->valid;
}

// Clears everything, also mark the formula as invalid.

void KCFormula::clear()
{
    d->expression.clear();
    d->dirty = true;
    d->valid = false;
    d->constants.clear();
    d->codes.clear();
}

// Returns list of token for the expression.
// this triggers again the lexical analysis step. it is however preferable
// (even when there's small performance penalty) because otherwise we need to
// store parsed tokens all the time which serves no good purpose.

Tokens KCFormula::tokens() const
{
    KLocale* locale = !d->cell.isNull() ? d->cell.locale() : 0;
    if ((!locale) && d->sheet)
        locale = d->sheet->map()->calculationSettings()->locale();
    return scan(d->expression, locale);
}

Tokens KCFormula::scan(const QString& expr, const KLocale* locale) const
{
    // to hold the result
    Tokens tokens;

    // parsing state
    enum { Start, Finish, InNumber, InDecimal, InExpIndicator, InExponent,
           InString, InIdentifier, InCell, InRange, InSheetOrAreaName, InError
         } state;

    // use locale settings if specified
    QString thousand = locale ? locale->thousandsSeparator() : "";
    QString decimal = locale ? locale->decimalSymbol() : ".";

    // initialize variables
    state = Start;
    bool parseError = false;
    int i = 0;
    QString ex = expr;
    QString tokenText;
    int tokenStart = 0;

    // first character must be equal sign (=)
    if (ex[0] != '=')
        return tokens;

    // but the scanner should not see this equal sign
    ex.remove(0, 1);

    // force a terminator
    ex.append(QChar());

    // main loop
    while ((state != Finish) && (i < ex.length())) {
        QChar ch = ex[i];

        switch (state) {

        case Start:

            tokenStart = i;

            // Whitespaces can be used as intersect-operator for two arrays.
            if (ch.isSpace()) { //TODO && (tokens.isEmpty() || !tokens.last().isRange())) {
                i++;
                break;
            }

            // check for number
            if (ch.isDigit()) {
                state = InNumber;
            }

            // a string ?
            else if (ch == '"') {
                tokenText.append(ex[i++]);
                state = InString;
            }

            // decimal dot ?
            else if (ch == decimal[0]) {
                tokenText.append(ex[i++]);
                state = InDecimal;
            }

            // beginning with alphanumeric ?
            // could be identifier, cell, range, or function...
            else if (KSpread::isIdentifier(ch)) {
                state = InIdentifier;
            }

            // aposthrophe (') marks sheet name for 3-d cell, e.g 'Sales Q3'!A4, or a named range
            else if (ch.unicode() == '\'') {
                i++;
                state = InSheetOrAreaName;
            }

            // error value?
            else if (ch == '#') {
                tokenText.append(ex[i++]);
                state = InError;
            }

            // terminator character
            else if (ch == QChar::Null)
                state = Finish;

            // look for operator match
            else {
                int op;
                QString s;

                // check for two-chars operator, such as '<=', '>=', etc
                s.append(ch).append(ex[i+1]);
                op = KCToken::matchOperator(s);

                // check for one-char operator, such as '+', ';', etc
                if (op == KCToken::InvalidOp) {
                    s = QString(ch);
                    op = KCToken::matchOperator(s);
                }

                // any matched operator ?
                if (op != KCToken::InvalidOp) {
                    int len = s.length();
                    i += len;
                    tokens.append(KCToken(KCToken::Operator, s.left(len), tokenStart));
                } else {
                    // not matched an operator, add an Unknown token and remember whe had a parse error
                    parseError = true;
                    tokens.append(KCToken(KCToken::Unknown, s.left(1), tokenStart));
                    i++;
                }
            }
            break;

        case InIdentifier:

            // consume as long as alpha, dollar sign, underscore, or digit
            if (KSpread::isIdentifier(ch)  || ch.isDigit()) tokenText.append(ex[i++]);

            // a '!' ? then this must be sheet name, e.g "Sheet4!", unless the next character is '='
            else if (ch == '!' && ex[i+1] != '=') {
                tokenText.append(ex[i++]);
                state = InCell;
            }

            // a '(' ? then this must be a function identifier
            else if (ch == '(') {
                tokens.append(KCToken(KCToken::Identifier, tokenText, tokenStart));
                tokenStart = i;
                tokenText.clear();
                state = Start;
            }

            // we're done with identifier
            else {
                // check for cell reference,  e.g A1, VV123, ...
                QRegExp exp("(\\$?)([a-zA-Z]+)(\\$?)([0-9]+)$");
                int n = exp.indexIn(tokenText);
                if (n >= 0)
                    state = InCell;
                else {
                    if (isNamedArea(tokenText))
                        tokens.append(KCToken(KCToken::Range, tokenText, tokenStart));
                    else
                        tokens.append(KCToken(KCToken::Identifier, tokenText, tokenStart));
                    tokenStart = i;
                    tokenText.clear();
                    state = Start;
                }
            }
            break;

        case InCell:

            // consume as long as alpha, dollar sign, underscore, or digit
            if (KSpread::isIdentifier(ch)  || ch.isDigit()) tokenText.append(ex[i++]);

            // we're done with cell ref, possibly with sheet name (like "Sheet2!B2")
            // note that "Sheet2!TotalSales" is also possible, in which "TotalSales" is a named area
            else {

                // check if it's a cell ref like A32, not named area
                QString cell;
                for (int j = tokenText.length() - 1; j >= 0; j--)
                    if (tokenText[j] == '!')
                        break;
                    else
                        cell.prepend(tokenText[j]);
                QRegExp exp("(\\$?)([a-zA-Z]+)(\\$?)([0-9]+)$");
                if (exp.indexIn(cell) != 0) {
                    // regexp failed, means we have something like "Sheet2!TotalSales"
                    // and not "Sheet2!A2"
                    // thus, assume so far that it's a named area
                    tokens.append(KCToken(KCToken::Range, tokenText, tokenStart));
                    tokenText.clear();
                    state = Start;
                }

                else {

                    // so up to now we've got something like A2 or Sheet2!F4
                    // check for range reference
                    if (ch == ':') {
                        tokenText.append(ex[i++]);
                        state = InRange;
                    } else {
                        // we're done with cell reference
                        tokens.append(KCToken(KCToken::KCCell, tokenText, tokenStart));
                        tokenText.clear();
                        state = Start;
                    }
                }
            }
            break;

        case InRange:

            // consume as long as alpha, dollar sign, underscore, or digit or !
            if (KSpread::isIdentifier(ch) || ch.isDigit() || ch == '!') tokenText.append(ex[i++]);

            // we're done with range reference
            else {
                tokens.append(KCToken(KCToken::Range, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            }
            break;

        case InSheetOrAreaName:

            // consume until '
            if (ch.unicode() != '\'') tokenText.append(ex[i++]);

            else {
                // eat the aposthrophe itself
                ++i;
                // must be followed by '!' to be sheet name
                if (ex[i] == '!') {
                    tokenText.append(ex[i++]);
                    state = InCell;
                } else {
                    if (isNamedArea(tokenText))
                        tokens.append(KCToken(KCToken::Range, tokenText, tokenStart));
                    else {
                        // for compatibility with oocalc (and the openformula spec), don't parse single-quoted
                        // text as an identifier, instead add an Unknown token and remember we had an error
                        parseError = true;
                        tokens.append(KCToken(KCToken::Unknown, '\'' + tokenText + '\'', tokenStart));
                    }
                    tokenStart = i;
                    tokenText.clear();
                    state = Start;
                }
            }
            break;

        case InNumber:

            // consume as long as it's digit
            if (ch.isDigit()) tokenText.append(ex[i++]);

            // skip thousand separator
            else if (!thousand.isEmpty() && (ch == thousand[0])) i++;

            // convert decimal separator to '.', also support '.' directly
            // we always support '.' because of bug #98455
            else if ((!decimal.isEmpty() && (ch == decimal[0])) || (ch == '.')) {
                tokenText.append('.');
                i++;
                state = InDecimal;
            }

            // exponent ?
            else if (ch.toUpper() == 'E') {
                tokenText.append('E');
                i++;
                state = InExpIndicator;
            }

            // reference sheet delimiter?
            else if (ch == '!') {
                tokenText.append(ex[i++]);
                state = InCell;
            }

            // identifier?
            else if (KSpread::isIdentifier(ch)) {
                // has to be a sheet or area name then
                state = InIdentifier;
            }

            // we're done with integer number
            else {
                tokens.append(KCToken(KCToken::Integer, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            };
            break;

        case InDecimal:

            // consume as long as it's digit
            if (ch.isDigit()) tokenText.append(ex[i++]);

            // exponent ?
            else if (ch.toUpper() == 'E') {
                tokenText.append('E');
                i++;
                state = InExpIndicator;
            }

            // we're done with floating-point number
            else {
                tokens.append(KCToken(KCToken::Float, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            };
            break;

        case InExpIndicator:

            // possible + or - right after E, e.g 1.23E+12 or 4.67E-8
            if ((ch == '+') || (ch == '-')) tokenText.append(ex[i++]);

            // consume as long as it's digit
            else if (ch.isDigit()) state = InExponent;

            // invalid thing here
            else {
                parseError = true;
                tokenText.append(ex[i++]);
                tokens.append(KCToken(KCToken::Unknown, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            }

            break;

        case InExponent:

            // consume as long as it's digit
            if (ch.isDigit()) tokenText.append(ex[i++]);

            // we're done with floating-point number
            else {
                tokens.append(KCToken(KCToken::Float, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            };
            break;

        case InString:

            // consume until "
            if (ch != '"') tokenText.append(ex[i++]);

            else {
                tokenText.append(ch); i++;
                tokens.append(KCToken(KCToken::String, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            }
            break;

        case InError:

            // consume until !
            if (ch != '!')
                tokenText.append(ex[i++]);
            else {
                tokenText.append(ex[i++]);
                tokens.append(KCToken(KCToken::Error, tokenText, tokenStart));
                tokenText.clear();
                state = Start;
            }
            break;

        default:
            break;
        };
    };

    // parse error if any text remains
    if (state != Finish) {
        tokens.append(KCToken(KCToken::Unknown, ex.mid(tokenStart, ex.length() - tokenStart - 1), tokenStart));
        parseError = true;
    }

    if (parseError)
        tokens.setValid(false);

    return tokens;
}

// will affect: dirty, valid, codes, constants
void KCFormula::compile(const Tokens& tokens) const
{
    // initialize variables
    d->dirty = false;
    d->valid = false;
    d->codes.clear();
    d->constants.clear();

    // sanity check
    if (tokens.count() == 0) return;

    TokenStack syntaxStack;
    QStack<int> argStack;
    unsigned argCount = 1;

    for (int i = 0; i <= tokens.count(); i++) {
        // helper token: InvalidOp is end-of-formula
        KCToken token = (i < tokens.count()) ? tokens[i] : KCToken(KCToken::Operator);
        KCToken::Type tokenType = token.type();

        // unknown token is invalid
        if (tokenType == KCToken::Unknown) break;

        // are we entering a function ?
        // if stack already has: id (
        if (syntaxStack.itemCount() >= 2) {
            KCToken par = syntaxStack.top();
            KCToken id = syntaxStack.top(1);
            if (par.asOperator() == KCToken::LeftPar)
                if (id.isIdentifier()) {
                    argStack.push(argCount);
                    argCount = 1;
                }
        }

#ifdef KSPREAD_INLINE_ARRAYS
        // are we entering an inline array ?
        // if stack already has: {
        if (syntaxStack.itemCount() >= 1) {
            KCToken bra = syntaxStack.top();
            if (bra.asOperator() == KCToken::CurlyBra) {
                argStack.push(argCount);
                argStack.push(1);   // row count
                argCount = 1;
            }
        }
#endif

        // for constants, push immediately to stack
        // generate code to load from a constant
        if ((tokenType == KCToken::Integer) || (tokenType == KCToken::Float) ||
                (tokenType == KCToken::String) || (tokenType == KCToken::Boolean) ||
                (tokenType == KCToken::Error)) {
            syntaxStack.push(token);
            d->constants.append(tokenAsValue(token));
            d->codes.append(Opcode(Opcode::Load, d->constants.count() - 1));
        }

        // for cell, range, or identifier, push immediately to stack
        // generate code to load from reference
        if ((tokenType == KCToken::KCCell) || (tokenType == KCToken::Range) ||
                (tokenType == KCToken::Identifier)) {
            syntaxStack.push(token);
            d->constants.append(KCValue(token.text()));
            if (tokenType == KCToken::KCCell)
                d->codes.append(Opcode(Opcode::KCCell, d->constants.count() - 1));
            else if (tokenType == KCToken::Range)
                d->codes.append(Opcode(Opcode::Range, d->constants.count() - 1));
            else
                d->codes.append(Opcode(Opcode::Ref, d->constants.count() - 1));
        }

        // special case for percentage
        if (tokenType == KCToken::Operator)
            if (token.asOperator() == KCToken::Percent)
                if (syntaxStack.itemCount() >= 1)
                    if (!syntaxStack.top().isOperator()) {
                        d->constants.append(KCValue(0.01));
                        d->codes.append(Opcode(Opcode::Load, d->constants.count() - 1));
                        d->codes.append(Opcode(Opcode::Mul));
                    }

        // for any other operator, try to apply all parsing rules
        if (tokenType == KCToken::Operator)
            if (token.asOperator() != KCToken::Percent) {
                // repeat until no more rule applies
                for (; ;) {
                    bool ruleFound = false;

                    // rule for function arguments, if token is ; or )
                    // id ( arg1 ; arg2 -> id ( arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 5)
                            if ((token.asOperator() == KCToken::RightPar) ||
                                    (token.asOperator() == KCToken::Semicolon)) {
                                KCToken arg2 = syntaxStack.top();
                                KCToken sep = syntaxStack.top(1);
                                KCToken arg1 = syntaxStack.top(2);
                                KCToken par = syntaxStack.top(3);
                                KCToken id = syntaxStack.top(4);
                                if (!arg2.isOperator())
                                    if (sep.asOperator() == KCToken::Semicolon)
                                        if (!arg1.isOperator())
                                            if (par.asOperator() == KCToken::LeftPar)
                                                if (id.isIdentifier()) {
                                                    ruleFound = true;
                                                    syntaxStack.pop();
                                                    syntaxStack.pop();
                                                    argCount++;
                                                }
                            }

                    // rule for empty function arguments, if token is ; or )
                    // id ( arg ; -> id ( arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3)
                            if ((token.asOperator() == KCToken::RightPar) ||
                                    (token.asOperator() == KCToken::Semicolon)) {
                                KCToken sep = syntaxStack.top();
                                KCToken arg = syntaxStack.top(1);
                                KCToken par = syntaxStack.top(2);
                                KCToken id = syntaxStack.top(3);
                                if (sep.asOperator() == KCToken::Semicolon)
                                    if (!arg.isOperator())
                                        if (par.asOperator() == KCToken::LeftPar)
                                            if (id.isIdentifier()) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                d->constants.append(KCValue::null());
                                                d->codes.append(Opcode(Opcode::Load, d->constants.count() - 1));
                                                argCount++;
                                            }
                            }

                    // rule for function last argument:
                    //  id ( arg ) -> arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 4) {
                            KCToken par2 = syntaxStack.top();
                            KCToken arg = syntaxStack.top(1);
                            KCToken par1 = syntaxStack.top(2);
                            KCToken id = syntaxStack.top(3);
                            if (par2.asOperator() == KCToken::RightPar)
                                if (!arg.isOperator())
                                    if (par1.asOperator() == KCToken::LeftPar)
                                        if (id.isIdentifier()) {
                                            ruleFound = true;
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.push(arg);
                                            d->codes.append(Opcode(Opcode::KCFunction, argCount));
                                            Q_ASSERT(!argStack.empty());
                                            argCount = argStack.empty() ? 0 : argStack.pop();
                                        }
                        }

                    // rule for function call with parentheses, but without argument
                    // e.g. "2*PI()"
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken par2 = syntaxStack.top();
                            KCToken par1 = syntaxStack.top(1);
                            KCToken id = syntaxStack.top(2);
                            if (par2.asOperator() == KCToken::RightPar)
                                if (par1.asOperator() == KCToken::LeftPar)
                                    if (id.isIdentifier()) {
                                        ruleFound = true;
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.push(KCToken(KCToken::Integer));
                                        d->codes.append(Opcode(Opcode::KCFunction, 0));
                                        Q_ASSERT(!argStack.empty());
                                        argCount = argStack.empty() ? 0 : argStack.pop();
                                    }
                        }

#ifdef KSPREAD_INLINE_ARRAYS
                    // rule for inline array elements, if token is ; or | or }
                    // { arg1 ; arg2 -> { arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 4)
                            if ((token.asOperator() == KCToken::Semicolon) ||
                                    (token.asOperator() == KCToken::CurlyKet) ||
                                    (token.asOperator() == KCToken::Pipe)) {
                                KCToken arg2 = syntaxStack.top();
                                KCToken sep = syntaxStack.top(1);
                                KCToken arg1 = syntaxStack.top(2);
                                KCToken bra = syntaxStack.top(3);
                                if (!arg2.isOperator())
                                    if (sep.asOperator() == KCToken::Semicolon)
                                        if (!arg1.isOperator())
                                            if (bra.asOperator() == KCToken::CurlyBra) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                argCount++;
                                            }
                            }

                    // rule for last array row element, if token is ; or | or }
                    //  { arg1 | arg2 -> { arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 4)
                            if ((token.asOperator() == KCToken::Semicolon) ||
                                    (token.asOperator() == KCToken::CurlyKet) ||
                                    (token.asOperator() == KCToken::Pipe)) {
                                KCToken arg2 = syntaxStack.top();
                                KCToken sep = syntaxStack.top(1);
                                KCToken arg1 = syntaxStack.top(2);
                                KCToken bra = syntaxStack.top(3);
                                if (!arg2.isOperator())
                                    if (sep.asOperator() == KCToken::Pipe)
                                        if (!arg1.isOperator())
                                            if (bra.asOperator() == KCToken::CurlyBra) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                int rowCount = argStack.pop();
                                                argStack.push(++rowCount);
                                                argCount = 1;
                                            }
                            }

                    // rule for last array element:
                    //  { arg } -> arg
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken ket = syntaxStack.top();
                            KCToken arg = syntaxStack.top(1);
                            KCToken bra = syntaxStack.top(2);
                            if (ket.asOperator() == KCToken::CurlyKet)
                                if (!arg.isOperator())
                                    if (bra.asOperator() == KCToken::CurlyBra) {
                                        ruleFound = true;
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.pop();
                                        syntaxStack.push(arg);
                                        const int rowCount = argStack.pop();
                                        d->constants.append(KCValue((int)argCount));     // cols
                                        d->constants.append(KCValue(rowCount));
                                        d->codes.append(Opcode(Opcode::Array, d->constants.count() - 2));
                                        Q_ASSERT(!argStack.empty());
                                        argCount = argStack.empty() ? 0 : argStack.pop();
                                    }
                        }
#endif
                    // rule for parenthesis:  ( Y ) -> Y
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken right = syntaxStack.top();
                            KCToken y = syntaxStack.top(1);
                            KCToken left = syntaxStack.top(2);
                            if (right.isOperator())
                                if (!y.isOperator())
                                    if (left.isOperator())
                                        if (right.asOperator() == KCToken::RightPar)
                                            if (left.asOperator() == KCToken::LeftPar) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.push(y);
                                            }
                        }

                    // rule for binary operator:  A (op) B -> A
                    // conditions: precedence of op >= precedence of token
                    // action: push (op) to result
                    // e.g. "A * B" becomes 'A' if token is operator '+'
                    if (!ruleFound)
                        if (syntaxStack.itemCount() >= 3) {
                            KCToken b = syntaxStack.top();
                            KCToken op = syntaxStack.top(1);
                            KCToken a = syntaxStack.top(2);
                            if (!a.isOperator())
                                if (!b.isOperator())
                                    if (op.isOperator())
                                        if (token.asOperator() != KCToken::LeftPar)
                                            if (opPrecedence(op.asOperator()) >= opPrecedence(token.asOperator())) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.push(b);
                                                switch (op.asOperator()) {
                                                    // simple binary operations
                                                case KCToken::Plus:         d->codes.append(Opcode::Add); break;
                                                case KCToken::Minus:        d->codes.append(Opcode::Sub); break;
                                                case KCToken::Asterisk:     d->codes.append(Opcode::Mul); break;
                                                case KCToken::Slash:        d->codes.append(Opcode::Div); break;
                                                case KCToken::Caret:        d->codes.append(Opcode::Pow); break;
                                                case KCToken::Ampersand:    d->codes.append(Opcode::Concat); break;
                                                case KCToken::Intersect:    d->codes.append(Opcode::Intersect); break;
                                                case KCToken::Union:        d->codes.append(Opcode::Union); break;

                                                    // simple value comparisons
                                                case KCToken::Equal:        d->codes.append(Opcode::Equal); break;
                                                case KCToken::Less:         d->codes.append(Opcode::Less); break;
                                                case KCToken::Greater:      d->codes.append(Opcode::Greater); break;

                                                    // NotEqual is Equal, followed by Not
                                                case KCToken::NotEqual:
                                                    d->codes.append(Opcode::Equal);
                                                    d->codes.append(Opcode::Not);
                                                    break;

                                                    // LessOrEqual is Greater, followed by Not
                                                case KCToken::LessEqual:
                                                    d->codes.append(Opcode::Greater);
                                                    d->codes.append(Opcode::Not);
                                                    break;

                                                    // GreaterOrEqual is Less, followed by Not
                                                case KCToken::GreaterEqual:
                                                    d->codes.append(Opcode::Less);
                                                    d->codes.append(Opcode::Not);
                                                    break;
                                                default: break;
                                                };
                                            }
                        }

                    // rule for unary operator:  (op1) (op2) X -> (op1) X
                    // conditions: op2 is unary, token is not '('
                    // action: push (op2) to result
                    // e.g.  "* - 2" becomes '*'
                    if (!ruleFound)
                        if (token.asOperator() != KCToken::LeftPar)
                            if (syntaxStack.itemCount() >= 3) {
                                KCToken x = syntaxStack.top();
                                KCToken op2 = syntaxStack.top(1);
                                KCToken op1 = syntaxStack.top(2);
                                if (!x.isOperator())
                                    if (op1.isOperator())
                                        if (op2.isOperator())
                                            if ((op2.asOperator() == KCToken::Plus) ||
                                                    (op2.asOperator() == KCToken::Minus)) {
                                                ruleFound = true;
                                                syntaxStack.pop();
                                                syntaxStack.pop();
                                                syntaxStack.push(x);
                                                if (op2.asOperator() == KCToken::Minus)
                                                    d->codes.append(Opcode(Opcode::Neg));
                                            }
                            }

                    // auxiliary rule for unary operator:  (op) X -> X
                    // conditions: op is unary, op is first in syntax stack, token is not '('
                    // action: push (op) to result
                    if (!ruleFound)
                        if (token.asOperator() != KCToken::LeftPar)
                            if (syntaxStack.itemCount() == 2) {
                                KCToken x = syntaxStack.top();
                                KCToken op = syntaxStack.top(1);
                                if (!x.isOperator())
                                    if (op.isOperator())
                                        if ((op.asOperator() == KCToken::Plus) ||
                                                (op.asOperator() == KCToken::Minus)) {
                                            ruleFound = true;
                                            syntaxStack.pop();
                                            syntaxStack.pop();
                                            syntaxStack.push(x);
                                            if (op.asOperator() == KCToken::Minus)
                                                d->codes.append(Opcode(Opcode::Neg));
                                        }
                            }

                    if (!ruleFound) break;
                }

                // can't apply rules anymore, push the token
                if (token.asOperator() != KCToken::Percent)
                    syntaxStack.push(token);
            }
    }

    // syntaxStack must left only one operand and end-of-formula (i.e. InvalidOp)
    d->valid = false;
    if (syntaxStack.itemCount() == 2)
        if (syntaxStack.top().isOperator())
            if (syntaxStack.top().asOperator() == KCToken::InvalidOp)
                if (!syntaxStack.top(1).isOperator())
                    d->valid = true;

    // bad parsing ? clean-up everything
    if (!d->valid) {
        d->constants.clear();
        d->codes.clear();
    }
}

bool KCFormula::isNamedArea(const QString& expr) const
{
    return d->sheet ? d->sheet->map()->namedAreaManager()->contains(expr) : false;
}


// Evaluates the formula, returns the result.

// evaluate the cellIndirections
KCValue KCFormula::eval(CellIndirection cellIndirections) const
{
    QHash<KCCell, KCValue> values;
    return evalRecursive(cellIndirections, values);
}

// We need to unroll arrays. Do use the same logic to unroll like OpenOffice.org and Excel are using.
KCValue KCFormula::Private::valueOrElement(FuncExtra &fe, const stackEntry& entry) const
{
    const KCValue& v = entry.val;
    const KCRegion& region = entry.reg;
    if(v.isArray()) {
        if(v.count() == 1) // if there is only one item, use that one
            return v.element(0);

        if(region.isValid() && entry.regIsNamedOrLabeled) {
            const QPoint position = region.firstRange().topLeft();
            const int idx = fe.myrow - position.y(); // do we need to do the same for columns?
            if(idx >= 0 && idx < int(v.count()))
                return v.element(idx); // within the range returns the selected element
        }
    }
    return v;
}

// On OO.org Calc and MS Excel operations done with +, -, * and / do fail if one of the values is
// non-numeric. This differs from formulas like SUM which just ignores non numeric values.
KCValue numericOrError(const KCValueConverter* converter, const KCValue &v)
{
    switch (v.type()) {
    case KCValue::Empty:
    case KCValue::Boolean:
    case KCValue::Integer:
    case KCValue::Float:
    case KCValue::Complex:
    case KCValue::Error:
        return v;
    case KCValue::String: {
        if (v.asString().isEmpty())
            return v;
        bool ok;
        converter->asNumeric(v, &ok);
        if (ok)
            return v;
    } break;
    case KCValue::Array:
    case KCValue::CellRange:
        return v;
    }
    return KCValue::errorVALUE();
}

KCValue KCFormula::evalRecursive(CellIndirection cellIndirections, QHash<KCCell, KCValue>& values) const
{
    QStack<stackEntry> stack;
    stackEntry entry;
    int index;
    KCValue val1, val2;
    QString c;
    QVector<KCValue> args;

    const KCMap* map = d->sheet ? d->sheet->map() : new KCMap(0 /*document*/);
    const KCValueConverter* converter = map->converter();
    KCValueCalc* calc = map->calc();

    QSharedPointer<KCFunction> function;
    FuncExtra fe;
    fe.mycol = fe.myrow = 0;
    if (!d->cell.isNull()) {
        fe.mycol = d->cell.column();
        fe.myrow = d->cell.row();
    }

    if (d->dirty) {
        Tokens tokens = scan(d->expression);
        d->valid = tokens.valid();
        if (tokens.valid())
            compile(tokens);
    }

    if (!d->valid)
        return KCValue::errorPARSE();

    for (int pc = 0; pc < d->codes.count(); pc++) {
        KCValue ret;   // for the function caller
        Opcode& opcode = d->codes[pc];
        index = opcode.index;
        switch (opcode.type) {
            // no operation
        case Opcode::Nop:
            break;

            // load a constant, push to stack
        case Opcode::Load:
            entry.reset();
            entry.val = d->constants[index];
            stack.push(entry);
            break;

            // unary operation
        case Opcode::Neg:
            entry.reset();
            entry.val = d->valueOrElement(fe, stack.pop());
            if (!entry.val.isError()) // do nothing if we got an error
                entry.val = calc->mul(entry.val, -1);
            stack.push(entry);
            break;

            // binary operation: take two values from stack, do the operation,
            // push the result to stack
        case Opcode::Add:
            entry.reset();
            val2 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val1 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val2 = calc->add(val1, val2);
            entry.reset();
            entry.val = val2;
            stack.push(entry);
            break;

        case Opcode::Sub:
            val2 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val1 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val2 = calc->sub(val1, val2);
            entry.reset();
            entry.val = val2;
            stack.push(entry);
            break;

        case Opcode::Mul:
            val2 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val1 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val2 = calc->mul(val1, val2);
            entry.reset();
            entry.val = val2;
            stack.push(entry);
            break;

        case Opcode::Div:
            val2 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val1 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val2 = calc->div(val1, val2);
            entry.reset();
            entry.val = val2;
            stack.push(entry);
            break;

        case Opcode::Pow:
            val2 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val1 = numericOrError(converter, d->valueOrElement(fe, stack.pop()));
            val2 = calc->pow(val1, val2);
            entry.reset();
            entry.val = val2;
            stack.push(entry);
            break;

            // string concatenation
        case Opcode::Concat:
            val1 = converter->asString(stack.pop().val);
            val2 = converter->asString(stack.pop().val);
            if (val1.isError() || val2.isError())
                val1 = KCValue::errorVALUE();
            else
                val1 = KCValue(val2.asString().append(val1.asString()));
            entry.reset();
            entry.val = val1;
            stack.push(entry);
            break;

           // array intersection
        case Opcode::Intersect: {
            val1 = stack.pop().val;
            val2 = stack.pop().val;
            KCRegion r1(d->constants[index].asString(), map, d->sheet);
            KCRegion r2(d->constants[index+1].asString(), map, d->sheet);
            if(!r1.isValid() || !r2.isValid()) {
                val1 = KCValue::errorNULL();
            } else {
                KCRegion r = r1.intersected(r2);
                QRect rect = r.boundingRect();
                KCCell cell;
                if(rect.top() == rect.bottom())
                    cell = KCCell(r.firstSheet(), fe.mycol, rect.top());
                else if(rect.left() == rect.right())
                    cell = KCCell(r.firstSheet(), rect.left(), fe.mycol);
                if(cell.isNull())
                    val1 = KCValue::errorNULL();
                else if(cell.isEmpty())
                    val1 = KCValue::errorNULL();
                else
                    val1 = cell.value();
            }
            entry.reset();
            entry.val = val1;
            stack.push(entry);
        } break;

          // region union
        case Opcode::Union: {
            KCRegion r = stack.pop().reg;
            KCRegion r2 = stack.pop().reg;
            entry.reset();
            if (!r.isValid() || !r2.isValid()) {
                val1 = KCValue::errorVALUE();
                r = KCRegion();
            } else {
                r.add(r2);
                r.firstSheet()->cellStorage()->valueRegion(r);
                // store the reference, so we can use it within functions (not entirely correct)
                entry.col1 = r.boundingRect().left();
                entry.row1 = r.boundingRect().top();
                entry.col2 = r.boundingRect().right();
                entry.row2 = r.boundingRect().bottom();
            }
            entry.val = val1;
            entry.reg = r;
            stack.push(entry);
        } break;

            // logical not
        case Opcode::Not:
            val1 = converter->asBoolean(d->valueOrElement(fe, stack.pop()));
            if (val1.isError())
                val1 = KCValue::errorVALUE();
            else
                val1 = KCValue(!val1.asBoolean());
            entry.reset();
            entry.val = val1;
            stack.push(entry);
            break;

            // comparison
        case Opcode::Equal:
            val1 = d->valueOrElement(fe, stack.pop());
            val2 = d->valueOrElement(fe, stack.pop());
            if (val1.isError())
                ;
            else if (val2.isError())
                val1 = val2;
            else if (val2.compare(val1) == 0)
                val1 = KCValue(true);
            else
                val1 = KCValue(false);
            entry.reset();
            entry.val = val1;
            stack.push(entry);
            break;

            // less than
        case Opcode::Less:
            val1 = d->valueOrElement(fe, stack.pop());
            val2 = d->valueOrElement(fe, stack.pop());
            if (val1.isError())
                ;
            else if (val2.isError())
                val1 = val2;
            else if (val2.compare(val1) < 0)
                val1 = KCValue(true);
            else
                val1 = KCValue(false);
            entry.reset();
            entry.val = val1;
            stack.push(entry);
            break;

            // greater than
        case Opcode::Greater: {
            val1 = d->valueOrElement(fe, stack.pop());
            val2 = d->valueOrElement(fe, stack.pop());
            if (val1.isError())
                ;
            else if (val2.isError())
                val1 = val2;
            else if (val2.compare(val1) > 0)
                val1 = KCValue(true);
            else
                val1 = KCValue(false);
            entry.reset();
            entry.val = val1;
            stack.push(entry);
        }
        break;

        // cell in a sheet
        case Opcode::KCCell: {
            c = d->constants[index].asString();
            val1 = KCValue::empty();
            entry.reset();

            const KCRegion region(c, map, d->sheet);
            if (!region.isValid()) {
                val1 = KCValue::errorREF();
            } else if (region.isSingular()) {
                const QPoint position = region.firstRange().topLeft();
                if (cellIndirections.isEmpty())
                    val1 = KCCell(region.firstSheet(), position).value();
                else {
                    KCCell cell(region.firstSheet(), position);
                    cell = cellIndirections.value(cell, cell);
                    if (values.contains(cell))
                        val1 = values.value(cell);
                    else {
                        values[cell] = KCValue::errorCIRCLE();
                        if (cell.isFormula())
                            val1 = cell.formula().evalRecursive(cellIndirections, values);
                        else
                            val1 = cell.value();
                        values[cell] = val1;
                    }
                }
                // store the reference, so we can use it within functions
                entry.col1 = entry.col2 = position.x();
                entry.row1 = entry.row2 = position.y();
                entry.reg = region;
                entry.regIsNamedOrLabeled = map->namedAreaManager()->contains(c);
            } else {
                kWarning() << "Unhandled non singular region in Opcode::KCCell with rects=" << region.rects();
            }
            entry.val = val1;
            stack.push(entry);
        }
        break;

        // selected range in a sheet
        case Opcode::Range: {
            c = d->constants[index].asString();
            val1 = KCValue::empty();
            entry.reset();

            const KCRegion region(c, map, d->sheet);
            if (region.isValid()) {
                val1 = region.firstSheet()->cellStorage()->valueRegion(region);
                // store the reference, so we can use it within functions
                entry.col1 = region.firstRange().left();
                entry.row1 = region.firstRange().top();
                entry.col2 = region.firstRange().right();
                entry.row2 = region.firstRange().bottom();
                entry.reg = region;
                entry.regIsNamedOrLabeled = map->namedAreaManager()->contains(c);
            }

            entry.val = val1; // any array is valid here
            stack.push(entry);
        }
        break;

        // reference
        case Opcode::Ref:
            val1 = d->constants[index];
            entry.reset();
            entry.val = val1;
            stack.push(entry);
            break;

            // calling function
        case Opcode::KCFunction:
            // sanity check, this should not happen unless opcode is wrong
            // (i.e. there's a bug in the compile() function)
            if (stack.count() < index)
                return KCValue::errorVALUE(); // not enough arguments

            args.clear();
            fe.ranges.clear();
            fe.ranges.resize(index);
            fe.regions.clear();
            fe.regions.resize(index);
            fe.sheet = d->sheet;
            for (; index; index--) {
                stackEntry e = stack.pop();
                args.insert(args.begin(), e.val);
                // fill the FunctionExtra object
                fe.ranges[index - 1].col1 = e.col1;
                fe.ranges[index - 1].row1 = e.row1;
                fe.ranges[index - 1].col2 = e.col2;
                fe.ranges[index - 1].row2 = e.row2;
                fe.regions[index - 1] = e.reg;
            }

            // function name as string value
            val1 = converter->asString(stack.pop().val);
            if (val1.isError())
                return val1;
            function = KCFunctionRepository::self()->function(val1.asString());
            if (!function)
                return KCValue::errorNAME(); // no such function

            ret = function->exec(args, calc, &fe);
            entry.reset();
            entry.val = ret;
            stack.push(entry);

            break;

#ifdef KSPREAD_INLINE_ARRAYS
            // creating an array
        case Opcode::Array: {
            const int cols = d->constants[index].asInteger();
            const int rows = d->constants[index+1].asInteger();
            // check if enough array elements are available
            if (stack.count() < cols * rows)
                return KCValue::errorVALUE();
            KCValue array(KCValue::Array);
            for (int row = rows - 1; row >= 0; --row) {
                for (int col = cols - 1; col >= 0; --col) {
                    array.setElement(col, row, stack.pop().val);
                }
            }
            entry.reset();
            entry.val = array;
            stack.push(entry);
            break;
        }
#endif
        default:
            break;
        }
    }

    if (!d->sheet)
        delete map;

    // more than one value in stack ? unsuccessful execution...
    if (stack.count() != 1)
        return KCValue::errorVALUE();

    return stack.pop().val;
}

KCFormula& KCFormula::operator=(const KCFormula & other)
{
    d = other.d;
    return *this;
}

bool KCFormula::operator==(const KCFormula& other) const
{
    return (d->expression == other.d->expression);
}

// Debugging aid

QString KCFormula::dump() const
{
    QString result;

    if (d->dirty) {
        Tokens tokens = scan(d->expression);
        compile(tokens);
    }

    result = QString("Expression: [%1]\n").arg(d->expression);
#if 0
    KCValue value = eval();
    result.append(QString("Result: %1\n").arg(
                      converter->asString(value).asString()));
#endif

    result.append("  Constants:\n");
    for (int c = 0; c < d->constants.count(); c++) {
        QString vtext;
        KCValue val = d->constants[c];
        if (val.isString()) vtext = QString("[%1]").arg(val.asString());
        else if (val.isNumber()) vtext = QString("%1").arg((double) numToDouble(val.asFloat()));
        else if (val.isBoolean()) vtext = QString("%1").arg(val.asBoolean() ? "True" : "False");
        else if (val.isError()) vtext = "error";
        else vtext = "???";
        result += QString("    #%1 = %2\n").arg(c).arg(vtext);
    }

    result.append("\n");
    result.append("  Code:\n");
    for (int i = 0; i < d->codes.count(); i++) {
        QString ctext;
        switch (d->codes[i].type) {
        case Opcode::Load:      ctext = QString("Load #%1").arg(d->codes[i].index); break;
        case Opcode::Ref:       ctext = QString("Ref #%1").arg(d->codes[i].index); break;
        case Opcode::KCFunction:  ctext = QString("KCFunction (%1)").arg(d->codes[i].index); break;
        case Opcode::Add:       ctext = "Add"; break;
        case Opcode::Sub:       ctext = "Sub"; break;
        case Opcode::Mul:       ctext = "Mul"; break;
        case Opcode::Div:       ctext = "Div"; break;
        case Opcode::Neg:       ctext = "Neg"; break;
        case Opcode::Concat:    ctext = "Concat"; break;
        case Opcode::Pow:       ctext = "Pow"; break;
        case Opcode::Intersect: ctext = "Intersect"; break;
        case Opcode::Equal:     ctext = "Equal"; break;
        case Opcode::Not:       ctext = "Not"; break;
        case Opcode::Less:      ctext = "Less"; break;
        case Opcode::Greater:   ctext = "Greater"; break;
        case Opcode::Array:     ctext = QString("Array (%1x%2)").arg(d->constants[d->codes[i].index].asInteger()).arg(d->constants[d->codes[i].index+1].asInteger()); break;
        case Opcode::Nop:       ctext = "Nop"; break;
        case Opcode::KCCell:      ctext = "KCCell"; break;
        case Opcode::Range:     ctext = "Range"; break;
        default: ctext = "Unknown"; break;
        }
        result.append("   ").append(ctext).append("\n");
    }

    return result;
}

QTextStream& operator<<(QTextStream& ts, KCFormula formula)
{
    ts << formula.dump();
    return ts;
}
