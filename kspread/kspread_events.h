#ifndef KSPREAD_EVENTS
#define KSPREAD_EVENTS

#include <qevent.h>
#include <qrect.h>
#include <qstring.h>

#include <string.h>

#include <kparts/event.h>

class KSpreadSelectionChanged : public KParts::Event
{
public:
    KSpreadSelectionChanged( const QRect&, const QString& table );
    ~KSpreadSelectionChanged();

    QRect rect() const { return m_rect; }
    QString table() const { return m_table; }

    static bool test( const QEvent* e ) { return KParts::Event::test( e, s_strSelectionChanged ); }

private:
    static const char *s_strSelectionChanged;
    QRect m_rect;
    QString m_table;
};

#endif
