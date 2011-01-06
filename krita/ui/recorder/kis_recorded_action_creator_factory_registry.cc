/*
 *  Copyright (c) 2009,2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_action_creator_factory_registry.h"

#include <KoGenericRegistry.h>

#include <qalgorithms.h>
#include <QList>
#include <QWidget>

#include <kglobal.h>

#include <kis_debug.h>
#include "kis_recorded_action_creator_factory.h"

struct KisRecordedActionCreatorFactoryRegistry::Private {
    KoGenericRegistry<KisRecordedActionCreatorFactory*> factories;
};

KisRecordedActionCreatorFactoryRegistry::KisRecordedActionCreatorFactoryRegistry()
        : d(new Private)
{
}

KisRecordedActionCreatorFactoryRegistry::~KisRecordedActionCreatorFactoryRegistry()
{
    delete d;
}

KisRecordedActionCreatorFactoryRegistry* KisRecordedActionCreatorFactoryRegistry::instance()
{
    K_GLOBAL_STATIC(KisRecordedActionCreatorFactoryRegistry, s_instance);
    return s_instance;
}

void KisRecordedActionCreatorFactoryRegistry::add(KisRecordedActionCreatorFactory* factory)
{
    d->factories.add(factory);
}

KisRecordedActionCreatorFactory* KisRecordedActionCreatorFactoryRegistry::get(const QString& _id) const
{
    return d->factories.get(_id);
}

QList<KoID> KisRecordedActionCreatorFactoryRegistry::creators() const
{
    QList<KoID> cs;
    foreach(QString id, d->factories.keys())
    {
	cs.push_back(KoID(id, d->factories.get(id)->name()));
    }
    return cs;
}