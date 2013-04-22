#include "Session.h"

Session::Session()
{
    m_queries = QList< Tomahawk::query_ptr >();
}

//Session* Session::operator<<( Tomahawk::query_ptr query )
//{
//    m_queries << query;
//    return this;
//}

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
