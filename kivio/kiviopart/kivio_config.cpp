/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "kivio_config.h"
#include "kivio_view.h"
#include "kiviooptionsdialog.h"

#include "tkunits.h"

#include <qfile.h>
#include <qtextstream.h>

#include <kiconloader.h>
#include <kdebug.h>
#include <kstandarddirs.h>

KivioOptions::KivioOptions()
{
  initGlobalConfig();
  initDefaultConfig();
}

KivioOptions::~KivioOptions()
{
  saveGlobalConfig();
}

void KivioOptions::initGlobalConfig()
{
  QDomDocument* doc = new QDomDocument("GlobalConfig");
  QDomElement root;

  QString path = locateLocal("appdata", "globalconfig");
  QFile f(path);
  if ( !f.open(IO_ReadOnly) ) {
    globalDefPageLayout.setDefault();
    globalDefStencilBarVisual.setDefault();
  } else {
    doc->setContent(&f);
    root = doc->documentElement();

    QDomElement ple = root.namedItem("PaperLayout").toElement();
    globalDefPageLayout.load(ple);

    QDomElement sbe = root.namedItem("StencilsBar").toElement();
    globalDefStencilBarVisual.load(sbe);
    setGlobalStencilsBarVisual(globalDefStencilBarVisual);
  }

  delete doc;
}

void KivioOptions::initDefaultConfig()
{
  defPageLayout = globalDefPageLayout;
}

void KivioOptions::paperLayoutSetup(KivioView* view)
{
  KivioOptionsDialog dlg(view, KivioOptionsDialog::PageSize);
  dlg.exec();
}

void KivioOptions::setup(KivioView* view)
{
  KivioOptionsDialog dlg(view);
  dlg.exec();
}

void KivioOptions::saveGlobalConfig()
{
  QDomDocument* doc = new QDomDocument("GlobalConfig");
  QDomElement root = doc->createElement("GlobalConfig");
  doc->appendChild(root);

  QDomElement ple = doc->createElement("PaperLayout");
  root.appendChild(ple);
  globalDefPageLayout.save(ple);

  QDomElement sbe = doc->createElement("StencilsBar");
  root.appendChild(sbe);
  globalDefStencilBarVisual.save(sbe);

  QString path = locateLocal("appdata", "globalconfig");
  QFile f(path);
  QTextStream ts(&f);
  if (f.open(IO_WriteOnly)) {
    ts << *doc;
    f.close();
  }
  delete doc;
}

void KivioOptions::setGlobalStencilsBarVisual(KivioIconViewVisual v)
{
  globalDefStencilBarVisual = v;
  globalDefStencilBarVisual.init();
  KivioIconView::setVisualData(globalDefStencilBarVisual);
  saveGlobalConfig();
}

void KivioOptions::setDefaultPageLayout(const TKPageLayout &pl)
{
  defPageLayout = pl;
}

void KivioOptions::setGlobalDefaultPageLayout(const TKPageLayout &pl)
{
  globalDefPageLayout = pl;
  saveGlobalConfig();
}

void KivioOptions::save(QDomElement& element)
{
  QDomElement e = element.ownerDocument().createElement("DefPaperLayout");
  element.appendChild(e);
  defPageLayout.save(e);
}

void KivioOptions::load(const QDomElement& element)
{
  QDomElement ple = element.namedItem("DefPaperLayout").toElement();
  defPageLayout.load(ple);
}
/**********************************************************************************/
static const char * connectorTarget_xpm[] = {
"7 7 3 1",
"       c None",
".      c #FFFFFF",
"+      c #051EFF",
" .   . ",
".+. .+.",
" .+.+. ",
"  .+.  ",
" .+.+. ",
".+. .+.",
" .   . "};



static const char * lock_xpm[] = {
"10 10 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"    ..    ",
"   .++.   ",
"  .+..+.  ",
"  .+..+.  ",
" .++++++. ",
" .+....+. ",
" .+.  .+. ",
" .+....+. ",
" .++++++. ",
"  ......  "};


KivioConfig *KivioConfig::s_config=0L;


/**
 * Constructs a config file from a filename
 *
 * @param fileName The file to read from
 *
 * This creates/reads a config file.  It sets default values
 * if certain values aren't found.
 */
KivioConfig::KivioConfig( const QString &fileName )
    : KSimpleConfig( fileName, false )
{
   kdDebug() << "KivioConfig - created" << endl;

    QColor *defColor = new QColor(0x4BD2FF/* 154, 250, 154 */);
    m_stencilBGType  = (KivioConfig::StencilBGType)readNumEntry( "StencilBackgroundType", (int)sbgtColor );
    m_stencilBGFile  = readEntry( "StencilBackgroundFile", "" );
    m_stencilBGColor = readColorEntry( "StencilBackgroundColor", defColor );
    delete defColor;

    m_pStencilBGPixmap = NULL;

    // Load the stencil bg pixmap if applicable.  If it doesn't load properly, then
    // set the type to color and delete the pixmap.
    if( m_stencilBGType == sbgtPixmap )
    {
        m_pStencilBGPixmap = new QPixmap( BarIcon( m_stencilBGFile ) );
        if( !m_pStencilBGPixmap || m_pStencilBGPixmap->isNull() )
        {
            if( m_pStencilBGPixmap )
            {
                delete m_pStencilBGPixmap;
                m_pStencilBGPixmap = NULL;
            }
            m_stencilBGType = sbgtColor;
            kdDebug() << "KivioConfig - Unable to load " << m_stencilBGFile << endl;
        }
        else
        {
	   kdDebug() << "KivioConfig::KivioConfig() - loaded background" << endl;
        }
    }

    m_pConnectorTargetPixmap = new QPixmap( (const char **)connectorTarget_xpm);
    m_pLockPixmap = new QPixmap( (const char **)lock_xpm);

    kdDebug() << "KivioConfig::KivioConfig() - StencilBackgroundType: " <<  m_stencilBGType << endl;
    kdDebug() << "KivioConfig::KivioConfig() - StencilBackgroundFile: " << m_stencilBGFile << endl;
    kdDebug() << "KivioConfig::KivioConfig() - StencilBackgroundColor: " << m_stencilBGColor.red()
	      << " " << m_stencilBGColor.green() << " " <<  m_stencilBGColor.blue() << endl;

    writeConfig();
}


/**
 * Destructor
 *
 * Deletes the background pixmap if applicable.  Also deletes the
 * pixmap which represents the connector targets.
 */
KivioConfig::~KivioConfig()
{
    if( m_pStencilBGPixmap )
    {
        delete m_pStencilBGPixmap;
        m_pStencilBGPixmap = NULL;
    }

    if( m_pConnectorTargetPixmap )
    {
        delete m_pConnectorTargetPixmap;
        m_pConnectorTargetPixmap = NULL;
    }

    if( m_pLockPixmap )
    {
       delete m_pLockPixmap;
       m_pLockPixmap = NULL;
    }

    kdDebug() << "KivioConfig::~KivioConfig() - deleted" << endl;
}


/**
 * Writes the KivioConfig to file.
 */
void KivioConfig::writeConfig()
{
    writeEntry( "StencilBackgroundType", QString::number((int)m_stencilBGType) );
    writeEntry( "StencilBackgroundFile", m_stencilBGFile );
    writeEntry( "StencilBackgroundColor", m_stencilBGColor.name() );

    sync();
}


/**
 * Destroys the static KivioConfig object.
 *
 * This function is called when the application is shutting down so
 * that all allocated memory is released.
 */
bool KivioConfig::deleteConfig()
{
    if( !KivioConfig::s_config )
        return false;

    delete KivioConfig::s_config;
    KivioConfig::s_config = NULL;

    return true;
}


/**
 * Reads the configuration into a static variable.
 *
 * This is called by the application during initialization so
 * the static variable s_config gets created.
 */
bool KivioConfig::readUserConfig()
{
    // Avoid double read
    if( s_config )
        return false;

    s_config = new KivioConfig( "kivio.conf" );


    return true;
}
#include "kivio_config.moc"
