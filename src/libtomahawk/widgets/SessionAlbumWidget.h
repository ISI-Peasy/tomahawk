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

//class DLLEXPORT PlaylistWidget : public QListView
//{
//Q_OBJECT

//public:
//    PlaylistWidget( QWidget* parent = 0 );

//    OverlayWidget* overlay() const { return m_overlay; }

//    virtual void setModel( QAbstractItemModel* model );

//signals:
//    void modelChanged();

//private:
//    OverlayWidget* m_overlay;
//};

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
