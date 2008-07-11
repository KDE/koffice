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

#include "HyperlinkStrategy.h"

#include "Selection.h"
#include "Sheet.h"
#include "Util.h"

#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoTool.h>

#include <KMessageBox>
#include <KMimeType>
#include <KNotification>
#include <KRun>

#include <QTimer>

using namespace KSpread;

class HyperlinkStrategy::Private
{
public:
    QPointF lastPoint;
    QRectF textRect;
    QString url;
};

HyperlinkStrategy::HyperlinkStrategy(KoTool* parent, KoCanvasBase* m_canvas, Selection* selection,
                                     const QPointF documentPos, Qt::KeyboardModifiers modifiers,
                                     const QString& url, const QRectF& textRect)
    : AbstractSelectionStrategy(parent, m_canvas, selection, documentPos, modifiers)
    , d(new Private)
{
    d->lastPoint = documentPos;
    d->textRect = textRect;
    d->url = url;
}

HyperlinkStrategy::~HyperlinkStrategy()
{
    delete d;
}

void HyperlinkStrategy::handleMouseMove(const QPointF& documentPos, Qt::KeyboardModifiers modifiers)
{
    d->lastPoint = documentPos;
    if (d->textRect.contains(documentPos)) {
        return;
    }
    AbstractSelectionStrategy::handleMouseMove(documentPos, modifiers);
}

void HyperlinkStrategy::finishInteraction(Qt::KeyboardModifiers modifiers)
{
    if (d->textRect.contains(d->lastPoint)) {
        return;
    }
    Q_UNUSED(modifiers)
    KNotification* const notify = new KNotification("LinkActivated");
    notify->setText(i18n("Link <i>%1</i> activated", d->url));
    notify->addContext("d->url", d->url);

    const KUrl url(d->url);
    if (!url.isValid() || url.isRelative()) {
        const Region region(d->url, selection()->activeSheet()->map(), selection()->activeSheet());
        if (region.isValid()) {
            if (region.firstSheet() != selection()->activeSheet()) {
                selection()->emitVisibleSheetRequested(region.firstSheet());
            }
            selection()->initialize(region);

            if (!region.firstRange().isNull()) {
                const Cell cell = Cell(region.firstSheet(), region.firstRange().topLeft());
                notify->addContext("region", cell.fullName());
            }
        }
    } else {
        const QString type = KMimeType::findByUrl(url, 0, url.isLocalFile())->name();
        notify->addContext("type", type);
        if (!Util::localReferenceAnchor(d->url)) {
            const bool executable = KRun::isExecutableFile(url, type);
            notify->addContext("executable", QVariant(executable).toString());
            if (executable) {
                const QString question = i18n("This link points to the program or script '%1'.\n"
                                              "Malicious programs can harm your computer. "
                                              "Are you sure that you want to run this program?", d->url);
                // this will also start local programs, so adding a "don't warn again"
                // checkbox will probably be too dangerous
                const int answer = KMessageBox::warningYesNo(m_canvas->canvasWidget(), question,
                                                                i18n("Open Link?"));
                if (answer != KMessageBox::Yes) {
                    return;
                }
            }
            new KRun(url, m_canvas->canvasWidget(), 0, url.isLocalFile());
        }
    }

    QTimer::singleShot(0, notify, SLOT(sendEvent()));
    m_parent->repaintDecorations();
}
