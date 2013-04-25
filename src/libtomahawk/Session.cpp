#include "Session.h"
#include "Source.h"

Session::Session()
{
    m_queries = QList< Tomahawk::query_ptr >();
    m_tracks = QList < QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> >() ;
}

Session::Session(const Session& copy)
{
    m_tracks = QList< QPair<Tomahawk::track_ptr , Tomahawk::PlaybackLog> >(copy.getTracks()) ;
}

Session::~Session() {

}

//Session* Session::operator<<( Tomahawk::query_ptr query )
//{
//    m_queries << query;
//    return this;
//}

QString
Session::getSessionOwner()
{
    return m_tracks.first().second.source->friendlyName() ;
}

Tomahawk::source_ptr
Session::getSessionSource()
{
    return m_tracks.first().second.source ;
}

QList<Tomahawk::query_ptr>
Session::getTrackstoQuery()
{
    QList<Tomahawk::query_ptr> queries = QList<Tomahawk::query_ptr>() ;
    QPair<Tomahawk::track_ptr , Tomahawk::PlaybackLog> currentTrack ;

    foreach (currentTrack , m_tracks) { queries << currentTrack.first->toQuery() ; };
    return queries ;
}

void
Session::addQuery( QPair<Tomahawk::track_ptr , Tomahawk::PlaybackLog>& track )
{
    m_tracks << track ;
}

QString
Session::getPredominantArtist()
{
    QString currentArtist = QString();
    QList< QString >aSessionArtists = QList< QString >();
    int currentArtistOccurs = 0;

    //first, get all artists of the session
    for( int i = 0; i < m_tracks.count(); i++ )
    {
        QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> track = m_tracks.at(i) ;
        aSessionArtists << track.first->artist() ;
    }

    //then, compute the predominant artist
    for( int i = 0; i < aSessionArtists.count(); i++ )
    {
        if( aSessionArtists.count( aSessionArtists.at(i) ) > currentArtistOccurs )
        {
            currentArtist = aSessionArtists.at(i);
            currentArtistOccurs = aSessionArtists.count( aSessionArtists.at(i) );
        }
    }
    return currentArtist;
}

QList< QString >
Session::getRecurentArtists( const int max ){
    QList< QString > artists = QList< QString >();
    for( int i = 0; i < m_tracks.count() && i < max; i++ )
    {
        QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> track = m_tracks.at(i) ;
        if( artists.count( track.first->artist() ) == 0 )
        {
            artists << track.first->artist();
        }
        else
        {
            i--;
        }
    }
    return artists;
}

QString
Session::getPredominantAlbum()
{
    QString currentAlbum = QString("empty");
    QList< QString >aSessionAlbum = QList< QString >();
    int currentAlbumOccurs = 0;

    //first, get all albums of the session
    for( int i = 0; i < m_tracks.count(); i++ )
    {
        QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> track = m_tracks.at(i) ;
        if(track.first->album().size() > 0)
        {
            aSessionAlbum << track.first->album() ;
        }
    }
    //then, compute the predominant album
    for( int i = 0; i < aSessionAlbum.count(); i++ )
    {
        if( aSessionAlbum.count( aSessionAlbum.at(i) ) > currentAlbumOccurs )
        {
            currentAlbum = aSessionAlbum.at(i);
            currentAlbumOccurs = aSessionAlbum.count( aSessionAlbum.at(i) );
        }
    }
    return currentAlbum;
}

int
Session::getStartTime()
{
    return m_tracks.last().second.timestamp ;
}

int
Session::getEndTime()
{
    return m_tracks.first().second.timestamp + m_tracks.first().second.secsPlayed;
}

int
Session::count()
{
    return m_tracks.size() ;
}

bool
Session::trackExist( QString id )
{
    for( int i = 0; i < m_tracks.count(); i++ )
    {
        QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> track = m_tracks.at(i) ;
        if( track.first->id() == id )
        {
            return true;
        }
    }
    return false;
}

QList < QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> >
Session::getTracks() const
{
    return m_tracks;
}

bool
Session::operator<( Session s )
{
    return this->getEndTime() < s.getEndTime();
}


//template <class Session> bool
//qGreater<Session>::operator()(const Session& s1, const Session& s2) const
//{
//    return s1->getEndTime() < s2->getEndTime();
//}


SessionGreatThan::SessionGreatThan()
{
}

bool
SessionGreatThan::operator ()( Session* s1, Session* s2 )
{
    return s1->getEndTime() > s2->getEndTime();
}

