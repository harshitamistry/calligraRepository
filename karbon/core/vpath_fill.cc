/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
   Copyright (C) 2002, The Karbon Developers
*/

#include "vpainter.h"
#include <qcolor.h>
#include <qpointarray.h>

#include "vpath_fill.h"

VPathFill::VPathFill( const VFill& fill )
	: VSegmentListTraverser(), m_fill( fill )
{
}

void
VPathFill::begin_draw( VPainter *painter, const double zoomFactor )
{
	m_painter = painter;
	m_zoomFactor = zoomFactor;
	m_pa.resize( 0 );
}

void
VPathFill::draw( const VSegmentList& list )
{
	traverse( list );

	// append the first point again to avoid mess with holes:
	m_pa.resize( m_pa.size() + 1 );
	m_pa.setPoint( m_pa.size() - 1, m_pa.point( 0 ) );
}

void
VPathFill::end_draw()
{
	m_painter->setRasterOp( Qt::CopyROP );
	m_painter->setPen( Qt::NoPen );
	m_painter->setBrush( m_fill );
	m_painter->fillPath();
}

bool
VPathFill::begin( const KoPoint& p )
{
	m_painter->moveTo( p );
	return true;
	m_pa.resize( m_pa.size() + 1 );
	m_pa.setPoint(
		m_pa.size() - 1,
		qRound( m_zoomFactor * p.x() ),
		qRound( m_zoomFactor * p.y() ) );

	setPreviousPoint( p );

	return true;
}

bool
VPathFill::curveTo( const KoPoint& p1, const KoPoint& p2, const KoPoint& p3 )
{
	m_painter->curveTo( p1, p2, p3 );
	return true;
	QPointArray pa( 4 );
	pa.setPoint(
		0,
		qRound( m_zoomFactor * previousPoint().x() ),
		qRound( m_zoomFactor * previousPoint().y() ) );
	pa.setPoint(
		1,
		qRound( m_zoomFactor * p1.x() ),
		qRound( m_zoomFactor * p1.y() ) );
	pa.setPoint(
		2,
		qRound( m_zoomFactor * p2.x() ),
		qRound( m_zoomFactor * p2.y() ) );
	pa.setPoint(
		3,
		qRound( m_zoomFactor * p3.x() ),
		qRound( m_zoomFactor * p3.y() ) );

	QPointArray pa2( pa.cubicBezier() );

	m_pa.resize( m_pa.size() + pa2.size() );
	m_pa.putPoints( m_pa.size() - pa2.size(), pa2.size(), pa2 );

	setPreviousPoint( p3 );

	return true;
}

bool
VPathFill::lineTo( const KoPoint& p )
{
	m_painter->lineTo( p );
	return true;
	m_pa.resize( m_pa.size() + 1 );
	m_pa.setPoint( m_pa.size() - 1,
		qRound( m_zoomFactor * p.x() ),
		qRound( m_zoomFactor * p.y() ) );

	setPreviousPoint( p );

	return true;
}
