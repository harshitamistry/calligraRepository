/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 or later of the License.
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

#ifndef KISMULTISENSORSMODEL_H_
#define KISMULTISENSORSMODEL_H_

#include <QAbstractListModel>
#include <kis_dynamic_sensor.h>

class KisCubicCurve;
class KisCurveOption;


class KisMultiSensorsModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit KisMultiSensorsModel(QObject* parent = 0);

    virtual ~KisMultiSensorsModel();

    void setCurveOption(KisCurveOption *curveOption);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    virtual Qt::ItemFlags flags(const QModelIndex & index) const;

    KisDynamicSensorSP getSensor(const QModelIndex& index);

    void setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve);

    /**
     * Create an index that correspond to the sensor given in argument.
     */
    QModelIndex sensorIndex(KisDynamicSensorSP arg1);

    void resetCurveOption();

signals:

    void sensorChanged(KisDynamicSensorSP sensor);

    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();

private:

    KisCurveOption *m_curveOption;
};

#endif
