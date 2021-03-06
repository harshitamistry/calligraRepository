/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Plugin.h"
#include "CommentShapeFactory.h"
#include "CommentToolFactory.h"

#include <KoToolRegistry.h>
#include <KoShapeRegistry.h>

#include <kpluginfactory.h>

K_PLUGIN_FACTORY(PluginFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(PluginFactory("CommentShape"))

Plugin::Plugin(QObject* parent, const QVariantList&)
: QObject(parent)
{
    KoShapeRegistry::instance()->add(new CommentShapeFactory());
    KoToolRegistry::instance()->add(new CommentToolFactory());
}

#include <Plugin.moc>
