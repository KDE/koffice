#ifndef KFORMULA_FACTORY_H
#define KFORMULA_FACTORY_H

#include <koFactory.h>
//#include <kaboutdata.h>

class KInstance;
class KInstance;
class KAboutData;

class KFormulaFactory : public KoFactory
{
    Q_OBJECT
public:
    KFormulaFactory( QObject* parent = 0, const char* name = 0 );
    ~KFormulaFactory();

    virtual KParts::Part *createPart( QWidget *parentWidget = 0, const char *widgetName = 0, QObject *parent = 0, const char *name = 0, const char *classname = "KoDocument", const QStringList &args = QStringList() );

    static KInstance* global();

    static KAboutData* aboutData();


private:
    static KInstance* s_global;
};

#endif
