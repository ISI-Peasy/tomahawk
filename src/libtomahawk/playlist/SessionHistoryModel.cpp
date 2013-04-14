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

#define HISTORY_SESSION_ITEMS 10

const static int MAX_TIME_BETWEEN_TRACKS = 10 * 60 * 60;

using namespace Tomahawk;


SessionHistoryModel::SessionHistoryModel( QObject* parent )
    //: PlaylistModel( parent )
      : QAbstractListModel( parent )
    , m_limit( HISTORY_SESSION_ITEMS )
{
}


SessionHistoryModel::~SessionHistoryModel()
{
}

void
SessionHistoryModel::loadHistory()
{
    if ( rowCount( QModelIndex() ) )
    {
       // beginResetModel(); // usefull ?
    }
    //startLoading(); // usefull ?

    retrievePlayBackSongs() ;
    retrieveLovedSongs();
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
    //TODO
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

    QList< QPair< QString, QList< Tomahawk::query_ptr > > > sessions = QList< QPair< QString, QList< Tomahawk::query_ptr> > >();
    QPair< QString, QList< Tomahawk::query_ptr > > aSession = QPair< QString, QList< Tomahawk::query_ptr > >();
    aSession.second = QList< Tomahawk::query_ptr >();

    QList< QString > aSessionArtists = QList< QString >();
    QString currentArtist;
    int currentArtistOccurs;

    unsigned int lastTimeStamp = 0;
    // TODO : sort peers

    for( int i = 0 ; i < queries.count() ; i++ ) // TODO : check duplicate inside queries to erase it
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
            aSessionArtists << query->artist();
        }
        else
        {
            //calculate the most recurent artist of the session
            currentArtist = QString();
            currentArtistOccurs = 0;
            for( int ca = 0; ca < aSessionArtists.count(); ca++ )
            {
                if( aSessionArtists.count( aSessionArtists.at(ca) ) > currentArtistOccurs )
                {
                    currentArtist =  aSessionArtists.at(ca);
                    currentArtistOccurs = aSessionArtists.count( aSessionArtists.at(ca) );
                }
            }

            aSessionArtists = QList< QString >();

            //add the curent session to the session list
            aSession.first = currentArtist;

            sessions << aSession;
            //new session
            aSession = QPair< QString, QList< Tomahawk::query_ptr > >();
            //add the current query in the new session
            aSession.second << ptr_q;
            aSessionArtists << query->artist();
        }

        lastTimeStamp = query->playedBy().second;

      //  tDebug() << "~~~~ session query " << i << " : " << query->toString() << " ~ " << query->playedBy().first << " ~ " << query->playedBy().second;
    }
    //debug : show sessions
    tDebug() << "We have calculate " << sessions.count() << " sessions :";
    for( int i = 0 ; i < sessions.count() ; i++ )
    {
        tDebug() << "session " << ( i + 1 ) << " : " << sessions.at(i).first << " [" <<  sessions.at(i).second.count() << "]";

        foreach ( const Tomahawk::query_ptr track, sessions.at(i).second )
        {
            tDebug() << "   - " << track->artist() << track->track() << " from " << track->playedBy().first->friendlyName() << " played at " << track->playedBy().second ;
        };
    }
    // TODO : find a proper way of return : emit or return ? to feed the model
    feedModelWithSessions(sessions) ;

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

    if (role == Qt::DisplayRole)
    {
        //
        QString source = m_sessionslist.at(index.row()).second.at(0)->playedBy().first->friendlyName() ;
        QString sessionLabel = m_sessionslist.at(index.row()).first ;
        return (source +" 'session : "+ sessionLabel) ;
    }
    else
        return QVariant();
}

int SessionHistoryModel::rowCount( const QModelIndex& ) const
{
    return m_sessionslist.count() ;
}

