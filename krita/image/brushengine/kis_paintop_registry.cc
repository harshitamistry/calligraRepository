/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_paintop_registry.h"

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoGenericRegistry.h>
#include <KoPluginLoader.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOp.h>
#include <KoID.h>

#include "kis_types.h"

#include "kis_paint_device.h"
#include "kis_paintop.h"
#include "kis_painter.h"
#include "kis_debug.h"
#include "kis_layer.h"
#include "kis_image.h"
#include "kis_paintop_settings_widget.h"

KisPaintOpRegistry::KisPaintOpRegistry()
{
}

KisPaintOpRegistry::~KisPaintOpRegistry()
{
    foreach(const QString &id, keys()) {
        delete get(id);
    }
    dbgRegistry << "Deleting KisPaintOpRegistry";
}

KisPaintOpRegistry* KisPaintOpRegistry::instance()
{
    K_GLOBAL_STATIC(KisPaintOpRegistry, s_instance);
    if (!s_instance.exists()) {
        KoPluginLoader::instance()->load("Krita/Paintop", "(Type == 'Service') and ([X-Krita-Version] == 28)");

        QStringList toBeRemoved;

        foreach(const QString &id, s_instance->keys()) {
            KisPaintOpFactory *factory = s_instance->get(id);
            if (!factory->settings()) {
                toBeRemoved << id;
            }
            else {
                factory->processAfterLoading();
            }
        }
        foreach(const QString &id, toBeRemoved) {
            s_instance->remove(id);
        }
    }
    return s_instance;
}

#ifdef HAVE_THREADED_TEXT_RENDERING_WORKAROUND
void KisPaintOpRegistry::preinitializePaintOpIfNeeded(const KisPaintOpPresetSP preset)
{
    if (!preset) return;

    KisPaintOpFactory *f = value(preset->paintOp().id());
    f->preinitializePaintOpIfNeeded(preset->settings());
}
#endif /* HAVE_THREADED_TEXT_RENDERING_WORKAROUND */

KisPaintOp * KisPaintOpRegistry::paintOp(const QString & id, const KisPaintOpSettingsSP settings, KisPainter * painter, KisImageWSP image) const
{
    if (painter == 0) {
        warnKrita << " KisPaintOpRegistry::paintOp painter is null";
        return 0;
    }

    Q_ASSERT(settings);

    KisPaintOpFactory* f = value(id);
    if (f) {
        KisPaintOp * op = f->createOp(settings, painter, image);
        if (op) {
            return op;
        }
    }
    qWarning() << "Could not create paintop for factory" << id << "with settings" << settings;
    return 0;
}

KisPaintOp * KisPaintOpRegistry::paintOp(const KisPaintOpPresetSP preset, KisPainter * painter, KisImageWSP image) const
{
    Q_ASSERT(preset);
    Q_ASSERT(painter);

    if (!preset) return 0;

    return paintOp(preset->paintOp().id(), preset->settings(), painter, image);
}

KisPaintOpSettingsSP KisPaintOpRegistry::settings(const KoID& id) const
{
    KisPaintOpFactory *f = value(id.id());
    Q_ASSERT(f);
    if (f) {
        KisPaintOpSettingsSP settings = f->settings();
        settings->setProperty("paintop", id.id());
        return settings;
    }
    return 0;
}

KisPaintOpPresetSP KisPaintOpRegistry::defaultPreset(const KoID& id) const
{
    KisPaintOpPresetSP preset = new KisPaintOpPreset();
    preset->setName(i18n("default"));

    KisPaintOpSettingsSP s = settings(id);

    if (s.isNull()) {
        return 0;
    }

    preset->setSettings(s);
    preset->setPaintOp(id);
    Q_ASSERT(!preset->paintOp().id().isEmpty());
    preset->setValid(true);
    return preset;
}

bool KisPaintOpRegistry::userVisible(const KoID & id, const KoColorSpace* cs) const
{

    KisPaintOpFactory* f = value(id.id());
    if (!f) {
        dbgRegistry << "No paintop" << id.id() << "";
        return false;
    }
    return f->userVisible(cs);

}

QString KisPaintOpRegistry::pixmap(const KoID & id) const
{
    KisPaintOpFactory* f = value(id.id());

    if (!f) {
        dbgRegistry << "No paintop" << id.id() << "";
        return "";
    }

    return f->pixmap();
}

QList<KoID> KisPaintOpRegistry::listKeys() const
{
    QList<KoID> answer;
    foreach (const QString &key, keys()) {
        answer.append(KoID(key, get(key)->name()));
    }

    return answer;
}

#include "kis_paintop_registry.moc"
