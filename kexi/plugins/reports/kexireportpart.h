/* This file is part of the KDE project
   Copyright (C) 2004 Lucijan Busch <lucijan@kde.org>
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KEXIREPORTPART_H
#define KEXIREPORTPART_H

#include <kexi.h>
#include <kexipart.h>
#include <kexidialogbase.h>

namespace KFormDesigner
{
	class FormManager;
	class Form;
}

namespace KexiDB
{
	class FieldList;
}

//! Kexi Report Plugin
/*! It just creates a \ref KexiReportView. See there for most of code. */
class KexiReportPart : public KexiPart::Part
{
	Q_OBJECT

	public:
		KexiReportPart(QObject *parent, const char *name, const QStringList &);
		virtual ~KexiReportPart();

		KFormDesigner::FormManager *manager() { return m_manager; }

		void generateForm(KexiDB::FieldList *list, QDomDocument &domDoc);

		class TempData : public KexiDialogTempData
		{
			public:
				TempData(QObject* parent);
				~TempData();
				QGuardedPtr<KFormDesigner::Form> form;
				QGuardedPtr<KFormDesigner::Form> previewForm;
				QString tempForm;
				QPoint scrollViewContentsPos; //!< to preserve contents pos after switching to other view
				int resizeMode; //!< form's window's resize mode -one of KexiFormView::ResizeMode items
		};

		virtual QString i18nMessage(const QCString& englishMessage) const;

	protected:
		virtual KexiDialogTempData* createTempData(KexiDialogBase* dialog);

		virtual KexiViewBase* createView(QWidget *parent, KexiDialogBase* dialog,
			KexiPart::Item &item, int viewMode = Kexi::DataViewMode);

		virtual void initPartActions();
		virtual void initInstanceActions();

	private:
		QGuardedPtr<KFormDesigner::FormManager> m_manager;
};

#endif

