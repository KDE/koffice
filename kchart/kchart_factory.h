/**
 * $Id$
 *
 * Kalle Dalheimer <kalle@kde.org>
 */

#ifndef KCHART_FACTORY_H
#define KCHART_FACTORY_H

#include <koFactory.h>

class KInstance;
class KAboutData;

class KChartFactory : public KoFactory
{
    Q_OBJECT
public:
    KChartFactory( QObject* parent = 0, const char* name = 0 );
    virtual ~KChartFactory();

    virtual KParts::Part* createPartObject( QWidget* = 0, const char * = 0, QObject* parent = 0, const char* name = 0, const char* classname = "KoDocument", const QStringList &args = QStringList() );

    static KInstance* global();

    // _Creates_ a KAboutData but doesn't keep ownership
    static KAboutData* aboutData();

private:
    static KInstance* s_global;
    static KAboutData *s_aboutData;
};

#endif
