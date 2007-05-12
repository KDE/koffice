/**
 * Kalle Dalheimer <kalle@kde.org>
 */

#ifndef KCHART_PART_H
#define KCHART_PART_H


#include "kchart_global.h"

#include <kconfig.h>
#include <KoXmlReader.h>

#include <koChart.h>

#if 0
#include "kchart_params.h"
#else
#include "TableModel.h"         // In kdchart/examples/tools
#include "KDChartChart.h"
#endif

#include "kchart_export.h"
//Added by qt3to4:
#include <QPixmap>

//class KDChartTableData;
class KoXmlWriter;
class KoGenStyles;


namespace KChart
{


class KCHART_EXPORT KChartPart : public KoChart::Part
{
    Q_OBJECT

public:
    explicit KChartPart( QWidget *parentWidget = 0,
			 QObject* parent = 0,
			 bool singleViewMode = false );
    ~KChartPart();

    // Methods inherited from KoDocument:

    virtual void  paintContent( QPainter& painter, const QRect& rect);

    // Methods unique to KChart, and available in the new interface
    // (see /interfaces/koChart.h.)

    virtual void  resizeData( int rows, int columns );
    virtual void  setCellData( int row, int column, const QVariant &);
    virtual void  analyzeHeaders( );
    virtual void  setCanChangeValue( bool b )  { m_bCanChangeValue = b;    }

    // ----------------------------------------------------------------

#if 0
    void  analyzeHeaders( const KDChartTableData & data );
    void  doSetData( const KDChartTableData&  data,
		     bool  firstRowHeader,
		     bool  firstColHeader );
#else
    void  analyzeHeaders( const TableModel &data );
    void  doSetData( const TableModel &data,
		     bool  firstRowHeader,
		     bool  firstColHeader );
#endif

    bool showWizard( QString &area );
    void initLabelAndLegend();
    void loadConfig(KConfig *conf);
    void saveConfig(KConfig *conf);
    void defaultConfig();

    OdfChartType       chartType() const       { return m_type;       }
    TableModel        *data()                  { return m_currentData; }
#if 0
    KChartParams      *params()    const       { return m_params;     }
#else
    KDChart::Chart    *chart()     const       { return m_chart;      }
#endif

    // Data in rows or columns.
    DataDirection  dataDirection() const    { return m_dataDirection; }
    void           setDataDirection( DataDirection _dir ) {
	m_dataDirection = _dir;
    }

    // First row / column as data or label?
    bool       firstRowAsLabel() const { return m_firstRowAsLabel; }
    void       setFirstRowAsLabel( bool _val );
    bool       firstColAsLabel() const { return m_firstColAsLabel; }
    void       setFirstColAsLabel( bool _val );

    // 
    QStringList       &rowLabelTexts()         { return m_rowLabels;  }
    QStringList       &colLabelTexts()         { return m_colLabels;  }

    // Save and load
    virtual QDomDocument  saveXML();
    virtual bool          loadXML( QIODevice *, const KoXmlDocument& doc );
    virtual bool          loadOasis( const KoXmlDocument& doc,
				     KoOasisStyles& oasisStyles,
				     const KoXmlDocument& settings,
				     KoStore *store );
    virtual bool          saveOasis( KoStore* store,
                                     KoXmlWriter* manifestWriter );

    bool  canChangeValue()   const             { return m_bCanChangeValue; }

    void  initNullChart();

    // Functions that generate templates (not used yet):
    void  generateBarChartTemplate();

    virtual bool showEmbedInitDialog(QWidget* parent);

public slots:
    void  slotModified();
    virtual void initEmpty();

signals:
    void docChanged();

protected:
    virtual KoView* createViewInstance( QWidget* parent );
#if 0
    bool  loadOldXML( const KoXmlDocument& doc );
    bool  loadAuxiliary( const KoXmlDocument& doc );
    bool  loadData( const KoXmlDocument& doc/* , KDChartTableData& currentData*/ );
#endif
    bool  loadOasisData( const KoXmlElement& tableElem );
    void  saveOasisData( KoXmlWriter* bodyWriter, KoGenStyles& mainStyles ) const;
    void writeAutomaticStyles( KoXmlWriter& contentWriter, KoGenStyles& mainStyles ) const;

private:
    // Helper methods for painting.
    int          createDisplayData();
    void         createLabelsAndLegend( QStringList  &longLabels,
					QStringList  &shortLabels );


    QDomElement  createElement(const QString &tagName,
			       const QFont &font,
			       QDomDocument &doc) const;
    QFont        toFont(QDomElement &element)     const;

    void         setChartDefaults();

private:
    // The chart and its contents
    OdfChartType             m_type;
    OdfChartSubtype          m_subtype;
#if 0
    KChartParams            *m_params;      // Everything about the chart
    KDChartTableData         m_currentData; // The data in the chart.
#else
    KDChart::Chart          *m_chart;
    TableModel              *m_currentData;

    // Info about the data.
    DataDirection  m_dataDirection; // Rows or Columns
    bool           m_firstRowAsLabel;
    bool           m_firstColAsLabel;
#endif

    QStringList              m_rowLabels;
    QStringList              m_colLabels;
    //QString                  m_regionName;

    // Other auxiliary values
    bool                     m_bCanChangeValue;

    // Graphics
    QWidget                 *m_parentWidget;

    // Used when displaying.
#if 0
    KDChartTableData         m_displayData;
#else
    TableModel               m_displayData;
#endif

    QPixmap                  m_bufferPixmap;
};


class WizardExt : public KoChart::WizardExtension
{
public:
    WizardExt( KoChart::Part *part )
        : KoChart::WizardExtension( part ) {}

    virtual bool show( QString &area ) {
        return static_cast<KChartPart *>( part() )->showWizard( area );
    }
};

}  //KChart namespace

#endif
