/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KCELLS_SPELLCHECK_COMMAND
#define KCELLS_SPELLCHECK_COMMAND

#include <sonnet/backgroundchecker.h>

class KCanvasBase;

class KCRegion;

/**
 * \ingroup Commands
 * \brief Spellchecks a cell range.
 */
class KCSpellCheckCommand : public Sonnet::BackgroundChecker
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    KCSpellCheckCommand(const KCRegion &region, KCanvasBase* canvasBase);

    /**
     * Destructor.
     */
    ~KCSpellCheckCommand();

protected:
    virtual QString fetchMoreText();
    virtual void finishedCurrentFeed();

private Q_SLOTS:
    void finishCommand();

private:
    class Private;
    Private * const d;
};

#endif // KCELLS_SPELLCHECK_COMMAND
