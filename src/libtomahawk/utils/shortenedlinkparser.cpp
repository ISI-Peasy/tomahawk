/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Leo Franchi <lfranchi@kde.org>
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

#include "shortenedlinkparser.h"

#include "utils/logger.h"
#include "utils/tomahawkutils.h"
#include "query.h"

#include <qjson/parser.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

using namespace Tomahawk;

ShortenedLinkParser::ShortenedLinkParser ( const QStringList& urls, QObject* parent )
    : QObject( parent )
{
    foreach ( const QString& url, urls )
        lengthenUrl( url );
}

ShortenedLinkParser::~ShortenedLinkParser() {}

void
ShortenedLinkParser::lengthenUrl( const QString& url )
{
    // Whitelisted links
    if ( !( url.contains( "t.co" ) ||
            url.contains( "bit.ly" ) ||
            url.contains( "j.mp" ) ||
            url.contains( "rd.io" ) ) )
        return;

    tDebug() << "Looking up..." << url;

    QNetworkReply* reply = TomahawkUtils::nam()->get( QNetworkRequest( QUrl( url ) ) );
    connect( reply, SIGNAL( finished() ), this, SLOT( lookupFinished() ) );

    m_queries.insert( reply );
}

void
ShortenedLinkParser::lookupFinished()
{
    QNetworkReply* r = qobject_cast< QNetworkReply* >( sender() );
    Q_ASSERT( r );

    QVariant redir = r->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( redir.isValid() && !redir.toUrl().isEmpty() )
    {
        tLog() << "Got a redirected url:" << redir.toUrl().toString();
        m_links << redir.toUrl().toString();
    }

    r->deleteLater();

    m_queries.remove( r );
    checkFinished();
}


void
ShortenedLinkParser::checkFinished()
{
    if ( m_queries.isEmpty() ) // we're done
    {
        qDebug() << "DONE and found redirected urls:" << m_links;
        emit urls( m_links );

        deleteLater();
    }

}
