/* This file is part of the KDE project
   Copyright (C) 2002   Lucijan Busch <lucijan@gmx.at>
   Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#ifndef KEXI_PROJECT_H
#define KEXI_PROJECT_H

#include <qobject.h>
#include <qmap.h>
#include <qvaluelist.h>

#include "kexiDB/kexidb.h"
#include "kexiformmanager.h"

class QDomElement;
class KoStore;
class KexiDoc;

struct FileReference
{
	QString group;
	QString name;
	QString location;
};

struct Credentials
{
	QString host,
			database,
			port,
			driver,
			user,
			password,
			socket;
	bool savePassword;
};

typedef QMap<QString, QDomElement> Groups;
typedef QValueList<FileReference> References;

class KexiProject : public QObject
{
Q_OBJECT
public:
	KexiProject(QObject* parent);
	~KexiProject();

	bool saveProject();
	bool saveProjectAs(const QString& url);
	bool loadProject(const QString& url);

	void addFileReference(FileReference);
	References fileReferences(QString group);

	bool initDbConnection(const Credentials& cred, const bool create = false);
	bool initHostConnection(const Credentials &cred);
	void clear();

	void setModified();

	KexiDB* db()const { return m_db; };
	KexiFormManager *formManager()const {return m_formManager;}
	QString url()const { return m_url; }
	bool modified()const { return m_modified; }
	bool dbIsAvaible()const { return m_dbAvaible; }
	QString boolToString(bool b);
	bool stringToBool(const QString s);

signals:
	void docModified();
	void dbAvaible();
	void saving(KoStore *);

protected:
	void setCurrentDB(){} ;

private:
	KexiDoc*	m_settings;
	KexiDB*		m_db;
	KexiFormManager	*m_formManager;
	Credentials	m_cred;
	QString		m_url;
	bool		m_modified;
	bool		m_dbAvaible;
	References	m_fileReferences;
	Groups		m_refGroups;
};

#endif
