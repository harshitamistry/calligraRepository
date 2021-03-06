/*
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISTOOLTIPMANAGER_H
#define KISTOOLTIPMANAGER_H

#include <QObject>
#include <QMap>

class KisView2;
class KisTooltipManager : public QObject
{
    Q_OBJECT

public:
    KisTooltipManager(KisView2* view);
    ~KisTooltipManager();

    void record();

private slots:
    void captureToolip();

private:
    KisView2* m_view;
    bool m_recording;
    QMap<QString, QString> m_tooltipMap;
};

#endif // KISTOOLTIPMANAGER_H
