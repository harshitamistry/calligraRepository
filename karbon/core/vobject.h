/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VOBJECT_H__
#define __VOBJECT_H__

#include "vrect.h"
#include "vtool.h"

class QPainter;
class KCommand;

class VAffineMap;

// The base class for all karbon objects.

class VObject
{
public:
	VObject();
	virtual ~VObject() {}

	virtual void draw( QPainter& painter, const QRect& rect,
		const double zoomFactor ) = 0;

	virtual KCommand* accept( VTool& tool ) { return tool.manipulate( this ); }

	virtual VObject& transform( const VAffineMap& affMap ) = 0;

	const VRect& boundingBox() const { return m_boundingBox; }

protected:
	// QRect as boundingBox is sufficent since it's not used for calculating
	// intersections
	VRect m_boundingBox;
};

#endif
