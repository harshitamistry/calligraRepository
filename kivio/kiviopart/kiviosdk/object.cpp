/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2005 Peter Simonsson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "object.h"

#include <qpainter.h>

#include <kdebug.h>

#include <koPoint.h>
#include <koRect.h>
#include <kozoomhandler.h>

namespace Kivio {

Object::Object()
{
  m_selected = false;
}

Object::~Object()
{
}

ShapeType Object::type()
{
  return kstNone;
}

QString Object::name() const
{
  return m_name;
}

void Object::setName(const QString& newName)
{
  m_name= newName;
}

QBrush Object::brush() const
{
  return m_brush;
}

void Object::setBrush(const QBrush& newBrush)
{
  m_brush = newBrush;
}

Pen Object::pen() const
{
  return m_pen;
}

void Object::setPen(const Pen& newPen)
{
  m_pen = newPen;
}

CollisionFeedback Object::contains(const KoPoint& point)
{
  KoRect rect = boundingBox();
  CollisionFeedback feedback;
  feedback.type = CTNone;

  if(rect.contains(point)) {
    feedback.type = CTBody;
  }

  return feedback;
}

bool Object::intersects(const KoRect& rect)
{
  return boundingBox().intersects(rect);
}

void Object::paintResizePoint(QPainter& painter, const QPoint& point)
{
  painter.save();
  painter.setPen(Qt::black);
  painter.setBrush(QColor(0, 200, 0));
  painter.drawRect(point.x() - 3, point.y() - 3, 6, 6);
  painter.restore();
}

bool Object::hitResizePoint(const KoPoint& resizePoint, const KoPoint& point)
{
  KoZoomHandler zoomHandler;
  double width = zoomHandler.zoomItX(6);
  double height = zoomHandler.zoomItY(6);

  KoRect rect(resizePoint.x() - (width * 0.5), resizePoint.y() - (height * 0.5), width, height);

  return rect.contains(point);
}

}
