/*
 *  kis_tool_pen.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_painter.h"
#include "kis_view.h"
#include "kis_tool_pen.h"
#include "integerwidget.h"
#include "kis_cmb_composite.h"
#include "kis_brush.h"


KisToolPen::KisToolPen()
        : super(),
          m_mode( HOVER ),
	  m_dragDist ( 0 )
{
	setName("tool_pen");
	setCursor(KisCursor::penCursor());

        m_painter = 0;
	m_currentImage = 0;
	m_optWidget = 0;

	m_lbOpacity = 0;
	m_slOpacity = 0;
	m_lbComposite= 0;
	m_cmbComposite = 0;

	m_opacity = OPACITY_OPAQUE;
	m_compositeOp = COMPOSITE_OVER;
}

KisToolPen::~KisToolPen()
{
}

void KisToolPen::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_currentImage = subject -> currentImg();

	super::update(m_subject);
}

void KisToolPen::mousePress(QMouseEvent *e)
{
        if (!m_subject) return;

        if (!m_subject->currentBrush()) return;

	if (!m_currentImage -> activeDevice()) return;

        if (e->button() == QMouseEvent::LeftButton) {
		m_mode = PAINT;
		initPaint(e -> pos());
		m_painter -> penAt(e->pos(), PRESSURE_DEFAULT, 0, 0);
		// XXX: get the rect that should be notified
		m_currentImage -> notify( m_painter -> dirtyRect() );
         }
}


void KisToolPen::mouseRelease(QMouseEvent* e)
{
	if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT) {
		endPaint();
        }
}


void KisToolPen::mouseMove(QMouseEvent *e)
{
	if (m_mode == PAINT) {
		paintLine(m_dragStart, e -> pos(), PRESSURE_DEFAULT, 0, 0);
	}
}

void KisToolPen::tabletEvent(QTabletEvent *e)
{
         if (e->device() == QTabletEvent::Stylus) {
		 if (!m_currentImage -> activeDevice()) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject) {
			 e -> accept();
			 return;
		 }

		 if (!m_subject -> currentBrush()) {
			 e->accept();
			 return;
		 }

		 double pressure = e -> pressure() / 255.0;

		 if (pressure < PRESSURE_THRESHOLD && m_mode == PAINT_STYLUS) {
			 endPaint();
		 } else if (pressure >= PRESSURE_THRESHOLD && m_mode == HOVER) {
			 m_mode = PAINT_STYLUS;
			 initPaint(e -> pos());
			 m_painter -> penAt(e -> pos(), pressure, e->xTilt(), e->yTilt());
			 // XXX: Get the rect that should be updated
			 m_currentImage -> notify( m_painter -> dirtyRect() );

		 } else if (pressure >= PRESSURE_THRESHOLD && m_mode == PAINT_STYLUS) {
			 paintLine(m_dragStart, e -> pos(), pressure, e -> xTilt(), e -> yTilt());
		 }
         }
	 e -> accept();
}


void KisToolPen::initPaint(const QPoint & pos)
{

	if (!m_currentImage -> activeDevice()) return;
	m_dragStart = pos;
	m_dragDist = 0;

	// Create painter
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		if (m_painter)
			delete m_painter;
		m_painter = new KisPainter( device );
		m_painter -> beginTransaction(i18n("pen"));
	}

	m_painter -> setPaintColor(m_subject -> fgColor());
	m_painter -> setBrush(m_subject -> currentBrush());
	m_painter -> setOpacity(m_opacity);
	m_painter -> setCompositeOp(m_compositeOp);

	// Set the cursor -- ideally. this should be a mask created from the brush,
	// now that X11 can handle colored cursors.
#if 0
	// Setting cursors has no effect until the tool is selected again; this
	// should be fixed.
	setCursor(KisCursor::penCursor());
#endif
}

void KisToolPen::endPaint() 
{
	m_mode = HOVER;
	KisPaintDeviceSP device;
	if (m_currentImage && (device = m_currentImage -> activeDevice())) {
		KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
		if (adapter && m_painter) {
			// If painting in mouse release, make sure painter
			// is destructed or end()ed
			adapter -> addCommand(m_painter->endTransaction());
		}
		delete m_painter;
		m_painter = 0;

	}
}

void KisToolPen::paintLine(const QPoint & pos1,
			     const QPoint & pos2,
			     const double pressure,
			     const double xtilt,
			     const double ytilt)
{
	if (!m_currentImage -> activeDevice()) return;

	m_dragDist = m_painter -> paintLine(PAINTOP_PEN, pos1, pos2, pressure, xtilt, ytilt, m_dragDist);
	m_currentImage -> notify( m_painter -> dirtyRect() );
	m_dragStart = pos2;
}


void KisToolPen::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("&Pen"),
					    "pencil", 0, this,
					    SLOT(activate()), collection,
					    name());
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

QWidget* KisToolPen::createoptionWidget(QWidget* parent)
{
	m_optWidget = new QWidget(parent);
	m_optWidget -> setCaption(i18n("Pen"));
	
	m_lbOpacity = new QLabel(i18n("Opacity: "), m_optWidget);
	m_slOpacity = new IntegerWidget( 0, 100, m_optWidget, "int_widget");
	m_slOpacity -> setTickmarks(QSlider::Below);
	m_slOpacity -> setTickInterval(10);
	m_slOpacity -> setValue(m_opacity / OPACITY_OPAQUE * 100);
	connect(m_slOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacity(int)));

	m_lbComposite = new QLabel(i18n("Mode: "), m_optWidget);
	m_cmbComposite = new KisCmbComposite(m_optWidget);
	connect(m_cmbComposite, SIGNAL(activated(int)), this, SLOT(slotSetCompositeMode(int)));

	QGridLayout *optionLayout = new QGridLayout(m_optWidget, 4, 2);

	optionLayout -> addWidget(m_lbOpacity, 1, 0);
	optionLayout -> addWidget(m_slOpacity, 1, 1);

	optionLayout -> addWidget(m_lbComposite, 2, 0);
	optionLayout -> addWidget(m_cmbComposite, 2, 1);

	return m_optWidget;
}

QWidget* KisToolPen::optionWidget()
{
	return m_optWidget;
}

void KisToolPen::slotSetOpacity(int opacityPerCent)
{
	m_opacity = opacityPerCent * OPACITY_OPAQUE / 100;
}

void KisToolPen::slotSetCompositeMode(int compositeOp)
{
	m_compositeOp = (CompositeOp)compositeOp;
}

#include "kis_tool_pen.moc"

