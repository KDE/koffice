
/* This file is part of the KDE project
   Copyright (C) 2001 Graham Short <grahshrt@netscape.net>

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

#include <kdebug.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>
#include <KoFilterChain.h>

#include <kspread_doc.h>
#include <kspread_sheet.h>
#include <kspread_cell.h>
#include <kspread_map.h>

#include <qproimport.h>

#include <qproformula.h>
#include <qpro/stream.h>
#include <qpro/record_factory.h>
#include <qfile.h>

using namespace KSpread;

typedef KGenericFactory<QpImport, KoFilter> QPROImportFactory;
K_EXPORT_COMPONENT_FACTORY( libqproimport, QPROImportFactory( "kofficefilters" ) )

// ---------------------------------------------------------------

QpTableList::QpTableList()
{
   for( int lIdx=0; lIdx<cNameCnt; ++lIdx )
   {
      cTable[lIdx] = 0;
   }
}

QpTableList::~QpTableList()
{
   // don't delete the list of tables
}


void
QpTableList::table(unsigned pIdx, Sheet* pTable)
{
   if(pIdx < cNameCnt)
   {
      cTable[pIdx] = pTable;
   }
}

Sheet*
QpTableList::table(unsigned pIdx)
{
   return (pIdx < cNameCnt ? cTable[pIdx] : 0);
}


// ---------------------------------------------------------------

QpImport::QpImport( KoFilter*, const char*, const QStringList& )
 : KoFilter()
{
//cout << "Hooray - in QpImport::QpImport" << endl; // ???
}

void
QpImport::InitTableName(int pIdx, QString& pResult)
{
   if( pIdx < 26 )
   {
      pResult = (char)('A' + pIdx);
   }
   else
   {
      pResult = (char)('A' -1 + pIdx / 26);
      pResult += (char)('A' + pIdx % 26);
   }
}

KoFilter::ConversionStatus QpImport::convert( const QCString& from, const QCString& to )
{
    bool bSuccess=true;

    KoDocument* document = m_chain->outputDocument();
    if ( !document )
        return KoFilter::StupidError;

    kdDebug(30523) << "here we go... " << document->className() << endl;

    if( !::qt_cast<const KSpread::Doc *>( document ) )  // it's safer that way :)
    {
        kdWarning(30501) << "document isn't a KSpread::Doc but a " << document->className() << endl;
        return KoFilter::NotImplemented;
    }
    if(from!="application/x-quattropro" || to!="application/x-kspread")
    {
        kdWarning(30501) << "Invalid mimetypes " << from << " " << to << endl;
        return KoFilter::NotImplemented;
    }

    kdDebug(30523) << "...still here..." << endl;

    // No need for a dynamic cast here, since we use Qt's moc magic
    Doc *ksdoc=(Doc*)document;

    if(ksdoc->mimeType()!="application/x-kspread")
    {
        kdWarning(30501) << "Invalid document mimetype " << ksdoc->mimeType() << endl;
        return KoFilter::NotImplemented;
    }

    QpIStream lIn( QFile::encodeName(m_chain->inputFile()) );

    if( !lIn )
    {
        KMessageBox::sorry( 0L, i18n("QPRO filter cannot open input file - please report.") );
        return KoFilter::FileNotFound;
    }

    Sheet *table=0;

    QString field;
    int value=0;
    emit sigProgress(value);

   QpRecFactory            lFactory(lIn);
   QpTableList             lTableNames;
   QP_UINT8                lPageIdx = 0;

   QpRec*                  lRec = 0;
   QpRecBop*               lRecBop = 0;
   QpRecIntegerCell*       lRecInt = 0;
   QpRecFloatingPointCell* lRecFloat = 0;
   QpRecFormulaCell*       lRecFormula = 0;
   QpRecLabelCell*         lRecLabel = 0;
   QpRecPageName*          lRecPageName = 0;

   do
   {
      field = "";
      lRec  = lFactory.nextRecord();

      switch( lRec->type() )
      {
      case QpBop:
         lRecBop = (QpRecBop*)lRec;
         lPageIdx = lRecBop->pageIndex();

         // find out if we know about this table already, if not create it
         table=lTableNames.table(lPageIdx);

         if( table == 0 )
         {
            table=ksdoc->map()->addNewSheet();
            // set up a default name for the table
            table->setSheetName( lTableNames.name(lPageIdx)
                               , TRUE
                               );
            lTableNames.table(lPageIdx, table);
         }
         break;

      case QpIntegerCell:
         lRecInt = (QpRecIntegerCell*)lRec;
         field.setNum( lRecInt->integer() );
//cout << "Setting R " << lRecInt->row()+1 << ", C " << ((unsigned)lRecInt->column()) << endl;
         table->setText( lRecInt->row()+1, ((unsigned)lRecInt->column())+1, field, false );
         break;

      case QpFormulaCell:
         lRecFormula = (QpRecFormulaCell*)lRec;
         {
           Formula lAnswer(*lRecFormula, lTableNames);

            char*     lFormula = lAnswer.formula();

            field = lFormula;

            delete [] lFormula;
         }

         // check for referenced tables that haven't been created yet
         for(unsigned lIdx=0; lIdx<lTableNames.cNameCnt; ++lIdx)
         {
            if(lTableNames.allocated(lIdx) && (lTableNames.table(lIdx) == 0) )
            {
               // we're about to reference a table that hasn't been created yet.
               // setText gets upset about this, so create a blank table

               Sheet* lNewTable=ksdoc->map()->addNewSheet();

               // set up a default name for the table
               lNewTable->setSheetName( lTableNames.name(lIdx)
                                      , TRUE
                                      );
               lTableNames.table(lIdx, lNewTable);
            }
         }

         table->setText( lRecFormula->row()+1, lRecFormula->column()+1, field, false );
         break;

      case QpFloatingPointCell:
         lRecFloat = (QpRecFloatingPointCell*)lRec;
         field.setNum( lRecFloat->value() );
         table->setText( lRecFloat->row()+1, lRecFloat->column()+1, field, false );
         break;

      case QpLabelCell:
         lRecLabel = (QpRecLabelCell*)lRec;
         field = "'";
         field += lRecLabel->label();
         table->setText( lRecLabel->row()+1, lRecLabel->column()+1, field, false );
         break;

      case QpPageName:
         lRecPageName = (QpRecPageName*)lRec;

         if( lTableNames.allocated(lPageIdx) && lTableNames.table(lPageIdx) )
         {
            lTableNames.table(lPageIdx)->setSheetName( lRecPageName->pageName()
//                                                     , TRUE
                                                     );
            lTableNames.name(lPageIdx, lRecPageName->pageName());
         }
         break;

      case QpPassword:
        KMessageBox::sorry( 0L, i18n("Unable to open password protected files.\n"
                                     "The password algorithm has not been published")
                          );
        return KoFilter::NotImplemented;
      }

      delete lRec;
      lRec = 0;
   } while( lIn );

    emit sigProgress(100);
    if ( bSuccess )
        return KoFilter::OK;
    else
        return KoFilter::StupidError;
}

#include <qproimport.moc>
