/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <kxmlguiwindow.h>
#include <KoCanvasSupervisor.h>

class DockerManager;
class RootSection;
class KComponentData;
class KoDockFactoryBase;
class View;
class KActionMenu;

class MainWindow : public KXmlGuiWindow, public KoCanvasSupervisor {
    Q_OBJECT
  public:
    MainWindow(RootSection* document, const KComponentData &componentData);
    ~MainWindow();
  public:
    QDockWidget* createDockWidget(KoDockFactoryBase* factory);
    DockerManager* dockerManager();
    void addStatusBarItem(QWidget*, int strech, View* view);
    void removeStatusBarItem(QWidget*);
  private:
    void setupActions();
  public:
    void activateView(View* view);
    QList<KoCanvasObserverBase*> canvasObservers();
  public slots:
    void forceDockTabFonts();
  private:
    RootSection* m_doc;
    View* view;
    View* m_activeView;
    QMap<QString, QDockWidget*> m_dockWidgetMap;
    QList<QDockWidget*> m_dockWidgets;
    KActionMenu* m_dockWidgetMenu;
    DockerManager* m_dockerManager;
    struct StatusBarItem;
    QMap<View*, QList<StatusBarItem*> > m_statusBarItems;
};

#endif