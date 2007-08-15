/* This file is part of the KDE project
  Copyright (C) 2007 Dag Andersen <danders@get2net.dk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation;
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "kptnodeitemmodel.h"

#include "kptglobal.h"
#include "kptcommand.h"
#include "kptfactory.h"
#include "kptproject.h"
#include "kptview.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QComboBox>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QHeaderView>
#include <QItemDelegate>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QMap>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVBoxLayout>
#include <QWidget>

#include <kicon.h>
#include <kaction.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktoggleaction.h>
#include <kactionmenu.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kaccelgen.h>
#include <kactioncollection.h>

#include <kdganttglobal.h>

namespace KPlato
{

void NodeModel::setProject( Project *project )
{
    kDebug()<<k_funcinfo<<m_project<<"->"<<project<<endl;
    m_project = project;
}

void NodeModel::setManager( ScheduleManager *sm )
{
    kDebug()<<k_funcinfo<<m_manager<<"->"<<sm<<endl;
    m_manager = sm;
}

QVariant NodeModel::name( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->name();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::leader( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->leader();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::allocation( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->requestNameList().join(",");
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::description( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            QString s = node->description();
            int i = s.indexOf( '\n' );
            s = s.left( i );
            if ( i > 0 ) {
                s += "...";
            }
            return s;
        }
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->description();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::type( const Node *node, int role ) const
{
    //kDebug()<<k_funcinfo<<node->name()<<", "<<role<<endl;
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->typeToString( true );
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case KDGantt::ItemTypeRole: {
            switch ( node->type() ) {
                case Node::Type_Summarytask: return KDGantt::TypeSummary;
                case Node::Type_Milestone: return KDGantt::TypeEvent;
                default: return KDGantt::TypeTask;
            }
        }
    }
    return QVariant();
}

QVariant NodeModel::constraint( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            return node->constraintToString( true );
        case Role::EnumList: 
            return Node::constraintList( true );
        case Qt::EditRole: 
        case Role::EnumListValue: 
            return (int)node->constraint();
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::constraintStartTime( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            int c = node->constraint();
            if ( ! ( c == Node::MustStartOn || c == Node::StartNotEarlier || c == Node::FixedInterval  ) ) {
                return " "; //HACK to show focus
            }
            return KGlobal::locale()->formatDateTime( node->constraintStartTime().dateTime() );
        }
        case Qt::ToolTipRole: {
            int c = node->constraint();
            if ( ! ( c == Node::MustStartOn || c == Node::StartNotEarlier || c == Node::FixedInterval  ) ) {
                return QVariant();
            }
            return KGlobal::locale()->formatDateTime( node->constraintStartTime().dateTime() );
        }
        case Qt::EditRole: {
            int c = node->constraint();
            if ( c == Node::MustStartOn || c == Node::StartNotEarlier || c == Node::FixedInterval  ) {
                return node->constraintStartTime().dateTime();
            }
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::constraintEndTime( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            int c = node->constraint();
            if ( ! ( c == Node::FinishNotLater || c == Node::MustFinishOn || c == Node::FixedInterval ) ) {
                return " "; //HACK to show focus
            }
            return KGlobal::locale()->formatDateTime( node->constraintEndTime().dateTime() );
        }
        case Qt::ToolTipRole: {
            int c = node->constraint();
            if ( ! ( c == Node::FinishNotLater || c == Node::MustFinishOn || c == Node::FixedInterval ) ) {
                return QVariant();
            }
            return KGlobal::locale()->formatDateTime( node->constraintEndTime().dateTime() );
        }
        case Qt::EditRole: {
            int c = node->constraint();
            if ( c == Node::FinishNotLater || c == Node::MustFinishOn || c == Node::FixedInterval ) {
                return node->constraintEndTime().dateTime();
            }
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::estimateType( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                return node->estimate()->typeToString( true );
            }
            return QString();
        case Role::EnumList: 
            return Estimate::typeToStringList( true );
        case Qt::EditRole:
        case Role::EnumListValue: 
            return (int)node->estimate()->type();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::estimate( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                Duration::Unit unit = node->estimate()->displayUnit();
                QList<double> scales; // TODO: week
                if ( node->estimate()->type() == Estimate::Type_Effort ) {
                    scales << m_project->standardWorktime()->day();
                    // rest is default
                }
                double v = Estimate::scale( node->estimate()->expected(), unit, scales );
                //kDebug()<<k_funcinfo<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales<<endl;
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            }
            break;
        case Qt::EditRole:
            return node->estimate()->expected().milliseconds();
        case Role::DurationScales: {
            QVariantList lst; // TODO: week
            if ( node->estimate()->type() == Estimate::Type_Effort ) {
                lst.append( m_project->standardWorktime()->day() );
            } else {
                lst.append( 24.0 );
            }
            lst << 60.0 << 60.0 << 1000.0;
            return lst;
        }
        case Role::DurationUnit:
            return static_cast<int>( node->estimate()->displayUnit() );
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::optimisticRatio( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                return node->estimate()->optimisticRatio();
            }
            return QString();
        case Role::Minimum:
            return -99;
        case Role::Maximum:
            return 0;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::pessimisticRatio( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                return node->estimate()->pessimisticRatio();
            }
            return QString();
        case Role::Minimum:
            return 0;
        case Role::Maximum:
            return INT_MAX;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::riskType( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                return node->estimate()->risktypeToString( true );
            }
            return QString();
        case Role::EnumList: 
            return Estimate::risktypeToStringList( true );
        case Qt::EditRole:
        case Role::EnumListValue: 
            return (int)node->estimate()->risktype();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::runningAccount( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                Account *a = node->runningAccount();
                return a == 0 ? i18n( "None" ) : a->name();
            }
            break;
        case Role::EnumListValue:
        case Qt::EditRole: {
            Account *a = node->runningAccount();
            return a == 0 ? 0 : ( m_project->accounts().costElements().indexOf( a->name() ) + 1 );
        }
        case Role::EnumList: {
            QStringList lst;
            lst << i18n("None");
            lst += m_project->accounts().costElements();
            return lst;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::startupAccount( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task  || node->type() == Node::Type_Milestone ) {
                Account *a = node->startupAccount();
                //kDebug()<<k_funcinfo<<node->name()<<": "<<a<<endl;
                return a == 0 ? i18n( "None" ) : a->name();
            }
            break;
        case Role::EnumListValue:
        case Qt::EditRole: {
            Account *a = node->startupAccount();
            return a == 0 ? 0 : ( m_project->accounts().costElements().indexOf( a->name() ) + 1 );
        }
        case Role::EnumList: {
            QStringList lst;
            lst << i18n("None");
            lst += m_project->accounts().costElements();
            return lst;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::startupCost( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) {
                return KGlobal::locale()->formatMoney( node->startupCost() );
            }
            break;
        case Qt::EditRole:
            return KGlobal::locale()->formatMoney( node->startupCost() );
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::shutdownAccount( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) {
                Account *a = node->shutdownAccount();
                return a == 0 ? i18n( "None" ) : a->name();
            }
            break;
        case Role::EnumListValue:
        case Qt::EditRole: {
            Account *a = node->shutdownAccount();
            return a == 0 ? 0 : ( m_project->accounts().costElements().indexOf( a->name() ) + 1 );
        }
        case Role::EnumList: {
            QStringList lst;
            lst << i18n("None");
            lst += m_project->accounts().costElements();
            return lst;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::shutdownCost( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) {
                return KGlobal::locale()->formatMoney( node->shutdownCost() );
            }
            break;
        case Qt::EditRole:
            return KGlobal::locale()->formatMoney( node->shutdownCost() );
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::startTime( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            //kDebug()<<k_funcinfo<<node->name()<<", "<<role<<endl;
            return node->startTime( id() ).dateTime();
            break;
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::endTime( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            //kDebug()<<k_funcinfo<<node->name()<<", "<<role<<endl;
            return node->endTime( id() ).dateTime();
            break;
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::duration( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                Duration::Unit unit = node->estimate()->displayUnit();
                double v = node->duration( id() ).toDouble( unit );
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            } else if ( node->type() == Node::Type_Project ) {
                Duration::Unit unit = Duration::Unit_d;
                double v = node->duration( id() ).toDouble( unit );
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            }
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::varianceDuration( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                Duration::Unit unit = node->estimate()->displayUnit();
                double v = node->variance( id(), unit );
                return KGlobal::locale()->formatNumber( v );
            }
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::varianceEstimate( const Estimate *est, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            Duration::Unit unit = Duration::Unit_h;
            double v = est->variance( unit );
            //kDebug()<<k_funcinfo<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales<<endl;
            return KGlobal::locale()->formatNumber( v );
        }
        case Qt::ToolTipRole: {
            Duration::Unit unit = Duration::Unit_d;
            double v = est->variance( unit );
            QString s = QString("%1%2").arg( KGlobal::locale()->formatNumber( v ) ).arg( Duration::unitToString( unit, true ) );
            
            unit = Duration::Unit_m;
            v = est->variance( unit );
            s = QString("%1, %2%3").arg( s ).arg( KGlobal::locale()->formatNumber( v ) ).arg( Duration::unitToString( unit, true ) );
            return s;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::optimisticDuration( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
                Duration d = node->duration( id() );
                d = ( d * ( 100 + node->estimate()->optimisticRatio() ) ) / 100;
                Duration::Unit unit = node->estimate()->displayUnit();
                double v = d.toDouble( unit );
                //kDebug()<<k_funcinfo<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales<<endl;
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::optimisticEstimate( const Estimate *est, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
                Duration::Unit unit = Duration::Unit_h;//est->displayUnit();
                QList<double> scales; // TODO: week
                if ( est->type() == Estimate::Type_Effort ) {
                    scales << m_project->standardWorktime()->day();
                    // rest is default
                }
                double v = Estimate::scale( est->optimistic(), unit, scales );
                //kDebug()<<k_funcinfo<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales<<endl;
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::pertExpected( const Estimate *est, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
            Duration::Unit unit = Duration::Unit_h;
            QList<double> scales; // TODO: week
            if ( est->type() == Estimate::Type_Effort ) {
                scales << m_project->standardWorktime()->day();
                // rest is default
            }
            double v = Estimate::scale( est->pertExpected(), unit, scales );
            return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::pessimisticDuration( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
                Duration d = node->duration( id() );
                d = ( d * ( 100 + node->estimate()->pessimisticRatio() ) ) / 100;
                Duration::Unit unit = node->estimate()->displayUnit();
                double v = d.toDouble( unit );
                //kDebug()<<k_funcinfo<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales<<endl;
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::pessimisticEstimate( const Estimate *est, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole: {
                Duration::Unit unit = Duration::Unit_h;//est->displayUnit();
                QList<double> scales; // TODO: week
                if ( est->type() == Estimate::Type_Effort ) {
                    scales << m_project->standardWorktime()->day();
                    // rest is default
                }
                double v = Estimate::scale( est->pessimistic(), unit, scales );
                //kDebug()<<k_funcinfo<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales<<endl;
                return KGlobal::locale()->formatNumber( v, m_prec ) +  Duration::unitToString( unit, true );
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::earlyStart( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->earlyStart( id() ).dateTime();
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatDate( t->earlyStart( id() ).date() );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::earlyFinish( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->earlyFinish( id() ).dateTime();
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatDate( t->earlyFinish( id() ).date() );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::lateStart( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->lateStart( id() ).dateTime();
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatDate( t->lateStart( id() ).date() );
        case Qt::EditRole:
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::lateFinish( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->lateFinish( id() ).dateTime();
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatDate( t->lateFinish( id() ).date() );
        case Qt::EditRole:
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::positiveFloat( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->positiveFloat( id() ).toString( Duration::Format_i18nHourFraction );
        case Qt::ToolTipRole:
            return t->positiveFloat( id() ).toString( Duration::Format_i18nDayTime );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::freeFloat( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->freeFloat( id() ).toString( Duration::Format_i18nHourFraction );
        case Qt::ToolTipRole:
            return t->freeFloat( id() ).toString( Duration::Format_i18nDayTime );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::negativeFloat( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->negativeFloat( id() ).toString( Duration::Format_i18nHourFraction );
        case Qt::ToolTipRole:
            return t->negativeFloat( id() ).toString( Duration::Format_i18nDayTime );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::startFloat( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->startFloat( id() ).toString( Duration::Format_i18nHourFraction );
        case Qt::ToolTipRole:
            return t->startFloat( id() ).toString( Duration::Format_i18nDayTime );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::finishFloat( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
            return t->finishFloat( id() ).toString( Duration::Format_i18nHourFraction );
        case Qt::ToolTipRole:
            return t->finishFloat( id() ).toString( Duration::Format_i18nDayTime );
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::completed( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return t->completion().percentFinished();
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::status( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole: {
            int st = t->state( id() );
            if ( st & Node::State_Finished ) {
                if ( st & Node::State_FinishedLate ) {
                    return i18n( "Finished late" );
                }
                if ( st & Node::State_FinishedEarly ) {
                    return i18n( "Finished early" );
                }
                return i18n( "Finished" );
            }
            if ( st & Node::State_Running ) {
                return i18n( "Running" );
            }
            if ( st & Node::State_Started ) {
                if ( st & Node::State_StartedLate ) {
                    return i18n( "Started late" );
                }
                if ( st & Node::State_StartedEarly ) {
                    return i18n( "Started early" );
                }
                return i18n( "Started" );
            }
            return i18n( "Not started" );
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::startedTime( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( t->completion().isStarted() ) {
                return KGlobal::locale()->formatDate( t->completion().startTime().date() );
            }
            break;
        case Qt::EditRole:
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::finishedTime( const Node *node, int role ) const
{
    if ( ! ( node->type() == Node::Type_Task || node->type() == Node::Type_Milestone ) ) {
        return QVariant();
    }
    const Task *t = static_cast<const Task*>( node );
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::ToolTipRole:
            if ( t->completion().isFinished() ) {
                return KGlobal::locale()->formatDate( t->completion().finishTime().date() );
            }
            break;
        case Qt::EditRole:
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::plannedEffortTo( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatNumber( node->plannedEffortTo( m_now, id() ).toDouble( Duration::Unit_h ), 1 );
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::actualEffortTo( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            //kDebug()<<k_funcinfo<<m_now<<node<<endl;
            return KGlobal::locale()->formatNumber( node->actualEffortTo( m_now, id() ).toDouble( Duration::Unit_h ), 1 );
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::remainingEffort( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole: {
            const Task *t = dynamic_cast<const Task*>( node );
            if ( t ) {
                return KGlobal::locale()->formatNumber( t->completion().remainingEffort().toDouble( Duration::Unit_h ), 1 );
            }
            break;
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::plannedCostTo( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatMoney( node->plannedCostTo( m_now ) );
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::actualCostTo( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return KGlobal::locale()->formatMoney( node->actualCostTo( m_now ) );
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::note( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            if ( node->type() == Node::Type_Task ) {
                Node *n = const_cast<Node*>( node );
                return static_cast<Task*>( n )->completion().note();
            }
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}


QVariant NodeModel::data( const Node *n, int property, int role ) const
{
    QVariant result;
    switch ( property ) {
        // Edited by user
        case 0: result = name( n, role ); break;
        case 1: result = type( n, role ); break;
        case 2: result = leader( n, role ); break;
        case 3: result = allocation( n, role ); break;
        case 4: result = estimateType( n, role ); break;
        case 5: result = estimate( n, role ); break;
        case 6: result = optimisticRatio( n, role ); break;
        case 7: result = pessimisticRatio( n, role ); break;
        case 8: result = riskType( n, role ); break;
        case 9: result = constraint( n, role ); break;
        case 10: result = constraintStartTime( n, role ); break;
        case 11: result = constraintEndTime( n, role ); break;
        case 12: result = runningAccount( n, role ); break;
        case 13: result = startupAccount( n, role ); break;
        case 14: result = startupCost( n, role ); break;
        case 15: result = shutdownAccount( n, role ); break;
        case 16: result = shutdownCost( n, role ); break;
        case 17: result = description( n, role ); break;
        
        // Based on edited values
        case 18: result = pertExpected( n->estimate(), role ); break;
        case 19: result = varianceEstimate( n->estimate(), role ); break;
        case 20: result = optimisticEstimate( n->estimate(), role ); break;
        case 21: result = pessimisticEstimate( n->estimate(), role ); break;

        // After scheduling
        case 22: result = startTime( n, role ); break;
        case 23: result = endTime( n, role ); break;
        case 24: result = earlyStart( n, role ); break;
        case 25: result = earlyFinish( n, role ); break;
        case 26: result = lateStart( n, role ); break;
        case 27: result = lateFinish( n, role ); break;
        case 28: result = positiveFloat( n, role ); break;
        case 29: result = freeFloat( n, role ); break;
        case 30: result = negativeFloat( n, role ); break;
        case 31: result = startFloat( n, role ); break;
        case 32: result = finishFloat( n, role ); break;

        // Based on scheduled values
        case 33: result = duration( n, role ); break;
        case 34: result = varianceDuration( n, role ); break;
        case 35: result = optimisticDuration( n, role ); break;
        case 36: result = pessimisticDuration( n, role ); break;
        
        // Completion
        case 37: result = status( n, role ); break;
        case 38: result = completed( n, role ); break;
        case 39: result = plannedEffortTo( n, role ); break;
        case 40: result = actualEffortTo( n, role ); break;
        case 41: result = remainingEffort( n, role ); break;
        case 42: result = plannedCostTo( n, role ); break;
        case 43: result = actualCostTo( n, role ); break;
        case 44: result = startedTime( n, role ); break;
        case 45: result = finishedTime( n, role ); break;
        case 46: result = note( n, role ); break;
        
        default:
            //kDebug()<<k_funcinfo<<"Invalid property number: "<<property<<endl;;
            return result;
    }
    return result;
}

int NodeModel::propertyCount()
{
    return 47;
}

bool NodeModel::setData( Node *node, int property, const QVariant & value, int role )
{
    return false;
}

QVariant NodeModel::headerData( int section, int role )
{
    if ( role == Qt::DisplayRole ) {
        switch ( section ) {
            case 0: return i18n( "Name" );
            case 1: return i18n( "Type" );
            case 2: return i18n( "Responsible" );
            case 3: return i18n( "Allocation" );
            case 4: return i18n( "Estimate Type" );
            case 5: return i18n( "Estimate" );
            case 6: return i18n( "Optimistic" ); // Ratio
            case 7: return i18n( "Pessimistic" ); // Ratio
            case 8: return i18n( "Risk" );
            case 9: return i18n( "Constraint" );
            case 10: return i18n( "Constraint Start" );
            case 11: return i18n( "Constraint End" );
            case 12: return i18n( "Running Account" );
            case 13: return i18n( "Startup Account" );
            case 14: return i18n( "Startup Cost" );
            case 15: return i18n( "Shutdown Account" );
            case 16: return i18n( "Shutdown Cost" );
            case 17: return i18n( "Description" );
            
            // Based on edited values
            case 18: return i18n( "Expected" );
            case 19: return i18n( "Variance (Est)" );
            case 20: return i18n( "Optimistic" );
            case 21: return i18n( "Pessimistic" );
            
            // After scheduling
            case 22: return i18n( "Start Time" );
            case 23: return i18n( "End Time" );
            case 24: return i18n( "Early Start" );
            case 25: return i18n( "Early Finish" );
            case 26: return i18n( "Late Start" );
            case 27: return i18n( "Late Finish" );
            case 28: return i18n( "Positive Float" );
            case 29: return i18n( "Free Float" );
            case 30: return i18n( "Negative Float" );
            case 31: return i18n( "Start Float" );
            case 32: return i18n( "Finish Float" );
            
            // Based on scheduled values
            case 33: return i18n( "Duration" );
            case 34: return i18n( "Variance (Dur)" );
            case 35: return i18n( "Optimistic" );
            case 36: return i18n( "Pessimistic" );

            // Completion
            case 37: return i18n( "Status" );
            case 38: return i18n( "% Completed" );
            case 39: return i18n( "Planned Effort" );
            case 40: return i18n( "Actual Effort" );
            case 41: return i18n( "Remaining Effort" );
            case 42: return i18n( "Planned Cost" );
            case 43: return i18n( "Actual Cost" );
            case 44: return i18n( "Started" );
            case 45: return i18n( "Finished" );
            case 46: return i18n( "Status Note" );
        
            default: return QVariant();
        }
    }
    if ( role == Qt::ToolTipRole ) {
        switch ( section ) {
            case 0: return ToolTip::NodeName;
            case 1: return ToolTip::NodeType;
            case 2: return ToolTip::NodeResponsible;
            case 3: return ToolTip::Allocation;
            case 4: return ToolTip::EstimateType;
            case 5: return ToolTip::Estimate;
            case 6: return ToolTip::OptimisticRatio;
            case 7: return ToolTip::PessimisticRatio;
            case 8: return ToolTip::RiskType;
            case 9: return ToolTip::NodeConstraint;
            case 10: return ToolTip::NodeConstraintStart;
            case 11: return ToolTip::NodeConstraintEnd;
            case 12: return ToolTip::NodeRunningAccount;
            case 13: return ToolTip::NodeStartupAccount;
            case 14: return ToolTip::NodeStartupCost;
            case 15: return ToolTip::NodeShutdownAccount;
            case 16: return ToolTip::NodeShutdownCost;
            case 17: return ToolTip::NodeDescription;

            // Based on edited values
            case 18: return ToolTip::EstimateExpected;
            case 19: return ToolTip::EstimateVariance;
            case 20: return ToolTip::EstimateOptimistic;
            case 21: return ToolTip::EstimatePessimistic;
            
            // After scheduling
            case 22: return ToolTip::NodeStartTime;
            case 23: return ToolTip::NodeEndTime;
            case 24: return ToolTip::NodeEarlyStart;
            case 25: return ToolTip::NodeEarlyFinish;
            case 26: return ToolTip::NodeLateStart;
            case 27: return ToolTip::NodeLateFinish;
            case 28: return ToolTip::NodePositiveFloat;
            case 29: return ToolTip::NodeFreeFloat;
            case 30: return ToolTip::NodeNegativeFloat;
            case 31: return ToolTip::NodeStartFloat;
            case 32: return ToolTip::NodeFinishFloat;

            // Based on scheduled values
            case 33: return ToolTip::NodeDuration;
            case 34: return ToolTip::NodeVarianceDuration;
            case 35: return ToolTip::NodeOptimisticDuration;
            case 36: return ToolTip::NodePessimisticDuration;

            // Completion
            case 37: return ToolTip::NodeStatus;
            case 38: return ToolTip::NodeCompletion;
            case 39: return ToolTip::NodePlannedEffortTo;
            case 40: return ToolTip::NodeActualEffortTo;
            case 41: return ToolTip::NodeRemainingEffort;
            case 42: return ToolTip::NodePlannedCostTo;
            case 43: return ToolTip::NodeActualCostTo;
            case 44: return ToolTip::CompletionStartedTime;
            case 45: return ToolTip::CompletionFinishedTime;
            case 46: return ToolTip::CompletionStatusNote;
    
            default: return QVariant();
        }
    }
    return QVariant();
}

//----------------------------
NodeItemModel::NodeItemModel( Part *part, QObject *parent )
    : ItemModelBase( part, parent ),
    m_node( 0 )
{
}

NodeItemModel::~NodeItemModel()
{
}
    
void NodeItemModel::slotNodeToBeInserted( Node *parent, int row )
{
    //kDebug()<<k_funcinfo<<node->name();
    Q_ASSERT( m_node == 0 );
    m_node = parent;
    beginInsertRows( index( parent ), row, row );
}

void NodeItemModel::slotNodeInserted( Node *node )
{
    //kDebug()<<k_funcinfo<<node->parentNode->name()<<"-->"<<node->name();
    Q_ASSERT( node->parentNode() == m_node );
    endInsertRows();
    m_node = 0;
}

void NodeItemModel::slotNodeToBeRemoved( Node *node )
{
    //kDebug()<<k_funcinfo<<node->name();
    Q_ASSERT( m_node == 0 );
    m_node = node;
    int row = index( node ).row();
    beginRemoveRows( index( node->parentNode() ), row, row );
}

void NodeItemModel::slotNodeRemoved( Node *node )
{
    //kDebug()<<k_funcinfo<<node->name();
    Q_ASSERT( node == m_node );
    endRemoveRows();
    m_node = 0;
}

void NodeItemModel::slotLayoutChanged()
{
    //kDebug()<<k_funcinfo<<node->name()<<endl;
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void NodeItemModel::setProject( Project *project )
{
    if ( m_project ) {
        disconnect( m_project, SIGNAL( nodeChanged( Node* ) ), this, SLOT( slotNodeChanged( Node* ) ) );
        disconnect( m_project, SIGNAL( nodeToBeAdded( Node*, int ) ), this, SLOT( slotNodeToBeInserted(  Node*, int ) ) );
        disconnect( m_project, SIGNAL( nodeToBeRemoved( Node* ) ), this, SLOT( slotNodeToBeRemoved( Node* ) ) );
        
        //disconnect( m_project, SIGNAL( nodeToBeMoved( Node* ) ), this, SLOT( slotLayoutToBeChanged() ) );
    
        disconnect( m_project, SIGNAL( nodeAdded( Node* ) ), this, SLOT( slotNodeInserted( Node* ) ) );
        disconnect( m_project, SIGNAL( nodeRemoved( Node* ) ), this, SLOT( slotNodeRemoved( Node* ) ) );
        //disconnect( m_project, SIGNAL( nodeMoved( Node* ) ), this, SLOT( slotLayoutChanged() ) );
    }
    m_project = project;
    kDebug()<<k_funcinfo<<m_project<<"->"<<project<<endl;
    m_nodemodel.setProject( project );
    if ( project ) {
        connect( m_project, SIGNAL( nodeChanged( Node* ) ), this, SLOT( slotNodeChanged( Node* ) ) );
        connect( m_project, SIGNAL( nodeToBeAdded( Node*, int ) ), this, SLOT( slotNodeToBeInserted(  Node*, int ) ) );
        connect( m_project, SIGNAL( nodeToBeRemoved( Node* ) ), this, SLOT( slotNodeToBeRemoved( Node* ) ) );
        //connect( m_project, SIGNAL( nodeToBeMoved( Node* ) ), this, SLOT( slotLayoutToBeChanged() ) );
    
        connect( m_project, SIGNAL( nodeAdded( Node* ) ), this, SLOT( slotNodeInserted( Node* ) ) );
        connect( m_project, SIGNAL( nodeRemoved( Node* ) ), this, SLOT( slotNodeRemoved( Node* ) ) );
        //connect( m_project, SIGNAL( nodeMoved( Node* ) ), this, SLOT( slotLayoutChanged() ) );
    }
    reset();
}

void NodeItemModel::setManager( ScheduleManager *sm )
{
    if ( m_nodemodel.manager() ) {
    }
    m_nodemodel.setManager( sm );
    if ( sm ) {
    }
    kDebug()<<k_funcinfo<<sm<<endl;
    reset();
}
    
Qt::ItemFlags NodeItemModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags( index );
    if ( !index.isValid() ) {
        if ( m_readWrite ) {
            flags |= Qt::ItemIsDropEnabled;
        }
        return flags;
    }
    if ( m_readWrite ) {
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        switch ( index.column() ) {
            case 1: break; // Node type
            case 2: // Responsible
                flags |= Qt::ItemIsEditable;
                break;
            case 3: // allocation
            case 4: // estimateType
            case 5: // estimate
            case 6: // optimisticRatio
            case 7: // pessimisticRatio
            {
                Node *n = node( index );
                if ( n && (n->type() == Node::Type_Task || n->type() == Node::Type_Milestone )) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case 8: // risktype
            {
                Node *n = node( index );
                if ( n && n->type() == Node::Type_Task ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case 9: // constraint type
                flags |= Qt::ItemIsEditable;
                break;
            case 10: { // constraint start
                Node *n = node( index );
                if ( n == 0 )
                    break;
                int c = n->constraint();
                if ( c == Node::MustStartOn || c == Node::StartNotEarlier || c == Node::FixedInterval ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case 11: { // constraint end
                Node *n = node( index );
                if ( n == 0 )
                    break;
                int c = n->constraint();
                if ( c == Node::MustFinishOn || c == Node::FinishNotLater || c ==  Node::FixedInterval ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case 12: // running account
            case 13: // startup account
            case 14: // startup cost
            case 15: // shutdown account
            case 16: { // shutdown cost
                Node *n = node( index );
                if ( n && (n->type() == Node::Type_Task || n->type() == Node::Type_Milestone) ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case 17: // description
                break;
            default: 
                flags &= ~Qt::ItemIsEditable;
        }
    }
    return flags;
}

    
QModelIndex NodeItemModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() ) {
        return QModelIndex();
    }
    //kDebug()<<k_funcinfo<<index.internalPointer()<<":"<<index.row()<<","<<index.column();
    Node *p = node( index )->parentNode();
    if ( p == 0 || p->type() == Node::Type_Project ) {
        return QModelIndex();
    }
    int row = p->parentNode()->findChildNode( p );
    return createIndex( row, 0, p );
}

bool NodeItemModel::hasChildren( const QModelIndex &parent ) const
{
    if ( m_nodemodel.manager() == 0 || m_project == 0 ) {
        return 0;
    }
    Node *p = node( parent );
    return p->numChildren() > 0;
}

QModelIndex NodeItemModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( m_nodemodel.manager() == 0 || m_project == 0 || column < 0 || column >= columnCount() || row < 0 ) {
        //kDebug()<<k_funcinfo<<"No index for"<<row<<","<<column;
        return QModelIndex();
    }
    Node *p = node( parent );
    if ( row >= p->numChildren() ) {
        //kDebug()<<k_funcinfo<<p->name()<<" row too high"<<row<<","<<column;
        return QModelIndex();
    }
    // now get the internal pointer for the index
    Node *n = p->childNode( row );

    return createIndex(row, column, n);
}

QModelIndex NodeItemModel::index( const Node *node ) const
{
    if ( m_project == 0 || node == 0 ) {
        return QModelIndex();
    }
    Node *par = node->parentNode();
    if ( par ) {
        //kDebug()<<k_funcinfo<<par<<"-->"<<node;
        return createIndex( par->indexOf( node ), 0, const_cast<Node*>(node) );
    }
    //kDebug()<<k_funcinfo<<node;
    return QModelIndex();
}

bool NodeItemModel::setName( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->name() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyNameCmd( m_part, *node, value.toString(), "Modify task name" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setLeader( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->leader() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyLeaderCmd( m_part, *node, value.toString(), "Modify task responsible" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setAllocation( Node *node, const QVariant &value, int role )
{
    Task *task = qobject_cast<Task*>( node );
    if ( task == 0 ) {
        return false;
    }
    switch ( role ) {
        case Qt::EditRole:
        {
            K3MacroCommand *cmd = 0;
            QStringList res = m_project->resourceNameList();
            QStringList req = node->requestNameList();
            QStringList alloc;
            foreach ( QString s, value.toString().split( QRegExp(" *, *"), QString::SkipEmptyParts ) ) {
                alloc << s.trimmed();
            }
            // first add all new resources (to "default" group)
            ResourceGroup *pargr = m_project->groupByName( i18n( "Resources" ) );
            foreach ( QString s, alloc ) {
                Resource *r = m_project->resourceByName( s.trimmed() );
                if ( r != 0 ) {
                    continue;
                }
                if ( cmd == 0 ) cmd = new K3MacroCommand( i18n( "Add Resource" ) );
                if ( pargr == 0 ) {
                    pargr = new ResourceGroup();
                    pargr->setName( i18n( "Resources" ) );
                    cmd->addCommand( new AddResourceGroupCmd( m_part, m_project, pargr ) );
                    //kDebug()<<k_funcinfo<<"add group:"<<pargr->name();
                }
                r = new Resource();
                r->setName( s.trimmed() );
                cmd->addCommand( new AddResourceCmd( m_part, pargr, r ) );
                //kDebug()<<k_funcinfo<<"add resource:"<<r->name();
                m_part->addCommand( cmd );
                cmd = 0;
            }
            
            QString c = i18n( "Modify Resource Allocations" );
            // Handle deleted requests
            foreach ( QString s, req ) {
                // if a request is not in alloc, it must have been be removed by the user
                if ( alloc.indexOf( s ) == -1 ) {
                    // remove removed resource request
                    ResourceRequest *r = node->resourceRequest( s );
                    if ( r ) {
                        if ( cmd == 0 ) cmd = new K3MacroCommand( c );
                        //kDebug()<<k_funcinfo<<"delete request:"<<r->resource()->name()<<" group:"<<r->parent()->group()->name();
                        cmd->addCommand( new RemoveResourceRequestCmd( m_part, r->parent(), r ) );
                    }
                }
            }
            // Handle new requests
            foreach ( QString s, alloc ) {
                // if an allocation is not in req, it must be added
                if ( req.indexOf( s ) == -1 ) {
                    ResourceGroup *pargr = 0;
                    Resource *r = m_project->resourceByName( s );
                    if ( r == 0 ) {
                        // Handle request to non exixting resource
                        pargr = m_project->groupByName( i18n( "Resources" ) );
                        if ( pargr == 0 ) {
                            pargr = new ResourceGroup();
                            pargr->setName( i18n( "Resources" ) );
                            cmd->addCommand( new AddResourceGroupCmd( m_part, m_project, pargr ) );
                            //kDebug()<<k_funcinfo<<"add group:"<<pargr->name();
                        }
                        r = new Resource();
                        r->setName( s );
                        cmd->addCommand( new AddResourceCmd( m_part, pargr, r ) );
                        //kDebug()<<k_funcinfo<<"add resource:"<<r->name();
                        m_part->addCommand( cmd );
                        cmd = 0;
                    } else {
                        pargr = r->parentGroup();
                        //kDebug()<<k_funcinfo<<"add '"<<r->name()<<"' to group:"<<pargr;
                    }
                    // add request
                    ResourceGroupRequest *g = node->resourceGroupRequest( pargr );
                    if ( g == 0 ) {
                        // create a group request
                        if ( cmd == 0 ) cmd = new K3MacroCommand( c );
                        g = new ResourceGroupRequest( pargr );
                        cmd->addCommand( new AddResourceGroupRequestCmd( m_part, *task, g ) );
                        //kDebug()<<k_funcinfo<<"add group request:"<<g;
                    }
                    if ( cmd == 0 ) cmd = new K3MacroCommand( c );
                    cmd->addCommand( new AddResourceRequestCmd( m_part, g, new ResourceRequest( r, 100 ) ) );
                    //kDebug()<<k_funcinfo<<"add request:"<<r->name()<<" group:"<<g;
                }
            }
            if ( cmd ) {
                m_part->addCommand( cmd );
            }
            return true;
        }
    }
    return false;
}

bool NodeItemModel::setDescription( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->description() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyDescriptionCmd( m_part, *node, value.toString(), "Modify task description" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setType( Node *, const QVariant &, int )
{
    return false;
}

bool NodeItemModel::setConstraint( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            Node::ConstraintType v = Node::ConstraintType( value.toInt() );
            //kDebug()<<k_funcinfo<<v;
            if ( v == node->constraint() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyConstraintCmd( m_part, *node, v, "Modify constraint type" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setConstraintStartTime( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toDateTime() == node->constraintStartTime().dateTime() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyConstraintStartTimeCmd( m_part, *node, value.toDateTime(), "Modify constraint start time" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setConstraintEndTime( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toDateTime() == node->constraintEndTime().dateTime() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyConstraintEndTimeCmd( m_part, *node, value.toDateTime(), "Modify constraint end time" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setEstimateType( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            Estimate::Type v = Estimate::Type( value.toInt() );
            if ( v == node->estimate()->type() ) {
                return false;
            }
            m_part->addCommand( new ModifyEstimateTypeCmd( m_part, *node, node->estimate()->type(), v, "Modify estimate type" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setEstimate( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            Duration d( value.toList()[0].toLongLong() );
            Duration::Unit unit = static_cast<Duration::Unit>( value.toList()[1].toInt() );
            //kDebug()<<k_funcinfo<<value.toList()[0].toLongLong()<<","<<unit<<" ->"<<d.milliseconds();
            if ( d == node->estimate()->expected() ) {
                return false;
            }
            K3MacroCommand *cmd = new K3MacroCommand( i18n( "Modify estimate" ) );
            cmd->addCommand( new ModifyEstimateCmd( m_part, *node, node->estimate()->expected(), d ) );
            cmd->addCommand( new ModifyEstimateUnitCmd( m_part, *node, node->estimate()->displayUnit(), unit ) );
            m_part->addCommand( cmd );
            return true;
    }
    return false;
}

bool NodeItemModel::setOptimisticRatio( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toInt() == node->estimate()->optimisticRatio() ) {
                return false;
            }
            m_part->addCommand( new EstimateModifyOptimisticRatioCmd( m_part, *node, node->estimate()->optimisticRatio(), value.toInt(), "Modify estimate" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setPessimisticRatio( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toInt() == node->estimate()->pessimisticRatio() ) {
                return false;
            }
            m_part->addCommand( new EstimateModifyPessimisticRatioCmd( m_part, *node, node->estimate()->pessimisticRatio(), value.toInt(), "Modify estimate" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setRiskType( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toInt() == node->estimate()->risktype() ) {
                return false;
            }
            Estimate::Risktype v = Estimate::Risktype( value.toInt() );
            m_part->addCommand( new EstimateModifyRiskCmd( m_part, *node, node->estimate()->risktype(), v, "Modify Risk Type" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setRunningAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<k_funcinfo<<node->name();
            QStringList lst = m_nodemodel.runningAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->runningAccount();
            if ( old != a ) {
                m_part->addCommand( new NodeModifyRunningAccountCmd( m_part, *node, old, a, i18n( "Modify Running Account" ) ) );
            }
            return true;
    }
    return false;
}

bool NodeItemModel::setStartupAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<k_funcinfo<<node->name();
            QStringList lst = m_nodemodel.startupAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->startupAccount();
            //kDebug()<<k_funcinfo<<(value.toInt())<<";"<<(lst.at( value.toInt()))<<":"<<a;
            if ( old != a ) {
                m_part->addCommand( new NodeModifyStartupAccountCmd( m_part, *node, old, a, i18n( "Modify Startup Account" ) ) );
            }
            return true;
    }
    return false;
}

bool NodeItemModel::setStartupCost( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            double v = KGlobal::locale()->readMoney( value.toString() );
            if ( v == node->startupCost() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyStartupCostCmd( m_part, *node, v, i18n( "Modify Startup Cost" ) ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setShutdownAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<k_funcinfo<<node->name();
            QStringList lst = m_nodemodel.shutdownAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->shutdownAccount();
            if ( old != a ) {
                m_part->addCommand( new NodeModifyShutdownAccountCmd( m_part, *node, old, a, i18n( "Modify Shutdown Account" ) ) );
            }
            return true;
    }
    return false;
}

bool NodeItemModel::setShutdownCost( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            double v = KGlobal::locale()->readMoney( value.toString() );
            if ( v == node->shutdownCost() ) {
                return false;
            }
            m_part->addCommand( new NodeModifyShutdownCostCmd( m_part, *node, v, i18n( "Modify Shutdown Cost" ) ) );
            return true;
    }
    return false;
}

QVariant NodeItemModel::data( const QModelIndex &index, int role ) const
{
    QVariant result;
    Node *n = node( index );
    if ( n != 0 ) {
        // Special for kdgantt
        if ( index.column() == 18 && role ==  KDGantt::StartTimeRole ) {
            QDateTime t = n->startTime( m_nodemodel.id() ).dateTime();
            //kDebug()<<k_funcinfo<<n->name()<<": "<<index.column()<<", "<<role<<t<<endl;
            return t;
        }
        if ( index.column() == 19 && role == KDGantt::EndTimeRole ) {
            QDateTime t = n->endTime( m_nodemodel.id() ).dateTime();
            //kDebug()<<k_funcinfo<<n->name()<<": "<<index.column()<<", "<<role<<t<<endl;
            return t;
        }

        result = m_nodemodel.data( n, index.column(), role );
    }
    if ( result.isValid() ) {
        if ( role == Qt::DisplayRole && result.type() == QVariant::String && result.toString().isEmpty()) {
            // HACK to show focus in empty cells
            result = " ";
        }
        return result;
    }
    return result;
}

bool NodeItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( ( flags(index) &Qt::ItemIsEditable ) == 0 || role != Qt::EditRole ) {
        return false;
    }
    Node *n = node( index );
    switch (index.column()) {
        case 0: return setName( n, value, role );
        case 1: return setType( n, value, role );
        case 2: return setLeader( n, value, role );
        case 3: return setAllocation( n, value, role );
        case 4: return setEstimateType( n, value, role );
        case 5: return setEstimate( n, value, role );
        case 6: return setOptimisticRatio( n, value, role );
        case 7: return setPessimisticRatio( n, value, role );
        case 8: return setRiskType( n, value, role );
        case 9: return setConstraint( n, value, role );
        case 10: return setConstraintStartTime( n, value, role );
        case 11: return setConstraintEndTime( n, value, role );
        case 12: return setRunningAccount( n, value, role );
        case 13: return setStartupAccount( n, value, role );
        case 14: return setStartupCost( n, value, role );
        case 15: return setShutdownAccount( n, value, role );
        case 16: return setShutdownCost( n, value, role );
        case 17: return setDescription( n, value, role );
        default:
            qWarning("data: invalid display value column %d", index.column());
            return false;
    }
    return false;
}

QVariant NodeItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole ) {
            return m_nodemodel.headerData( section, role );
        } else if ( role == Qt::TextAlignmentRole ) {
            switch (section) {
                case 1: return Qt::AlignCenter;
                default: return QVariant();
            }
        }
    }
    if ( role == Qt::ToolTipRole ) {
        return NodeModel::headerData( section, role );
    }
    return ItemModelBase::headerData(section, orientation, role);
}

QItemDelegate *NodeItemModel::createDelegate( int column, QWidget *parent ) const
{
    switch ( column ) {
        //case 3: return new ??Delegate( parent );
        case 4: return new EnumDelegate( parent );
        case 5: return new DurationSpinBoxDelegate( parent );
        case 6: return new SpinBoxDelegate( parent );
        case 7: return new SpinBoxDelegate( parent );
        case 8: return new EnumDelegate( parent );
        case 9: return new EnumDelegate( parent );
        case 12: return new EnumDelegate( parent );
        case 13: return new EnumDelegate( parent );
        case 14: return new MoneyDelegate( parent );
        case 15: return new EnumDelegate( parent );
        case 16: return new MoneyDelegate( parent );
        default: return 0;
    }
    return 0;
}

int NodeItemModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return m_nodemodel.propertyCount();
}

int NodeItemModel::rowCount( const QModelIndex &parent ) const
{
    Node *p = node( parent );
    return p->numChildren();
}

Qt::DropActions NodeItemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}


QStringList NodeItemModel::mimeTypes() const
{
    return QStringList() << "application/x-vnd.kde.kplato.nodeitemmodel.internal";
}

QMimeData *NodeItemModel::mimeData( const QModelIndexList & indexes ) const
{
    QMimeData *m = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QList<int> rows;
    foreach (QModelIndex index, indexes) {
        if ( index.isValid() && !rows.contains( index.row() ) ) {
            //kDebug()<<k_funcinfo<<index.row();
            Node *n = node( index );
            if ( n ) {
                rows << index.row();
                stream << n->id();
            }
        }
    }
    m->setData("application/x-vnd.kde.kplato.nodeitemmodel.internal", encodedData);
    return m;
}

bool NodeItemModel::dropAllowed( const QModelIndex &index, int dropIndicatorPosition, const QMimeData *data )
{
    //kDebug()<<k_funcinfo;
    Node *dn = node( index );
    if ( dn == 0 ) {
        kError()<<k_funcinfo<<"no node to drop on!"<<endl;
        return false; // hmmm
    }
    switch ( dropIndicatorPosition ) {
        case TreeViewBase::AboveItem:
        case TreeViewBase::BelowItem:
            // dn == sibling
            return dropAllowed( dn->parentNode(), data );
        case TreeViewBase::OnItem:
            // dn == new parent
            return dropAllowed( dn, data );
        default:
            break;
    }
    return false;
}

bool NodeItemModel::dropAllowed( Node *on, const QMimeData *data )
{
    if ( !data->hasFormat("application/x-vnd.kde.kplato.nodeitemmodel.internal") ) {
        return false;
    }
    if ( on == m_project ) {
        return true;
    }
    QByteArray encodedData = data->data( "application/x-vnd.kde.kplato.nodeitemmodel.internal" );
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<Node*> lst = nodeList( stream );
    foreach ( Node *n, lst ) {
        if ( on == n || on->isChildOf( n ) ) {
            return false;
        }
    }
    lst = removeChildNodes( lst );
    foreach ( Node *n, lst ) {
        if ( ! m_project->canMoveTask( n, on ) ) {
            return false;
        }
    }
    return true;
}

QList<Node*> NodeItemModel::nodeList( QDataStream &stream )
{
    QList<Node*> lst;
    while (!stream.atEnd()) {
        QString id;
        stream >> id;
        Node *node = m_project->findNode( id );
        if ( node ) {
            lst << node;
        }
    }
    return lst;
}

QList<Node*> NodeItemModel::removeChildNodes( QList<Node*> nodes )
{
    QList<Node*> lst;
    foreach ( Node *node, nodes ) {
        bool ins = true;
        foreach ( Node *n, lst ) {
            if ( node->isChildOf( n ) ) {
                //kDebug()<<k_funcinfo<<node->name()<<" is child of"<<n->name();
                ins = false;
                break;
            }
        }
        if ( ins ) {
            //kDebug()<<k_funcinfo<<" insert"<<node->name();
            lst << node;
        }
    }
    QList<Node*> nl = lst;
    QList<Node*> nlst = lst;
    foreach ( Node *node, nl ) {
        foreach ( Node *n, nlst ) {
            if ( n->isChildOf( node ) ) {
                //kDebug()<<k_funcinfo<<n->name()<<" is child of"<<node->name();
                int i = nodes.indexOf( n );
                lst.removeAt( i );
            }
        }
    }
    return lst;
}

bool NodeItemModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int /*column*/, const QModelIndex &parent )
{
    //kDebug()<<k_funcinfo<<action;
    if (action == Qt::IgnoreAction) {
        return true;
    }
    if ( !data->hasFormat( "application/x-vnd.kde.kplato.nodeitemmodel.internal" ) ) {
        return false;
    }
    if ( action == Qt::MoveAction ) {
        //kDebug()<<k_funcinfo<<"MoveAction";
        
        QByteArray encodedData = data->data( "application/x-vnd.kde.kplato.nodeitemmodel.internal" );
        QDataStream stream(&encodedData, QIODevice::ReadOnly);
        Node *par = 0;
        if ( parent.isValid() ) {
            par = node( parent );
        } else {
            par = m_project;
        }
        QList<Node*> lst = nodeList( stream );
        QList<Node*> nodes = removeChildNodes( lst ); // children goes with their parent
        foreach ( Node *n, nodes ) {
            if ( ! m_project->canMoveTask( n, par ) ) {
                //kDebug()<<k_funcinfo<<"Can't move task:"<<n->name();
                return false;
            }
        }
        int offset = 0;
        K3MacroCommand *cmd = 0;
        foreach ( Node *n, nodes ) {
            if ( cmd == 0 ) cmd = new K3MacroCommand( i18n( "Move tasks" ) );
            cmd->addCommand( new NodeMoveCmd( m_part, m_project, n, par, row + offset ) );
            offset++;
        }
        if ( cmd ) {
            m_part->addCommand( cmd );
        }
        //kDebug()<<k_funcinfo<<row<<","<<column<<" parent="<<parent.row()<<","<<parent.column()<<":"<<par->name();
        return true;
    }
    return false;
}

Node *NodeItemModel::node( const QModelIndex &index ) const
{
    Node *n = m_project;
    if ( index.isValid() ) {
        if ( index.internalPointer() == 0 ) {
            
        }
        //kDebug()<<k_funcinfo<<index.internalPointer()<<":"<<index.row()<<","<<index.column();
        n = static_cast<Node*>( index.internalPointer() );
        Q_ASSERT( n );
    }
    return n;
}

void NodeItemModel::slotNodeChanged( Node *node )
{
    if ( node == 0 || node->type() == Node::Type_Project ) {
        return;
    }
    int row = node->parentNode()->findChildNode( node );
    emit dataChanged( createIndex( row, 0, node ), createIndex( row, columnCount(), node ) );
}

QModelIndex NodeItemModel::insertTask( Node *node, Node *after )
{
    m_part->addCommand( new TaskAddCmd( m_part, m_project, node, after, i18n( "Add Task") ) );
    int row = -1;
    if ( node->parentNode() ) {
        row = node->parentNode()->indexOf( node );
    }
    if ( row != -1 ) {
        //kDebug()<<k_funcinfo<<"Inserted:"<<account->name();
        return createIndex( row, 0, node );
    }
    //kDebug()<<k_funcinfo<<"Can't find"<<node->name();
    return QModelIndex();
}

QModelIndex NodeItemModel::insertSubtask( Node *node, Node *parent )
{
    m_part->addCommand( new SubtaskAddCmd( m_part, m_project, node, parent, i18n( "Add Subtask" ) ) );
    int row = -1;
    if ( node->parentNode() ) {
        row = node->parentNode()->indexOf( node );
    }
    if ( row != -1 ) {
        //kDebug()<<k_funcinfo<<"Inserted:"<<account->name();
        return createIndex( row, 0, node );
    }
    //kDebug()<<k_funcinfo<<"Can't find"<<node->name();
    return QModelIndex();
}

} //namespace KPlato

#include "kptnodeitemmodel.moc"
