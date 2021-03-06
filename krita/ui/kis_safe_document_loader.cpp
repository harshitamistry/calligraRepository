/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_safe_document_loader.h"

#include <QTimer>
#include <QFileSystemWatcher>
#include <QApplication>

#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_signal_compressor.h"


struct KisSafeDocumentLoader::Private
{
    Private()
        : doc(new KisDoc2()),
          fileChangedSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE),
          isLoading(false),
          fileChangedFlag(false)
    {
    }

    QScopedPointer<KisDoc2> doc;
    QFileSystemWatcher fileWatcher;
    KisSignalCompressor fileChangedSignalCompressor;
    QTimer delayedLoadTimer;
    bool isLoading;
    bool fileChangedFlag;
    QString path;
    QString temporaryPath;

    qint64 initialFileSize;
    QDateTime initialFileTimeStamp;
};

KisSafeDocumentLoader::KisSafeDocumentLoader(const QString &path, QObject *parent)
    : QObject(parent),
      m_d(new Private())
{
    connect(&m_d->fileWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged()));

    connect(&m_d->fileWatcher, SIGNAL(fileChanged(QString)),
            &m_d->fileChangedSignalCompressor, SLOT(start()));

    connect(&m_d->fileChangedSignalCompressor, SIGNAL(timeout()),
            SLOT(fileChangedCompressed()));

    connect(&m_d->delayedLoadTimer, SIGNAL(timeout()),
            SLOT(delayedLoadStart()));

    m_d->delayedLoadTimer.setSingleShot(true);
    m_d->delayedLoadTimer.setInterval(100 /* ms */);

    setPath(path);
}

KisSafeDocumentLoader::~KisSafeDocumentLoader()
{
    delete m_d;
}

KisImageWSP KisSafeDocumentLoader::image() const
{
    return m_d->doc->image();
}

void KisSafeDocumentLoader::setPath(const QString &path)
{
    if (path.isEmpty()) return;

    if (!m_d->path.isEmpty()) {
        m_d->fileWatcher.removePath(m_d->path);
    }

    m_d->path = path;
    m_d->fileWatcher.addPath(m_d->path);
}

void KisSafeDocumentLoader::reloadImage()
{
    fileChangedCompressed(true);
}

void KisSafeDocumentLoader::fileChanged()
{
    m_d->fileChangedFlag = true;
}

void KisSafeDocumentLoader::fileChangedCompressed(bool sync)
{
    if (m_d->isLoading) return;

    QFileInfo initialFileInfo(m_d->path);
    m_d->initialFileSize = initialFileInfo.size();
    m_d->initialFileTimeStamp = initialFileInfo.lastModified();

    // it may happen when the file is flushed by
    // so other application
    if (!m_d->initialFileSize) return;

    m_d->isLoading = true;
    m_d->fileChangedFlag = false;

    m_d->temporaryPath =
        QDir::tempPath() + QDir::separator() +
        QString("krita_file_layer_copy_%1_%2.%3")
        .arg(QApplication::applicationPid())
        .arg(qrand())
        .arg(initialFileInfo.suffix());

    QFile::copy(m_d->path, m_d->temporaryPath);


    if (!sync) {
        m_d->delayedLoadTimer.start();
    } else {
        QApplication::processEvents();
        delayedLoadStart();
    }
}

void KisSafeDocumentLoader::delayedLoadStart()
{
    QFileInfo originalInfo(m_d->path);
    QFileInfo tempInfo(m_d->temporaryPath);
    bool successfullyLoaded = false;

    if (!m_d->fileChangedFlag &&
        originalInfo.size() == m_d->initialFileSize &&
        originalInfo.lastModified() == m_d->initialFileTimeStamp &&
        tempInfo.size() == m_d->initialFileSize) {

        successfullyLoaded = m_d->doc->openUrl(m_d->temporaryPath);
    } else {
        qDebug() << "File was modified externally. Restarting.";
        qDebug() << ppVar(m_d->fileChangedFlag);
        qDebug() << ppVar(m_d->initialFileSize);
        qDebug() << ppVar(m_d->initialFileTimeStamp);
        qDebug() << ppVar(originalInfo.size());
        qDebug() << ppVar(originalInfo.lastModified());
        qDebug() << ppVar(tempInfo.size());
    }

    QFile::remove(m_d->temporaryPath);
    m_d->isLoading = false;

    if (!successfullyLoaded) {
        // Restart the attempt
        m_d->fileChangedSignalCompressor.start();
    } else {
        emit loadingFinished();
    }
}


