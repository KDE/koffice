/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <limits.h>

#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <config.h>

#include LCMS_HEADER

#include "kis_global.h"
#include "kis_config.h"

namespace {
    const double IMG_DEFAULT_RESOLUTION = 100.0;
    const Q_INT32 IMG_DEFAULT_WIDTH = 512;
    const Q_INT32 IMG_DEFAULT_HEIGHT = 512;
    const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_OUTLINE;
    const Q_INT32 DEFAULT_MAX_THREADS = 4;
    const Q_INT32 DEFAULT_MAX_TILES_MEM = 500; // 8192 kilobytes given 64x64 tiles with 32bpp
    const Q_INT32 DEFAULT_SWAPPINESS = 100;
    const Q_INT32 DEFAULT_PRESSURE_CORRECTION = 50;
    const Q_INT32 DEFAULT_DOCKABILITY = 0;
    const Q_INT32 DEFAULT_UNDO_LIMIT = 50;
}

KisConfig::KisConfig()
{

    m_cfg = KGlobal::config();
    if (!m_cfg) {
        // Allow unit tests to test parts of the code without having to run the
        // full application.
        m_cfg = new KConfig();
    }
    m_cfg->setGroup("");
}

KisConfig::~KisConfig()
{
    m_cfg->sync();
}


bool KisConfig::fixDockerWidth() const
{
    return m_cfg->readBoolEntry("fixDockerWidth", true);
}

void KisConfig::setFixedDockerWidth(bool fix)
{
    m_cfg->writeEntry("fixDockerWidth", fix);
}

bool KisConfig::undoEnabled() const
{
    return m_cfg->readBoolEntry("undoEnabled", true);
}

void KisConfig::setUndoEnabled(bool undo)
{
    m_cfg->writeEntry("undoEnabled", undo);
}


Q_INT32 KisConfig::defUndoLimit() const
{
    return m_cfg->readNumEntry("undolimit", DEFAULT_UNDO_LIMIT);
}

void KisConfig::defUndoLimit(Q_INT32 limit)
{
    m_cfg->writeEntry("undolimit", limit);
}

Q_INT32 KisConfig::defImgWidth() const
{
    return m_cfg->readNumEntry("imgWidthDef", IMG_DEFAULT_WIDTH);
}

Q_INT32 KisConfig::defImgHeight() const
{
    return m_cfg->readNumEntry("imgHeightDef", IMG_DEFAULT_HEIGHT);
}

double KisConfig::defImgResolution() const
{
    return m_cfg->readDoubleNumEntry("imgResolutionDef", IMG_DEFAULT_RESOLUTION);
}

void KisConfig::defImgWidth(Q_INT32 width)
{
    m_cfg->writeEntry("imgWidthDef", width);
}

void KisConfig::defImgHeight(Q_INT32 height)
{
    m_cfg->writeEntry("imgHeightDef", height);
}

void KisConfig::defImgResolution(double res)
{
    m_cfg->writeEntry("imgResolutionDef", res);
}

enumCursorStyle KisConfig::cursorStyle() const
{
    return (enumCursorStyle) m_cfg->readNumEntry("cursorStyleDef", DEFAULT_CURSOR_STYLE);
}

enumCursorStyle KisConfig::getDefaultCursorStyle() const
{
    return DEFAULT_CURSOR_STYLE;
}

void KisConfig::setCursorStyle(enumCursorStyle style)
{
    m_cfg->writeEntry("cursorStyleDef", style);
}


QString KisConfig::monitorProfile() const
{
    return m_cfg->readEntry("monitorProfile", "");
}

void KisConfig::setMonitorProfile(QString monitorProfile)
{
    m_cfg->writeEntry("monitorProfile", monitorProfile);
}


QString KisConfig::workingColorSpace() const
{
    return m_cfg->readEntry("workingColorSpace", "RGBA");
}

void KisConfig::setWorkingColorSpace(QString workingColorSpace)
{
    m_cfg->writeEntry(workingColorSpace, workingColorSpace);
}


QString KisConfig::printerColorSpace() const
{
    return m_cfg->readEntry("printerColorSpace", "CMYK");
}

void KisConfig::setPrinterColorSpace(QString printerColorSpace)
{
    m_cfg->writeEntry("printerColorSpace", printerColorSpace);
}


QString KisConfig::printerProfile() const
{
    return m_cfg->readEntry("printerProfile", "");
}

void KisConfig::setPrinterProfile(QString printerProfile)
{
    m_cfg->writeEntry("printerProfile", printerProfile);
}


bool KisConfig::useBlackPointCompensation() const
{
    return m_cfg->readBoolEntry("useBlackPointCompensation", false);
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation)
{
    m_cfg->writeEntry("useBlackPointCompensation", useBlackPointCompensation);
}


bool KisConfig::showRulers() const
{
    return m_cfg->readBoolEntry("showrulers", false);
}

void KisConfig::setShowRulers(bool rulers)
{
    m_cfg->writeEntry("showrulers", rulers);
}


Q_INT32 KisConfig::pasteBehaviour() const
{
    return m_cfg->readNumEntry("pasteBehaviour", 2);
}

void KisConfig::setPasteBehaviour(Q_INT32 renderIntent)
{
    m_cfg->writeEntry("pasteBehaviour", renderIntent);
}


Q_INT32 KisConfig::renderIntent() const
{
    return m_cfg->readNumEntry("renderIntent", INTENT_PERCEPTUAL);
}

void KisConfig::setRenderIntent(Q_INT32 renderIntent)
{
    m_cfg->writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL() const
{
    return m_cfg->readBoolEntry("useOpenGL", false);
}

void KisConfig::setUseOpenGL(bool useOpenGL)
{
    m_cfg->writeEntry("useOpenGL", useOpenGL);
}

bool KisConfig::useOpenGLShaders() const
{
    return m_cfg->readBoolEntry("useOpenGLShaders", false);
}

void KisConfig::setUseOpenGLShaders(bool useOpenGLShaders)
{
    m_cfg->writeEntry("useOpenGLShaders", useOpenGLShaders);
}

Q_INT32 KisConfig::maxNumberOfThreads()
{
    return m_cfg->readNumEntry("maxthreads", DEFAULT_MAX_THREADS);
}

void KisConfig::setMaxNumberOfThreads(Q_INT32 maxThreads)
{
    m_cfg->writeEntry("maxthreads", maxThreads);
}

Q_INT32 KisConfig::maxTilesInMem() const
{
    return m_cfg->readNumEntry("maxtilesinmem", DEFAULT_MAX_TILES_MEM);
}

void KisConfig::setMaxTilesInMem(Q_INT32 tiles)
{
    m_cfg->writeEntry("maxtilesinmem", tiles);
}

Q_INT32 KisConfig::swappiness() const
{
    return m_cfg->readNumEntry("swappiness", DEFAULT_SWAPPINESS);
}

void KisConfig::setSwappiness(Q_INT32 swappiness)
{
    m_cfg->writeEntry("swappiness", swappiness);
}

Q_INT32 KisConfig::getPressureCorrection()
{
    return m_cfg->readNumEntry( "pressurecorrection", DEFAULT_PRESSURE_CORRECTION );
}

void KisConfig::setPressureCorrection( Q_INT32 correction )
{
    m_cfg->writeEntry( "pressurecorrection",  correction );
}

Q_INT32 KisConfig::getDefaultPressureCorrection()
{
    return DEFAULT_PRESSURE_CORRECTION;
}

bool KisConfig::tabletDeviceEnabled(const QString& tabletDeviceName) const
{
    return m_cfg->readBoolEntry("TabletDevice" + tabletDeviceName + "Enabled", false);
}

void KisConfig::setTabletDeviceEnabled(const QString& tabletDeviceName, bool enabled)
{
    m_cfg->writeEntry("TabletDevice" + tabletDeviceName + "Enabled", enabled);
}

Q_INT32 KisConfig::tabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, Q_INT32 defaultAxis) const
{
    return m_cfg->readNumEntry("TabletDevice" + tabletDeviceName + axisName, defaultAxis);
}

void KisConfig::setTabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, Q_INT32 axis) const
{
    m_cfg->writeEntry("TabletDevice" + tabletDeviceName + axisName, axis);
}

void KisConfig::setDockability( Q_INT32 dockability )
{
    m_cfg->writeEntry( "palettesdockability", dockability );
}

Q_INT32 KisConfig::dockability()
{
    return m_cfg->readNumEntry("palettesdockability", DEFAULT_DOCKABILITY);
}

Q_INT32 KisConfig::getDefaultDockability()
{
    return DEFAULT_DOCKABILITY;
}

float KisConfig::dockerFontSize()
{
    return (float) m_cfg->readNumEntry("palettefontsize", (int)getDefaultDockerFontSize());
}

float KisConfig::getDefaultDockerFontSize()
{
    float ps = QMIN(9, KGlobalSettings::generalFont().pointSize() * 0.8);
    if (ps < 6) ps = 6;
    return ps;
}

void KisConfig::setDockerFontSize(float size)
{
    m_cfg->writeEntry("palettefontsize", size);
}

Q_UINT32 KisConfig::getGridMainStyle()
{
    Q_UINT32 v = m_cfg->readNumEntry("gridmainstyle", 0);
    if (v > 2)
        v = 2;
    return v;
}

void KisConfig::setGridMainStyle(Q_UINT32 v)
{
    m_cfg->writeEntry("gridmainstyle", v);
}

Q_UINT32 KisConfig::getGridSubdivisionStyle()
{
    Q_UINT32 v = m_cfg->readNumEntry("gridsubdivisionstyle", 1);
    if (v > 2) v = 2;
    return v;
}

void KisConfig::setGridSubdivisionStyle(Q_UINT32 v)
{
    m_cfg->writeEntry("gridsubdivisionstyle", v);
}

QColor KisConfig::getGridMainColor()
{
    return m_cfg->readColorEntry("gridmaincolor", new QColor(99,99,99));
}

void KisConfig::setGridMainColor(QColor v)
{
    m_cfg->writeEntry("gridmaincolor", v);
}

QColor KisConfig::getGridSubdivisionColor()
{
    return m_cfg->readColorEntry("gridsubdivisioncolor", new QColor(150,150,150));
}

void KisConfig::setGridSubdivisionColor(QColor v)
{
    m_cfg->writeEntry("gridsubdivisioncolor", v);
}

Q_UINT32 KisConfig::getGridHSpacing()
{
    Q_INT32 v = m_cfg->readNumEntry("gridhspacing", 10);
    return (Q_UINT32)QMAX(1, v );
}

void KisConfig::setGridHSpacing(Q_UINT32 v)
{
    m_cfg->writeEntry("gridhspacing", v);
}

Q_UINT32 KisConfig::getGridVSpacing()
{
    Q_INT32 v = m_cfg->readNumEntry("gridvspacing", 10);
    return (Q_UINT32)QMAX(1, v );
}

void KisConfig::setGridVSpacing(Q_UINT32 v)
{
    m_cfg->writeEntry("gridvspacing", v);
}

Q_UINT32 KisConfig::getGridSubdivisions()
{
    Q_INT32 v = m_cfg->readNumEntry("gridsubsivisons", 2);
    return (Q_UINT32)QMAX(1, v );
}

void KisConfig::setGridSubdivisions(Q_UINT32 v)
{
    return m_cfg->writeEntry("gridsubsivisons", v);
}

Q_UINT32 KisConfig::getGridOffsetX()
{
    Q_INT32 v = m_cfg->readNumEntry("gridoffsetx", 0);
    return (Q_UINT32)QMAX(0, v );
}

void KisConfig::setGridOffsetX(Q_UINT32 v)
{
    m_cfg->writeEntry("gridoffsetx", v);
}

Q_UINT32 KisConfig::getGridOffsetY()
{
    Q_INT32 v = m_cfg->readNumEntry("gridoffsety", 0);
    return (Q_UINT32)QMAX(0, v );
}

void KisConfig::setGridOffsetY(Q_UINT32 v)
{
    m_cfg->writeEntry("gridoffsety", v);
}

