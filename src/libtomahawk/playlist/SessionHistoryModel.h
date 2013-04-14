/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
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

#ifndef SESSIONHISTORYMODEL_H
#define SESSIONHISTORYMODEL_H

#include <QList>
#include <QHash>

#include "Typedefs.h"
#include <QModelIndex>
#include "PlaylistModel.h"

#include "DllMacro.h"


class DLLEXPORT SessionHistoryModel : public QAbstractListModel
{
Q_OBJECT

public:
    explicit SessionHistoryModel( QObject* parent = 0 );
    ~SessionHistoryModel();

    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

    unsigned int limit() const { return m_limit; }
    void setLimit( unsigned int limit ) { m_limit = limit; }

public slots:
    void setSource( const Tomahawk::source_ptr& source );

private slots:
    void onSourcesReady(); // PlaylistModel
    void onSourceAdded( const Tomahawk::source_ptr& source );
    void onPlaybackFinished( const Tomahawk::query_ptr& query );

    void loadHistory();
    void retrieveLovedSongs() ;
    void retrievePlayBackSongs() ;
    void sessionsFromQueries( const QList< Tomahawk::query_ptr >& queries ) ;
    void feedModelWithSessions ( const QList< QPair< QString, QList< Tomahawk::query_ptr > > > sessions) ;

private:
    unsigned int m_limit;
    Tomahawk::source_ptr m_source;
    QList< QPair< QString, QList< Tomahawk::query_ptr > > > m_sessionslist ;
};

#endif // SESSIONHISTORYMODEL_H
