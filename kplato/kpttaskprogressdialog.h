/* This file is part of the KDE project
   Copyright (C) 2005-2007 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation;; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPTTASKPROGRESSDIALOG_H
#define KPTTASKPROGRESSDIALOG_H

#include <kdialog.h>

class K3Command;

namespace KPlato
{

class TaskProgressPanel;
class Task;
class Part;
class StandardWorktime;
class ScheduleManager;

class TaskProgressDialog : public KDialog {
    Q_OBJECT
public:
    TaskProgressDialog(Task &task, ScheduleManager *sm, StandardWorktime *workTime, QWidget *parent=0);

    K3Command *buildCommand(Part *part);

protected slots:
    void slotChanged();
    void slotOk();

private:
    TaskProgressPanel *m_panel;

};

} //KPlato namespace

#endif // TASKPROGRESSDIALOG_H
