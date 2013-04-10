#include "SessionAlbumWidget.h"
#include "ui_SessionAlbumWidget.h"

SessionAlbumWidget::SessionAlbumWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionAlbumWidget)
{
    ui->setupUi(this);
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

void
SessionAlbumWidget::loadData()
{
   // m_recentAlbumsModel->addFilteredCollection( collection_ptr(), 20, DatabaseCommand_AllAlbums::ModificationTime, true );
}

