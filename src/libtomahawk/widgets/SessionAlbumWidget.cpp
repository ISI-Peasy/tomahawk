#include "SessionAlbumWidget.h"
#include "../playlist/SessionHistoryModel.h"
#include "ui_SessionAlbumWidget.h"

using namespace Tomahawk;

SessionAlbumWidget::SessionAlbumWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionAlbumWidget)
{
    ui->setupUi(this);
    m_sessionsModel = new SessionHistoryModel(ui->sessionsView) ;

    //m_sessionsModel = new RecentlyPlayedModel( ui->sessionsView );
    //ui->sessionsView->proxyModel()->setStyle( PlayableProxyModel::ShortWithAvatars );
    //ui->sessionsView->overlay()->setEnabled( false );
    //ui->sessionsView->setPlaylistModel( m_tracksModel );
    m_sessionsModel->setSource( source_ptr() );

    // TODO : connecter la view avec le model : SessionHistoryModel ( actuellement copie du RecentlyPlayed mais amené a varier :) )
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

