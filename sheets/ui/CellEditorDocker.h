/* This file is part of the KDE project
   Copyright 2011 Marijn Kruisselbrink <mkruisselbrink@kde.org>

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

#ifndef CALLIGRA_SHEETS_CELL_EDITOR_DOCKER
#define CALLIGRA_SHEETS_CELL_EDITOR_DOCKER

#include <QDockWidget>

#include <KoCanvasObserverBase.h>
#include <KoDockFactoryBase.h>

namespace Calligra
{
namespace Sheets
{
class Sheet;

class CellEditorDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    explicit CellEditorDocker();
    virtual ~CellEditorDocker();

    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

protected: // reimplementations
    virtual void resizeEvent(QResizeEvent *event);

private Q_SLOTS:
    void toolChanged(const QString& toolId);

private:
    class Private;
    Private *const d;
};

class CellEditorDockerFactory : public KoDockFactoryBase
{
public:
    CellEditorDockerFactory();

    virtual QString id() const;
    virtual QDockWidget* createDockWidget();
    DockPosition defaultDockPosition() const {
        return DockTop;
    }
};

} // namespace Sheets
} // namespace Calligra

#endif // CALLIGRA_SHEETS_CELL_EDITOR_DOCKER
