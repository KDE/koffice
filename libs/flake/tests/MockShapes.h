#ifndef MOCKSHAPES_H
#define MOCKSHAPES_H

#include <KoShapeGroup.h>
#include <KoCanvasBase.h>
#include <KoShapeControllerBase.h>
#include <KoShapeContainerModel.h>
#include <QPainter>

#include "kdebug.h"

class MockShape : public KoShape
{
public:
    MockShape() : paintedCount(0) {}
    void paint(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Shape" << kBacktrace( 10 );
        paintedCount++;
    }
    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    int paintedCount;
};

class MockContainer : public KoShapeContainer
{
public:
    MockContainer(KoShapeContainerModel *model) : KoShapeContainer(model), paintedCount(0) {}
    MockContainer() : paintedCount(0) {}
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
        //qDebug() << "Container:" << kBacktrace( 10 );
        paintedCount++;
    }

    virtual void saveOdf(KoShapeSavingContext &) const {}
    virtual bool loadOdf(const KoXmlElement &, KoShapeLoadingContext &) {
        return true;
    }
    int paintedCount;
};

class MockGroup : public KoShapeGroup
{
    void paintComponent(QPainter &painter, const KoViewConverter &converter) {
        Q_UNUSED(painter);
        Q_UNUSED(converter);
    }
};

class KoToolProxy;

class MockCanvas : public KoCanvasBase
{
public:
    MockCanvas()
            : KoCanvasBase(0) {}
    ~MockCanvas() {}

    void gridSize(qreal *, qreal *) const {}
    bool snapToGrid() const  {
        return false;
    }
    void addCommand(QUndoCommand*) { }
    KoShapeManager *shapeManager() const  {
        return 0;
    }
    void updateCanvas(const QRectF&)  {}
    KoToolProxy * toolProxy() const {
        return 0;
    }
    KoViewConverter *viewConverter() const {
        return 0;
    }
    QWidget* canvasWidget() {
        return 0;
    }
    const QWidget* canvasWidget() const {
        return 0;
    }
    KoUnit unit() const {
        return KoUnit(KoUnit::Millimeter);
    }
    void updateInputMethodInfo() {}
};

class MockViewConverter : public KoViewConverter
{
    QPointF documentToView(const QPointF &documentPoint) const {
        return documentPoint;
    }
    QPointF viewToDocument(const QPointF &viewPoint) const {
        return viewPoint;
    }
    QRectF documentToView(const QRectF &documentRect) const {
        return documentRect;
    }
    QRectF viewToDocument(const QRectF &viewRect) const {
        return viewRect;
    }
    QSizeF documentToView(const QSizeF& documentSize) const {
        return documentSize;
    }
    QSizeF viewToDocument(const QSizeF& viewSize) const {
        return viewSize;
    }
    qreal documentToViewX(qreal documentX) const {
        return documentX;
    }
    qreal documentToViewY(qreal documentY) const {
        return documentY;
    }
    qreal viewToDocumentX(qreal viewX) const {
        return viewX;
    }
    qreal viewToDocumentY(qreal viewY) const {
        return viewY;
    }
    void zoom(qreal *zoomX, qreal *zoomY) const {
        *zoomX = 1.0; *zoomY = 1.0;
    }
};

class MockShapeController : public KoShapeControllerBase
{
public:
    void addShape(KoShape* shape) {
        m_shapes.insert(shape);
    }
    void removeShape(KoShape* shape) {
        m_shapes.remove(shape);
    }
    bool contains(KoShape* shape) {
        return m_shapes.contains(shape);
    }
    QMap<QString, KoDataCenter *>  dataCenterMap() const {
        return QMap<QString, KoDataCenter *>();
    }
private:
    QSet<KoShape * > m_shapes;
};

class MockContainerModel : public KoShapeContainerModel
{
public:
    MockContainerModel() {
        resetCounts();
    }

    /// reimplemented
    void add(KoShape *child) {
        m_children.append(child); // note that we explicitly do not check for duplicates here!
    }
    /// reimplemented
    void remove(KoShape *child) {
        m_children.removeAll(child);
    }

    /// reimplemented
    void setClipping(const KoShape *, bool) { }  // ignored
    /// reimplemented
    bool childClipped(const KoShape *) const {
        return false;
    }// ignored
    /// reimplemented
    bool isChildLocked(const KoShape *child) const {
        return child->isGeometryProtected();
    }
    /// reimplemented
    int count() const {
        return m_children.count();
    }
    /// reimplemented
    QList<KoShape*> iterator() const {
        return m_children;
    }
    /// reimplemented
    void containerChanged(KoShapeContainer *) {
        m_containerChangedCalled++;
    }
    /// reimplemented
    void proposeMove(KoShape *, QPointF &) {
        m_proposeMoveCalled++;
    }
    /// reimplemented
    void childChanged(KoShape *, KoShape::ChangeType) {
        m_childChangedCalled++;
    }

    int containerChangedCalled() const {
        return m_containerChangedCalled;
    }
    int childChangedCalled() const {
        return m_childChangedCalled;
    }
    int proposeMoveCalled() const {
        return m_proposeMoveCalled;
    }

    void resetCounts() {
        m_containerChangedCalled = 0;
        m_childChangedCalled = 0;
        m_proposeMoveCalled = 0;
    }

private:
    QList<KoShape*> m_children;
    int m_containerChangedCalled, m_childChangedCalled, m_proposeMoveCalled;
};

#endif
