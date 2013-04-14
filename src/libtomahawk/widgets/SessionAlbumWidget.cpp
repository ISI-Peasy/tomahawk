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
#include "utils/TomahawkUtils.h"
#include "TomahawkSettings.h"
#include "../playlist/SessionHistoryModel.h"
#include "ui_SessionAlbumWidget.h"
#include "playlist/RecentlyPlayedModel.h"
#include "utils/TomahawkUtilsGui.h"
#include "SourceList.h"

#include <QPainter>


using namespace Tomahawk;

SessionAlbumWidget::SessionAlbumWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionAlbumWidget)
{
    ui->setupUi(this);
    m_sessionsModel = new SessionHistoryModel(ui->sessionsView) ;
    ui->sessionsView->setModel( m_sessionsModel );
    //ui->sessionsView->setItemDelegate( new SessionDelegate() );
    m_sessionsModel->setSource( source_ptr() );

    connect( SourceList::instance(), SIGNAL( ready() ), SLOT( onSourcesReady() ) );
}

void
SessionAlbumWidget::loadData()
{
    //m_sessionsModel->loadHistory(); // doesn't work because the db isn't ready when we call it
}

void SessionAlbumWidget::onSourcesReady()
{
    foreach ( const source_ptr& source, SourceList::instance()->sources() )
        onSourceAdded( source );
}

void SessionAlbumWidget::onSourceAdded( const Tomahawk::source_ptr& source )
{

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

// Delegate part :

void SessionDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    // TODO : Delegate Paint - Laumaned'job
}

QSize SessionDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( option );
    Q_UNUSED( index );

    // Calculates the size for the bold line + 3 normal lines + margins
    int height = 2 * 6; // margins
    QFont font = option.font;
    QFontMetrics fm1( font );
    font.setPointSize( TomahawkUtils::defaultFontSize() - 1 );
    height += fm1.height() * 3;
    font.setPointSize( TomahawkUtils::defaultFontSize() );
    QFontMetrics fm2( font );
    height += fm2.height();

    return QSize( 0, height );
}
