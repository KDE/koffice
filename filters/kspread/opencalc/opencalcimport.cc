/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres <nandres@web.de>

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

#include <float.h>
#include <math.h>

#include <qcolor.h>
#include <qfile.h>
#include <qfont.h>
#include <qpen.h>

#include "opencalcimport.h"

#include <kdebug.h>
#include <koDocumentInfo.h>
#include <kgenericfactory.h>
#include <kmessagebox.h>
#include <kmdcodec.h>
#include <koFilterChain.h>
#include <koGlobal.h>
#include <koUnit.h>

#include <kspread_cell.h>
#include <kspread_doc.h>
#include <kspread_global.h>
#include <kspread_map.h>
#include <kspread_sheet.h>
#include <kspread_sheetprint.h>
#include <kspread_style.h>
#include <kspread_style_manager.h>
#include <kspread_util.h>
#include <kspread_value.h>

#define SECSPERDAY (24 * 60 * 60)

class OpenCalcImportFactory : KGenericFactory<OpenCalcImport, KoFilter>
{
public:
    OpenCalcImportFactory(void) : KGenericFactory<OpenCalcImport, KoFilter> ("kspreadopencalcimport")
    {}
protected:
    virtual void setupTranslations( void )
    {
        KGlobal::locale()->insertCatalogue( "kspreadopencalcfilter" );
    }
};

K_EXPORT_COMPONENT_FACTORY( libopencalcimport, OpenCalcImportFactory() )

OpenCalcImport::OpenCalcPoint::OpenCalcPoint( QString const & str )
  : isRange( false )
{
  bool inQuote = false;

  int l = str.length();
  int colonPos = -1;
  QString range;

  // replace '.' with '!'
  for ( int i = 0; i < l; ++i )
  {
    if ( str[i] == '$' )
      continue;
    if ( str[i] == '\'' )
    {
      inQuote = !inQuote;
    }
    else if ( str[i] == '.' )
    {
      if ( !inQuote )
      {
        if ( i != 0 && i != (colonPos + 1) ) // no empty table names
          range += '!';
      }
      else
        range += '.';
    }
    else if ( str[i] == ':' )
    {
      if ( !inQuote )
      {
        isRange  = true;
        colonPos = i;
      }
      range += ':';
    }
    else
      range += str[i];
  }

  translation = range;

  if ( isRange )
  {
    KSpreadRange newRange( range );
    table    = newRange.tableName;
    topLeft  = newRange.range.topLeft();
    botRight = newRange.range.bottomRight();
  }
  else
  {
    KSpreadPoint newPoint( range );
    table    = newPoint.tableName;
    topLeft  = newPoint.pos;
    botRight = newPoint.pos;
  }
}


OpenCalcImport::OpenCalcImport( KoFilter *, const char *, const QStringList & )
  : KoFilter(),
    m_styles( 17, true ),
    m_defaultStyles( 17, true ),
    m_formats( 17, true )
{
  m_styles.setAutoDelete( true );
  m_defaultStyles.setAutoDelete( true );
  m_formats.setAutoDelete( true );
}

OpenCalcImport::~OpenCalcImport()
{
}

double timeToNum( int h, int m, int s )
{
  int secs = h * 3600 + m * 60 + s;
  return (double) secs / (double) SECSPERDAY;
}

bool OpenCalcImport::readRowFormat( QDomElement & rowNode, QDomElement * rowStyle,
                                    KSpreadSheet * table, int & row, int & number,
                                    bool isLast )
{
  if ( rowNode.isNull() )
    return false;

  QDomNode node;
  if ( rowStyle )
  {
    node = rowStyle->firstChild();
    kdDebug(30518) << "RowStyle: " << rowStyle << ", " << rowStyle->tagName() << endl;
  }

  double height = -1.0;
  bool insertPageBreak = false;
  KSpreadFormat layout( table, table->doc()->styleManager()->defaultStyle() );

  while( !node.isNull() )
  {
    QDomElement property = node.toElement();

    kdDebug(30518) << "Row: Child exists: " << property.tagName() << endl;
    if ( !property.isNull() && property.tagName() == "style:properties" )
    {
      if ( property.hasAttribute( "style:row-height" ) )
      {
          height = KoUnit::parseValue( property.attribute( "style:row-height" ) , -1 );
      }

      if ( property.hasAttribute( "fo:break-before" ) )
      {
        if ( property.attribute( "fo:break-before" ) == "page" )
        {
          insertPageBreak = true;
        }
      }

      loadStyleProperties( &layout, property );
    }

    node = node.nextSibling();
  }

  if ( rowNode.hasAttribute( "table:number-rows-repeated" ) )
  {
    bool ok = true;
    int n = rowNode.attribute( "table:number-rows-repeated" ).toInt( &ok );
    if ( ok )
      number = n;
    kdDebug(30518) << "Row repeated: " << number << endl;
  }

  if ( isLast )
  {
    if ( number > 30 )
      number = 30;
  }
  else
  {
    if ( number > 256 )
      number = 256;
  }

  for ( int i = 0; i < number; ++i )
  {
    RowFormat * rowL = table->nonDefaultRowFormat( row );
    rowL->copy( layout );

    if ( height != -1 )
    {
      kdDebug(30518) << "Setting row height to " << height << endl;
      rowL->setHeight( height );
    }

    // if ( insertPageBreak ) TODO:
    //   rowL->setPageBreak( true )

    //    kdDebug(30518) << "Added RowFormat: " << row << endl;
    ++row;
  }

  return true;
}

QString OpenCalcImport::translatePar( QString & par ) const
{
  OpenCalcPoint point( par );
  kdDebug(30518) << "   Parameter: " << par << ", Translation: " << point.translation << endl;

  return point.translation;
}

void OpenCalcImport::checkForNamedAreas( QString & formula ) const
{
  int l = formula.length();
  int i = 0;
  QString word;
  int start = 0;
  while ( i < l )
  {
    if ( formula[i].isLetterOrNumber() )
    {
      word += formula[i];
      ++i;
      continue;
    }
    if ( word.length() > 0 )
    {
      if ( m_namedAreas.find( word ) != m_namedAreas.end() )
      {
        formula = formula.replace( start, word.length(), "'" + word + "'" );
        l = formula.length();
        ++i;
        kdDebug(30518) << "Formula: " << formula << ", L: " << l << ", i: " << i + 1 <<endl;
      }
    }

    ++i;
    word = "";
    start = i;
  }
  if ( word.length() > 0 )
  {
    if ( m_namedAreas.find( word ) != m_namedAreas.end() )
    {
      formula = formula.replace( start, word.length(), "'" + word + "'" );
      l = formula.length();
      ++i;
      kdDebug(30518) << "Formula: " << formula << ", L: " << l << ", i: " << i + 1 <<endl;
    }
  }
}

void OpenCalcImport::convertFormula( QString & text, QString const & f ) const
{
  kdDebug(30518) << "Parsing formula: " << f << endl;

  QString formula;
  QString parameter;

  int l = f.length();
  int p = 0;

  while ( p < l )
  {
    if ( f[p] == '(' )
    {
      break;
    }
    else if ( f[p] == '[' )
      break;

    formula += f[p];
    ++p;
  }

  if ( parameter.isEmpty() )
  {
    checkForNamedAreas( formula );
  }

  kdDebug(30518) << "Formula: " << formula << ", Parameter: " << parameter << ", P: " << p << endl;


  // replace formula names here
  if ( formula == "=MULTIPLE.OPERATIONS" )
    formula = "=MULTIPLEOPERATIONS";

  QString par;
  bool isPar   = false;
  bool inQuote = false;

  while ( p < l )
  {
    if ( f[p] == '"' )
    {
      inQuote = !inQuote;
      parameter += '"';
    }
    else if ( f[p] == '[' )
    {
      if ( !inQuote )
        isPar = true;
      else
        parameter += '[';
    }
    else if ( f[p] == ']' )
    {
      if ( inQuote )
      {
        parameter += ']';
        continue;
      }

      isPar = false;
      parameter += translatePar( par );
      par = "";
    }
    else if ( isPar )
    {
      par += f[p];
    }
    else if ( f[p] == '=' ) // TODO: check if StarCalc has a '==' sometimes
    {
      if ( inQuote )
        parameter += '=';
      else
        parameter += "==";
    }
    else if ( f[p] == ')' )
    {
      if ( !inQuote )
        parameter += ")";
    }
    else
      parameter += f[p];

    ++p;
    if ( p == l )
      checkForNamedAreas( parameter );
  }

  text = formula + parameter;
  kdDebug(30518) << "New formula: " << text << endl;
}

bool OpenCalcImport::readCells( QDomElement & rowNode, KSpreadSheet  * table, int row, int & columns )
{
  bool ok = true;
  int spanC = 1;
  int spanR = 1;
  KSpreadCell * defCell = table->defaultCell();

  QDomNode cellNode = rowNode.namedItem( "table:table-cell" );

  while ( !cellNode.isNull() )
  {
    spanR = 1; spanC = 1;

    QDomElement e = cellNode.toElement();
    if ( e.isNull() )
    {
      ++columns;

      cellNode = cellNode.nextSibling();
      continue;
    }

    KSpreadCell * cell = 0;

    kdDebug(30518) << " Cell: " << columns << ", " << row << endl;

    // ="3" table:number-rows-spanned="1"
    if ( e.hasAttribute( "table:number-columns-spanned" ) )
    {
      int span = e.attribute( "table:number-columns-spanned" ).toInt( &ok );
      if ( ok )
        spanC = span;
    }
    if ( e.hasAttribute( "table:number-rows-spanned" ) )
    {
      int span = e.attribute( "table:number-rows-spanned" ).toInt( &ok );
      if ( ok )
        spanR = span;
    }

    QString text;
    QDomElement textP = e.namedItem( "text:p" ).toElement();
    if ( !textP.isNull() )
    {
      QDomElement subText = textP.firstChild().toElement();
      if ( !subText.isNull() )
      {
        // something in <text:p>, e.g. links
        text = subText.text();

        if ( subText.hasAttribute( "xlink:href" ) )
        {
          QString link = subText.attribute( "xlink:href" );
          text = "!<a href=\"" + link + "\"><i>" + text + "</i></a>";
        }
      }
      else
        text = textP.text(); // our text, could contain formating for value or result of formula
    }

    QDomElement annotation = e.namedItem( "office:annotation" ).toElement();
    if ( !annotation.isNull() )
    {
       QString comment;
        QDomNode node = annotation.firstChild();
        while( !node.isNull() )
        {
            QDomElement commentElement = node.toElement();
            if( !commentElement.isNull() )
                if( commentElement.tagName() == "text:p" )
                {
                    if( !comment.isEmpty() ) comment.append( '\n' );
                    comment.append( commentElement.text() );
                }

            node = node.nextSibling();
        }

        if( !comment.isEmpty() )
        {
            if ( !cell )
                cell = table->nonDefaultCell( columns, row );
            cell->setComment( comment );
        }
    }

    kdDebug(30518) << "Contains: " << text << endl;
    bool isFormula = false;

    if ( e.hasAttribute( "table:style-name" ) )
    {
      if ( !cell )
        cell = table->nonDefaultCell( columns, row );

      QString psName( "Default" );
      if ( e.hasAttribute( "style:parent-style-name" ) )
        psName = e.attribute( "style:parent-style-name" );

      kdDebug(30518) << "Default style: " << psName << endl;
      KSpreadFormat * layout = m_defaultStyles[psName];

      if ( layout )
        cell->copy( *layout );

      QDomElement * st = 0;
      if ( e.hasAttribute( "table:style-name" ) )
      {
        kdDebug(30518) << "Style: " << e.attribute( "table:style-name" ) << endl;
        st = m_styles[ e.attribute( "table:style-name" ) ];
      }
      if ( st )
      {
        kdDebug(30518) << "Style: adapting " << endl;
        QDomNode node = st->firstChild();

        while( !node.isNull() )
        {
          QDomElement property = node.toElement();

          if ( !property.isNull() && property.tagName() == "style:properties" )
          {
            loadStyleProperties( cell, property );

            if ( cell->getAngle( columns, row ) != 0 )
            {
              QFontMetrics fm( cell->textFont( columns, row ) );
              int tmpAngle = cell->getAngle( columns, row );
              int textHeight = static_cast<int>( cos( tmpAngle * M_PI / 180 )
                                                 * ( fm.ascent() + fm.descent() )
                                                 + abs ( ( int )(  fm.width( cell->strOutText() )
                                                                   * sin( tmpAngle * M_PI / 180 )  ) ) );
              /*
              int textWidth = static_cast<int>( abs ( ( int ) ( sin( tmpAngle * M_PI / 180 )
              * ( fm.ascent() + fm.descent() ) ) )
              + fm.width( cell->strOutText() )
              * cos( tmpAngle * M_PI / 180 ) );
              */
              kdDebug(30518) << "Rotation: height: " << textHeight << endl;

              RowFormat * l = table->rowFormat( row );
              if ( l->height() < textHeight )
              {
                if ( l->isDefault() )
                  l = table->nonDefaultRowFormat( row );

                l->setHeight( textHeight + 2 );
              }
            }
          }
          node = node.nextSibling();
        }
      }
    }
    else
    {
      if ( !cell )
        cell = table->nonDefaultCell( columns, row );

      QString psName( "Default" );
      kdDebug(30518) << "Default style: " << psName << endl;
      KSpreadFormat * layout = m_defaultStyles[psName];

      if ( layout )
        cell->copy( *layout );
    }
    if ( e.hasAttribute( "table:formula" ) )
    {
      isFormula = true;
      QString formula;
      convertFormula( formula, e.attribute( "table:formula" ) );

      if ( !cell )
        cell = table->nonDefaultCell( columns, row );
      cell->setCellText( formula );
    }
    if ( e.hasAttribute( "table:value-type" ) )
    {
      if ( !cell )
        cell = table->nonDefaultCell( columns, row );

      cell->setCellText( text );

      QString value = e.attribute( "table:value" );
      QString type  = e.attribute( "table:value-type" );

      kdDebug(30518) << "Value: " << value << ", type: " << type << endl;

      bool ok = false;
      double dv = 0.0;

      if ( ( type == "float" ) || ( type == "currency" ) )
      {
        dv = value.toDouble( &ok );
        if ( ok )
        {
          if ( !isFormula )
            cell->setValue( dv );

          if ( type == "currency" )
          {
            cell->setCurrency( 1, e.attribute( "table:currency" ) );
            cell->setFormatType( KSpreadFormat::Money );
          }
        }
      }
      else
        if ( type == "percentage" )
        {
          dv = value.toDouble( &ok );
          if ( ok )
          {
            if ( !isFormula )
              cell->setValue( dv );
            cell->setFactor( 100 );
            // TODO: replace with custom...
            cell->setFormatType( KSpreadFormat::Percentage );
          }
        }
        else if ( type == "boolean" )
        {
          if ( value.isEmpty() )
            value = e.attribute( "table:boolean-value" );

          kdDebug(30518) << "Type: boolean" << endl;
          if ( value == "true" )
            cell->setValue( true );
          else
            cell->setValue( false );
          ok = true;
          cell->setFormatType( KSpreadFormat::Custom );
        }
        else if ( type == "date" )
        {
          if ( value.isEmpty() )
            value = e.attribute( "table:date-value" );
          kdDebug(30518) << "Type: date, value: " << value << endl;

          // "1980-10-15"
          int year, month, day;
          ok = false;

          int p1 = value.find( '-' );
          if ( p1 > 0 )
            year  = value.left( p1 ).toInt( &ok );

          kdDebug(30518) << "year: " << value.left( p1 ) << endl;

          int p2 = value.find( '-', ++p1 );

          if ( ok )
            month = value.mid( p1, p2 - p1  ).toInt( &ok );

          kdDebug(30518) << "month: " << value.mid( p1, p2 - p1 ) << endl;

          if ( ok )
            day = value.right( value.length() - p2 - 1 ).toInt( &ok );

          kdDebug(30518) << "day: " << value.right( value.length() - p2 ) << endl;

          if ( ok )
          {
            QDateTime dt( QDate( year, month, day ) );
            //            KSpreadValue kval( dt );
            // cell->setValue( kval );
            cell->setValue( QDate( year, month, day ) );
            kdDebug(30518) << "Set QDate: " << year << " - " << month << " - " << day << endl;
          }
        }
        else if ( type == "time" )
        {
          if ( value.isEmpty() )
            value = e.attribute( "table:time-value" );

          kdDebug(30518) << "Type: time: " << value << endl;
          // "PT15H10M12S"
          int hours, minutes, seconds;
          int l = value.length();
          QString num;

          for ( int i = 0; i < l; ++i )
          {
            if ( value[i].isNumber() )
            {
              num += value[i];
              continue;
            }
            else if ( value[i] == 'H' )
              hours   = num.toInt( &ok );
            else if ( value[i] == 'M' )
              minutes = num.toInt( &ok );
            else if ( value[i] == 'S' )
              seconds = num.toInt( &ok );
            else
              continue;

            kdDebug(30518) << "Num: " << num << endl;

            num = "";
            if ( !ok )
              break;
          }

          kdDebug(30518) << "Hours: " << hours << ", " << minutes << ", " << seconds << endl;

          if ( ok )
          {
            // KSpreadValue kval( timeToNum( hours, minutes, seconds ) );
            // cell->setValue( kval );
            cell->setValue( QTime( hours % 24, minutes, seconds ) );
            cell->setFormatType( KSpreadFormat::Custom );
          }
        }


      if ( !ok ) // just in case we couldn't set the value directly
        cell->setCellText( text );
    }
    else if ( !text.isEmpty() )
    {
      if ( !cell )
        cell = table->nonDefaultCell( columns, row );
      cell->setCellText( text );
    }

    if ( spanR > 1 || spanC > 1 )
    {
      if ( !cell )
        cell = table->nonDefaultCell( columns, row );
      cell->forceExtraCells( columns, row, spanC - 1, spanR - 1 );
    }

    cellNode = cellNode.nextSibling();

    if ( e.hasAttribute( "table:number-columns-repeated" ) )
    {
      // copy cell from left
      bool ok = false;
      int number = e.attribute( "table:number-columns-repeated" ).toInt( &ok );
      KSpreadCell * cellDest = 0;

      // don't repeat more than 10 if it is the last cell and empty
      if ( !ok || cellNode.isNull() )
      {
        if ( number > 10 )
          number = 10;
      }

      for ( int i = 1; i < number; ++i )
      {
        ++columns;

        if ( cell )
        {
          cellDest = table->nonDefaultCell( columns, row );
          cellDest->copyAll( cell );
        }
      }
    }

    ++columns;
  }

  return true;
}

bool OpenCalcImport::readRowsAndCells( QDomElement & content, KSpreadSheet * table )
{
  kdDebug(30518) << endl << "Reading in rows " << endl;

  int i   = 1;
  int row = 1;
  int columns = 1;
  int backupRow = 1;
  QDomElement * rowStyle = 0;
  KSpreadCell * cell = 0;
  KSpreadCell * cellDest = 0;
  KSpreadCell * defCell = table->defaultCell();
  QDomNode rowNode = content.namedItem( "table:table-row" );

  while ( !rowNode.isNull() )
  {
    bool collapsed = false;

    int number = 1;
    QDomElement r = rowNode.toElement();

    if ( r.isNull() )
      return false;

    if ( r.hasAttribute( "table:style-name" ) )
    {
      QString style = r.attribute( "table:style-name" );
      rowStyle = m_styles[ style ];
      kdDebug(30518) << "Row style: " << style << endl;
    }

    collapsed = ( r.attribute( "table:visibility" ) == "collapse" );

    backupRow = row;

    rowNode = rowNode.nextSibling();

    if ( !readRowFormat( r, rowStyle, table, row, number, rowNode.isNull() ) ) // updates "row"
      return false;

    if ( !readCells( r, table, backupRow, columns ) )
      return false;

    RowFormat * srcLayout = table->nonDefaultRowFormat( backupRow );
    RowFormat * layout = 0;

    if ( collapsed )
      srcLayout->setHide( true );

    for ( i = 1; i < number; ++i )
    {
      layout = table->nonDefaultRowFormat( backupRow + i );

      layout->copy( *srcLayout );

      /*
       * TODO: Test: do we need to copy the cells, too?
       *       if so we will probably also need to copy them for repeated col layouts.
      for ( j = 1; j <= columns; ++j )
      {
        KSpreadCell * cell = table->cellAt( j, backupRow );

        kdDebug(30518) << "Cell: " << cell << "DefCell: " << defCell << endl;
        if ( cell && (cell != defCell) )
        {
          cellDest = table->nonDefaultCell( j, backupRow + i );
          cellDest->copyAll( cell );
        }
      }
      */
    }

    rowStyle = 0;
    columns = 1;
  }

  kdDebug(30518) << "Reading in rows done" << endl << endl;

  return true;
}

bool OpenCalcImport::readColLayouts( QDomElement & content, KSpreadSheet * table )
{
  kdDebug(30518) << endl << "Reading in columns..." << endl;

  QDomNode colLayout = content.namedItem( "table:table-column" );
  int column = 1;

  while ( !colLayout.isNull() )
  {
    if ( colLayout.nodeName() != "table:table-column" )
      return true; // all cols read in.

    QDomElement e = colLayout.toElement();

    if ( e.isNull() )
      return false; // error, that's it...

    kdDebug(30518) << "New column: " << column << endl;

    int number     = 1;
    double width   = POINT_TO_MM( colWidth );
    bool collapsed = ( e.attribute( "table:visibility" ) == "collapse" );
    bool insertPageBreak = false;
    KSpreadFormat styleLayout( table, table->doc()->styleManager()->defaultStyle() );

    kdDebug(30518) << "Check table:number-columns-repeated" << endl;
    if ( e.hasAttribute( "table:number-columns-repeated" ) )
    {
      bool ok = true;
      number = e.attribute( "table:number-columns-repeated" ).toInt( &ok );
      if ( !ok )
        number = 1;

      kdDebug(30518) << "Repeated: " << number << endl;
    }

    kdDebug(30518) << "Checking table:default-cell-style-name" << endl;
    if ( e.hasAttribute( "table:default-cell-style-name" ) )
    {
      QString n( e.attribute( "table:default-cell-style-name" ) );
      kdDebug(30518) << "Has attribute default-cell-style-name: " << n << endl;
      KSpreadFormat * defaultStyle = m_defaultStyles[ n ];
      if ( !defaultStyle )
      {
        QString name = e.attribute( "table:default-cell-style-name" );
        QDomElement * st = m_styles[ name ];

        kdDebug(30518) << "Default cell style: " << name << endl;

        if ( st && !st->isNull() )
        {
          KSpreadFormat * layout = new KSpreadFormat( 0, m_doc->styleManager()->defaultStyle() );

          readInStyle( layout, *st );

          m_defaultStyles.insert( name, layout );
          kdDebug(30518) << "Insert default cell style: " << name << endl;

          defaultStyle = layout;
        }
      }

      if ( defaultStyle )
      {
        //        kdDebug(30518) << "Copying default style, Font: " << defaultStyle->font().toString() << endl;
        styleLayout.copy( *defaultStyle );
      }
    }

    QDomElement * colStyle = 0;
    if ( e.hasAttribute( "table:style-name" ) )
    {
      QString style = e.attribute( "table:style-name" );
      colStyle = m_styles[ style ];

      kdDebug(30518) << "Col Style: " << style << endl;
    }

    QDomNode node;

    if ( colStyle )
      node = colStyle->firstChild();

    while( !node.isNull() )
    {
      QDomElement property = node.toElement();
      if ( !property.isNull() && property.tagName() == "style:properties" )
      {
        if ( property.hasAttribute( "style:column-width" ) )
        {
          QString sWidth = property.attribute( "style:column-width" );
          kdDebug(30518) << "Col Width: " << sWidth << endl;
          int p = sWidth.find( "cm" );
          if ( p != -1 )
          {
            sWidth = sWidth.left( p );
            bool ok = true;
            double d = sWidth.toDouble( &ok );
            if ( ok )
              width = d * 10; // we work with mm
          }
          else
          {
            p = sWidth.find( "mm" );
            if ( p != -1 )
            {
              sWidth = sWidth.left( p );
              bool ok = true;
              double d = sWidth.toDouble( &ok );
              if ( ok )
                width = d;
            }
          }
        }

        if ( property.hasAttribute( "fo:break-before" ) )
        {
          if ( property.attribute( "fo:break-before" ) == "page" )
          {
            insertPageBreak = true;
          }
        }

        loadStyleProperties( &styleLayout, property );
      }

      node = node.nextSibling();
    }

    colLayout = colLayout.nextSibling();

    if ( colLayout.isNull() && ( number > 30 ) )
      number = 30;

    for ( int i = 0; i < number; ++i )
    {
      kdDebug(30518) << "Inserting colLayout: " << column << endl;

      ColumnFormat * col = new ColumnFormat( table, column );
      col->copy( styleLayout );
      col->setMMWidth( width );

      // if ( insertPageBreak )
      //   col->setPageBreak( true )

      if ( collapsed )
        col->setHide( true );

      table->insertColumnFormat( col );
      ++column;
    }
  }

  return true;
}

void replaceMacro( QString & text, QString const & old, QString const & newS )
{
  int n = text.find( old );
  if ( n != -1 )
    text = text.replace( n, old.length(), newS );
}

QString getPart( QDomNode const & part )
{
  QString result;
  QDomElement e = part.namedItem( "text:p" ).toElement();
  while ( !e.isNull() )
  {
    QString text = e.text();
    kdDebug(30518) << "PART: " << text << endl;

    QDomElement macro = e.namedItem( "text:time" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<time>" );

    macro = e.namedItem( "text:date" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<date>" );

    macro = e.namedItem( "text:page-number" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<page>" );

    macro = e.namedItem( "text:page-count" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<pages>" );

    macro = e.namedItem( "text:sheet-name" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<sheet>" );

    macro = e.namedItem( "text:title" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<name>" );

    macro = e.namedItem( "text:file-name" ).toElement();
    if ( !macro.isNull() )
      replaceMacro( text, macro.text(), "<file>" );

    if ( !result.isEmpty() )
      result += '\n';
    result += text;
    e = e.nextSibling().toElement();
  }

  return result;
}

void OpenCalcImport::loadTableMasterStyle( KSpreadSheet * table,
                                           QString const & stylename )
{
  kdDebug(30518) << "Loading table master style: " << stylename << endl;

  QDomElement * style = m_styles[ stylename ];

  if ( !style )
  {
    kdDebug(30518) << "Master style not found! " << endl;
    return;
  }

  QDomNode header = style->namedItem( "style:header" );
  kdDebug(30518) << "Style header " << endl;

  QString hleft, hmiddle, hright;
  QString fleft, fmiddle, fright;

  if ( !header.isNull() );
  {
    kdDebug(30518) << "Header exists" << endl;
    QDomNode part = header.namedItem( "style:region-left" );
    if ( !part.isNull() )
    {
      hleft = getPart( part );
      kdDebug(30518) << "Header left: " << hleft << endl;
    }
    else
      kdDebug(30518) << "Style:region:left doesn't exist!" << endl;
    part = header.namedItem( "style:region-center" );
    if ( !part.isNull() )
    {
      hmiddle = getPart( part );
      kdDebug(30518) << "Header middle: " << hmiddle << endl;
    }
    part = header.namedItem( "style:region-right" );
    if ( !part.isNull() )
    {
      hright = getPart( part );
      kdDebug(30518) << "Header right: " << hright << endl;
    }
  }

  QDomNode footer = style->namedItem( "style:footer" );

  if ( !footer.isNull() );
  {
    QDomNode part = footer.namedItem( "style:region-left" );
    if ( !part.isNull() )
    {
      fleft = getPart( part );
      kdDebug(30518) << "Footer left: " << fleft << endl;
    }
    part = footer.namedItem( "style:region-center" );
    if ( !part.isNull() )
    {
      fmiddle = getPart( part );
      kdDebug(30518) << "Footer middle: " << fmiddle << endl;
    }
    part = footer.namedItem( "style:region-right" );
    if ( !part.isNull() )
    {
      fright = getPart( part );
      kdDebug(30518) << "Footer right: " << fright << endl;
    }
  }

  table->print()->setHeadFootLine( hleft, hmiddle, hright,
                                   fleft, fmiddle, fright );
}

bool OpenCalcImport::parseBody( int numOfTables )
{
  QDomElement content = m_content.documentElement();
  QDomNode body = content.namedItem( "office:body" );

  if ( body.isNull() )
    return false;

  QDomNode namedAreas = body.namedItem( "table:named-expressions" );
  if ( !namedAreas.isNull() )
  {
    QDomNode area = namedAreas.firstChild();
    while ( !area.isNull() )
    {
      QDomElement e = area.toElement();
      if ( e.isNull() || !e.hasAttribute( "table:name" ) || !e.hasAttribute( "table:cell-range-address" ) )
      {
        kdDebug(30518) << "Reading in named area failed" << endl;
        area = area.nextSibling();
        continue;
      }

      // TODO: what is: table:base-cell-address
      QString name  = e.attribute( "table:name" );
      QString areaPoint = e.attribute( "table:cell-range-address" );

      m_namedAreas.append( name );
      kdDebug(30518) << "Reading in named area, name: " << name << ", area: " << areaPoint << endl;

      OpenCalcPoint point( areaPoint );
      kdDebug(30518) << "Area: " << point.translation << endl;

      QString range( point.translation );

      if ( point.translation.find( ':' ) == -1 )
      {
        KSpreadPoint p( point.translation );

        int n = range.find( '!' );
        if ( n > 0 )
          range = range + ":" + range.right( range.length() - n - 1);

        kdDebug(30518) << "=> Area: " << range << endl;
      }

      KSpreadRange p( range );

      m_doc->addAreaName( p.range, name, p.tableName );
      kdDebug(30518) << "Area range: " << p.tableName << endl;

      area = area.nextSibling();
    }
  }

  KSpreadSheet * table;
  QDomNode sheet = body.namedItem( "table:table" );

  if ( sheet.isNull() )
    return false;

  while ( !sheet.isNull() )
  {
    QDomElement t = sheet.toElement();
    if ( t.isNull() )
    {
      sheet = sheet.nextSibling();
      continue;
    }
    if ( t.nodeName() != "table:table" )
    {
      sheet = sheet.nextSibling();
      continue;
    }

    table = m_doc->createTable();
    m_doc->addTable( table );

    table->setTableName( t.attribute( "table:name" ), true, false );
    sheet = sheet.nextSibling();
  }

  sheet = body.firstChild();

  int step = (int) ( 80 / numOfTables );
  int progress = 15;

  KSpreadFormat::setGlobalColWidth( MM_TO_POINT( 22.7 ) );
  KSpreadFormat::setGlobalRowHeight( MM_TO_POINT( 4.3 ) );
  kdDebug(30518) << "Global Height: " << MM_TO_POINT( 4.3 ) << ", Global width: " << MM_TO_POINT( 22.7) << endl;

  while ( !sheet.isNull() )
  {
    QDomElement t = sheet.toElement();
    if ( t.isNull() )
    {
      KMessageBox::sorry( 0, i18n( "The file seems to be corrupt. Skipping a table." ) );
      sheet = sheet.nextSibling();
      continue;
    }
    if ( t.nodeName() != "table:table" )
    {
      sheet = sheet.nextSibling();
      continue;
    }

    table = m_doc->map()->findTable( t.attribute( "table:name" ) );
    if ( !table )
    {
      KMessageBox::sorry( 0, i18n( "Skipping a table." ) );
      sheet = sheet.nextSibling();
      continue;
    }

    KSpreadFormat * defaultStyle = m_defaultStyles[ "Default" ];
    if ( defaultStyle )
    {
      KSpreadCell * defaultCell = table->defaultCell();
      kdDebug(30518) << "Copy default style to default cell" << endl;
      defaultCell->copy( *defaultStyle );
    }
    table->setDefaultHeight( MM_TO_POINT( 4.3 ) );
    table->setDefaultWidth( MM_TO_POINT( 22.7 ) );

    kdDebug(30518) << "Added table: " << t.attribute( "table:name" ) << endl;

    if ( t.hasAttribute( "table:style-name" ) )
    {
      QString style = t.attribute( "table:style-name" );
      QDomElement * tableStyle = m_styles[ style ];

      QDomNode node;

      if ( tableStyle )
        node = tableStyle->firstChild();

      while( !node.isNull() )
      {
        QDomElement property = node.toElement();
        if ( !property.isNull() && property.tagName() == "style:properties" )
        {
          if ( property.hasAttribute( "table:display" ) )
          {
            bool visible = (property.attribute( "table:display" ) == "true" ? true : false );
            table->hideTable( !visible );
            kdDebug(30518) << "Table: " << table->tableName() << ", hidden: " << !visible << endl;
          }
        }

        node = node.nextSibling();
      }

      if ( tableStyle && tableStyle->hasAttribute( "style:master-page-name" ) )
      {
        QString stylename = "pm" + tableStyle->attribute( "style:master-page-name" );

        loadTableMasterStyle( table, stylename );
      }
    }
    if ( t.hasAttribute( "table:print-ranges" ) )
    {
      // e.g.: Sheet4.A1:Sheet4.E28
      QString range = t.attribute( "table:print-ranges" );
      OpenCalcPoint point( range );

      kdDebug(30518) << "Print range: " << point.translation << endl;
      KSpreadRange p( point.translation );

      kdDebug(30518) << "Print table: " << p.tableName << endl;

      if ( table->tableName() == p.tableName )
        table->print()->setPrintRange( p.range );
    }

    if ( !readColLayouts( t, table ) )
      return false;

    if ( !readRowsAndCells( t, table ) )
      return false;

    if ( t.hasAttribute( "table:protected" ) )
    {
      QCString passwd( "" );
      if ( t.hasAttribute( "table:protection-key" ) )
      {
        QString p = t.attribute( "table:protection-key" );
        QCString str( p.latin1() );
        kdDebug(30518) << "Decoding password: " << str << endl;
        passwd = KCodecs::base64Decode( str );
      }
      kdDebug(30518) << "Password hash: '" << passwd << "'" << endl;
      table->setProtected( passwd );
    }

    progress += step;
    emit sigProgress( progress );
    sheet = sheet.nextSibling();
  }

  QDomElement b = body.toElement();
  if ( b.hasAttribute( "table:structure-protected" ) )
  {
    QCString passwd( "" );
    if ( b.hasAttribute( "table:protection-key" ) )
    {
      QString p = b.attribute( "table:protection-key" );
      QCString str( p.latin1() );
      kdDebug(30518) << "Decoding password: " << str << endl;
      passwd = KCodecs::base64Decode( str );
    }
    kdDebug(30518) << "Password hash: '" << passwd << "'" << endl;

    m_doc->map()->setProtected( passwd );
  }

  emit sigProgress( 98 );

  return true;
}

void OpenCalcImport::insertStyles( QDomElement const & element )
{
  if ( element.isNull() )
    return;

  QDomNode n = element.firstChild();

  while( !n.isNull() )
  {
    QDomElement e = n.toElement();
    if ( e.isNull() || !e.hasAttribute( "style:name" ) )
    {
      n = n.nextSibling();
      continue;
    }

    QString name = e.attribute( "style:name" );
    kdDebug(30518) << "Style: '" << name << "' loaded " << endl;
    m_styles.insert( name, new QDomElement( e ) );

    n = n.nextSibling();
  }
}

QString * OpenCalcImport::loadFormat( QDomElement * element,
                                      KSpreadFormat::FormatType & formatType,
                                      QString name )
{
  if ( !element )
    return 0;

  int  i;
  bool ok;

  QString * format = 0;
  QDomElement e = element->firstChild( ).toElement();
  int precision = 0;
  int leadingZ  = 1;
  bool thousandsSep = false;
  bool negRed = false;

  if ( element->tagName() == "number:time-style" )
    formatType = KSpreadFormat::Custom;
  else if ( element->tagName() == "number:date-style" )
    formatType = KSpreadFormat::Custom;
  else if ( element->tagName() == "number:percentage-style" )
    formatType = KSpreadFormat::Custom;
  else if ( element->tagName() == "number:number-style" )
    formatType = KSpreadFormat::Custom;
  else if ( element->tagName() == "number:currency-style" )
    formatType = KSpreadFormat::Custom;
  else if ( element->tagName() == "number:boolean-style" )
    formatType = KSpreadFormat::Custom;

  if ( !e.isNull() )
    format = new QString();

  // TODO (element):
  // number:automatic-order="true"
  // number:truncate-on-overflow="false"
  // style:volatile="true"

  while ( !e.isNull() )
  {
    if ( e.tagName() == "style:properties" )
    {
      if ( e.hasAttribute( "fo:color" ) )
        negRed = true; // we only support red...
    }
    else
    if ( e.tagName() == "number:text" )
    {
      if ( negRed && ( e.text() == "-" ) )
        ;
      else
        format->append( e.text() );
    }
    else
    if ( e.tagName() == "number:currency-symbol" )
    {
      QString sym( e.text() );
      kdDebug(30518) << "Currency: " << sym << endl;
      format->append( sym );
      // number:language="de" number:country="DE">€</number:currency-symbol>
    }
    else
    if ( e.tagName() == "number:day-of-week" )
    {
      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "dddd" );
        else
          format->append( "ddd" );
      }
      else
        format->append( "ddd" );
    }
    else
    if ( e.tagName() == "number:day" )
    {
      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "dd" );
        else
          format->append( "d" );
      }
      else
        format->append( "d" );
    }
    else
    if ( e.tagName() == "number:month" )
    {
      if ( e.hasAttribute( "number:textual" ) )
      {
        if ( e.attribute( "number:textual" ) == "true" )
          format->append( "mm" );
      }

      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "mm" );
        else
          format->append( "m" );
      }
      else
        format->append( "m" );
    }
    else
    if ( e.tagName() == "number:year" )
    {
      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "yyyy" );
        else
          format->append( "yy" );
      }
      else
        format->append( "yy" );
    }
    else
    if ( e.tagName() == "number:hours" )
    {
      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "hh" );
        else
          format->append( "h" );
      }
      else
        format->append( "h" );
    }
    else
    if ( e.tagName() == "number:minutes" )
    {
      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "mm" );
        else
          format->append( "m" );
      }
      else
        format->append( "m" );
    }
    else
    if ( e.tagName() == "number:seconds" )
    {
      if ( e.hasAttribute( "number:style" ) )
      {
        if ( e.attribute( "number:style" ) == "long" )
          format->append( "ss" );
        else
          format->append( "s" );
      }
      else
        format->append( "s" );
    }
    else
    if ( e.tagName() == "number:am-pm" )
    {
      format->append( "AM/PM" );
    }
    else
    if ( e.tagName() == "number:number" )
    {
      // TODO: number:grouping="true"

      if ( e.hasAttribute( "number:decimal-places" ) )
      {
        int d = e.attribute( "number:decimal-places" ).toInt( &ok );
        if ( ok )
          precision = d;
      }

      if ( e.hasAttribute( "number:min-integer-digits" ) )
      {
        int d = e.attribute( "number:min-integer-digits" ).toInt( &ok );
        if ( ok )
          leadingZ = d;
      }

      if ( thousandsSep && leadingZ <= 3 )
      {
        format->append( "#," );
        for ( i = leadingZ; i <= 3; ++i )
          format->append( '#' );
      }

      for ( i = 1; i <= leadingZ; ++i )
      {
        format->append( '0' );
        if ( ( i % 3 == 0 ) && thousandsSep )
          format->append( ',' );
      }

      format->append( '.' );
      for ( i = 0; i < precision; ++i )
        format->append( '0' );
    }
    else
    if ( e.tagName() == "number:scientific-number" )
    {
      int exp = 2;

      if ( e.hasAttribute( "number:decimal-places" ) )
      {
        int d = e.attribute( "number:decimal-places" ).toInt( &ok );
        if ( ok )
          precision = d;
      }

      if ( e.hasAttribute( "number:min-integer-digits" ) )
      {
        int d = e.attribute( "number:min-integer-digits" ).toInt( &ok );
        if ( ok )
          leadingZ = d;
      }

      if ( e.hasAttribute( "number:min-exponent-digits" ) )
      {
        int d = e.attribute( "number:min-exponent-digits" ).toInt( &ok );
        if ( ok )
          exp = d;
        if ( exp <= 0 )
          exp = 1;
      }

      if ( thousandsSep && leadingZ <= 3 )
      {
        format->append( "#," );
        for ( i = leadingZ; i <= 3; ++i )
          format->append( '#' );
      }

      for ( i = 1; i <= leadingZ; ++i )
      {
        format->append( '0' );
        if ( ( i % 3 == 0 ) && thousandsSep )
          format->append( ',' );
      }

      format->append( '.' );
      for ( i = 0; i < precision; ++i )
        format->append( '0' );

      format->append( "E+" );
      for ( i = 0; i < exp; ++i )
        format->append( '0' );

      formatType = KSpreadFormat::Custom;
    }
    else
    if ( e.tagName() == "number:fraction" )
    {
      int integer = 0;
      int numerator = 1;
      int denominator = 1;

      if ( e.hasAttribute( "number:min-integer-digits" ) )
      {
        int d = e.attribute( "number:min-integer-digits" ).toInt( &ok );
        if ( ok )
          integer = d;
      }
      if ( e.hasAttribute( "number:min-numerator-digits" ) )
      {
        int d = e.attribute( "number:min-numerator-digits" ).toInt( &ok );
        if ( ok )
          numerator = d;
      }
      if ( e.hasAttribute( "number:min-denominator-digits" ) )
      {
        int d = e.attribute( "number:min-denominator-digits" ).toInt( &ok );
        if ( ok )
          denominator = d;
      }

      for ( i = 0; i <= integer; ++i )
        format->append( '#' );

      format->append( ' ' );

      for ( i = 0; i <= numerator; ++i )
        format->append( '?' );

      format->append( '/' );

      for ( i = 0; i <= denominator; ++i )
        format->append( '?' );
    }
    // Not needed:
    //  <style:map style:condition="value()&gt;=0" style:apply-style-name="N106P0"/>
    // we handle painting negative numbers in red differently

    e = e.nextSibling().toElement();
  }

  if ( negRed )
  {
    QString f( *format );
    format->append( ";[Red]" );
    format->append( f );
  }

  kdDebug(30518) << "*** New FormatString: " << *format << endl << endl;

  m_formats.insert( name, format );

  return format;
}

void OpenCalcImport::loadFontStyle( KSpreadFormat * layout, QDomElement const * font ) const
{
  if ( !font || !layout )
    return;

  kdDebug(30518) << "Copy font style from the layout " << font->tagName() << ", " << font->nodeName() << endl;

  if ( font->hasAttribute( "fo:font-family" ) )
    layout->setTextFontFamily( font->attribute( "fo:font-family" ) );
  if ( font->hasAttribute( "fo:color" ) )
    layout->setTextColor( QColor( font->attribute( "fo:color" ) ) );
  if ( font->hasAttribute( "fo:font-size" ) )
    layout->setTextFontSize(  KoUnit::parseValue( font->attribute( "fo:font-size" ),10.0 ));
  else
    layout->setTextFontSize( 10 );
  if ( font->hasAttribute( "fo:font-style" ) )
  {
    kdDebug(30518) << "italic" << endl;
    layout->setTextFontItalic( true ); // only thing we support
  }
  if ( font->hasAttribute( "fo:font-weight" ) )
    layout->setTextFontBold( true ); // only thing we support
  if ( font->hasAttribute( "fo:text-underline" ) || font->hasAttribute( "style:text-underline" ) )
    layout->setTextFontUnderline( true ); // only thing we support
  if ( font->hasAttribute( "style:text-crossing-out" ) )
    layout->setTextFontStrike( true ); // only thing we support
  if ( font->hasAttribute( "style:font-pitch" ) )
  {
    // TODO: possible values: fixed, variable
  }
  // TODO:
  // text-underline-color
}

void OpenCalcImport::loadBorder( KSpreadFormat * layout, QString const & borderDef, bPos pos ) const
{
  if ( borderDef == "none" )
    return;

  int p = borderDef.find( ' ' );
  if ( p < 0 )
    return;

  QPen pen;
  QString w = borderDef.left( p );
  pen.setWidth( (int)KoUnit::parseValue( w ) );


  ++p;
  int p2 = borderDef.find( ' ', p );
  QString s = borderDef.mid( p, p2 - p );

  kdDebug(30518) << "Borderstyle: " << s << endl;

  if ( s == "solid" || s == "double" )
    pen.setStyle( Qt::SolidLine );
  else
  {
    // TODO:
    pen.setStyle( Qt::DashLine );
    pen.setStyle( Qt::DotLine );
    pen.setStyle( Qt::DashDotLine );
    pen.setStyle( Qt::DashDotDotLine );
  }

  ++p2;
  p = borderDef.find( ' ', p2 );
  if ( p == -1 )
    p = borderDef.length();

  pen.setColor( QColor( borderDef.right( p - p2 ) ) );

  if ( pos == Left )
    layout->setLeftBorderPen( pen );
  else if ( pos == Top )
    layout->setTopBorderPen( pen );
  else if ( pos == Right )
    layout->setRightBorderPen( pen );
  else if ( pos == Bottom )
    layout->setBottomBorderPen( pen );
  else if ( pos == Border )
  {
    layout->setLeftBorderPen( pen );
    layout->setTopBorderPen( pen );
    layout->setRightBorderPen( pen );
    layout->setBottomBorderPen( pen );
  }
  // TODO Diagonals
}

void OpenCalcImport::loadStyleProperties( KSpreadFormat * layout, QDomElement const & property ) const
{
  kdDebug(30518) << "*** Loading style properties *****" << endl;

  if ( property.hasAttribute( "style:decimal-places" ) )
  {
    bool ok = false;
    int p = property.attribute( "style:decimal-places" ).toInt( &ok );
    if (ok )
      layout->setPrecision( p );
  }

  if ( property.hasAttribute( "style:font-name" ) )
  {
    QDomElement * font = m_styles[ property.attribute( "style:font-name" ) ];
    loadFontStyle( layout, font ); // generell font style
  }

  loadFontStyle( layout, &property ); // specific font style

  // TODO:
  //   diagonal: fall + goup
  //   fo:direction="ltr"
  //   style:text-align-source  ("fix")
  //   style:shadow
  //   style:text-outline
  //   indents from right, top, bottom
  //   style:condition="cell-content()=15"
  //     => style:apply-style-name="Result" style:base-cell-address="Sheet6.A5"/>

  if ( property.hasAttribute( "style:rotation-angle" ) )
  {
    bool ok = false;
    int a = property.attribute( "style:rotation-angle" ).toInt( &ok );
    if ( ok )
      layout->setAngle( -a + 1 );
  }

  if (  property.hasAttribute( "fo:margin-left" ) )
  {
      kdDebug()<<"margin-left :"<<KoUnit::parseValue( property.attribute( "fo:margin-left" ),0.0 )<<endl;
      layout->setIndent( KoUnit::parseValue( property.attribute( "fo:margin-left" ),0.0 ) );
  }
  
  if ( property.hasAttribute( "fo:text-align" ) )
  {
    QString s = property.attribute( "fo:text-align" );
    if ( s == "center" )
      layout->setAlign( KSpreadFormat::Center );
    else if ( s == "end" )
      layout->setAlign( KSpreadFormat::Right );
    else if ( s == "start" )
      layout->setAlign( KSpreadFormat::Left );
    else if ( s == "justify" ) // TODO in KSpread!
      layout->setAlign( KSpreadFormat::Center );
  }

  if ( property.hasAttribute( "fo:background-color" ) )
    layout->setBgColor( QColor( property.attribute( "fo:background-color" ) ) );

  if ( property.hasAttribute( "style:print-content" ) )
  {
    if ( property.attribute( "style:print-content" ) == "false" )
      layout->setDontPrintText( false );
  }
  if ( property.hasAttribute( "style:cell-protect" ) )
  {
    QString prot( property.attribute( "style:cell-protect" ) );
    if ( prot == "none" )
    {
      layout->setNotProtected( true );
      layout->setHideFormula( false );
      layout->setHideAll( false );
    }
    else if ( prot == "formula-hidden" )
    {
      layout->setNotProtected( true );
      layout->setHideFormula( true );
      layout->setHideAll( false );
    }
    else if ( prot == "protected formula-hidden" )
    {
      layout->setNotProtected( false );
      layout->setHideFormula( true );
      layout->setHideAll( false );
    }
    else if ( prot == "hidden-and-protected" )
    {
      layout->setNotProtected( false );
      layout->setHideFormula( false );
      layout->setHideAll( true );
    }
    else if ( prot == "protected" )
    {
      layout->setNotProtected( false );
      layout->setHideFormula( false );
      layout->setHideAll( false );
    }
    kdDebug(30518) << "Cell " << prot << endl;
  }

  if ( property.hasAttribute( "fo:padding-left" ) )
    layout->setIndent( KoUnit::parseValue( property.attribute( "fo:padding-left" ) ) );

  if ( property.hasAttribute( "fo:vertical-align" ) )
  {
    QString s = property.attribute( "fo:vertical-align" );
    if ( s == "middle" )
      layout->setAlignY( KSpreadFormat::Middle );
    else if ( s == "bottom" )
      layout->setAlignY( KSpreadFormat::Bottom );
    else
      layout->setAlignY( KSpreadFormat::Top );
  }
  else
      layout->setAlignY( KSpreadFormat::Bottom );

  if ( property.hasAttribute( "fo:wrap-option" ) )
  {
    layout->setMultiRow( true );

    /* we do not support anything else yet
      QString s = property.attribute( "fo:wrap-option" );
      if ( s == "wrap" )
      layout->setMultiRow( true );
    */
  }

  if ( property.hasAttribute( "fo:border-bottom" ) )
  {
    loadBorder( layout, property.attribute( "fo:border-bottom" ), Bottom );
    // TODO: style:border-line-width-bottom if double!
  }

  if ( property.hasAttribute( "fo:border-right" ) )
  {
    loadBorder( layout, property.attribute( "fo:border-right" ), Right );
    // TODO: style:border-line-width-right
  }

  if ( property.hasAttribute( "fo:border-top" ) )
  {
    loadBorder( layout, property.attribute( "fo:border-top" ), Top );
    // TODO: style:border-line-width-top
  }

  if ( property.hasAttribute( "fo:border-left" ) )
  {
    loadBorder( layout, property.attribute( "fo:border-left" ), Left );
    // TODO: style:border-line-width-left
  }

  if ( property.hasAttribute( "fo:border" ) )
  {
    loadBorder( layout, property.attribute( "fo:border" ), Border );
    // TODO: style:border-line-width-left
  }
}

void OpenCalcImport::readInStyle( KSpreadFormat * layout, QDomElement const & style )
{
  kdDebug(30518) << "** Reading Style: " << style.tagName() << "; " << style.attribute("style:name") << endl;
  if ( style.tagName() == "style:style" )
  {
    if ( style.hasAttribute( "style:parent-style-name" ) )
    {
      KSpreadFormat * cp
        = m_defaultStyles.find( style.attribute( "style:parent-style-name" ) );
      kdDebug(30518) << "Copying layout from " << style.attribute( "style:parent-style-name" ) << endl;

      if ( cp != 0 )
        layout->copy( *cp );
    }
    else if ( style.hasAttribute( "style:family") )
    {
      QString name = style.attribute( "style-family" ) + "default";
      KSpreadFormat * cp = m_defaultStyles.find( name );

      kdDebug(30518) << "Copying layout from " << name << ", " << !cp << endl;

      if ( cp != 0 )
        layout->copy( *cp );
    }

    if ( style.hasAttribute( "style:data-style-name" ) )
    {
      QString * format = m_formats[ style.attribute( "style:data-style-name" ) ];
      KSpreadFormat::FormatType formatType;

      if ( !format )
      {
        // load and convert it
        QString name( style.attribute( "style:data-style-name" ) );
        format = loadFormat( m_styles[ name ], formatType, name );
      }

      if ( format )
      {
        layout->setFormatString( *format );
        layout->setFormatType( formatType );
      }

      // <number:currency-symbol number:language="de" number:country="DE">€</number:currency-symbol>
    }
  }

  QDomElement property = style.firstChild().toElement();
  while( !property.isNull() )
  {
    if ( property.tagName() == "style:properties" )
      loadStyleProperties( layout, property );

    kdDebug(30518) << layout->textFontFamily( 0, 0 ) << endl;

    property = property.nextSibling().toElement();
  }
}

bool OpenCalcImport::createStyleMap( QDomDocument const & styles )
{
  QDomElement content  = styles.documentElement();
  QDomNode docStyles   = content.namedItem( "office:document-styles" );

  if ( content.hasAttribute( "office:version" ) )
  {
    bool ok = true;
    double d = content.attribute( "office:version" ).toDouble( &ok );

    if ( ok )
    {
      kdDebug(30518) << "OpenCalc version: " << d << endl;
      if ( d > 1.0 )
      {
        QString message( i18n("This document was created with the OpenOffice version '%1'. This filter was written for version for 1.0. Reading this file could cause strange behavior, crashes or incorrect display of the data. Do you want to continue converting the document?") );
        message.arg( content.attribute( "office:version" ) );
        if ( KMessageBox::warningYesNo( 0, message, i18n( "Unsupported document version" ) ) == KMessageBox::No )
          return false;
      }
    }
  }

  QDomNode fontStyles = content.namedItem( "office:font-decls" );

  if ( !fontStyles.isNull() )
  {
    kdDebug(30518) << "Starting reading in font-decl..." << endl;

    insertStyles( fontStyles.toElement() );
  }
  else
    kdDebug(30518) << "No items found" << endl;

  kdDebug(30518) << "Starting reading in auto:styles" << endl;

  QDomNode autoStyles = content.namedItem( "office:automatic-styles" );
  if ( !autoStyles.isNull() )
    insertStyles( autoStyles.toElement() );
  else
    kdDebug(30518) << "No items found" << endl;


  kdDebug(30518) << "Reading in master styles" << endl;

  QDomNode masterStyles = content.namedItem( "office:master-styles" );

  if ( masterStyles.isNull() )
  {
    kdDebug(30518) << "Nothing found " << endl;
  }

  QDomElement master = masterStyles.namedItem( "style:master-page").toElement();
  if ( !master.isNull() )
  {
    QString name( "pm" );
    name += master.attribute( "style:name" );
    kdDebug(30518) << "Master style: '" << name << "' loaded " << endl;
    m_styles.insert( name, new QDomElement( master ) );

    master = master.nextSibling().toElement();
  }


  kdDebug(30518) << "Starting reading in office:styles" << endl;

  QDomNode fixedStyles = content.namedItem( "office:styles" );

  kdDebug(30518) << "Reading in default styles" << endl;

  QDomNode def = fixedStyles.namedItem( "style:default-style" );
  while ( !def.isNull() )
  {
    QDomElement e = def.toElement();
    kdDebug(30518) << "Style found " << e.nodeName() << ", tag: " << e.tagName() << endl;

    if ( e.nodeName() != "style:default-style" )
    {
      def = def.nextSibling();
      continue;
    }

    if ( !e.isNull() )
    {
      KSpreadFormat * layout = new KSpreadFormat( 0, m_doc->styleManager()->defaultStyle() );

      readInStyle( layout, e );
      kdDebug(30518) << "Default style " << e.attribute( "style:family" ) << "default" << " loaded " << endl;

      m_defaultStyles.insert( e.attribute( "style:family" ) + "default", layout );
      //      QFont font = layout->font();
      //      kdDebug(30518) << "Font: " << font.family() << ", " << font.toString() << endl;
    }

    def = def.nextSibling();
  }

  QDomElement defs = fixedStyles.namedItem( "style:style" ).toElement();
  while ( !defs.isNull() )
  {
    if ( defs.nodeName() != "style:style" )
      break; // done

    if ( !defs.hasAttribute( "style:name" ) )
    {
      // ups...
      defs = defs.nextSibling().toElement();
      continue;
    }

    KSpreadFormat * layout = new KSpreadFormat( 0, m_doc->styleManager()->defaultStyle() );
    readInStyle( layout, defs );
    kdDebug(30518) << "Default style " << defs.attribute( "style:name" ) << " loaded " << endl;

    m_defaultStyles.insert( defs.attribute( "style:name" ), layout );
    //    kdDebug(30518) << "Font: " << layout->font().family() << ", " << layout->font().toString() << endl;

    defs = defs.nextSibling().toElement();
  }

  if ( !fixedStyles.isNull() )
    insertStyles( fixedStyles.toElement() );

  kdDebug(30518) << "Starting reading in automatic styles" << endl;

  content = m_content.documentElement();
  autoStyles = content.namedItem( "office:automatic-styles" );

  if ( !autoStyles.isNull() )
    insertStyles( autoStyles.toElement() );

  fontStyles = content.namedItem( "office:font-decls" );

  if ( !fontStyles.isNull() )
  {
    kdDebug(30518) << "Starting reading in special font decl" << endl;

    insertStyles( fontStyles.toElement() );
  }

  kdDebug(30518) << "Styles read in." << endl;

  return true;
}

int OpenCalcImport::readMetaData()
{
  int result = 5;
  KoDocumentInfo * docInfo          = m_doc->documentInfo();
  KoDocumentInfoAbout  * aboutPage  = static_cast<KoDocumentInfoAbout *>(docInfo->page( "about" ));
  KoDocumentInfoAuthor * authorPage = static_cast<KoDocumentInfoAuthor*>(docInfo->page( "author" ));

  QDomNode meta   = m_meta.namedItem( "office:document-meta" );
  QDomNode office = meta.namedItem( "office:meta" );

  if ( office.isNull() )
    return 2;

  QDomElement e = office.namedItem( "dc:creator" ).toElement();
  if ( !e.isNull() && !e.text().isEmpty() )
    authorPage->setFullName( e.text() );

  e = office.namedItem( "dc:title" ).toElement();
  if ( !e.isNull() && !e.text().isEmpty() )
    aboutPage->setTitle( e.text() );

  e = office.namedItem( "dc:description" ).toElement();
  if ( !e.isNull() && !e.text().isEmpty() )
    aboutPage->setAbstract( e.text() );

  /*
   * TODO:
   *
   * dc:subject
   * <meta:keywords>
   *   <meta:keyword>Borders</meta:keyword>
   *   <meta:keyword>Fonts</meta:keyword>
   * </meta:keywords>
   */

  e = office.namedItem( "meta:document-statistic" ).toElement();
  if ( !e.isNull() && e.hasAttribute( "meta:table-count" ) )
  {
    bool ok = false;
    result = e.attribute( "meta:table-count" ).toInt( &ok );
    if ( !ok )
      result = 5;
  }

  m_meta.clear(); // not needed anymore

  return result;
}

KoFilter::ConversionStatus OpenCalcImport::convert( QCString const & from, QCString const & to )
{
  kdDebug(30518) << "Entering OpenCalc Import filter: " << from << " - " << to << endl;

  KoDocument * document = m_chain->outputDocument();
  if ( !document )
    return KoFilter::StupidError;

  if ( strcmp(document->className(), "KSpreadDoc") != 0 )  // it's safer that way :)
  {
    kdWarning(30518) << "document isn't a KSpreadDoc but a " << document->className() << endl;
    return KoFilter::NotImplemented;
  }

  if ( from != "application/vnd.sun.xml.calc" || to != "application/x-kspread" )
  {
    kdWarning(30518) << "Invalid mimetypes " << from << " " << to << endl;
    return KoFilter::NotImplemented;
  }

  m_doc = ( KSpreadDoc * ) document;

  if ( m_doc->mimeType() != "application/x-kspread" )
  {
    kdWarning(30518) << "Invalid document mimetype " << m_doc->mimeType() << endl;
    return KoFilter::NotImplemented;
  }

  kdDebug(30518) << "Opening file " << endl;

  KoFilter::ConversionStatus preStatus = openFile();

  if ( preStatus != KoFilter::OK )
    return preStatus;

  emit sigProgress( 13 );
  int tables = readMetaData();

  emit sigProgress( 15 );

  if ( !parseBody( tables ) )
    return KoFilter::StupidError;

  emit sigProgress( 100 );
  return KoFilter::OK;
}

KoFilter::ConversionStatus OpenCalcImport::openFile()
{
  KoStore * store = KoStore::createStore( m_chain->inputFile(), KoStore::Read);

  kdDebug(30518) << "Store created" << endl;

  if ( !store )
  {
    kdWarning(30518) << "Couldn't open the requested file." << endl;
    return KoFilter::FileNotFound;
  }

  kdDebug(30518) << "Trying to open content.xml" << endl;
  if ( !store->open( "content.xml" ) )
  {
    kdWarning(30518) << "This file doesn't seem to be a valid OpenCalc file" << endl;
    delete store;
    return KoFilter::WrongFormat;
  }
  kdDebug(30518) << "Opened" << endl;

  QDomDocument styles;
  QCString totalString;
  char tempData[1024];

  Q_LONG size = store->read( &tempData[0], 1023 );
  while ( size > 0 )
  {
    QCString tempString( tempData, size + 1);
    totalString += tempString;

    size = store->read( &tempData[0], 1023 );
  }

  m_content.setContent( totalString );
  totalString = "";
  store->close();

  kdDebug(30518) << "file content.xml loaded " << endl;

  if ( store->open( "styles.xml" ) )
  {
    size = store->read( &tempData[0], 1023 );
    while ( size > 0 )
    {
      QCString tempString( tempData, size + 1);
      totalString += tempString;

      size = store->read( &tempData[0], 1023 );
    }

    styles.setContent( totalString );
    totalString = "";
    store->close();
    kdDebug(30518) << "file containing styles loaded" << endl;
  }
  else
    kdWarning(30518) << "Style definitions do not exist!" << endl;

  if ( store->open( "meta.xml" ) )
  {
    size = store->read( &tempData[0], 1023 );
    while ( size > 0 )
    {
      QCString tempString( tempData, size + 1);
      totalString += tempString;

      size = store->read( &tempData[0], 1023 );
    }

    m_meta.setContent( totalString );
    totalString = "";
    store->close();
    kdDebug(30518) << "File containing meta definitions loaded" << endl;
  }
  else
    kdWarning(30518) << "Meta definitions do not exist!" << endl;

  if ( store->open( "settings.xml" ) )
  {
    size = store->read( &tempData[0], 1023 );
    while ( size > 0 )
    {
      QCString tempString( tempData, size + 1);
      totalString += tempString;

      size = store->read( &tempData[0], 1023 );
    }

    m_settings.setContent( totalString );
    totalString = "";
    store->close();
    kdDebug(30518) << "File containing settings loaded" << endl;
  }
  else
    kdWarning(30518) << "Settings do not exist!" << endl;

  delete store;

  emit sigProgress( 10 );

  if ( !createStyleMap( styles ) )
    return KoFilter::UserCancelled;

  return KoFilter::OK;
}


#include <opencalcimport.moc>

