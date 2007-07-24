/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_BOOKMARKED_CONFIGURATIONS_MODEL_H_
#define _KIS_BOOKMARKED_CONFIGURATIONS_MODEL_H_

#include <QAbstractListModel>

class KisBookmarkedConfigurationManager;
class KisSerializableConfiguration;

/**
 * This class provides basic functionnalities for a model of a bookmark
 * of configurations.
 */
class KisBookmarkedConfigurationsModel : public QAbstractListModel {
    public:
        /**
         * Initialized thee model with the bookmarks manager
         */
        KisBookmarkedConfigurationsModel(KisBookmarkedConfigurationManager*);
        ~KisBookmarkedConfigurationsModel();
        /**
         * @return the number of configurations (the minimum is allways 2, the default
         * configuration and the last used configuration are allways present)
         */
        virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
        /**
         * When role == Qt::DisplayRole, this function will return the name of the
         * configuration.
         */
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        /**
         * @return the configuration at the given index
         */
        KisSerializableConfiguration* configuration(const QModelIndex &index) const;
    private:
        struct Private;
        Private* const d;
};

#endif
