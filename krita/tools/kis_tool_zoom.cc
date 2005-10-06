/*
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qpainter.h>
#include <kaction.h>
#include <klocale.h>
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_tool_zoom.h"
#include "kis_tool_zoom.moc"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_image.h"

KisToolZoom::KisToolZoom()
{
    setName("tool_zoom");
    m_subject = 0;
    m_dragging = false;
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
    setCursor(KisCursor::zoomCursor());
}

KisToolZoom::~KisToolZoom()
{
}

void KisToolZoom::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolZoom::paint(QPainter& gc)
{
    if (m_dragging)
        paintOutline(gc, QRect());
}

void KisToolZoom::paint(QPainter& gc, const QRect& rc)
{
    if (m_dragging)
        paintOutline(gc, rc);
}

void KisToolZoom::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject && m_subject -> currentImg() && !m_dragging) {
        if (e -> button() == Qt::LeftButton) {
            m_startPos = e -> pos().floorQPoint();
            m_endPos = e -> pos().floorQPoint();
            m_dragging = true;
        }
        else if (e -> button() == Qt::RightButton) {

            KisCanvasController *controller = m_subject -> canvasController();
            controller -> zoomOut(e -> pos().floorX(), e -> pos().floorY());
        }
    }
}

void KisToolZoom::move(KisMoveEvent *e)
{
    if (m_subject && m_dragging) {
        if (m_startPos != m_endPos)
            paintOutline();

        m_endPos = e -> pos().floorQPoint();
        paintOutline();
    }
}

void KisToolZoom::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_dragging && e -> button() == Qt::LeftButton) {

        KisCanvasController *controller = m_subject -> canvasController();
        m_endPos = e -> pos().floorQPoint();
        m_dragging = false;

        QPoint delta = m_endPos - m_startPos;

        if (sqrt(delta.x() * delta.x() + delta.y() * delta.y()) < 10) {
            controller -> zoomIn(m_endPos.x(), m_endPos.y());
        } else {
            controller -> zoomTo(QRect(m_startPos, m_endPos));
        }
    }
}

void KisToolZoom::paintOutline()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject -> canvasController();
        QWidget *canvas = controller -> canvas();
        QPainter gc(canvas);
        QRect rc;

        paintOutline(gc, rc);
    }
}

void KisToolZoom::paintOutline(QPainter& gc, const QRect&)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject -> canvasController();
        RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::DotLine);
        QPoint start;
        QPoint end;

        Q_ASSERT(controller);
        start = controller -> windowToView(m_startPos);
        end = controller -> windowToView(m_endPos);

        gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.drawRect(QRect(start, end));
        gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolZoom::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Zoom"), "viewmag", Qt::Key_Z, this, SLOT(activate()), collection, name());
        m_action -> setToolTip(i18n("Zoom"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

