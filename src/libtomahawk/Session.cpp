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

Tomahawk::source_ptr
Session::getSessionSource()
{
    Tomahawk::Query *query = m_queries.first().data();
    return query->playedBy().first;
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
    QString currentAlbum = QString("empty");
    QList< QString >aSessionAlbum = QList< QString >();
    int currentAlbumOccurs = 0;

    //first, get all albums of the session
    for( int i = 0; i < m_queries.count(); i++ )
    {
        Tomahawk::Query *query = m_queries.at(i).data();
        if(query->album().size() > 0)
        {
            aSessionAlbum << query->album();
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
    Tomahawk::Query *query = m_queries.first().data();
    return query->playedBy().second;
}

int
Session::getEndTime()
{
    Tomahawk::Query *query = m_queries.last().data();
    return query->playedBy().second +  query->duration();
}

int
Session::count()
{
    return m_queries.size();
}

QList< Tomahawk::query_ptr >
Session::getTracks()
{
    return m_queries;
}

bool
Session::operator<( Session s )
{
    return this->getEndTime() < s.getEndTime();
}

/*
template <class Session> bool
qGreater<Session>::operator()(const Session& s1, const Session& s2) const
{
    return s1->getEndTime() < s2->getEndTime();
}
*/

SessionGreatThan::SessionGreatThan()
{

}

bool
SessionGreatThan::operator ()(Session* s1, Session* s2)
{
    return s1->getEndTime() > s2->getEndTime();
}
