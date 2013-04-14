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

#ifndef SESSIONALBUMWIDGET_H
#define SESSIONALBUMWIDGET_H

#include <QWidget>
#include "PlaylistInterface.h"
#include "ViewPage.h"

class SessionHistoryModel;

namespace Ui {
    class SessionAlbumWidget;
}

namespace Tomahawk
{
    class ChartDataLoader;
    class ChartsPlaylistInterface;
    class ChartDataLoader;
}


class SessionAlbumWidget : public QWidget, public Tomahawk::ViewPage
{
    Q_OBJECT
    
public:
    explicit SessionAlbumWidget(QWidget *parent = 0);
    ~SessionAlbumWidget();

    virtual QWidget* widget() { return this; }
    virtual Tomahawk::playlistinterface_ptr playlistInterface() const;

    virtual QString title() const { return tr( "New Releases" ); }
    virtual QString description() const { return QString(); }

    virtual bool showInfoBar() const { return false; }
    virtual bool isBeingPlayed() const;

    virtual bool jumpToCurrentTrack();

    
private:
    Ui::SessionAlbumWidget *ui;
    SessionHistoryModel *m_sessionsModel ;
    Tomahawk::playlistinterface_ptr m_playlistInterface;
};

#endif // SESSIONALBUMWIDGET_H
