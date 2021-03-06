/* This file is part of the KDE project
 *
 * Copyright (C) 2013-2014 Inge Wallin <inge@lysator.liu.se>
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


#ifndef KOODF_OPC_RELSET_MANAGER_H
#define KOODF_OPC_RELSET_MANAGER_H

#include "koodf_export.h"


class QString;
class KoStore;
class KoXmlStreamReader;
class KoXmlWriter;
class OpcRelSet;


class OpcRelSetManager
{
 public:
    OpcRelSetManager();
    ~OpcRelSetManager();

    OpcRelSet *relSet(const QString &path) const;
    void setRelSet(const QString &path, OpcRelSet *relSet);

    OpcRelSet *documentRelSet() const;
    void setDocumentRelSet(OpcRelSet *relSet);

    void clear();

    bool loadRelSets(KoStore *odfStore);
    bool saveRelSets(KoStore *odfStore);

 private:
    class Private;
    Private * const d;
};


#endif
