/* This file is part of the KDE project
   Original file (serialletter.cc): Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

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

#include "serialletter_classicplugin.h"
#include "serialletter_classicplugin.moc"
#include "serialletter.h"

/******************************************************************
 *
 * Class: KWClassicSerialDataSource
 *
 ******************************************************************/

KWClassicSerialDataSource::KWClassicSerialDataSource( /*KWDocument *doc_ */)
	: KWSerialLetterDataSource()
//    : doc( doc_ )
{
}

KWClassicSerialDataSource::~KWClassicSerialDataSource( /*KWDocument *doc_ */)
{
}

QString KWClassicSerialDataSource::getValue( const QString &name, int record ) const
{
    int num = record;
/*    if ( num == -1 )
        num = doc->getSerialLetterRecord(); Is this really needed ?*/

    if ( num < 0 || num > (int)db.count() )
        return name;

    return db[ num ][ name ];
}

void KWClassicSerialDataSource::setValue( const QString &name, const QString &value, int record )
{
    int num = record;
/*    if ( num == -1 )
        num = doc->getSerialLetterRecord(); Is this really needed?*/

    if ( num < 0 || num > (int)db.count() )
        return;

    db[ num ][ name ] = value;
}

void KWClassicSerialDataSource::appendRecord()
{
    DbRecord record( sampleRecord );
    db.append( record );
}

void KWClassicSerialDataSource::addEntry( const QString &name )
{
    sampleRecord[ name ] = i18n( "No Value" );
    Db::Iterator it = db.begin();
    for ( ; it != db.end(); ++it )
        ( *it )[ name ] = sampleRecord[ name ];
}

void KWClassicSerialDataSource::removeEntry( const QString &name )
{
    sampleRecord.remove( name );
    Db::Iterator it = db.begin();
    for ( ; it != db.end(); ++it )
        ( *it ).remove( name );
}

void KWClassicSerialDataSource::removeRecord( int i )
{
    if ( i < 0 || i > (int)db.count() - 1 )
        return;

    Db::Iterator it = db.at( i );
    db.remove( it );
}

void KWClassicSerialDataSource::save( QDomElement& /*parentElem*/ )
{
#if 0
    out << otag << "<SAMPLE>" << endl;

    DbRecord::Iterator it = sampleRecord.begin();
    for ( ; it != sampleRecord.end(); ++it )
        out << indent << "<ENTRY key=\"" << correctQString( it.key() )
            << "\" value=\"" << correctQString( *it ) << "\"/>" << endl;

    out << etag << "</SAMPLE>" << endl;

    out << otag << "<DB>" << endl;
    Db::Iterator it2 = db.begin();
    for ( ; it2 != db.end(); ++it2 ) {
        out << otag << "<RECORD>" << endl;
        it = ( *it2 ).begin();
        for ( ; it != ( *it2 ).end(); ++it ) {
            out << indent << "<ENTRY key=\"" << correctQString( it.key() )
                << "\" value=\"" << correctQString( *it ) << "\"/>" << endl;
        }
        out << etag << "</RECORD>" << endl;
    }
    out << etag << "</DB>" << endl;
#endif
}

void KWClassicSerialDataSource::load( QDomElement& /*elem*/ )
{
    db.clear();
    sampleRecord.clear();

#if 0
    QString tag;
    QString name;

    while ( parser.open( QString::null, tag ) ) {
        parser.parseTag( tag, name, lst );

        if ( name == "SAMPLE" ) {
            parser.parseTag( tag, name, lst );
            while ( parser.open( QString::null, tag ) ) {
                parser.parseTag( tag, name, lst );
                if ( name == "ENTRY" ) {
                    parser.parseTag( tag, name, lst );
                    QValueList<KOMLAttrib>::ConstIterator it = lst.begin();
                    for( ; it != lst.end(); ++it ) {
                        if ( ( *it ).m_strName == "key" )
                            addEntry( ( *it ).m_strValue );
                    }
                } else
                    kdError(32001) << "Unknown tag '" << name << "' in SAMPLE" << endl;

                if ( !parser.close( tag ) ) {
                    kdError(32001) << "Closing " << tag << endl;
                    return;
                }
            }
        } else if ( name == "DB" ) {
            parser.parseTag( tag, name, lst );
            while ( parser.open( QString::null, tag ) ) {
                parser.parseTag( tag, name, lst );
                if ( name == "RECORD" ) {
                    parser.parseTag( tag, name, lst );
                    appendRecord();
                    while ( parser.open( QString::null, tag ) ) {
                        parser.parseTag( tag, name, lst );
                        if ( name == "ENTRY" ) {
                            parser.parseTag( tag, name, lst );
                            QValueList<KOMLAttrib>::ConstIterator it = lst.begin();
                            QString key;
                            for( ; it != lst.end(); ++it ) {
                                if ( ( *it ).m_strName == "key" )
                                    key = ( *it ).m_strValue;
                                else if ( ( *it ).m_strName == "value" )
                                    setValue( key, ( *it ).m_strValue, db.count() - 1 );
                            }
                        } else
                            kdError(32001) << "Unknown tag '" << name << "' in RECORD" << endl;

                        if ( !parser.close( tag ) ) {
                            kdError(32001) << "Closing " << tag << endl;
                            return;
                        }
                    }
                } else
                    kdError(32001) << "Unknown tag '" << name << "' in DB" << endl;

                if ( !parser.close( tag ) ) {
                    kdError(32001) << "Closing " << tag << endl;
                    return;
                }
            }
        } else
            kdError(32001) << "Unknown tag '" << name << "' in SERIALL" << endl;

        if ( !parser.close( tag ) ) {
            kdError(32001) << "Closing " << tag << endl;
            return;
        }
    }
#endif
}

bool KWClassicSerialDataSource::showConfigDialog(QWidget *par,int action)
{
   if (action==KWSLCreate)
   {
   	db.clear();
   	sampleRecord.clear();
   }
   KWClassicSerialLetterEditor *dia=new KWClassicSerialLetterEditor( par, this );
   bool ret=(dia->exec()==QDialog::Accepted);
   delete dia;
   return ret;
}


/******************************************************************
 *
 * Class: KWClassicSerialLetterEditorListItem
 *
 ******************************************************************/

KWClassicSerialLetterEditorListItem::KWClassicSerialLetterEditorListItem( QListView *parent )
    : QListViewItem( parent )
{
    editWidget = new QLineEdit( listView()->viewport() );
    listView()->addChild( editWidget );
}

KWClassicSerialLetterEditorListItem::KWClassicSerialLetterEditorListItem( QListView *parent, QListViewItem *after )
    : QListViewItem( parent, after )
{
    editWidget = new QLineEdit( listView()->viewport() );
    listView()->addChild( editWidget );
}

KWClassicSerialLetterEditorListItem::~KWClassicSerialLetterEditorListItem()
{
    delete editWidget;
}

void KWClassicSerialLetterEditorListItem::setText( int i, const QString &text )
{
    QListViewItem::setText( i, text );
    if ( i == 1 )
        editWidget->setText( text );
}

QString KWClassicSerialLetterEditorListItem::text( int i ) const
{
    if ( i == 1 )
        return editWidget->text();
    return QListViewItem::text( i );
}

void KWClassicSerialLetterEditorListItem::setup()
{
    setHeight( QMAX( listView()->fontMetrics().height(),
                     editWidget->sizeHint().height() ) );
    if ( listView()->columnWidth( 1 ) < editWidget->sizeHint().width() )
        listView()->setColumnWidth( 1, editWidget->sizeHint().width() );
}

void KWClassicSerialLetterEditorListItem::update()
{
    editWidget->resize( listView()->header()->cellSize( 1 ), height() );
    listView()->moveChild( editWidget, listView()->header()->cellPos( 1 ),
                           listView()->itemPos( this ) + listView()->contentsY() );
    editWidget->show();
}

/******************************************************************
 *
 * Class: KWClassicSerialLetterEditorList
 *
 ******************************************************************/

KWClassicSerialLetterEditorList::KWClassicSerialLetterEditorList( QWidget *parent, KWClassicSerialDataSource *db_ )
    : QListView( parent ), db( db_ )
{
    setSorting( -1 );
    addColumn( i18n( "Name" ) );
    addColumn( i18n( "Value" ) );
    header()->setMovingEnabled( FALSE );
    connect( header(), SIGNAL( sizeChange( int, int, int ) ),
             this, SLOT( columnSizeChange( int, int, int ) ) );
//     connect( header(), SIGNAL( sectionClicked( int ) ),
//           this, SLOT( sectionClicked( int ) ) );
    disconnect( header(), SIGNAL( sectionClicked( int ) ),
                this, SLOT( changeSortColumn( int ) ) );

    currentRecord = -1;
}

KWClassicSerialLetterEditorList::~KWClassicSerialLetterEditorList()
{
    if ( currentRecord == -1 )
        return;

    QListViewItemIterator lit( this );
    QMap< QString, QString >::ConstIterator it = db->getRecordEntries().begin();
    for ( ; it != db->getRecordEntries().end(); ++it ) {
        QListViewItem *item = 0;
        item = lit.current();
        ++lit;
        if ( currentRecord != -1 && item )
            db->setValue( it.key(), item->text( 1 ), currentRecord );
    }
}

void KWClassicSerialLetterEditorList::columnSizeChange( int c, int, int )
{
    if ( c == 0 || c == 1 )
        updateItems();
}

void KWClassicSerialLetterEditorList::sectionClicked( int )
{
    updateItems();
}

void KWClassicSerialLetterEditorList::updateItems()
{
    QListViewItemIterator it( this );
    for ( ; it.current(); ++it )
        ( (KWClassicSerialLetterEditorListItem*)it.current() )->update();
}

void KWClassicSerialLetterEditorList::displayRecord( int i )
{
    if ( i < 0 || i >= db->getNumRecords() )
        return;

    bool create = !firstChild();
    QListViewItemIterator lit( this );
    QMap< QString, QString >::ConstIterator it = db->getRecordEntries().begin();
    QListViewItem *after = 0;
    for ( ; it != db->getRecordEntries().end(); ++it ) {
        QListViewItem *item = 0;
        if ( create ) {
            item = new KWClassicSerialLetterEditorListItem( this, after );
            item->setText( 0, it.key() );
            after = item;
        } else {
            item = lit.current();
            ++lit;
            if ( currentRecord != -1 && item )
                db->setValue( it.key(), item->text( 1 ), currentRecord );
        }

        if ( item )
            item->setText( 1, db->getValue( it.key(), i ) );
    }
    updateItems();
    currentRecord = i;
}

/******************************************************************
 *
 * Class: KWClassicSerialLetterEditor
 *
 ******************************************************************/

KWClassicSerialLetterEditor::KWClassicSerialLetterEditor( QWidget *parent, KWClassicSerialDataSource *db_ )
    : KDialogBase( Plain, i18n( "Serial Letter - Editor" ), Ok | Cancel, Ok, parent, "", true ), db( db_ )
{
    QWidget *page = plainPage();

    back = new QVBox( page );
    back->setSpacing( 5 );
    back->setMargin( 5 );

    QHBox *toolbar = new QHBox( back );

    first = new QToolButton( toolbar );
    first->setPixmap( BarIcon( "start" ) );
    first->setFixedSize( first->sizeHint() );
    connect(first, SIGNAL(clicked()), this, SLOT(firstRecord()));

    back_ = new QToolButton( toolbar );
    back_->setPixmap( BarIcon( "back" ) );
    back_->setFixedSize( back_->sizeHint() );
    connect(back_, SIGNAL(clicked()), this, SLOT(prevRecord()));

    records = new QSpinBox( 1, db->getNumRecords(), 1, toolbar );
    records->setMaximumHeight( records->sizeHint().height() );
    connect( records, SIGNAL( valueChanged( int ) ),
             this, SLOT( changeRecord( int ) ) );

    forward = new QToolButton( toolbar );
    forward->setPixmap( BarIcon( "forward" ) );
    forward->setFixedSize( forward->sizeHint() );
    connect(forward, SIGNAL(clicked()), this, SLOT(nextRecord()));

    finish = new QToolButton( toolbar );
    finish->setPixmap( BarIcon( "finish" ) );
    finish->setFixedSize( finish->sizeHint() );
    connect(finish, SIGNAL(clicked()), this, SLOT(lastRecord()));

    QWidget *sep = new QWidget( toolbar );
    sep->setMaximumWidth( 10 );

    newRecord = new QToolButton( toolbar );
    newRecord->setPixmap( KWBarIcon( "sl_addrecord" ) );
    newRecord->setFixedSize( newRecord->sizeHint() );
    connect( newRecord, SIGNAL( clicked() ),
             this, SLOT( addRecord() ) );
    QToolTip::add( newRecord, i18n( "Add Record" ) );

    newEntry = new QToolButton( toolbar );
    newEntry->setPixmap( KWBarIcon( "sl_addentry" ) );
    newEntry->setFixedSize( newEntry->sizeHint() );
    connect( newEntry, SIGNAL( clicked() ),
             this, SLOT( addEntry() ) );
    QToolTip::add( newEntry, i18n( "Add Entry" ) );

    deleteRecord = new QToolButton( toolbar );
    deleteRecord->setPixmap( KWBarIcon( "sl_delrecord" ) );
    deleteRecord->setFixedSize( deleteRecord->sizeHint() );
    connect( deleteRecord, SIGNAL( clicked() ),
             this, SLOT( removeRecord() ) );
    QToolTip::add( deleteRecord, i18n( "Remove Record" ) );

    deleteEntry = new QToolButton( toolbar );
    deleteEntry->setPixmap( KWBarIcon( "sl_delentry" ) );
    deleteEntry->setFixedSize( deleteEntry->sizeHint() );
    connect( deleteEntry, SIGNAL( clicked() ),
             this, SLOT( removeEntry() ) );
    QToolTip::add( deleteEntry, i18n( "Remove Entry" ) );

    dbList = new KWClassicSerialLetterEditorList( back, db );

    if ( db->getNumRecords() > 0 ) {
        records->setValue( 1 );
        dbList->updateItems();
    } else {
        first->setEnabled(false);
        back_->setEnabled(false);
        forward->setEnabled(false);
        finish->setEnabled(false);
        newRecord->setEnabled(false);
        deleteEntry->setEnabled(false);
        deleteRecord->setEnabled(false);
        records->setEnabled(true);
    }
    setInitialSize( QSize( 600, 400 ) );
}

void KWClassicSerialLetterEditor::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    back->resize( size() );
}

void KWClassicSerialLetterEditor::changeRecord( int i )
{
    dbList->displayRecord( i - 1 );
}

void KWClassicSerialLetterEditor::addEntry()
{
    KWVariableNameDia
        *dia = new KWVariableNameDia( this );
    if ( dia->exec() == QDialog::Accepted ) {
        if ( db->getNumRecords() == 0 ) {
            first->setEnabled(true);
            back_->setEnabled(true);
            forward->setEnabled(true);
            finish->setEnabled(true);
            newRecord->setEnabled(true);
            deleteEntry->setEnabled(true);
            deleteRecord->setEnabled(true);
            records->setEnabled(true);
            addRecord();
        }
        dbList->clear();
        db->addEntry( dia->getName() );
        changeRecord( records->value() );
        dbList->updateItems();
    }
    delete dia;
}

void KWClassicSerialLetterEditor::addRecord()
{
    db->appendRecord();
    records->setRange( records->minValue(), records->maxValue() + 1 );
    records->setValue( db->getNumRecords() );
}

void KWClassicSerialLetterEditor::removeEntry()
{

#warning reimplement
/*
    if ( db->getNumRecords() == 0 )
        return;

    KWSerialLetterVariableInsertDia
        *dia = new KWSerialLetterVariableInsertDia( this, db );
    if ( dia->exec() == QDialog::Accepted ) {
        dbList->clear();
        db->removeEntry( dia->getName() );
        changeRecord( records->value() + 1 );
        dbList->updateItems();
    }
    delete dia;
*/
}

void KWClassicSerialLetterEditor::removeRecord()
{
    if ( db->getNumRecords() == 0 )
        return;

    db->removeRecord( records->value() - 1 );
    if ( db->getNumRecords() > 0 ) {
        records->setRange( records->minValue(), records->maxValue() - 1 );
        records->setValue( 1 );
        dbList->updateItems();
    } else
        records->setEnabled( FALSE );
}

extern "C" {
	KWSerialLetterDataSource *create_kwserialletter_classic()
	{
		return new KWClassicSerialDataSource();
	}
}
