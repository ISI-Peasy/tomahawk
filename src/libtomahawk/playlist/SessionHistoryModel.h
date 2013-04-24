/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *   Copyright 2010-2011, Franck Arrecot <franck.arrecot@gmail.com>
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

#ifndef SESSIONHISTORYMODEL_H
#define SESSIONHISTORYMODEL_H

#include <QList>
#include <QHash>

#include "Session.h"

#include "Typedefs.h"
#include <QModelIndex>
#include "PlaylistModel.h"

#include "DllMacro.h"


class DLLEXPORT SessionHistoryModel : public QAbstractListModel
{
Q_OBJECT

public:
    enum ItemRoles
    { SessionRole = Qt::DisplayRole , SourceRole, PlaytimeRole, SizeRole , SessionItemRole };
    explicit SessionHistoryModel( QObject* parent = 0 );
    virtual ~SessionHistoryModel();

    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    unsigned int limit() const { return m_limit; }
    void setLimit( unsigned int limit ) { m_limit = limit; }

public slots:
    void setSource( const Tomahawk::source_ptr& source );
    void loadHistory();

private slots:
    void onSourcesReady();
    void onSourceAdded( const Tomahawk::source_ptr& source );
    void onPlaybackFinished( const Tomahawk::track_ptr& track, const Tomahawk::PlaybackLog& log );


    void retrieveLovedSongs() ;
    void retrievePlayBackSongs() ;
    void sessionsFromQueries( const QList<Tomahawk::track_ptr>& tracks, const QList<Tomahawk::PlaybackLog>& logs ) ;
    void feedModelWithSessions ( const QList< Session* >& sessions ) ;


private:
    unsigned int m_limit;
    Tomahawk::source_ptr m_source;
    QList< Session* > m_sessionslist ;
};

#endif // SESSIONHISTORYMODEL_H
