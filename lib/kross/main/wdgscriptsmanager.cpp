/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
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
#include "wdgscriptsmanager.h"

#include <qobjectlist.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <klistview.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <ktoolbar.h>
#include <kiconloader.h>
#include <kfiledialog.h>

#include "scriptguiclient.h"
#include "scriptaction.h"

namespace Kross { namespace Api {

class ListItem : public QListViewItem
{
    private:
        ScriptActionCollection* m_collection;
        ScriptAction::Ptr m_action;
    public:
        ListItem(QListView* parentview, ScriptActionCollection* collection)
            : QListViewItem(parentview), m_collection(collection), m_action(0) {}

        ListItem(ListItem* parentitem, QListViewItem* afteritem, ScriptAction::Ptr action)
            : QListViewItem(parentitem, afteritem), m_collection( parentitem->collection() ), m_action(action) {}

        ScriptAction::Ptr action() const { return m_action; }
        ScriptActionCollection* collection() const { return m_collection; }
        //ScriptActionMenu* actionMenu() const { return m_menu; }
};

class WdgScriptsManagerPrivate
{
    friend class WdgScriptsManager;
    ScriptGUIClient* m_scripguiclient;
    enum { LoadBtn = 0, InstallBtn, UninstallBtn, ExecBtn, RemoveBtn };
};

WdgScriptsManager::WdgScriptsManager(ScriptGUIClient* scr, QWidget* parent, const char* name, WFlags fl )
    : WdgScriptsManagerBase(parent, name, fl)
    , d(new WdgScriptsManagerPrivate)
{
    d->m_scripguiclient = scr;

    scriptsList->header()->hide();
    //scriptsList->header()->setClickEnabled(false);
    scriptsList->setAllColumnsShowFocus(true);
    //scriptsList->setRootIsDecorated(true);
    scriptsList->setSorting(-1);

    scriptsList->addColumn("");
    //scriptsList->addColumn(i18n("Name"));
    //scriptsList->addColumn(i18n("Action"));

    fillScriptsList();

    connect(scriptsList, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(slotSelectionChanged(QListViewItem*)));

    //toolBar->setIconText( KToolBar::IconTextRight );

    toolBar->insertButton("exec", WdgScriptsManagerPrivate::ExecBtn, false, i18n("Execute"));
    toolBar->addConnection(WdgScriptsManagerPrivate::ExecBtn, SIGNAL(clicked()), this, SLOT(slotExecuteScript()));

    toolBar->insertButton("fileopen", WdgScriptsManagerPrivate::LoadBtn, true, i18n("Load"));
    toolBar->addConnection(WdgScriptsManagerPrivate::LoadBtn, SIGNAL(clicked()), this, SLOT(slotLoadScript()));

    toolBar->insertButton("fileimport", WdgScriptsManagerPrivate::InstallBtn, true, i18n("Install"));
    toolBar->addConnection(WdgScriptsManagerPrivate::InstallBtn, SIGNAL(clicked()), this, SLOT(slotInstallScript()));

    toolBar->insertButton("fileclose", WdgScriptsManagerPrivate::UninstallBtn, false, i18n("Uninstall"));
    toolBar->addConnection(WdgScriptsManagerPrivate::UninstallBtn, SIGNAL(clicked()), this, SLOT(slotUninstallScript()));

    toolBar->insertButton("fileclose", WdgScriptsManagerPrivate::RemoveBtn, false, i18n("Remove"));
    toolBar->addConnection(WdgScriptsManagerPrivate::RemoveBtn, SIGNAL(clicked()), this, SLOT(slotRemoveScript()));

    connect(scr, SIGNAL( collectionChanged(ScriptActionCollection*) ),
            this, SLOT( fillScriptsList() ));
}

WdgScriptsManager::~WdgScriptsManager()
{
    delete d;
}

void WdgScriptsManager::fillScriptsList()
{
    scriptsList->clear();

    addItem( d->m_scripguiclient->getActionCollection("executedscripts") );
    addItem( d->m_scripguiclient->getActionCollection("loadedscripts") );
    addItem( d->m_scripguiclient->getActionCollection("installedscripts") );
}

void WdgScriptsManager::addItem(ScriptActionCollection* collection)
{
    if(! collection)
        return;

    ListItem* i = new ListItem(scriptsList, collection);
    i->setText(0, collection->actionMenu()->text());
    i->setOpen(true);

    QValueList<ScriptAction::Ptr> list = collection->actions();
    QListViewItem* lastitem = 0;
    for(QValueList<ScriptAction::Ptr>::Iterator it = list.begin(); it != list.end(); ++it)
        lastitem = addItem(*it, i, lastitem);
}

QListViewItem* WdgScriptsManager::addItem(ScriptAction::Ptr action, QListViewItem* parentitem, QListViewItem* afteritem)
{
    if(! action)
        return 0;

    ListItem* i = new ListItem(dynamic_cast<ListItem*>(parentitem), afteritem, action);
    i->setText(0, action->text());

    QPixmap pm;
    if(action->hasIcon()) {
        KIconLoader* icons = KGlobal::iconLoader();
        pm = icons->loadIconSet(action->icon(), KIcon::Small).pixmap(QIconSet::Small, QIconSet::Active);
    }
    else {
        pm = action->iconSet(KIcon::Small, 16).pixmap(QIconSet::Small, QIconSet::Active);
    }
    if(! pm.isNull())
        i->setPixmap(0, pm); // display the icon

    return i;
}

void WdgScriptsManager::slotSelectionChanged(QListViewItem* item)
{
    ListItem* i = dynamic_cast<ListItem*>(item);
    Kross::Api::ScriptActionCollection* installedcollection = d->m_scripguiclient->getActionCollection("installedscripts");

    toolBar->setItemEnabled(WdgScriptsManagerPrivate::ExecBtn, i && i->action());
    toolBar->setItemEnabled(WdgScriptsManagerPrivate::UninstallBtn, i && i->action() && i->collection() == installedcollection);
    toolBar->setItemEnabled(WdgScriptsManagerPrivate::RemoveBtn, i && i->action() && i->collection() != installedcollection);
}

void WdgScriptsManager::slotLoadScript()
{
    if(d->m_scripguiclient->loadScriptFile())
        fillScriptsList();
}

void WdgScriptsManager::slotInstallScript()
{
    KFileDialog* filedialog = new KFileDialog(
        QString::null, // startdir
        "*.tar.gz *.bz2", // filter
        this, // parent widget
        "WdgScriptsManagerInstallFileDialog", // name
        true // modal
    );
    filedialog->setCaption( i18n("Install Script Package") );

    if(! filedialog->exec())
        return;

    if(! d->m_scripguiclient->installScriptPackage( filedialog->selectedURL().path() )) {
        kdWarning() << "Failed to install scriptpackage" << endl;
        return;
    }

    fillScriptsList();
}

void WdgScriptsManager::slotUninstallScript()
{
    ListItem* item = dynamic_cast<ListItem*>( scriptsList->currentItem() );
    if( !item || !item->action() )
        return;

    Kross::Api::ScriptActionCollection* installedcollection = d->m_scripguiclient->getActionCollection("installedscripts");
    if( !item->collection() || item->collection() != installedcollection)
        return;

    const QString packagepath = item->action()->getPackagePath();
    if( !packagepath)
        return;

    if( KMessageBox::warningContinueCancel(0,
        i18n("Uninstall the scriptpackage \"%1\" and delete the package's directory \"%2\"?")
            .arg(item->action()->text()).arg(packagepath),
        i18n("Uninstall")) != KMessageBox::Continue )
    {
        return;
    }

    if(! d->m_scripguiclient->uninstallScriptPackage(packagepath)) {
        kdWarning() << "Failed to uninstall scriptpackage" << endl;
        return;
    }

    fillScriptsList();
}

void WdgScriptsManager::slotExecuteScript()
{
    ListItem* item = dynamic_cast<ListItem*>( scriptsList->currentItem() );
    if(item && item->action())
        item->action()->activate();
}

void WdgScriptsManager::slotRemoveScript()
{
    ListItem* item = dynamic_cast<ListItem*>( scriptsList->currentItem() );
    if(item && item->action()) {
        item->collection()->detach( item->action() );
        fillScriptsList();
    }
}

}}

#include "wdgscriptsmanager.moc"
