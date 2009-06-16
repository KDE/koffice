/*
Copyright 2008  Peter Simonsson <peter.simonsson@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "KoZoomInput.h"

#include <kdebug.h>
#include <klocale.h>

#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPalette>
#include <QAbstractItemView>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>

class KoZoomInput::Private
{
    public:
        QComboBox* combo;
        QLabel* label;
        bool inside;
};

KoZoomInput::KoZoomInput(QWidget* parent)
    : QStackedWidget(parent), d(new Private)
{

#ifdef Q_WS_MAC
    setAttribute(Qt::WA_MacMiniSize, true);
#endif

    setToolTip(i18n("Zoom"));

    QWidget* first = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(first);
    layout->setSpacing(0);
    layout->setMargin(0);
    d->label = new QLabel(first);
    d->label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    layout->addWidget(d->label, 10);
    QLabel* icon = new QLabel(first);
    QStyleOption option;
    option.state = QStyle::State_Enabled;
    QPixmap pixmap(16, 16);
    pixmap.fill(QColor(255, 255, 255, 0));
    QPainter painter(&pixmap);
    painter.translate(8, 8);
    style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &option, &painter);
    icon->setPixmap(pixmap);
    layout->addWidget(icon);
    addWidget(first);
    d->combo = new QComboBox(this);
    d->combo->setMaxVisibleItems(15);
    d->combo->setEditable(true);
    d->combo->installEventFilter(this);
    addWidget(d->combo);
    d->inside = false;

    connect(d->combo, SIGNAL(activated(const QString&)), this, SIGNAL(zoomLevelChanged(const QString&)));
}

KoZoomInput::~KoZoomInput()
{
    delete d;
}

void KoZoomInput::enterEvent(QEvent* event)
{
    Q_UNUSED(event);

    d->inside = true;
    if (d->combo->view())
    {
        d->combo->view()->removeEventFilter(this);
    }

    setCurrentIndex(1);
}

void KoZoomInput::leaveEvent(QEvent* event)
{
    Q_UNUSED(event);

    d->inside = false;
    if(d->combo->view() && d->combo->view()->isVisible())
    {
        d->combo->view()->installEventFilter(this);
        return;
    }

    if(!d->combo->hasFocus())
        setCurrentIndex(0);
}

void KoZoomInput::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return ||
        event->key() == Qt::Key_Enter)
    {
        focusNextChild();
    }
}

void KoZoomInput::setZoomLevels(const QStringList& levels)
{
    d->combo->clear();
    d->combo->addItems(levels);
}

void KoZoomInput::setCurrentZoomLevel(const QString& level)
{
    d->combo->setCurrentIndex(d->combo->findText(level));
    d->label->setText(level);
}

bool KoZoomInput::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == d->combo->view() && event->type() == QEvent::Hide)
    {
        focusNextChild();
        setCurrentIndex(0);
    }
    else if (watched == d->combo && event->type() == QEvent::FocusOut &&
            (d->combo->view() && !d->combo->view()->hasFocus()) && !d->inside)
    {
        setCurrentIndex(0);
    }
    return false;
}

#include "KoZoomInput.moc"
