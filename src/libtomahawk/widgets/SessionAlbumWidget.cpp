/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2012, Casey Link <unnamedrambler@gmail.com>
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *   Copyright 2011, Leo Franchi <lfranchi@kde.org>
 *   Copyright 2011, Jeff Mitchell <jeff@tomahawk-player.org>
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

#include "SessionAlbumWidget.h"
#include "../playlist/SessionHistoryModel.h"
#include "ui_SessionAlbumWidget.h"
#include "playlist/RecentlyPlayedModel.h"

using namespace Tomahawk;

SessionAlbumWidget::SessionAlbumWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionAlbumWidget)
{
    ui->setupUi(this);

    m_sessionsModel = new SessionHistoryModel(ui->sessionsView) ;
    //ui->sessionsView->setItemDelegate( new PlaylistDelegate() ); TODO : Delegate
    //ui->sessionsView->overlay()->resize( 380, 86 );
    //ui->sessionsView->setVerticalScrollMode( QAbstractItemView::ScrollPerPixel );
    ui->sessionsView->setModel( m_sessionsModel );
    m_sessionsModel->setSource( source_ptr() );
}

SessionAlbumWidget::~SessionAlbumWidget()
{
    delete ui;
}

Tomahawk::playlistinterface_ptr
SessionAlbumWidget::playlistInterface() const
{
    return m_playlistInterface;
}


bool
SessionAlbumWidget::isBeingPlayed() const
{
    return false;
}


bool
SessionAlbumWidget::jumpToCurrentTrack()
{
    return false;
}

