/*
 *  kis_tool_select_freehand.cc - part of Krayon
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
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
#include <qregion.h>
#include <kdebug.h>

#include "kis_doc.h"
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_vec.h"
#include "kis_cursor.h"
#include "kis_tool_select_freehand.h"


FreehandSelectTool::FreehandSelectTool( KisDoc* _doc, KisView* _view, KisCanvas* _canvas )
  : KisTool( _doc, _view)
  , m_dragging( false ) 
  , m_view( _view )  
  , m_canvas( _canvas )

{
    m_dragStart = QPoint(-1,-1);
    m_dragEnd =   QPoint(-1,-1);

    mStart  = QPoint(-1, -1);
    mFinish = QPoint(-1, -1);     
      
    setSelectCursor();

    m_index = 0;
    m_dragging = false;
    moveSelectArea = false;
}

FreehandSelectTool::~FreehandSelectTool()
{
}


void FreehandSelectTool::start( QPoint p )
{
    mStart = p;
}


void FreehandSelectTool::finish( QPoint p )
{
    mFinish = p;
    drawLine( mStart, mFinish );
    m_pointArray.putPoints( m_index, 1, mFinish.x(),mFinish.y() );
}


void FreehandSelectTool::clearOld()
{
   if (m_pDoc->isEmpty()) return;

   // clear everything in 
   QRect updateRect(0, 0, m_pDoc->current()->width(), m_pDoc->current()->height());
   m_view->updateCanvas(updateRect);
   m_selectRegion = QRegion();

   m_dragStart = QPoint(-1,-1);
   m_dragEnd =   QPoint(-1,-1);
}

void FreehandSelectTool::mousePress( QMouseEvent* event )
{
    if ( m_pDoc->isEmpty() )
        return;

    // start the freehand line.
    if( event->button() == LeftButton && !moveSelectArea ) {
        m_dragging = true;
        clearOld();
        start( event->pos() );
        
        m_dragStart = event->pos();
        m_dragEnd = event->pos();
        dragSelectArea = false;
    }
    else if( event->button() == LeftButton && moveSelectArea ) {
        dragSelectArea = true;
        dragFirst = true;
        m_dragStart = event->pos();
        m_dragdist = 0;

        m_hotSpot = event->pos();
        int x = zoomed( m_hotSpot.x() );
        int y = zoomed( m_hotSpot.y() );

        m_hotSpot = QPoint( x - m_imageRect.topLeft().x(), y - m_imageRect.topLeft().y() );

        oldDragPoint = event->pos();
        setClipImage();
    }
    else if( event->button() == RightButton ) {
        // TODO
        return;
    }
}


void FreehandSelectTool::mouseMove( QMouseEvent* event )
{
    if (m_pDoc->isEmpty()) return;

    if( event->button() == RightButton ) return;

    if( m_dragging && !dragSelectArea ) {
        m_dragEnd = event->pos();

        m_pointArray.putPoints( m_index, 1, m_dragStart.x(),m_dragStart.y() );
        ++m_index;

        drawLine( m_dragStart, m_dragEnd );
        m_dragStart = m_dragEnd;
    }
    else if ( !m_dragging && !dragSelectArea ) {
        if ( !m_selectRegion.isNull() && m_selectRegion.contains( event->pos() ) ) {
            setMoveCursor();
            moveSelectArea = true;
        }
        else {
            setSelectCursor();
            moveSelectArea = false;
        }
    }
    else if ( dragSelectArea ) {
        if ( dragFirst ) {
            // remove select image
            m_pDoc->getSelection()->erase();

            // refresh canvas
            clearOld();
            m_pView->slotUpdateImage();
            dragFirst = false;
        }

        int spacing = 10;
        float zF = m_pView->zoomFactor();
        QPoint pos = event->pos();
        int mouseX = pos.x();
        int mouseY = pos.y();

        KisVector end( mouseX, mouseY );
        KisVector start( m_dragStart.x(), m_dragStart.y() );

        KisVector dragVec = end - start;
        float saved_dist = m_dragdist;
        float new_dist = dragVec.length();
        float dist = saved_dist + new_dist;

        if ( (int)dist < spacing ) {
            m_dragdist += new_dist;
            m_dragStart = pos;
            return;
        }
        else
            m_dragdist = 0;

        dragVec.normalize();
        KisVector step = start;

        while ( dist >= spacing ) {
            if ( saved_dist > 0 ) {
                step += dragVec * ( spacing - saved_dist );
                saved_dist -= spacing;
            }
            else
                step += dragVec * spacing;

            QPoint p( qRound( step.x() ), qRound( step.y() ) );

            QRect ur( zoomed( oldDragPoint.x() ) - m_hotSpot.x() - m_pView->xScrollOffset(),
                      zoomed( oldDragPoint.y() ) - m_hotSpot.y() - m_pView->yScrollOffset(),
                      (int)( clipPixmap.width() * ( zF > 1.0 ? zF : 1.0 ) ),
                      (int)( clipPixmap.height() * ( zF > 1.0 ? zF : 1.0 ) ) );

            m_pView->updateCanvas( ur );

            dragSelectImage( p, m_hotSpot );

            oldDragPoint = p;
            dist -= spacing;
        }

        if ( dist > 0 ) 
            m_dragdist = dist;
        m_dragStart = pos;
    }
}


void FreehandSelectTool::mouseRelease( QMouseEvent* event )
{
    if( event->button() == RightButton ) {
        // TODO
        return;
    }

    if ( !moveSelectArea ) {
        // stop drawing freehand.
        m_dragging = false;

        m_imageRect = getDrawRect( m_pointArray );
        QPointArray points = zoomPointArray( m_pointArray );

        // need to connect start and end positions to close the freehand line.
        finish( event->pos() );
        
        // we need a bounding rectangle and a point array of 
        // points in the freehand line        

        m_pDoc->getSelection()->setPolygonalSelection( m_imageRect, points, m_pDoc->current()->getCurrentLayer() );

        kdDebug(0) << "selectRect" 
                   << " left: "   << m_imageRect.left() 
                   << " top: "    << m_imageRect.top()
                   << " right: "  << m_imageRect.right() 
                   << " bottom: " << m_imageRect.bottom()
                   << endl;

        if ( m_pointArray.size() > 1 )
            m_selectRegion = QRegion( m_pointArray, true );
        else
            m_selectRegion = QRegion();

        // Initialize
        m_index = 0;
        m_pointArray.resize( 0 );
    }
    else {
        // Initialize
        dragSelectArea = false;
        m_selectRegion = QRegion();
        setSelectCursor();
        moveSelectArea = false;

        QPoint pos = event->pos();

        KisImage *img = m_pDoc->current();
        if ( !img )
            return;
        if( !img->getCurrentLayer()->visible() )
            return;
        if( pasteClipImage( zoomed( pos ) - m_hotSpot ) )
            img->markDirty( QRect( zoomed( pos ) - m_hotSpot, clipPixmap.size() ) );
    }
}


void FreehandSelectTool::drawLine( const QPoint& start, const QPoint& end )
{
    QPainter p;
    
    p.begin( m_canvas );
    p.setRasterOp( Qt::NotROP );
    p.setPen( QPen( Qt::DotLine ) );
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

void FreehandSelectTool::setupAction(QObject *collection)
{
	KToggleAction *p = new KToggleAction(i18n("&Freehand select"), "freehand", 0, this,  SLOT(toolSelect()),
			collection, "tool_select_freehand");

	p -> setExclusiveGroup("tools");
}

