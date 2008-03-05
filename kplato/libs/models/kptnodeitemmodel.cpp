/* This file is part of the KDE project
  Copyright (C) 2007 Dag Andersen <danders@get2net.dk>

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
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#include "kptnodeitemmodel.h"

#include "kptglobal.h"
#include "kptcommand.h"
#include "kptduration.h"
#include "kptproject.h"
#include "kptnode.h"

#include <QAbstractItemModel>
#include <QMimeData>
#include <QItemDelegate>
#include <QModelIndex>
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
#include <QtGui>
#include <math.h>

namespace KPlato
{


//--------------------------------------
NodeModel::NodeModel()
    : QObject(),
    m_project( 0 ),
    m_manager( 0 ),
    m_now( QDate::currentDate() ),
    m_prec( 1 )
{
}

const QMetaEnum NodeModel::columnMap() const
{
    return metaObject()->enumerator( metaObject()->indexOfEnumerator("Properties") );
}

void NodeModel::setProject( Project *project )
{
    kDebug()<<m_project<<"->"<<project;
    m_project = project;
}

void NodeModel::setManager( ScheduleManager *sm )
{
    kDebug()<<m_manager<<"->"<<sm;
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
    //kDebug()<<node->name()<<", "<<role;
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
                Duration::Unit unit = node->estimate()->unit();
                return KGlobal::locale()->formatNumber( node->estimate()->expectedEstimate(), m_prec ) +  Duration::unitToString( unit, true );
            }
            break;
        case Qt::EditRole:
            return node->estimate()->expectedEstimate();
        case Role::DurationUnit:
            return static_cast<int>( node->estimate()->unit() );
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
                //kDebug()<<node->name()<<": "<<a;
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
            //kDebug()<<node->name()<<", "<<role;
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
            //kDebug()<<node->name()<<", "<<role;
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
                Duration::Unit unit = node->estimate()->unit();
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
                Duration::Unit unit = node->estimate()->unit();
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
            //kDebug()<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales;
            return KGlobal::locale()->formatNumber( v );
        }
        case Qt::ToolTipRole: {
            //TODO check unit
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
                Duration::Unit unit = node->estimate()->unit();
                double v = d.toDouble( unit );
                //kDebug()<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales;
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
            Duration::Unit unit = est->unit();
            return KGlobal::locale()->formatNumber( est->optimisticEstimate(), m_prec ) +  Duration::unitToString( unit, true );
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
            //TODO: fix unit
            Duration::Unit unit = est->unit();
            return KGlobal::locale()->formatNumber( est->pertExpected().toDouble( unit ), m_prec ) +  Duration::unitToString( unit, true );
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
            Duration::Unit unit = node->estimate()->unit();
            double v = d.toDouble( unit );
            //kDebug()<<node->name()<<": "<<v<<" "<<unit<<" : "<<scales;
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
            //TODO fix unit
            Duration::Unit unit = est->unit();
            return KGlobal::locale()->formatNumber( est->pessimisticEstimate(), m_prec ) +  Duration::unitToString( unit, true );
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

QVariant NodeModel::assignedResources( const Node *node, int role ) const
{
    if ( node->type() != Node::Type_Task ) {
        return QVariant();
    }
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->assignedNameList( id() ).join(",");
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
            if ( st & Node::State_ReadyToStart ) {
                return i18n( "Can start" );
            }
            if ( st & Node::State_NotReadyToStart ) {
                return i18n( "Cannot start" );
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
            //kDebug()<<m_now<<node;
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

QVariant NodeModel::resourceIsMissing( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->resourceError( id() );
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::resourceIsOverbooked( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->resourceOverbooked( id() );
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::resourceIsNotAvailable( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->resourceNotAvailable( id() );
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::schedulingConstraintsError( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->schedulingError( id() );
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::nodeIsNotScheduled( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->notScheduled( id() );
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::effortNotMet( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->effortMetError( id() );
            break;
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant NodeModel::wbsCode( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return node->wbs();
            break;
        case Qt::ToolTipRole:
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
        case NodeName: result = name( n, role ); break;
        case NodeType: result = type( n, role ); break;
        case NodeResponsible: result = leader( n, role ); break;
        case NodeAllocation: result = allocation( n, role ); break;
        case NodeEstimateType: result = estimateType( n, role ); break;
        case NodeEstimate: result = estimate( n, role ); break;
        case NodeOptimisticRatio: result = optimisticRatio( n, role ); break;
        case NodePessimisticRatio: result = pessimisticRatio( n, role ); break;
        case NodeRisk: result = riskType( n, role ); break;
        case NodeConstraint: result = constraint( n, role ); break;
        case NodeConstraintStart: result = constraintStartTime( n, role ); break;
        case NodeConstraintEnd: result = constraintEndTime( n, role ); break;
        case NodeRunningAccount: result = runningAccount( n, role ); break;
        case NodeStartupAccount: result = startupAccount( n, role ); break;
        case NodeStartupCost: result = startupCost( n, role ); break;
        case NodeShutdownAccount: result = shutdownAccount( n, role ); break;
        case NodeShutdownCost: result = shutdownCost( n, role ); break;
        case NodeDescription: result = description( n, role ); break;
        
        // Based on edited values
        case NodeExpected: result = pertExpected( n->estimate(), role ); break;
        case NodeVarianceEstimate: result = varianceEstimate( n->estimate(), role ); break;
        case NodeOptimistic: result = optimisticEstimate( n->estimate(), role ); break;
        case NodePessimistic: result = pessimisticEstimate( n->estimate(), role ); break;

        // After scheduling
        case NodeStartTime: result = startTime( n, role ); break;
        case NodeEndTime: result = endTime( n, role ); break;
        case NodeEarlyStart: result = earlyStart( n, role ); break;
        case NodeEarlyFinish: result = earlyFinish( n, role ); break;
        case NodeLateStart: result = lateStart( n, role ); break;
        case NodeLateFinish: result = lateFinish( n, role ); break;
        case NodePositiveFloat: result = positiveFloat( n, role ); break;
        case NodeFreeFloat: result = freeFloat( n, role ); break;
        case NodeNegativeFloat: result = negativeFloat( n, role ); break;
        case NodeStartFloat: result = startFloat( n, role ); break;
        case NodeFinishFloat: result = finishFloat( n, role ); break;
        case NodeAssigments: result = assignedResources( n, role ); break;

        // Based on scheduled values
        case NodeDuration: result = duration( n, role ); break;
        case NodeVarianceDuration: result = varianceDuration( n, role ); break;
        case NodeOptimisticDuration: result = optimisticDuration( n, role ); break;
        case NodePessimisticDuration: result = pessimisticDuration( n, role ); break;
        
        // Completion
        case NodeStatus: result = status( n, role ); break;
        case NodeCompleted: result = completed( n, role ); break;
        case NodePlannedEffort: result = plannedEffortTo( n, role ); break;
        case NodeActualEffort: result = actualEffortTo( n, role ); break;
        case NodeRemainingEffort: result = remainingEffort( n, role ); break;
        case NodePlannedCost: result = plannedCostTo( n, role ); break;
        case NodeActualCost: result = actualCostTo( n, role ); break;
        case NodeStarted: result = startedTime( n, role ); break;
        case NodeFinished: result = finishedTime( n, role ); break;
        case NodeStatusNote: result = note( n, role ); break;
        
        // Scheduling errors
        case NodeNotScheduled: result = nodeIsNotScheduled( n, role ); break;
        case NodeAssigmentMissing: result = resourceIsMissing( n, role ); break;
        case NodeResourceOverbooked: result = resourceIsOverbooked( n, role ); break;
        case NodeResourceUnavailable: result = resourceIsNotAvailable( n, role ); break;
        case NodeConstraintsError: result = schedulingConstraintsError( n, role ); break;
        case NodeEffortNotMet: result = effortNotMet( n, role ); break;
        
        case NodeWBSCode: result = wbsCode( n, role ); break;
        
        default:
            //kDebug()<<"Invalid property number: "<<property;;
            return result;
    }
    return result;
}

int NodeModel::propertyCount() const
{
    return columnMap().keyCount();
}

bool NodeModel::setData( Node *node, int property, const QVariant & value, int role )
{
    return false;
}

QVariant NodeModel::headerData( int section, int role )
{
    if ( role == Qt::DisplayRole ) {
        switch ( section ) {
            case NodeName: return i18n( "Name" );
            case NodeType: return i18n( "Type" );
            case NodeResponsible: return i18n( "Responsible" );
            case NodeAllocation: return i18n( "Allocation" );
            case NodeEstimateType: return i18n( "Estimate Type" );
            case NodeEstimate: return i18n( "Estimate" );
            case NodeOptimisticRatio: return i18n( "Optimistic" ); // Ratio
            case NodePessimisticRatio: return i18n( "Pessimistic" ); // Ratio
            case NodeRisk: return i18n( "Risk" );
            case NodeConstraint: return i18n( "Constraint" );
            case NodeConstraintStart: return i18n( "Constraint Start" );
            case NodeConstraintEnd: return i18n( "Constraint End" );
            case NodeRunningAccount: return i18n( "Running Account" );
            case NodeStartupAccount: return i18n( "Startup Account" );
            case NodeStartupCost: return i18n( "Startup Cost" );
            case NodeShutdownAccount: return i18n( "Shutdown Account" );
            case NodeShutdownCost: return i18n( "Shutdown Cost" );
            case NodeDescription: return i18n( "Description" );
            
            // Based on edited values
            case NodeExpected: return i18n( "Expected" );
            case NodeVarianceEstimate: return i18n( "Variance (Est)" );
            case NodeOptimistic: return i18n( "Optimistic" );
            case NodePessimistic: return i18n( "Pessimistic" );
            
            // After scheduling
            case NodeStartTime: return i18n( "Start Time" );
            case NodeEndTime: return i18n( "End Time" );
            case NodeEarlyStart: return i18n( "Early Start" );
            case NodeEarlyFinish: return i18n( "Early Finish" );
            case NodeLateStart: return i18n( "Late Start" );
            case NodeLateFinish: return i18n( "Late Finish" );
            case NodePositiveFloat: return i18n( "Positive Float" );
            case NodeFreeFloat: return i18n( "Free Float" );
            case NodeNegativeFloat: return i18n( "Negative Float" );
            case NodeStartFloat: return i18n( "Start Float" );
            case NodeFinishFloat: return i18n( "Finish Float" );
            case NodeAssigments: return i18n( "Assigments" );
            
            // Based on scheduled values
            case NodeDuration: return i18n( "Duration" );
            case NodeVarianceDuration: return i18n( "Variance (Dur)" );
            case NodeOptimisticDuration: return i18n( "Optimistic (Dur)" );
            case NodePessimisticDuration: return i18n( "Pessimistic (Dur)" );

            // Completion
            case NodeStatus: return i18n( "Status" );
            // xgettext: no-c-format
            case NodeCompleted: return i18n( "% Completed" );
            case NodePlannedEffort: return i18n( "Planned Effort" );
            case NodeActualEffort: return i18n( "Actual Effort" );
            case NodeRemainingEffort: return i18n( "Remaining Effort" );
            case NodePlannedCost: return i18n( "Planned Cost" );
            case NodeActualCost: return i18n( "Actual Cost" );
            case NodeStarted: return i18n( "Started" );
            case NodeFinished: return i18n( "Finished" );
            case NodeStatusNote: return i18n( "Status Note" );
            
            // Scheduling errors
            case NodeNotScheduled: return i18n( "Not Scheduled" );
            case NodeAssigmentMissing: return i18n( "Assigment Missing" );
            case NodeResourceOverbooked: return i18n( "Resource Overbooked" );
            case NodeResourceUnavailable: return i18n( "Resource Unavailable" );
            case NodeConstraintsError: return i18n( "Constraints Error" );
            case NodeEffortNotMet: return i18n( "Effort Not Met" );
            
            case NodeWBSCode: return i18n( "WBS Code" );
            
            default: return QVariant();
        }
    }
    if ( role == Qt::ToolTipRole ) {
        switch ( section ) {
            case NodeName: return ToolTip::nodeName();
            case NodeType: return ToolTip::nodeType();
            case NodeResponsible: return ToolTip::nodeResponsible();
            case NodeAllocation: return ToolTip::allocation();
            case NodeEstimateType: return ToolTip::estimateType();
            case NodeEstimate: return ToolTip::estimate();
            case NodeOptimisticRatio: return ToolTip::optimisticRatio();
            case NodePessimisticRatio: return ToolTip::pessimisticRatio();
            case NodeRisk: return ToolTip::riskType();
            case NodeConstraint: return ToolTip::nodeConstraint();
            case NodeConstraintStart: return ToolTip::nodeConstraintStart();
            case NodeConstraintEnd: return ToolTip::nodeConstraintEnd();
            case NodeRunningAccount: return ToolTip::nodeRunningAccount();
            case NodeStartupAccount: return ToolTip::nodeStartupAccount();
            case NodeStartupCost: return ToolTip::nodeStartupCost();
            case NodeShutdownAccount: return ToolTip::nodeShutdownAccount();
            case NodeShutdownCost: return ToolTip::nodeShutdownCost();
            case NodeDescription: return ToolTip::nodeDescription();

            // Based on edited values
            case NodeExpected: return ToolTip::estimateExpected();
            case NodeVarianceEstimate: return ToolTip::estimateVariance();
            case NodeOptimistic: return ToolTip::estimateOptimistic();
            case NodePessimistic: return ToolTip::estimatePessimistic();
            
            // After scheduling
            case NodeStartTime: return ToolTip::nodeStartTime();
            case NodeEndTime: return ToolTip::nodeEndTime();
            case NodeEarlyStart: return ToolTip::nodeEarlyStart();
            case NodeEarlyFinish: return ToolTip::nodeEarlyFinish();
            case NodeLateStart: return ToolTip::nodeLateStart();
            case NodeLateFinish: return ToolTip::nodeLateFinish();
            case NodePositiveFloat: return ToolTip::nodePositiveFloat();
            case NodeFreeFloat: return ToolTip::nodeFreeFloat();
            case NodeNegativeFloat: return ToolTip::nodeNegativeFloat();
            case NodeStartFloat: return ToolTip::nodeStartFloat();
            case NodeFinishFloat: return ToolTip::nodeFinishFloat();
            case NodeAssigments: return ToolTip::nodeAssignment();

            // Based on scheduled values
            case NodeDuration: return ToolTip::nodeDuration();
            case NodeVarianceDuration: return ToolTip::nodeVarianceDuration();
            case NodeOptimisticDuration: return ToolTip::nodeOptimisticDuration();
            case NodePessimisticDuration: return ToolTip::nodePessimisticDuration();

            // Completion
            case NodeStatus: return ToolTip::nodeStatus();
            case NodeCompleted: return ToolTip::nodeCompletion();
            case NodePlannedEffort: return ToolTip::nodePlannedEffortTo();
            case NodeActualEffort: return ToolTip::nodeActualEffortTo();
            case NodeRemainingEffort: return ToolTip::nodeRemainingEffort();
            case NodePlannedCost: return ToolTip::nodePlannedCostTo();
            case NodeActualCost: return ToolTip::nodeActualCostTo();
            case NodeStarted: return ToolTip::completionStartedTime();
            case NodeFinished: return ToolTip::completionFinishedTime();
            case NodeStatusNote: return ToolTip::completionStatusNote();
    
            // Scheduling errors
            case NodeNotScheduled: return ToolTip::nodeNotScheduled();
            case NodeAssigmentMissing: return ToolTip::nodeAssigmentMissing();
            case NodeResourceOverbooked: return ToolTip::nodeResourceOverbooked();
            case NodeResourceUnavailable: return ToolTip::nodeResourceUnavailable();
            case NodeConstraintsError: return ToolTip::nodeConstraintsError();
            case NodeEffortNotMet: return ToolTip::nodeEffortNotMet();
            
            case NodeWBSCode: return ToolTip::nodeWBS();
            
            default: return QVariant();
        }
    }
    return QVariant();
}

//----------------------------
NodeItemModel::NodeItemModel( QObject *parent )
    : ItemModelBase( parent ),
    m_node( 0 )
{
}

NodeItemModel::~NodeItemModel()
{
}
    
void NodeItemModel::slotNodeToBeInserted( Node *parent, int row )
{
    //kDebug()<<parent->name()<<"; "<<row;
    Q_ASSERT( m_node == 0 );
    m_node = parent;
    beginInsertRows( index( parent ), row, row );
}

void NodeItemModel::slotNodeInserted( Node *node )
{
    //kDebug()<<node->parentNode()->name()<<"-->"<<node->name();
    Q_ASSERT( node->parentNode() == m_node );
    endInsertRows();
    m_node = 0;
}

void NodeItemModel::slotNodeToBeRemoved( Node *node )
{
    //kDebug()<<node->name();
    Q_ASSERT( m_node == 0 );
    m_node = node;
    int row = index( node ).row();
    beginRemoveRows( index( node->parentNode() ), row, row );
}

void NodeItemModel::slotNodeRemoved( Node *node )
{
    //kDebug()<<node->name();
    Q_ASSERT( node == m_node );
    endRemoveRows();
    m_node = 0;
}

void NodeItemModel::slotLayoutChanged()
{
    //kDebug()<<node->name();
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
    kDebug()<<m_project<<"->"<<project;
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
    kDebug()<<sm;
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
    Node *n = node( index );
    if ( m_readWrite && n != 0 ) {
        flags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        switch ( index.column() ) {
            case NodeModel::NodeName: // name
                flags |= Qt::ItemIsEditable;
                break;
            case NodeModel::NodeType: break; // Node type
            case NodeModel::NodeResponsible: // Responsible
                flags |= Qt::ItemIsEditable;
                break;
            case NodeModel::NodeAllocation: // allocation
                if ( n->type() == Node::Type_Task ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            case NodeModel::NodeEstimateType: // estimateType
            case NodeModel::NodeEstimate: // estimate
            case NodeModel::NodeOptimisticRatio: // optimisticRatio
            case NodeModel::NodePessimisticRatio: // pessimisticRatio
            {
                if ( n->type() == Node::Type_Task || n->type() == Node::Type_Milestone ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeRisk: // risktype
            {
                if ( n->type() == Node::Type_Task ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeConstraint: // constraint type
                if ( n->type() == Node::Type_Task || n->type() == Node::Type_Milestone ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            case NodeModel::NodeConstraintStart: { // constraint start
                if ( ! ( n->type() == Node::Type_Task || n->type() == Node::Type_Milestone ) ) {
                    break;
                }
                int c = n->constraint();
                if ( c == Node::MustStartOn || c == Node::StartNotEarlier || c == Node::FixedInterval ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeConstraintEnd: { // constraint end
                if ( ! ( n->type() == Node::Type_Task || n->type() == Node::Type_Milestone ) ) {
                    break;
                }
                int c = n->constraint();
                if ( c == Node::MustFinishOn || c == Node::FinishNotLater || c ==  Node::FixedInterval ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeRunningAccount: // running account
                if ( n->type() == Node::Type_Task ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            case NodeModel::NodeStartupAccount: // startup account
            case NodeModel::NodeStartupCost: // startup cost
            case NodeModel::NodeShutdownAccount: // shutdown account
            case NodeModel::NodeShutdownCost: { // shutdown cost
                if ( n->type() == Node::Type_Task || n->type() == Node::Type_Milestone ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeDescription: // description
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
    //kDebug()<<index.internalPointer()<<":"<<index.row()<<","<<index.column();
    Node *p = node( index )->parentNode();
    if ( p == 0 || p->type() == Node::Type_Project ) {
        return QModelIndex();
    }
    int row = p->parentNode()->indexOf( p );
    if ( row == -1 ) {
        return QModelIndex();
    }
    return createIndex( row, 0, p );
}

QModelIndex NodeItemModel::index( int row, int column, const QModelIndex &parent ) const
{
    //kDebug()<<parent<<row<<column;
    if ( m_project == 0 || column < 0 || column >= columnCount() || row < 0 ) {
        //kDebug()<<m_project<<parent<<"No index for"<<row<<","<<column;
        return QModelIndex();
    }
    Node *p = node( parent );
    if ( row >= p->numChildren() ) {
        kError()<<p->name()<<" row too high"<<row<<","<<column;
        return QModelIndex();
    }
    // now get the internal pointer for the index
    Node *n = p->childNode( row );
    QModelIndex idx = createIndex(row, column, n);
    //kDebug()<<idx;
    return idx;
}

QModelIndex NodeItemModel::index( const Node *node ) const
{
    if ( m_project == 0 || node == 0 ) {
        return QModelIndex();
    }
    Node *par = node->parentNode();
    if ( par ) {
        //kDebug()<<par<<"-->"<<node;
        return createIndex( par->indexOf( node ), 0, const_cast<Node*>(node) );
    }
    //kDebug()<<node;
    return QModelIndex();
}

bool NodeItemModel::setName( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->name() ) {
                return false;
            }
            emit executeCommand( new NodeModifyNameCmd( *node, value.toString(), "Modify task name" ) );
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
            emit executeCommand( new NodeModifyLeaderCmd( *node, value.toString(), "Modify task responsible" ) );
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
            MacroCommand *cmd = 0;
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
                if ( cmd == 0 ) cmd = new MacroCommand( i18n( "Add Resource" ) );
                if ( pargr == 0 ) {
                    pargr = new ResourceGroup();
                    pargr->setName( i18n( "Resources" ) );
                    cmd->addCommand( new AddResourceGroupCmd( m_project, pargr ) );
                    //kDebug()<<"add group:"<<pargr->name();
                }
                r = new Resource();
                r->setName( s.trimmed() );
                cmd->addCommand( new AddResourceCmd( pargr, r ) );
                //kDebug()<<"add resource:"<<r->name();
                emit executeCommand( cmd );
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
                        if ( cmd == 0 ) cmd = new MacroCommand( c );
                        //kDebug()<<"delete request:"<<r->resource()->name()<<" group:"<<r->parent()->group()->name();
                        cmd->addCommand( new RemoveResourceRequestCmd( r->parent(), r ) );
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
                            cmd->addCommand( new AddResourceGroupCmd( m_project, pargr ) );
                            //kDebug()<<"add group:"<<pargr->name();
                        }
                        r = new Resource();
                        r->setName( s );
                        cmd->addCommand( new AddResourceCmd( pargr, r ) );
                        //kDebug()<<"add resource:"<<r->name();
                        emit executeCommand( cmd );
                        cmd = 0;
                    } else {
                        pargr = r->parentGroup();
                        //kDebug()<<"add '"<<r->name()<<"' to group:"<<pargr;
                    }
                    // add request
                    ResourceGroupRequest *g = node->resourceGroupRequest( pargr );
                    if ( g == 0 ) {
                        // create a group request
                        if ( cmd == 0 ) cmd = new MacroCommand( c );
                        g = new ResourceGroupRequest( pargr );
                        cmd->addCommand( new AddResourceGroupRequestCmd( *task, g ) );
                        //kDebug()<<"add group request:"<<g;
                    }
                    if ( cmd == 0 ) cmd = new MacroCommand( c );
                    cmd->addCommand( new AddResourceRequestCmd( g, new ResourceRequest( r, 100 ) ) );
                    //kDebug()<<"add request:"<<r->name()<<" group:"<<g;
                }
            }
            if ( cmd ) {
                emit executeCommand( cmd );
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
            emit executeCommand( new NodeModifyDescriptionCmd( *node, value.toString(), "Modify task description" ) );
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
            //kDebug()<<v;
            if ( v == node->constraint() ) {
                return false;
            }
            emit executeCommand( new NodeModifyConstraintCmd( *node, v, "Modify constraint type" ) );
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
            emit executeCommand( new NodeModifyConstraintStartTimeCmd( *node, value.toDateTime(), "Modify constraint start time" ) );
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
            emit executeCommand( new NodeModifyConstraintEndTimeCmd( *node, value.toDateTime(), "Modify constraint end time" ) );
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
            emit executeCommand( new ModifyEstimateTypeCmd( *node, node->estimate()->type(), v, "Modify estimate type" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setEstimate( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            double d( value.toList()[0].toDouble() );
            Duration::Unit unit = static_cast<Duration::Unit>( value.toList()[1].toInt() );
            //kDebug()<<value.toList()[0].toDouble()<<","<<unit<<" ->"<<d.milliseconds();
            MacroCommand *cmd = 0;
            if ( d != node->estimate()->expectedEstimate() ) {
                if ( cmd == 0 ) cmd = new MacroCommand( i18n( "Modify estimate" ) );
                cmd->addCommand( new ModifyEstimateCmd( *node, node->estimate()->expectedEstimate(), d ) );
            }
            if ( unit != node->estimate()->unit() ) {
                if ( cmd == 0 ) cmd = new MacroCommand( i18n( "Modify estimate" ) );
                cmd->addCommand( new ModifyEstimateUnitCmd( *node, node->estimate()->unit(), unit ) );
            }
            if ( cmd == 0 ) {
                return false;
            }
            emit executeCommand( cmd );
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
            emit executeCommand( new EstimateModifyOptimisticRatioCmd( *node, node->estimate()->optimisticRatio(), value.toInt(), "Modify estimate" ) );
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
            emit executeCommand( new EstimateModifyPessimisticRatioCmd( *node, node->estimate()->pessimisticRatio(), value.toInt(), "Modify estimate" ) );
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
            emit executeCommand( new EstimateModifyRiskCmd( *node, node->estimate()->risktype(), v, "Modify Risk Type" ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setRunningAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<node->name();
            QStringList lst = m_nodemodel.runningAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->runningAccount();
            if ( old != a ) {
                emit executeCommand( new NodeModifyRunningAccountCmd( *node, old, a, i18n( "Modify Running Account" ) ) );
            }
            return true;
    }
    return false;
}

bool NodeItemModel::setStartupAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<node->name();
            QStringList lst = m_nodemodel.startupAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->startupAccount();
            //kDebug()<<(value.toInt())<<";"<<(lst.at( value.toInt()))<<":"<<a;
            if ( old != a ) {
                emit executeCommand( new NodeModifyStartupAccountCmd( *node, old, a, i18n( "Modify Startup Account" ) ) );
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
            emit executeCommand( new NodeModifyStartupCostCmd( *node, v, i18n( "Modify Startup Cost" ) ) );
            return true;
    }
    return false;
}

bool NodeItemModel::setShutdownAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<node->name();
            QStringList lst = m_nodemodel.shutdownAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->shutdownAccount();
            if ( old != a ) {
                emit executeCommand( new NodeModifyShutdownAccountCmd( *node, old, a, i18n( "Modify Shutdown Account" ) ) );
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
            emit executeCommand( new NodeModifyShutdownCostCmd( *node, v, i18n( "Modify Shutdown Cost" ) ) );
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
        if ( index.column() == 22 && role ==  KDGantt::StartTimeRole ) {
            QDateTime t = n->startTime( m_nodemodel.id() ).dateTime();
            //kDebug()<<n->name()<<": "<<index.column()<<", "<<role<<t;
            return t;
        }
        if ( index.column() == 23 && role == KDGantt::EndTimeRole ) {
            QDateTime t = n->endTime( m_nodemodel.id() ).dateTime();
            //kDebug()<<n->name()<<": "<<index.column()<<", "<<role<<t;
            return t;
        }
        result = m_nodemodel.data( n, index.column(), role );
        //kDebug()<<n->name()<<": "<<index.column()<<", "<<role<<result;
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
        case NodeModel::NodeName: return setName( n, value, role );
        case NodeModel::NodeType: return setType( n, value, role );
        case NodeModel::NodeResponsible: return setLeader( n, value, role );
        case NodeModel::NodeAllocation: return setAllocation( n, value, role );
        case NodeModel::NodeEstimateType: return setEstimateType( n, value, role );
        case NodeModel::NodeEstimate: return setEstimate( n, value, role );
        case NodeModel::NodeOptimisticRatio: return setOptimisticRatio( n, value, role );
        case NodeModel::NodePessimisticRatio: return setPessimisticRatio( n, value, role );
        case NodeModel::NodeRisk: return setRiskType( n, value, role );
        case NodeModel::NodeConstraint: return setConstraint( n, value, role );
        case NodeModel::NodeConstraintStart: return setConstraintStartTime( n, value, role );
        case NodeModel::NodeConstraintEnd: return setConstraintEndTime( n, value, role );
        case NodeModel::NodeRunningAccount: return setRunningAccount( n, value, role );
        case NodeModel::NodeStartupAccount: return setStartupAccount( n, value, role );
        case NodeModel::NodeStartupCost: return setStartupCost( n, value, role );
        case NodeModel::NodeShutdownAccount: return setShutdownAccount( n, value, role );
        case NodeModel::NodeShutdownCost: return setShutdownCost( n, value, role );
        case NodeModel::NodeDescription: return setDescription( n, value, role );
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
                case NodeModel::NodeType: return Qt::AlignCenter;
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
        //case NodeModel::NodeAllocation: return new ??Delegate( parent );
        case NodeModel::NodeEstimateType: return new EnumDelegate( parent );
        case NodeModel::NodeEstimate: return new DurationSpinBoxDelegate( parent );
        case NodeModel::NodeOptimisticRatio: return new SpinBoxDelegate( parent );
        case NodeModel::NodePessimisticRatio: return new SpinBoxDelegate( parent );
        case NodeModel::NodeRisk: return new EnumDelegate( parent );
        case NodeModel::NodeConstraint: return new EnumDelegate( parent );
        case NodeModel::NodeRunningAccount: return new EnumDelegate( parent );
        case NodeModel::NodeStartupAccount: return new EnumDelegate( parent );
        case NodeModel::NodeStartupCost: return new MoneyDelegate( parent );
        case NodeModel::NodeShutdownAccount: return new EnumDelegate( parent );
        case NodeModel::NodeShutdownCost: return new MoneyDelegate( parent );
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
    return p == 0 ? 0 : p->numChildren();
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
            //kDebug()<<index.row();
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
    //kDebug();
    Node *dn = node( index );
    if ( dn == 0 ) {
        kError()<<"no node to drop on!";
        return false; // hmmm
    }
    switch ( dropIndicatorPosition ) {
        case ItemModelBase::AboveItem:
        case ItemModelBase::BelowItem:
            // dn == sibling
            return dropAllowed( dn->parentNode(), data );
        case ItemModelBase::OnItem:
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
                //kDebug()<<node->name()<<" is child of"<<n->name();
                ins = false;
                break;
            }
        }
        if ( ins ) {
            //kDebug()<<" insert"<<node->name();
            lst << node;
        }
    }
    QList<Node*> nl = lst;
    QList<Node*> nlst = lst;
    foreach ( Node *node, nl ) {
        foreach ( Node *n, nlst ) {
            if ( n->isChildOf( node ) ) {
                //kDebug()<<n->name()<<" is child of"<<node->name();
                int i = nodes.indexOf( n );
                lst.removeAt( i );
            }
        }
    }
    return lst;
}

bool NodeItemModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int /*column*/, const QModelIndex &parent )
{
    //kDebug()<<action;
    if (action == Qt::IgnoreAction) {
        return true;
    }
    if ( !data->hasFormat( "application/x-vnd.kde.kplato.nodeitemmodel.internal" ) ) {
        return false;
    }
    if ( action == Qt::MoveAction ) {
        //kDebug()<<"MoveAction";
        
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
                //kDebug()<<"Can't move task:"<<n->name();
                return false;
            }
        }
        int offset = 0;
        MacroCommand *cmd = 0;
        foreach ( Node *n, nodes ) {
            if ( cmd == 0 ) cmd = new MacroCommand( i18n( "Move tasks" ) );
            cmd->addCommand( new NodeMoveCmd( m_project, n, par, row + offset ) );
            offset++;
        }
        if ( cmd ) {
            emit executeCommand( cmd );
        }
        //kDebug()<<row<<","<<column<<" parent="<<parent.row()<<","<<parent.column()<<":"<<par->name();
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
        //kDebug()<<index.internalPointer()<<":"<<index.row()<<","<<index.column();
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
    emit executeCommand( new TaskAddCmd( m_project, node, after, i18n( "Add Task") ) );
    int row = -1;
    if ( node->parentNode() ) {
        row = node->parentNode()->indexOf( node );
    }
    if ( row != -1 ) {
        //kDebug()<<"Inserted: "<<node->name()<<"; "<<row;
        return createIndex( row, 0, node );
    }
    //kDebug()<<"Can't find "<<node->name();
    return QModelIndex();
}

QModelIndex NodeItemModel::insertSubtask( Node *node, Node *parent )
{
    emit executeCommand( new SubtaskAddCmd( m_project, node, parent, i18n( "Add Subtask" ) ) );
    reset();
    int row = -1;
    if ( node->parentNode() ) {
        row = node->parentNode()->indexOf( node );
    }
    if ( row != -1 ) {
        //kDebug()<<node->parentNode()<<" inserted: "<<node->name()<<"; "<<row;
        return createIndex( row, 0, node );
    }
    //kDebug()<<"Can't find "<<node->name();
    return QModelIndex();
}

//----------------------------
MilestoneItemModel::MilestoneItemModel( QObject *parent )
    : ItemModelBase( parent )
{
}

MilestoneItemModel::~MilestoneItemModel()
{
}
    
void MilestoneItemModel::slotNodeToBeInserted( Node *parent, int row )
{
}

void MilestoneItemModel::slotNodeInserted( Node *node )
{
    //kDebug()<<node->name();
    if ( node && node->type() == Node::Type_Milestone && m_mslist.indexOf( node ) == -1 ) {
        beginInsertRows( QModelIndex(), m_mslist.count(), m_mslist.count() );
        m_mslist.append( node );
        endInsertRows();
        //kDebug()<<node->name()<<": "<<m_mslist.count();
    }
}

void MilestoneItemModel::slotNodeToBeRemoved( Node *node )
{
    //kDebug()<<node->name();
    int row = m_mslist.indexOf( node );
    if ( row != -1 ) {
        beginRemoveRows( QModelIndex(), row, row );
        m_mslist.removeAt( row );
        endRemoveRows();
    }
}

void MilestoneItemModel::slotNodeRemoved( Node *node )
{
}

void MilestoneItemModel::slotLayoutChanged()
{
    //kDebug()<<node->name();
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void MilestoneItemModel::setProject( Project *project )
{
    if ( m_project ) {
        disconnect( m_project, SIGNAL( nodeChanged( Node* ) ), this, SLOT( slotNodeChanged( Node* ) ) );
        disconnect( m_project, SIGNAL( nodeToBeAdded( Node*, int ) ), this, SLOT( slotNodeToBeInserted(  Node*, int ) ) );
        disconnect( m_project, SIGNAL( nodeToBeRemoved( Node* ) ), this, SLOT( slotNodeToBeRemoved( Node* ) ) );
        
        disconnect( m_project, SIGNAL( nodeAdded( Node* ) ), this, SLOT( slotNodeInserted( Node* ) ) );
        disconnect( m_project, SIGNAL( nodeRemoved( Node* ) ), this, SLOT( slotNodeRemoved( Node* ) ) );
    }
    m_project = project;
    kDebug()<<m_project<<"->"<<project;
    m_nodemodel.setProject( project );
    if ( project ) {
        connect( m_project, SIGNAL( nodeChanged( Node* ) ), this, SLOT( slotNodeChanged( Node* ) ) );
        connect( m_project, SIGNAL( nodeToBeAdded( Node*, int ) ), this, SLOT( slotNodeToBeInserted(  Node*, int ) ) );
        connect( m_project, SIGNAL( nodeToBeRemoved( Node* ) ), this, SLOT( slotNodeToBeRemoved( Node* ) ) );
    
        connect( m_project, SIGNAL( nodeAdded( Node* ) ), this, SLOT( slotNodeInserted( Node* ) ) );
        connect( m_project, SIGNAL( nodeRemoved( Node* ) ), this, SLOT( slotNodeRemoved( Node* ) ) );
    }
    resetModel();
}

void MilestoneItemModel::setManager( ScheduleManager *sm )
{
    if ( m_nodemodel.manager() ) {
    }
    m_nodemodel.setManager( sm );
    if ( sm ) {
    }
    kDebug()<<sm;
    reset();
}
    
void MilestoneItemModel::resetModel()
{
    m_mslist.clear();
    if ( m_project != 0 ) {
        foreach ( Node *n, m_project->allNodes() ) {
            if ( n->type() == Node::Type_Milestone ) {
                m_mslist.append( n );
            }
        }
    }
    reset();
}

Qt::ItemFlags MilestoneItemModel::flags( const QModelIndex &index ) const
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
            case NodeModel::NodeName: // name
                flags |= Qt::ItemIsEditable;
                break;
            case NodeModel::NodeType: break; // Node type
            case NodeModel::NodeResponsible: // Responsible
                flags |= Qt::ItemIsEditable;
                break;
            case NodeModel::NodeConstraint: // constraint type
                flags |= Qt::ItemIsEditable;
                break;
            case NodeModel::NodeConstraintStart: { // constraint start
                Node *n = node( index );
                if ( n == 0 )
                    break;
                int c = n->constraint();
                if ( c == Node::MustStartOn || c == Node::StartNotEarlier || c == Node::FixedInterval ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeConstraintEnd: { // constraint end
                Node *n = node( index );
                if ( n == 0 )
                    break;
                int c = n->constraint();
                if ( c == Node::MustFinishOn || c == Node::FinishNotLater || c ==  Node::FixedInterval ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeStartupAccount: // startup account
            case NodeModel::NodeStartupCost: // startup cost
            case NodeModel::NodeShutdownAccount: // shutdown account
            case NodeModel::NodeShutdownCost: { // shutdown cost
                Node *n = node( index );
                if ( n && (n->type() == Node::Type_Task || n->type() == Node::Type_Milestone) ) {
                    flags |= Qt::ItemIsEditable;
                }
                break;
            }
            case NodeModel::NodeDescription: // description
                break;
            default: 
                flags &= ~Qt::ItemIsEditable;
        }
    }
    return flags;
}

QModelIndex MilestoneItemModel::parent( const QModelIndex &index ) const
{
    return QModelIndex();
}

QModelIndex MilestoneItemModel::index( int row, int column, const QModelIndex &parent ) const
{
    //kDebug()<<parent<<row<<", "<<m_mslist.count();
    if ( m_project == 0 || row < 0 || column < 0 ) {
        //kDebug()<<"No project"<<m_project<<" or illegal row, column"<<row<<column;
        return QModelIndex();
    }
    if ( parent.isValid() || row >= m_mslist.count() ) {
        //kDebug()<<"No index for"<<parent<<row<<","<<column;
        return QModelIndex();
    }
    return createIndex( row, column, m_mslist[row] );
}

QModelIndex MilestoneItemModel::index( const Node *node ) const
{
    if ( m_project == 0 || node == 0 ) {
        return QModelIndex();
    }
    return createIndex( m_mslist.indexOf( const_cast<Node*>( node ) ), 0, const_cast<Node*>(node) );
}

bool MilestoneItemModel::setName( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->name() ) {
                return false;
            }
            emit executeCommand( new NodeModifyNameCmd( *node, value.toString(), "Modify task name" ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setLeader( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->leader() ) {
                return false;
            }
            emit executeCommand( new NodeModifyLeaderCmd( *node, value.toString(), "Modify task responsible" ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setDescription( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toString() == node->description() ) {
                return false;
            }
            emit executeCommand( new NodeModifyDescriptionCmd( *node, value.toString(), "Modify task description" ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setType( Node *, const QVariant &, int )
{
    return false;
}

bool MilestoneItemModel::setConstraint( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            Node::ConstraintType v = Node::ConstraintType( value.toInt() );
            //kDebug()<<v;
            if ( v == node->constraint() ) {
                return false;
            }
            emit executeCommand( new NodeModifyConstraintCmd( *node, v, "Modify constraint type" ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setConstraintStartTime( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toDateTime() == node->constraintStartTime().dateTime() ) {
                return false;
            }
            emit executeCommand( new NodeModifyConstraintStartTimeCmd( *node, value.toDateTime(), "Modify constraint start time" ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setConstraintEndTime( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            if ( value.toDateTime() == node->constraintEndTime().dateTime() ) {
                return false;
            }
            emit executeCommand( new NodeModifyConstraintEndTimeCmd( *node, value.toDateTime(), "Modify constraint end time" ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setRunningAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<node->name();
            QStringList lst = m_nodemodel.runningAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->runningAccount();
            if ( old != a ) {
                emit executeCommand( new NodeModifyRunningAccountCmd( *node, old, a, i18n( "Modify Running Account" ) ) );
            }
            return true;
    }
    return false;
}

bool MilestoneItemModel::setStartupAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<node->name();
            QStringList lst = m_nodemodel.startupAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->startupAccount();
            //kDebug()<<(value.toInt())<<";"<<(lst.at( value.toInt()))<<":"<<a;
            if ( old != a ) {
                emit executeCommand( new NodeModifyStartupAccountCmd( *node, old, a, i18n( "Modify Startup Account" ) ) );
            }
            return true;
    }
    return false;
}

bool MilestoneItemModel::setStartupCost( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            double v = KGlobal::locale()->readMoney( value.toString() );
            if ( v == node->startupCost() ) {
                return false;
            }
            emit executeCommand( new NodeModifyStartupCostCmd( *node, v, i18n( "Modify Startup Cost" ) ) );
            return true;
    }
    return false;
}

bool MilestoneItemModel::setShutdownAccount( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            //kDebug()<<node->name();
            QStringList lst = m_nodemodel.shutdownAccount( node, Role::EnumList ).toStringList();
            if ( value.toInt() >= lst.count() ) {
                return false;
            }
            Account *a = m_project->accounts().findAccount( lst.at( value.toInt() ) );
            Account *old = node->shutdownAccount();
            if ( old != a ) {
                emit executeCommand( new NodeModifyShutdownAccountCmd( *node, old, a, i18n( "Modify Shutdown Account" ) ) );
            }
            return true;
    }
    return false;
}

bool MilestoneItemModel::setShutdownCost( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole:
            double v = KGlobal::locale()->readMoney( value.toString() );
            if ( v == node->shutdownCost() ) {
                return false;
            }
            emit executeCommand( new NodeModifyShutdownCostCmd( *node, v, i18n( "Modify Shutdown Cost" ) ) );
            return true;
    }
    return false;
}

QVariant MilestoneItemModel::data( const QModelIndex &index, int role ) const
{
    QVariant result;
    Node *n = node( index );
    if ( n != 0 ) {
        // Special for kdgantt
        if ( index.column() == 22 && role ==  KDGantt::StartTimeRole ) {
            QDateTime t = n->startTime( m_nodemodel.id() ).dateTime();
            //kDebug()<<n->name()<<": "<<index.column()<<", "<<role<<t;
            return t;
        }
        if ( index.column() == 23 && role == KDGantt::EndTimeRole ) {
            QDateTime t = n->endTime( m_nodemodel.id() ).dateTime();
            //kDebug()<<n->name()<<": "<<index.column()<<", "<<role<<t;
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

bool MilestoneItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( ( flags(index) &Qt::ItemIsEditable ) == 0 || role != Qt::EditRole ) {
        return false;
    }
    Node *n = node( index );
    switch (index.column()) {
        case NodeModel::NodeName: return setName( n, value, role );
        case NodeModel::NodeType: return setType( n, value, role );
        case NodeModel::NodeResponsible: return setLeader( n, value, role );
        case NodeModel::NodeAllocation: return false;
        case NodeModel::NodeConstraint: return setConstraint( n, value, role );
        case NodeModel::NodeConstraintStart: return setConstraintStartTime( n, value, role );
        case NodeModel::NodeConstraintEnd: return setConstraintEndTime( n, value, role );
        case NodeModel::NodeRunningAccount: return setRunningAccount( n, value, role );
        case NodeModel::NodeStartupAccount: return setStartupAccount( n, value, role );
        case NodeModel::NodeStartupCost: return setStartupCost( n, value, role );
        case NodeModel::NodeShutdownAccount: return setShutdownAccount( n, value, role );
        case NodeModel::NodeShutdownCost: return setShutdownCost( n, value, role );
        case NodeModel::NodeDescription: return setDescription( n, value, role );
        default:
            qWarning("data: invalid display value column %d", index.column());
            return false;
    }
    return false;
}

QVariant MilestoneItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole ) {
            return m_nodemodel.headerData( section, role );
        } else if ( role == Qt::TextAlignmentRole ) {
            switch (section) {
                case NodeModel::NodeType: return Qt::AlignCenter;
                default: return QVariant();
            }
        }
    }
    if ( role == Qt::ToolTipRole ) {
        return NodeModel::headerData( section, role );
    }
    return ItemModelBase::headerData(section, orientation, role);
}

QItemDelegate *MilestoneItemModel::createDelegate( int column, QWidget *parent ) const
{
    switch ( column ) {
        case NodeModel::NodeConstraint: return new EnumDelegate( parent );
        case NodeModel::NodeRunningAccount: return new EnumDelegate( parent );
        case NodeModel::NodeStartupAccount: return new EnumDelegate( parent );
        case NodeModel::NodeStartupCost: return new MoneyDelegate( parent );
        case NodeModel::NodeShutdownAccount: return new EnumDelegate( parent );
        case NodeModel::NodeShutdownCost: return new MoneyDelegate( parent );
        default: return 0;
    }
    return 0;
}

int MilestoneItemModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return m_nodemodel.propertyCount();
}

int MilestoneItemModel::rowCount( const QModelIndex &parent ) const
{
    //kDebug()<<parent;
    if ( parent.isValid() ) {
        return 0;
    }
    //kDebug()<<m_mslist.count();
    return m_mslist.count();
}

Qt::DropActions MilestoneItemModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}


QStringList MilestoneItemModel::mimeTypes() const
{
    return QStringList();
}

QMimeData *MilestoneItemModel::mimeData( const QModelIndexList & indexes ) const
{
    QMimeData *m = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QList<int> rows;
    foreach (QModelIndex index, indexes) {
        if ( index.isValid() && !rows.contains( index.row() ) ) {
            //kDebug()<<index.row();
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

bool MilestoneItemModel::dropAllowed( const QModelIndex &index, int dropIndicatorPosition, const QMimeData *data )
{
    //kDebug();
    Node *dn = node( index );
    if ( dn == 0 ) {
        kError()<<"no node to drop on!";
        return false; // hmmm
    }
    switch ( dropIndicatorPosition ) {
        case ItemModelBase::AboveItem:
        case ItemModelBase::BelowItem:
            // dn == sibling
            return dropAllowed( dn->parentNode(), data );
        case ItemModelBase::OnItem:
            // dn == new parent
            return dropAllowed( dn, data );
        default:
            break;
    }
    return false;
}

bool MilestoneItemModel::dropAllowed( Node *on, const QMimeData *data )
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

QList<Node*> MilestoneItemModel::nodeList( QDataStream &stream )
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

QList<Node*> MilestoneItemModel::removeChildNodes( QList<Node*> nodes )
{
    QList<Node*> lst;
    foreach ( Node *node, nodes ) {
        bool ins = true;
        foreach ( Node *n, lst ) {
            if ( node->isChildOf( n ) ) {
                //kDebug()<<node->name()<<" is child of"<<n->name();
                ins = false;
                break;
            }
        }
        if ( ins ) {
            //kDebug()<<" insert"<<node->name();
            lst << node;
        }
    }
    QList<Node*> nl = lst;
    QList<Node*> nlst = lst;
    foreach ( Node *node, nl ) {
        foreach ( Node *n, nlst ) {
            if ( n->isChildOf( node ) ) {
                //kDebug()<<n->name()<<" is child of"<<node->name();
                int i = nodes.indexOf( n );
                lst.removeAt( i );
            }
        }
    }
    return lst;
}

bool MilestoneItemModel::dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int /*column*/, const QModelIndex &parent )
{
    //kDebug()<<action;
    if (action == Qt::IgnoreAction) {
        return true;
    }
    if ( !data->hasFormat( "application/x-vnd.kde.kplato.nodeitemmodel.internal" ) ) {
        return false;
    }
    if ( action == Qt::MoveAction ) {
        //kDebug()<<"MoveAction";
        
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
                //kDebug()<<"Can't move task:"<<n->name();
                return false;
            }
        }
        int offset = 0;
        MacroCommand *cmd = 0;
        foreach ( Node *n, nodes ) {
            if ( cmd == 0 ) cmd = new MacroCommand( i18n( "Move tasks" ) );
            cmd->addCommand( new NodeMoveCmd( m_project, n, par, row + offset ) );
            offset++;
        }
        if ( cmd ) {
            emit executeCommand( cmd );
        }
        //kDebug()<<row<<","<<column<<" parent="<<parent.row()<<","<<parent.column()<<":"<<par->name();
        return true;
    }
    return false;
}

Node *MilestoneItemModel::node( const QModelIndex &index ) const
{
    Node *n = m_project;
    if ( index.isValid() ) {
        //kDebug()<<index;
        n = static_cast<Node*>( index.internalPointer() );
    }
    return n;
}

void MilestoneItemModel::slotNodeChanged( Node *node )
{
    //kDebug()<<node->name();
    if ( node == 0 ) {
        return;
    }
    if ( node->type() != Node::Type_Milestone ) {
        // Type may have changed from Milestone, so try to remove
        slotNodeToBeRemoved( node );
        return;
    }
    int row = m_mslist.indexOf( node );
    if ( row == -1 ) {
        // Type may have changed to Milestone, so insert
        slotNodeInserted( node );
        return;
    }
    //kDebug()<<node->name()<<": "<<row;
    emit dataChanged( createIndex( row, 0, node ), createIndex( row, columnCount(), node ) );
}


} //namespace KPlato

#include "kptnodeitemmodel.moc"
