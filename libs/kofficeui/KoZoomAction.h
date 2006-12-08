/* This file is part of the KDE libraries
    Copyright (C) 2004 Ariya Hidayat <ariya@kde.org>
    Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef kozoomaction_h
#define kozoomaction_h

#include <kaction.h>
#include <koffice_export.h>
#include <kselectaction.h>
#include <KoZoomMode.h>

class QLineEdit;
class QSlider;
class QRadioButton;

/**
 * Class KoZoomAction implements an action to provide zoom values.
 * In a toolbar, KoZoomAction will show a dropdown list, also with 
 * the possibility for the user to enter arbritrary zoom value
 * (must be an integer). The values shown on the list are alwalys
 * sorted.
 * In a statusbar it provides a scale plus an editable value plus some buttons for special zoommodes
 */
class KOFFICEUI_EXPORT KoZoomAction : public KSelectAction
{
Q_OBJECT

public:

  /**
   * Creates a new zoom action.
   * @param zoomModes which zoom modes that should be shown
   */
  KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, const QIcon& pix,
    const KShortcut& cut = KShortcut(), KActionCollection* parent = 0, const char* name = 0 );

  /**
   * Creates a new zoom action.
   * @param zoomModes which zoom modes that should be shown
   */
  KoZoomAction( KoZoomMode::Modes zoomModes, const QString& text, const QString& pix,
    const KShortcut& cut = KShortcut(), KActionCollection* parent = 0, const char* name = 0 );

    /**
     * Reimplemented from @see QActionWidgetFactory.
     */
    virtual QWidget* createWidget(QWidget* parent);

public Q_SLOTS:

  /**
   * Sets the zoom. If it's not yet on the list of zoom values, it will be inserted
   * into the list at proper place so that the the values remain sorted.
   */
  void setZoom( const QString& zoom );

  /**
   * Sets the zoom. If it's not yet on the list of zoom values, it will be inserted
   * into the list at proper place so that the the values remain sorted.
   */
  void setZoom( int zoom );

  /**
   * Change the zoom modes that should be shown
   */
  void setZoomModes( KoZoomMode::Modes zoomModes );

protected Q_SLOTS:

  void triggered( const QString& text );
  void sliderValueChanged(int value);
  void numberValueChanged();

Q_SIGNALS:

  /**
   * Signal zoomChanged is triggered when user changes the zoom value, either by
   * choosing it from the list or by entering new value.
   * @param mode The selected zoom mode
   * @param zoom the zoom in percent, only defined if @p mode is KoZoomMode::ZOOM_CONSTANT
   */
  void zoomChanged( KoZoomMode::Mode mode, int zoom );

protected:

  void init(KActionCollection* parent);

  /// Regenerates the action's items
  void regenerateItems( const QString& zoomString );

  KoZoomMode::Modes m_zoomModes;
    QLineEdit *m_number;
    QSlider *m_slider;
    QRadioButton *m_actualButton;
    QRadioButton *m_fitWidthButton;
    QRadioButton *m_fitPageButton;
    int m_sliderLookup[33];
};

#endif // kozoomaction_h
