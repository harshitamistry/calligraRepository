/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "fastcolortransfer.h"

#include <math.h>

#include <kpluginfactory.h>

#include <kundo2command.h>

#include <KoColorSpaceRegistry.h>
#include <KoProgressUpdater.h>
#include <KoUpdater.h>

#include <filter/kis_filter_registry.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_selection.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>

#include "kis_wdg_fastcolortransfer.h"
#include "ui_wdgfastcolortransfer.h"
#include <kis_iterator_ng.h>

K_PLUGIN_FACTORY(KritaFastColorTransferFactory, registerPlugin<FastColorTransferPlugin>();)
K_EXPORT_PLUGIN(KritaFastColorTransferFactory("krita"))


FastColorTransferPlugin::FastColorTransferPlugin(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterFastColorTransfer());

}

FastColorTransferPlugin::~FastColorTransferPlugin()
{
}

KisFilterFastColorTransfer::KisFilterFastColorTransfer() : KisFilter(id(), categoryColors(), i18n("&Color Transfer..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsThreading(false);
    setSupportsPainting(false);
    setSupportsAdjustmentLayers(false);
}


KisConfigWidget * KisFilterFastColorTransfer::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const
{
    Q_UNUSED(dev);
    return new KisWdgFastColorTransfer(parent);
}

KisFilterConfiguration* KisFilterFastColorTransfer::factoryConfiguration(const KisPaintDeviceSP) const
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    config->setProperty("filename", "");  // TODO: put an exemple image in share/krita, like a sunset that what's give the best results
    return config;
}

#define CLAMP(x,l,u) ((x)<(l)?(l):((x)>(u)?(u):(x)))

void KisFilterFastColorTransfer::processImpl(KisPaintDeviceSP device,
                                             const QRect& applyRect,
                                             const KisFilterConfiguration* config,
                                             KoUpdater* progressUpdater) const
{
    Q_ASSERT(device != 0);

    dbgPlugins << "Start transferring color";

    // Convert ref and src to LAB
    const KoColorSpace* labCS = KoColorSpaceRegistry::instance()->lab16();
    if (!labCS) {
        dbgPlugins << "The LAB colorspace is not available.";
        return;
    }
    
    dbgPlugins << "convert a copy of src to lab";
    const KoColorSpace* oldCS = device->colorSpace();
    KisPaintDeviceSP srcLAB = new KisPaintDevice(*device.data());
    dbgPlugins << "srcLab : " << srcLAB->extent();
    KUndo2Command* cmd = srcLAB->convertTo(labCS, KoColorConversionTransformation::InternalRenderingIntent, KoColorConversionTransformation::InternalConversionFlags);
    delete cmd;

    if (progressUpdater) {
        progressUpdater->setRange(0, 2 * applyRect.width() * applyRect.height());
    }
    int count = 0;

    // Compute the means and sigmas of src
    dbgPlugins << "Compute the means and sigmas of src";
    double meanL_src = 0., meanA_src = 0., meanB_src = 0.;
    double sigmaL_src = 0., sigmaA_src = 0., sigmaB_src = 0.;

    KisSequentialConstIterator srcIt(srcLAB, applyRect);

    do {
        const quint16* data = reinterpret_cast<const quint16*>(srcIt.oldRawData());
        quint32 L = data[0];
        quint32 A = data[1];
        quint32 B = data[2];
        meanL_src += L;
        meanA_src += A;
        meanB_src += B;
        sigmaL_src += L * L;
        sigmaA_src += A * A;
        sigmaB_src += B * B;
        if (progressUpdater) progressUpdater->setValue(++count);
    } while (srcIt.nextPixel() && !(progressUpdater && progressUpdater->interrupted()));
    
    double totalSize = 1. / (applyRect.width() * applyRect.height());
    meanL_src *= totalSize;
    meanA_src *= totalSize;
    meanB_src *= totalSize;
    sigmaL_src *= totalSize;
    sigmaA_src *= totalSize;
    sigmaB_src *= totalSize;
    
    dbgPlugins << totalSize << "" << meanL_src << "" << meanA_src << "" << meanB_src << "" << sigmaL_src << "" << sigmaA_src << "" << sigmaB_src;
    
    double meanL_ref = config->getDouble("meanL");
    double meanA_ref = config->getDouble("meanA");
    double meanB_ref = config->getDouble("meanB");
    double sigmaL_ref = config->getDouble("sigmaL");
    double sigmaA_ref = config->getDouble("sigmaA");
    double sigmaB_ref = config->getDouble("sigmaB");
    
    
    // Transfer colors
    dbgPlugins << "Transfer colors";
    {
        double coefL = sqrt((sigmaL_ref - meanL_ref * meanL_ref) / (sigmaL_src - meanL_src * meanL_src));
        double coefA = sqrt((sigmaA_ref - meanA_ref * meanA_ref) / (sigmaA_src - meanA_src * meanA_src));
        double coefB = sqrt((sigmaB_ref - meanB_ref * meanB_ref) / (sigmaB_src - meanB_src * meanB_src));
        KisHLineConstIteratorSP srcLABIt = srcLAB->createHLineConstIteratorNG(applyRect.x(), applyRect.y(), applyRect.width());
        KisHLineIteratorSP dstIt = device->createHLineIteratorNG(applyRect.x(), applyRect.y(), applyRect.width());
        quint16 labPixel[4];
        for (int y = 0; y < applyRect.height() && !(progressUpdater && progressUpdater->interrupted()); ++y) {
            do {
                const quint16* data = reinterpret_cast<const quint16*>(srcLABIt->oldRawData());
                labPixel[0] = (quint16)CLAMP(((double)data[0] - meanL_src) * coefL + meanL_ref, 0., 65535.);
                labPixel[1] = (quint16)CLAMP(((double)data[1] - meanA_src) * coefA + meanA_ref, 0., 65535.);
                labPixel[2] = (quint16)CLAMP(((double)data[2] - meanB_src) * coefB + meanB_ref, 0., 65535.);
                labPixel[3] = data[3];
                oldCS->fromLabA16(reinterpret_cast<const quint8*>(labPixel), dstIt->rawData(), 1);
                if (progressUpdater) progressUpdater->setValue(++count);
                srcLABIt->nextPixel();
            } while(dstIt->nextPixel());
            dstIt->nextRow();
            srcLABIt->nextRow();
        }

    }
}
