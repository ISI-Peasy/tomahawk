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

#include "Source.h"
#include "SourceList.h"
#include "database/Database.h"
#include "database/DatabaseCommand_PlaybackHistory.h"
#include "database/DatabaseCommand_GenericSelect.h"
#include "PlayableItem.h"
#include "utils/TomahawkUtils.h"
#include "utils/Logger.h"

#define HISTORY_TRACK_ITEMS 25

const static int MAX_TIME_BETWEEN_TRACKS = 10 * 60 * 60;

using namespace Tomahawk;


SessionHistoryModel::SessionHistoryModel( QObject* parent )
    : PlaylistModel( parent )
    , m_limit( HISTORY_TRACK_ITEMS )
{
    loadHistory();
}


SessionHistoryModel::~SessionHistoryModel()
{
}


void
SessionHistoryModel::loadHistory()
{
    if ( rowCount( QModelIndex() ) )
    {
        clear();
    }
    startLoading();

    // Retrieve Data from DB and pass it out to session builder
    retrievePlayBackSongs() ;
    retrieveLovedSongs();

    // TODO get the return from the session maker ! slot&co

}

void
SessionHistoryModel::retrievePlayBackSongs()
{
    DatabaseCommand_PlaybackHistory* cmd = new DatabaseCommand_PlaybackHistory( m_source );
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

    loadHistory();

    foreach ( const source_ptr& source, SourceList::instance()->sources() )
        onSourceAdded( source );
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
    int count = trackCount();
    unsigned int playtime = query->playedBy().second;

    if ( count )
    {
        PlayableItem* oldestItem = itemFromIndex( index( count - 1, 0, QModelIndex() ) );
        if ( oldestItem->query()->playedBy().second >= playtime )
            return;

        PlayableItem* youngestItem = itemFromIndex( index( 0, 0, QModelIndex() ) );
        if ( youngestItem->query()->playedBy().second <= playtime )
            insertQuery( query, 0 );
        else
        {
            for ( int i = 0; i < count - 1; i++ )
            {
                PlayableItem* item1 = itemFromIndex( index( i, 0, QModelIndex() ) );
                PlayableItem* item2 = itemFromIndex( index( i + 1, 0, QModelIndex() ) );

                if ( item1->query()->playedBy().second >= playtime && item2->query()->playedBy().second <= playtime )
                {
                    insertQuery( query, i + 1 );
                    break;
                }
            }
        }
    }
    else
        insertQuery( query, 0 );

    if ( trackCount() > (int)m_limit )
        remove( m_limit );

    ensureResolved();
}

void
SessionHistoryModel::sessionsFromQueries( const QList< Tomahawk::query_ptr >& queries )
{
    tDebug() << "Session Calculate From Queries Beginin" ;

    if ( !queries.count() )
    {
        emit itemCountChanged( rowCount( QModelIndex() ) );
        finishLoading();
        return;
    }
    else tDebug() << "Session Calculate starting" ;

    QList< QPair< QString, QList< Tomahawk::query_ptr > > > sessions = QList< QPair< QString, QList< Tomahawk::query_ptr> > >();

    QPair< QString, QList< Tomahawk::query_ptr > > aSession = QPair< QString, QList< Tomahawk::query_ptr > >();
    aSession.second = QList< Tomahawk::query_ptr >();

    QList< QString > aSessionArtists = QList< QString >();

    unsigned int lastTimeStamp = 0;

    for( int i = 0 ; i < queries.count() ; i++ )
    {
        Tomahawk::query_ptr ptr_q = queries.at( i );
        Tomahawk::Query *query = ptr_q.data();

        if( lastTimeStamp == 0 )
        {
            lastTimeStamp = query->playedBy().second;
        }

        //TODO: enhancement : use the duration to calculate better the sessions
        if( lastTimeStamp - query->playedBy().second < MAX_TIME_BETWEEN_TRACKS )
        {
            //it's the same session, we add it
            aSession.second << ptr_q;
        }
        else
        {
            //add the curent session to the session list
            aSession.first = QString("Session ");

            sessions << aSession;
            //new session
            aSession = QPair< QString, QList< Tomahawk::query_ptr > >();
            //add the current query in the new session
            aSession.second << ptr_q;
        }

        lastTimeStamp = query->playedBy().second;
    }

    //debug : show sessions
    for( int i = 0 ; i < sessions.count() ; i++ )
    {
        tDebug() << "Session " << i << " : " << sessions.at(i).first << " [ " <<  sessions.at(i).second.count() << "]";
        foreach ( const Tomahawk::query_ptr track, sessions.at(i).second )
        {
            tDebug() << "   -  " << track->toString();
        };
    }
     // TODO : find a way of return : emit or return ?
}


bool
SessionHistoryModel::isTemporary() const
{
    return true;
}
