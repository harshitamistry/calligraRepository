/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000 Michael Johnson <mikej@xnet.com>
   Copyright (C) 2001, 2002, 2004 Nicolas GOUTTE <goutte@kde.org>
   Copyright (C) 2010, Thorsten Zachmann <zachmann@kde.org>

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

#include "AsciiImport.h"

#include <QTextCodec>
#include <QFile>

#include <kdebug.h>
#include <kpluginfactory.h>

#include <kofficeversion.h>
#include <KoFilterChain.h>
#include <KoFilterManager.h>
#include <KoStore.h>
#include <KoOdfWriteStore.h>
#include <KoGenStyles.h>
#include <KoXmlWriter.h>

#include "ImportDialog.h"

#define MAXLINES 10000

K_PLUGIN_FACTORY(AsciiImportFactory, registerPlugin<AsciiImport>();)
K_EXPORT_PLUGIN(AsciiImportFactory("kwordasciiimportng", "kofficefilters"))

AsciiImport::AsciiImport(QObject *parent, const QVariantList &)
: KoFilter(parent)
{
}

AsciiImport::~AsciiImport()
{
}

KoFilter::ConversionStatus AsciiImport::convert(const QByteArray& from, const QByteArray& to)
{
    // check for proper conversion
    if (to != "application/vnd.oasis.opendocument.text" || from != "text/plain") {
        return KoFilter::NotImplemented;
    }

    AsciiImportDialog* dialog = 0;
    if (!m_chain->manager()->getBatchMode()) {
        dialog = new AsciiImportDialog(QApplication::activeWindow());
        if (!dialog) {
            kError(30502) << "Dialog has not been created! Aborting!" << endl;
            return KoFilter::StupidError;
        }
        if (!dialog->exec()) {
            kDebug(30502) << "Dialog was aborted! Aborting filter!"; // this isn't an error!
            return KoFilter::UserCancelled;
        }
    }

    QTextCodec* codec;

    int paragraphStrategy = 0;
    if (dialog) {
        codec = dialog->getCodec();
        paragraphStrategy = dialog->getParagraphStrategy();
        delete dialog;
    }
    else {
        codec = QTextCodec::codecForName("UTF-8");
    }

    if (!codec) {
        kError(30502) << "Could not create QTextCodec! Aborting" << endl;
        return KoFilter::StupidError;
    }

    kDebug(30502) << "Charset used:" << codec->name();

    //create output files
    KoStore *store = KoStore::createStore(m_chain->outputFile(), KoStore::Write, to, KoStore::Zip);
    if (!store || store->bad()) {
        kWarning(30003) << "Unable to open output file!";
        delete store;
        return KoFilter::FileNotFound;
    }
    store->disallowNameExpansion();
    kDebug(30003) << "created store.";
    KoOdfWriteStore odfStore(store);
    odfStore.manifestWriter(to);

    KoXmlWriter* contentWriter = odfStore.contentWriter();
    if (!contentWriter) {
        delete store;
        return KoFilter::CreationError;
    }

    KoGenStyles mainStyles;
    KoXmlWriter *bodyWriter = odfStore.bodyWriter();

    bodyWriter->startElement("office:body");
    bodyWriter->startElement("office:text");

    QFile in(m_chain->inputFile());
    if (!in.open(QIODevice::ReadOnly)) {
        kError(30502) << "Unable to open input file!" << endl;
        in.close();
        return KoFilter::FileNotFound;
    }

    QTextStream stream(&in);
    stream.setCodec(codec);

    QString styleName("Preformated Text");
    KoGenStyle style(KoGenStyle::ParagraphStyle, "paragraph");
    style.addAttribute("style:display-name", styleName);
    style.addProperty("fo:font-family", "dejavu sans mono", KoGenStyle::TextType);
    style.addProperty("fo:font-family-generic", "modern", KoGenStyle::TextType);
    style.addProperty("fo:font-size", "10pt", KoGenStyle::TextType);

    style.addProperty("fo:font-weight", "normal", KoGenStyle::TextType);
    QString name(QString(QUrl::toPercentEncoding(styleName, "", " ")).replace('%', '_'));

    name = mainStyles.insert(style, name, KoGenStyles::DontAddNumberToName);

    switch (paragraphStrategy) {
    case 1:
        convertSentence(stream, bodyWriter, name);
        break;
    case 2:
        convertEmptyLine(stream, bodyWriter, name);
        break;
    default:
        convertAsIs(stream, bodyWriter, name);
        break;
    }

    bodyWriter->endElement(); // office:text
    bodyWriter->endElement(); // office:body

    mainStyles.saveOdfStyles(KoGenStyles::DocumentAutomaticStyles, contentWriter);
    odfStore.closeContentWriter();

    //add manifest line for content.xml
    odfStore.manifestWriter()->addManifestEntry("content.xml", "text/xml");
    if (!mainStyles.saveOdfStylesDotXml(odfStore.store(), odfStore.manifestWriter())) {
        delete store;
        return KoFilter::CreationError;
    }

    if (!createMeta(odfStore)) {
        kWarning() << "Error while trying to write 'meta.xml'. Partition full?";
        delete store;
        return KoFilter::CreationError;
    }


    if ( !odfStore.closeManifestWriter() ) {
        kWarning() << "Error while trying to write 'META-INF/manifest.xml'. Partition full?";
        delete store;
        return KoFilter::CreationError;
    }

    delete store;
    return KoFilter::OK;
}

void AsciiImport::convertAsIs(QTextStream &stream, KoXmlWriter *bodyWriter, const QString &styleName)
{
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (!line.isNull()) {
            bodyWriter->startElement("text:p");
            bodyWriter->addAttribute("text:style-name", styleName);
            bodyWriter->addTextSpan(line);
            bodyWriter->endElement();
        }
    }
}

void AsciiImport::convertSentence(QTextStream &stream, KoXmlWriter *bodyWriter, const QString &styleName)
{
    QString stoppingPunctuation(".!?");
    QString skippingEnd(" \"')");

    while (!stream.atEnd()) {
        QString paragraph;
        for (;;) {
            const QString line = stream.readLine();
            if (line.isEmpty()) {
                break;
            }
            paragraph.append(line);
            paragraph += ' ';

            int lastPos = line.length() - 1;
            int maxCheck = lastPos >= 10 ? 10: lastPos + 1;
            QChar lastChar;
            // Skip a maximum of 10 quotes (or similar) at the end of the line
            for (int i = 0; i < maxCheck; i++) {
                lastChar = line[lastPos];
                if (lastChar.isNull() || skippingEnd.indexOf(lastChar) == -1) {
                    break;
                }

                lastPos--;
            }

            lastChar = line[lastPos];
            if (lastChar.isNull())
                continue;
            else if (stoppingPunctuation.indexOf(lastChar) != -1)
                break;
        }

        if (!paragraph.isNull()) {
            bodyWriter->startElement("text:p");
            bodyWriter->addAttribute("text:style-name", styleName);
            bodyWriter->addTextSpan(paragraph.simplified());
            bodyWriter->endElement();
        }
    }
}

void AsciiImport::convertEmptyLine(QTextStream &stream, KoXmlWriter *bodyWriter, const QString &styleName)
{
    while (!stream.atEnd()) {
        QString paragraph;
        for (int line_no = 0; line_no < MAXLINES; ++line_no) {
            const QString line = stream.readLine();
            if (line.isEmpty()) {
                break;
            }
            paragraph.append(line);
            paragraph += ' ';
        }

        if (!paragraph.isNull()) {
            bodyWriter->startElement("text:p");
            bodyWriter->addAttribute("text:style-name", styleName);
            bodyWriter->addTextSpan(paragraph.simplified());
            bodyWriter->endElement();
        }
    }
}

bool AsciiImport::createMeta(KoOdfWriteStore &store)
{
    if (!store.store()->open("meta.xml")) {
        return false;
    }

    KoStoreDevice dev(store.store());
    KoXmlWriter* xmlWriter = KoOdfWriteStore::createOasisXmlWriter(&dev, "office:document-meta");
    xmlWriter->startElement("office:meta");

    xmlWriter->startElement("meta:generator");
    xmlWriter->addTextNode(QString("KOConverter/%1").arg(KOFFICE_VERSION_STRING));
    xmlWriter->endElement();

    xmlWriter->startElement("meta:creation-date");
    xmlWriter->addTextNode(QDateTime::currentDateTime().toString(Qt::ISODate));
    xmlWriter->endElement();

    xmlWriter->endElement();
    xmlWriter->endElement(); // root element
    xmlWriter->endDocument(); // root element
    delete xmlWriter;
    if (!store.store()->close()) {
        return false;
    }
    store.manifestWriter()->addManifestEntry("meta.xml", "text/xml" );
    return true;
}