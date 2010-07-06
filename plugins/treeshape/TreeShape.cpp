/* This file is part of the KDE project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "TreeShape.h"
#include "Layout.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoShapeContainerModel.h"
#include "KoConnectionShape.h"
#include "KoShapeLayer.h"
#include "SimpleShapeContainerModel.h"
#include "KoShapeSavingContext.h"
#include "KoShapeLoadingContext.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include "KoShapeRegistry.h"
#include "KoShapeBorderModel.h"
#include "KoShapeShadow.h"

#include <KoShapeRegistry.h>

#include <QPainter>

#include "kdebug.h"

TreeShape::TreeShape(): KoShapeContainer(new Layout(this))
{
    m_nextShape = 0;
    KoShape *root = KoShapeRegistry::instance()->value("EllipseShape")->createDefaultShape();
    root->setSize(QSizeF(50,50));
    root->setParent(this);
    layout()->setRoot(root);
    for (int i=0; i<3; i++){
        addNewChild();
    }
}

TreeShape::TreeShape(KoShape *shape): KoShapeContainer(new Layout(this))
{
    m_nextShape = 0;
    setShapeId("TreeShape");
    addShape(shape);
    layout()->setRoot(shape);
    layout()->layout();
    update();
}

TreeShape::~TreeShape()
{
}

KoShape* TreeShape::connector(KoShape *shape)
{
    Layout *layout = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(layout);
    return layout->connector(shape);
}

QList<KoShape*> TreeShape::addNewChild()
{
    kDebug() << "start";
    QList<KoShape*> shapes;

    KoShape *root = KoShapeRegistry::instance()->value("EllipseShape")->createDefaultShape();
    root->setSize(QSizeF(30,30));
    KoShape *child = new TreeShape(root);
    addShape(child);
    shapes.append(child);
    kDebug() << "Child added";

    KoShape *connector = KoShapeRegistry::instance()->value("KoConnectionShape")->createDefaultShape();
    addShape(connector);
    shapes.append(connector);
    kDebug() << "Connector added";

    layout()->attachConnector(child, dynamic_cast<KoConnectionShape*>(connector));
    layout()->layout();
    update();
    kDebug() << "end";
    return shapes;
}

void TreeShape::setNextShape(KoShape* shape)
{
    m_nextShape = shape;
}

KoShape* TreeShape::nextShape()
{
    return m_nextShape;
}

void TreeShape::paintComponent(QPainter &painter, const KoViewConverter &converter)
{
    kDebug() << "start";
    Q_UNUSED(painter);
    Q_UNUSED(converter);
    //will be relayouted only if needed
    layout()->layout();
    kDebug() << "end";
}

bool TreeShape::hitTest(const QPointF &position) const
{
    return layout()->root()->hitTest(position);
}

// void TreeShape::shapeChanged(ChangeType type, KoShape *shape)
// {
//     Q_UNUSED(shape);
//     Q_UNUSED(type);
//     kDebug() << "";
// }

void TreeShape::saveOdf(KoShapeSavingContext &context) const
{
    Q_UNUSED(context);
}

bool TreeShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    Q_UNUSED(context);
    return true;
}

Layout* TreeShape::layout() const
{
    Layout *l = dynamic_cast<Layout*>(KoShapeContainer::model());
    Q_ASSERT(l);
    return l;
}