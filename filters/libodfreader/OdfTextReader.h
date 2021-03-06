/* This file is part of the KDE project

   Copyright (C) 2012-2013 Inge Wallin            <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef ODFTEXTREADER_H
#define ODFTEXTREADER_H

// Qt
#include <QHash>
#include <QString>

// Calligra
#include <KoXmlStreamReader.h>

// this library
#include "koodfreader_export.h"


class QSizeF;

class KoXmlWriter;
class KoStore;

class OdfTextReaderBackend;
class OdfReaderContext;


/** @brief Read the XML tree of the content of an ODT file.
 *
 * The OdfTextReader is used to traverse (read) the text contents of
 * an ODF file using an XML stream reader.  For every XML element that
 * the reading process comes across it will call a specific function
 * in a backend class: @see OdfTextReaderBackend.  The OdfTextReader
 * is used as a common way to reat text content and is called from all
 * readers for different ODF formats.  @see OdtReader, @see OdsReader,
 * @see OdpReader.
 */
class KOODFREADER_EXPORT OdfTextReader
{
 public:
    OdfTextReader();
    ~OdfTextReader();

    // Read all common text level elements like text:p, text:h, draw:frame, etc.
    // This is the main entry point for text reading.
    void readTextLevelElement(KoXmlStreamReader &reader);

    void setBackend(OdfTextReaderBackend *backend);
    void setContext(OdfReaderContext *context);

 protected:
    // All readElement*() are named after the full qualifiedName of
    // the element in ODF that they handle.

    // ----------------------------------------------------------------
    // Text level functions: paragraphs, headings, sections, frames, objects, etc

    void readElementOfficeAnnotation(KoXmlStreamReader &reader);
    void readElementOfficeAnnotationEnd(KoXmlStreamReader &reader);

    void readElementDcCreator(KoXmlStreamReader &reader);
    void readElementDcDate(KoXmlStreamReader &reader);

    void readElementTextH(KoXmlStreamReader &reader);
    void readElementTextP(KoXmlStreamReader &reader);
    void readElementTextA(KoXmlStreamReader &reader);

    void readElementTableTable(KoXmlStreamReader &reader);
    void readElementTableTableColumn(KoXmlStreamReader &reader);
    void readElementTableTableHeaderRows(KoXmlStreamReader &reader);
    void readElementTableTableRow(KoXmlStreamReader &reader);
    void readElementTableTableCell(KoXmlStreamReader &reader);
    void readElementTableCoveredTableCell(KoXmlStreamReader &reader);

    // ----------------------------------------------------------------
    // Paragraph level functions: spans, annotations, notes, text content itself, etc.

    void readParagraphContents(KoXmlStreamReader &reader);

    void readElementTextLineBreak(KoXmlStreamReader &reader);
    void readElementTextS(KoXmlStreamReader &reader);
    void readElementTextSpan(KoXmlStreamReader &reader);

    // ----------------------------------------------------------------
    // Other functions

    void readElementTextSoftPageBreak(KoXmlStreamReader &reader);
    void readUnknownElement(KoXmlStreamReader &reader);


 private:
    OdfTextReaderBackend  *m_backend;
    OdfReaderContext      *m_context;
};

#endif // ODFTEXTREADER_H
