/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers
*/

#include <qcstring.h>
#include <qdom.h>
#include <qfile.h>
#include <qstring.h>

#include <kgenericfactory.h>
#include <koFilter.h>
#include <koFilterChain.h>
#include <koStore.h>

#include "epsexport.h"
#include "vdocument.h"
#include "vfill.h"
#include "vgroup.h"
#include "vlayer.h"
#include "vpath.h"
#include "vsegment.h"
#include "vsegmentlist.h"
#include "vselection.h"
#include "vstroke.h"

#include <kdebug.h>


class EpsExportFactory : KGenericFactory<EpsExport, KoFilter>
{
public:
	EpsExportFactory( void )
		: KGenericFactory<EpsExport, KoFilter>( "karbonepsexport" )
	{}

protected:
	virtual void setupTranslations( void )
	{
		KGlobal::locale()->insertCatalogue( "karbonepsfilter" );
	}
};

K_EXPORT_COMPONENT_FACTORY( libkarbonepsexport, EpsExportFactory() );

EpsExport::EpsExport( KoFilter*, const char*, const QStringList& )
	: KoFilter()
{
}

KoFilter::ConversionStatus
EpsExport::convert( const QCString& from, const QCString& to )
{
	if ( to != "image/x-eps" || from != "application/x-karbon" )
	{
		return KoFilter::NotImplemented;
	}

	KoStore* storeIn = KoStore::createStore( m_chain->inputFile(), KoStore::Read );
	if( !storeIn->open( "root" ) )
	{
		delete storeIn;
		return KoFilter::StupidError;
	}

	QFile fileOut( m_chain->outputFile() );
	if( !fileOut.open( IO_WriteOnly ) ) {
		delete storeIn;
		return KoFilter::StupidError;
	}

	QByteArray byteArrayIn = storeIn->read( storeIn->size() );
	storeIn->close();

	QDomDocument domIn;
	domIn.setContent( byteArrayIn );
	QDomElement docNode = domIn.documentElement();

	m_stream = new QTextStream( &fileOut );


	// load the document and export it:
	VDocument doc;
	doc.load( docNode );
	exportDocument( doc );


	fileOut.close();

	delete m_stream;
	delete storeIn;

	return KoFilter::OK;
}

void
EpsExport::exportDocument( const VDocument& document )
{
	// select all objects:
	document.select();

	// get the bounding box of all selected objects:
	const KoRect& rect = document.selection()->boundingBox();

	// print a header:
	*m_stream <<
		"%!PS-Adobe-2.0 EPSF-1.2\n"
		"%%Creator: Karbon14 EPS 0.5\n"
		"%%BoundingBox: "
		<< rect.left() << " "
		<< rect.top()  << " "
		<< rect.right() << " "
		<< rect.bottom() << "\n"
	<< endl;

	// we dont need the selection anymore:
	document.deselect();

	// print some defines:
	*m_stream <<
		"/N {newpath} def\n"
		"/C {closepath} def\n"
		"/m {moveto} def\n"
		"/c {curveto} def\n"
		"/l {lineto} def\n"
		"/s {stroke} def\n"
		"/f {fill} def\n"
	<< endl;

	// export layers:
	VLayerListIterator itr( document.layers() );
	for( ; itr.current(); ++itr )
		exportLayer( *itr.current() );
}

void
EpsExport::exportGroup( const VGroup& group )
{
	// export objects:
	VObjectListIterator itr( group.objects() );
	for( ; itr.current(); ++itr )
	{
		if( VPath* path = dynamic_cast<VPath*>( itr.current() ) )
			exportPath( *path );
		else if( VGroup* group = dynamic_cast<VGroup*>( itr.current() ) )
			exportGroup( *group );
	}
}

void
EpsExport::exportLayer( const VLayer& layer )
{
	exportGroup( layer );
}

void
EpsExport::exportPath( const VPath& path )
{
	// export segmentlists:
	VSegmentListListIterator itr( path.segmentLists() );
	for( ; itr.current(); ++itr )
		exportSegmentList( *itr.current() );

	if( path.stroke()->type() != stroke_none )
		*m_stream << "s ";

	if( path.fill()->type() != fill_none )
		*m_stream << "f ";

	*m_stream << endl;
}

void
EpsExport::exportSegmentList( const VSegmentList& segmentList )
{
	*m_stream << "N\n";

	// export segments:
	VSegmentListIterator itr( segmentList );
	for( ; itr.current(); ++itr )
	{
		switch( itr.current()->type() )
		{
			case segment_curve:
				*m_stream <<
					itr.current()->ctrlPoint1().x() << " " <<
					itr.current()->ctrlPoint1().y() << " " <<
					itr.current()->ctrlPoint2().x() << " " <<
					itr.current()->ctrlPoint2().y() << " " <<
					itr.current()->knot2().x() << " " <<
					itr.current()->knot2().y() << " " <<
					"c\n";
			break;
			case segment_curve1:
				*m_stream <<
					itr.current()->knot1().x() << " " <<
					itr.current()->knot1().y() << " " <<
					itr.current()->ctrlPoint2().x() << " " <<
					itr.current()->ctrlPoint2().y() << " " <<
					itr.current()->knot2().x() << " " <<
					itr.current()->knot2().y() << " " <<
					"c\n";
			break;
			case segment_curve2:
				*m_stream <<
					itr.current()->ctrlPoint1().x() << " " <<
					itr.current()->ctrlPoint1().y() << " " <<
					itr.current()->knot2().x() << " " <<
					itr.current()->knot2().y() << " " <<
					itr.current()->knot2().x() << " " <<
					itr.current()->knot2().y() << " " <<
					"c\n";
			break;
			case segment_line:
				*m_stream <<
					itr.current()->knot2().x() << " " <<
					itr.current()->knot2().y() << " " <<
					"l\n";
			break;
			case segment_begin:
				*m_stream <<
					itr.current()->knot2().x() << " " <<
					itr.current()->knot2().y() << " " <<
					"m\n";
			break;
			case segment_end:
			break;
		}
	}

	if( segmentList.isClosed() )
		*m_stream << "C ";
}

#include "epsexport.moc"

