#include "Session.h"
#include "Source.h"

Session::Session()
{
    m_queries = QList< Tomahawk::query_ptr >();
}

//Session* Session::operator<<( Tomahawk::query_ptr query )
//{
//    m_queries << query;
//    return this;
//}

QString
Session::getSessionOwner()
{
    Tomahawk::Query *query = m_queries.first().data();
    return query->playedBy().first->friendlyName();
}

void
Session::addQuery( Tomahawk::query_ptr q )
{
    m_queries << q;
}

QString
Session::getPredominantArtist()
{
    QString currentArtist = QString();
    QList< QString >aSessionArtists = QList< QString >();
    int currentArtistOccurs = 0;

    //first, get all artists of the session
    for( int i = 0; i < m_queries.count(); i++ )
    {
        Tomahawk::Query *query = m_queries.at(i).data();
        aSessionArtists << query->artist();
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

QString
Session::getPredominantAlbum()
{
    QString currentAlbum = QString();
    QList< QString >aSessionAlbum = QList< QString >();
    int currentAlbumOccurs = 0;

    //first, get all albums of the session
    for( int i = 0; i < m_queries.count(); i++ )
    {
        Tomahawk::Query *query = m_queries.at(i).data();
        aSessionAlbum << query->album();
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
    Tomahawk::Query *query = m_queries.first().data();
    return query->playedBy().second;
}

int
Session::getEndTime()
{
    Tomahawk::Query *query = m_queries.last().data();
    return query->playedBy().second +  query->duration();
}
