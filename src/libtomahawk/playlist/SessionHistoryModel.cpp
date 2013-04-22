/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2012, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *   Franck Arrecot <franck.arrecot@gmail.com>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "SessionHistoryModel.h"

#include <QMimeData>
#include <QTreeView>

#include "Session.h"
#include "Source.h"
#include "SourceList.h"
#include "database/Database.h"
#include "database/DatabaseCommand_PlaybackHistorySessions.h"
#include "database/DatabaseCommand_GenericSelect.h"
#include "PlayableItem.h"
#include "utils/TomahawkUtils.h"
#include "utils/Logger.h"

#define HISTORY_SESSION_ITEMS 100

const static int MAX_TIME_BETWEEN_TRACKS = 10 * 60 * 60;
const static int MIN_SESSION_COUNT = 1;

using namespace Tomahawk;

SessionHistoryModel::SessionHistoryModel( QObject* parent )
    //: PlaylistModel( parent )
      : QAbstractListModel( parent )
    , m_limit( HISTORY_SESSION_ITEMS )
{
    connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), this, SLOT( onSourceAdded( Tomahawk::source_ptr ) ), Qt::QueuedConnection );
}


SessionHistoryModel::~SessionHistoryModel()
{
}

void
SessionHistoryModel::loadHistory()
{
    if ( rowCount( QModelIndex() ) )
    {
       //beginResetModel(); // usefull ?
    }
    //startLoading(); // to replace

    retrievePlayBackSongs() ;
    //retrieveLovedSongs(); // TODO : Reason of segfault : to debug
}

void
SessionHistoryModel::retrievePlayBackSongs()
{
    DatabaseCommand_PlaybackHistorySessions* cmd = new DatabaseCommand_PlaybackHistorySessions( m_source );
    cmd->setLimit( m_limit );

    // Collect the return from db into SessionsFromQueries to build sessions
    connect( cmd, SIGNAL( tracks( QList<Tomahawk::query_ptr> ) ),
                    SLOT( sessionsFromQueries( QList<Tomahawk::query_ptr> ) ), Qt::QueuedConnection );

    Database::instance()->enqueue( QSharedPointer<DatabaseCommand>( cmd ) );
}

void
SessionHistoryModel::retrieveLovedSongs()
{
    QString sql;
    if ( m_source.isNull() )
    {
        sql = QString( "SELECT track.name, artist.name, source, COUNT(*) as counter "
                       "FROM social_attributes, track, artist "
                       "WHERE social_attributes.id = track.id AND artist.id = track.artist AND social_attributes.k = 'Love' AND social_attributes.v = 'true' "
                       "GROUP BY track.id "
                       "ORDER BY counter DESC, social_attributes.timestamp DESC LIMIT 0, 50" );
    }
    else
    {
        sql = QString( "SELECT track.name, artist.name, COUNT(*) as counter "
                       "FROM social_attributes, track, artist "
                       "WHERE social_attributes.id = track.id AND artist.id = track.artist AND social_attributes.k = 'Love' AND social_attributes.v = 'true' AND social_attributes.source %1 "
                       "GROUP BY track.id "
                       "ORDER BY counter DESC, social_attributes.timestamp DESC " ).arg( m_source->isLocal() ? "IS NULL" : QString( "= %1" ).arg( m_source->id() ) );
    }

    DatabaseCommand_GenericSelect* cmd = new DatabaseCommand_GenericSelect( sql, DatabaseCommand_GenericSelect::Track, -1, 0 );
    //connect( cmd, SIGNAL( tracks( QList<Tomahawk::query_ptr> ) ), this, SLOT( tracksLoaded( QList<Tomahawk::query_ptr> ) ) );

    connect( cmd, SIGNAL( tracks( QList<Tomahawk::query_ptr> ) ),
                    SLOT( sessionsFromQueries( QList<Tomahawk::query_ptr> ) ), Qt::QueuedConnection );

    Database::instance()->enqueue( QSharedPointer<DatabaseCommand>( cmd ) );
}


void
SessionHistoryModel::onSourcesReady()
{
    Q_ASSERT( m_source.isNull() );

    foreach ( const source_ptr& source, SourceList::instance()->sources() )
        onSourceAdded( source );

    loadHistory();
}


void
SessionHistoryModel::setSource( const Tomahawk::source_ptr& source )
{
    m_source = source;
    if ( source.isNull() )
    {
        if ( SourceList::instance()->isReady() )
            onSourcesReady();
        else
            connect( SourceList::instance(), SIGNAL( ready() ), SLOT( onSourcesReady() ) );

        connect( SourceList::instance(), SIGNAL( sourceAdded( Tomahawk::source_ptr ) ), SLOT( onSourceAdded( Tomahawk::source_ptr ) ) );
    }
    else
    {
        onSourceAdded( source );
        loadHistory();
    }
}


void
SessionHistoryModel::onSourceAdded( const Tomahawk::source_ptr& source )
{
    connect( source.data(), SIGNAL( playbackFinished( Tomahawk::query_ptr ) ), SLOT( onPlaybackFinished( Tomahawk::query_ptr ) ), Qt::UniqueConnection );
}


void
SessionHistoryModel::onPlaybackFinished( const Tomahawk::query_ptr& query )
{
    loadHistory();
}


void
SessionHistoryModel::sessionsFromQueries( const QList< Tomahawk::query_ptr >& queries )
{
    tDebug() << "Session Calculate From Queries Beginin" ;

    if ( !queries.count() )
    {
        //emit itemCountChanged( rowCount( QModelIndex() ) ); // TODO : Replace
        //finishLoading(); // TODO : Replace
        return;
    }
    else tDebug() << "Session Calculate starting" ;

    tDebug() << "Number of Queries retireved" <<queries.count() ;

    QList< Session* > sessions = QList< Session* >();

    Session* oneSession = new Session();

    QString currentPeer = QString();
    unsigned int lastTimeStamp = 0;

    for( int i = 0 ; i < queries.count() ; i++ )
    {
        Tomahawk::query_ptr ptr_q = queries.at( i );
        Tomahawk::Query *query = ptr_q.data();

        if( lastTimeStamp == 0 )
        {
            lastTimeStamp = query->playedBy().second;
        }

        if( currentPeer == "")
        {
            currentPeer = query->playedBy().first->friendlyName();
        }

        if( lastTimeStamp - query->playedBy().second < MAX_TIME_BETWEEN_TRACKS && currentPeer == query->playedBy().first->friendlyName() )
        {
            //it's the same session, we add it
            oneSession->addQuery(ptr_q);

        }
        else
        {
            //add the current session in the session list only if session contains MIN_SESSION_COUNT at least
            if( oneSession->count() >= MIN_SESSION_COUNT )
            {
                sessions << oneSession;

            }

            //new session
            oneSession = new Session();
            currentPeer = query->playedBy().first->friendlyName();
            //add the current query in the new session
            oneSession->addQuery(ptr_q);
        }

        lastTimeStamp = query->playedBy().second + query->duration();

    }

    //add the last session in the session list only if session contains MIN_SESSION_COUNT at least
    if( oneSession->count() >= MIN_SESSION_COUNT )
    {
        sessions << oneSession;

    }

    //sort sessions list
    qSort(sessions.begin(), sessions.end(), SessionGreatThan());

    // insert the sessions inside the model
    feedModelWithSessions(QList< QPair< QString, QList< Tomahawk::query_ptr> > >()) ;
    //feedModelWithSessions( sessions );

    //debug : show sessions
    tDebug() << "We have calculate " << sessions.count() << " sessions : ";
    for( int i = 0 ; i < sessions.count() ; i++ )
    {
        tDebug() << "session " << ( i + 1 ) << " : " << sessions.at(i)->getSessionOwner() << " [" <<  sessions.at(i)->count() << "]";
        //tDebug() << "   by Session : " << mySessions.at(i)->getSessionOwner() << " played " << mySessions.at(i)->getPredominantAlbum() << " of " << mySessions.at(i)->getPredominantArtist() << " from " << mySessions.at(i)->getStartTime() << " to " << mySessions.at(i)->getEndTime();

        foreach ( const Tomahawk::query_ptr track, sessions.at(i)->getTracks() )
        {
            tDebug() << "   - " << track->artist() << track->track() << " from " << track->playedBy().first->friendlyName() << " played at " << track->playedBy().second ;
        };
    }

}

void
//SessionHistoryModel::feedModelWithQueries ( const QPair< QString, QList< Tomahawk::query_ptr > > &  aSession)
SessionHistoryModel::feedModelWithSessions ( const QList< QPair< QString, QList< Tomahawk::query_ptr > > > sessions)
{
    if (sessions.count()) // TODO : add tracks as child of the session name
    {
        beginResetModel();
        m_sessionslist.clear();
        QPair< QString, QList< Tomahawk::query_ptr > > aSession = QPair< QString, QList< Tomahawk::query_ptr > >();

        for( int i = 0 ; i < sessions.count() ; i++ )
        {
            aSession = sessions.at(i) ;
            if(!aSession.first.isNull() && aSession.second.count()) m_sessionslist << aSession ;
        }
    }
    endResetModel();
}

QVariant SessionHistoryModel::data( const QModelIndex& index, int role ) const
{
    if( !index.isValid() || !hasIndex( index.row(), index.column(), index.parent() ) )
        return QVariant();

    QPair< QString, QList< Tomahawk::query_ptr> > mySession =  m_sessionslist.at(index.row()) ;

    switch( role )
    {
    case Qt::DisplayRole:
    {
        return (mySession.second.at(0)->playedBy().first->friendlyName() +" : "+ mySession.first ) ;
    }
    case SourceRole:
    {
        return mySession.second.at(0)->playedBy().first->friendlyName() ;
    }
    case SessionRole:
    {
        return mySession.first ;
    }
    case PlaytimeRole:
    {
        return QVariant::fromValue(mySession.second.at(0)->playedBy().second );
    }
    default:
        return QVariant();
    }
}

int SessionHistoryModel::rowCount( const QModelIndex& ) const
{
    return m_sessionslist.count() ;
}

