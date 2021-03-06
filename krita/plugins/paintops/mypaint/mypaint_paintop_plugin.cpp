/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "mypaint_paintop_plugin.h"
#include <klocale.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_paintop_registry.h>

#include "mypaint_paintop_factory.h"
#include "kis_global.h"

K_PLUGIN_FACTORY(MyPaintPluginFactory, registerPlugin<MyPaintPlugin>();)
K_EXPORT_PLUGIN(MyPaintPluginFactory("krita"))


MyPaintPlugin::MyPaintPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    MyPaintFactory* factory = new MyPaintFactory;
    r->add(factory);
}

MyPaintPlugin::~MyPaintPlugin()
{
}

#include "mypaint_paintop_plugin.moc"
