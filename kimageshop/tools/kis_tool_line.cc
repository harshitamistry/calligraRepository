/*
 *  linetool.cc - part of Krayon
 *
 *  Copyright (c) 2000 John Califf 
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qpainter.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_painter.h"
#include "kis_color.h"
#include "kis_canvas.h"
#include "kis_tool_line.h"
#include "opts_line_dlg.h"

LineTool::LineTool( KisDoc* _doc, KisView* _view, KisCanvas* _canvas)
  : KisTool( _doc, _view )
  , m_dragging( false )
  , pCanvas( _canvas )
{
    lineThickness = 4;
    lineOpacity = 255;
}


LineTool::~LineTool()
{
}


void LineTool::mousePress( QMouseEvent* event )
{
    if ( m_pDoc->isEmpty() )
        return;

    if( event->button() == LeftButton )
    {
        m_dragging = true;
        m_dragStart = event->pos();
        m_dragEnd = event->pos();
    }
}


void LineTool::mouseMove( QMouseEvent* event )
{
    if ( m_pDoc->isEmpty() )
        return;

    if( m_dragging )
    {
        drawLine( m_dragStart, m_dragEnd );
        m_dragEnd = event->pos();
        drawLine( m_dragStart, m_dragEnd );
    }
}


void LineTool::mouseRelease( QMouseEvent* event )
{
    if ( m_pDoc->isEmpty() )
        return;

    if(( m_dragging) 
    && ( event->state() == LeftButton))
    {
        drawLine( m_dragStart, m_dragEnd );
        m_dragging = false;
        drawLine( m_dragStart, m_dragEnd );
    }
    
    KisPainter *p = m_pView->kisPainter();
    p->drawLine(m_dragStart.x(), m_dragStart.y(),
                m_dragEnd.x(),   m_dragEnd.y());
}


void LineTool::drawLine( const QPoint& start, const QPoint& end )
{
    QPainter p;
    QPen pen;
    pen.setWidth(lineThickness);

    p.begin( pCanvas );
    p.setPen(pen);    
    p.setRasterOp( Qt::NotROP );
    float zF = m_pView->zoomFactor();

    p.drawLine( QPoint( start.x() + m_pView->xPaintOffset() 
                          - (int)(zF * m_pView->xScrollOffset()),
                        start.y() + m_pView->yPaintOffset() 
                           - (int)(zF * m_pView->yScrollOffset())), 
                QPoint( end.x() + m_pView->xPaintOffset() 
                          - (int)(zF * m_pView->xScrollOffset()),
                        end.y() + m_pView->yPaintOffset() 
                           - (int)(zF * m_pView->yScrollOffset())) );

    p.end();
}

void LineTool::optionsDialog()
{
    LineOptionsDialog *pOptsDialog 
        = new LineOptionsDialog(lineThickness, lineOpacity);
    pOptsDialog->exec();
    if(!pOptsDialog->result() == QDialog::Accepted)
        return;

    lineThickness = pOptsDialog->thickness();
    lineOpacity   = pOptsDialog->opacity();
    
    KisPainter *p = m_pView->kisPainter();
    p->setLineThickness(lineThickness);
    p->setLineOpacity(lineOpacity);
}

