/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_FIXED_PAINT_DEVICE_TESTER_H
#define KIS_FIXED_PAINT_DEVICE_TESTER_H

#include <QtTest>

class KisFixedPaintDeviceTest : public QObject
{
    Q_OBJECT

private slots:

    void testCreation();
    void testSilly();
    void testClear();
    void testFill();
    void testRoundtripQImageConversion();
    void testBltFixed();
    void testBltFixedOpacity();
    void testBltFixedSmall();
    void testColorSpaceConversion();
    void testBltPerformance();
    void testMirroring_data();
    void testMirroring();
};

#endif

