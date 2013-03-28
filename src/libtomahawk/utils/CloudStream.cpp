/* This file is part of Clementine.
   Copyright 2012, David Sansome <me@davidsansome.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CloudStream.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPair>
#include <QStringList>

#include <taglib/id3v2framefactory.h>
#include <taglib/mpegfile.h>

#include "utils/Logger.h"

#include "resolvers/QtScriptResolver.h"

namespace
{
    static const int kTaglibPrefixCacheBytes = 64 * 1024;  // Should be enough.
    static const int kTaglibSuffixCacheBytes = 8 * 1024;
}

const static int MAX_ALLOW_ERROR_QUERY = 2;

CloudStream::CloudStream( QUrl& url,
                          const QString& filename,
                          const QString& fileId,
                          const long length,
                          QVariantMap& headers,
                          QNetworkAccessManager* network,
                          QtScriptResolver* scriptResolver,
                          const QString & javascriptRefreshUrlFunction,
                          const bool refreshUrlEachTime )
    : m_url( url )
    , m_filename( filename )
    , m_fileId( fileId )
    , m_encoded_filename( m_filename.toUtf8() )
    , m_length( length )
    , m_headers( headers )
    , m_cursor( 0 )
    , m_network( network )
    , m_cache( length )
    , m_num_requests( 0 )
    , m_num_requests_in_error( 0 )
    , m_scriptResolver( scriptResolver )
    , m_javascriptRefreshUrlFunction( javascriptRefreshUrlFunction )
    , m_refreshUrlEachTime( refreshUrlEachTime )
{
    tDebug( LOGINFO ) << "#### Cloudstream : CloudStream object created for " << m_filename << " : "
                      << m_url.toString();
}

TagLib::FileName
CloudStream::name() const
{
    return m_encoded_filename.data();
}

bool
CloudStream::CheckCache( int start, int end )
{
    for ( int i = start; i <= end; ++i ) {
        if ( !m_cache.test( i ) ) {
            return false;
        }
    }
    return true;
}

void
CloudStream::FillCache( int start, TagLib::ByteVector data )
{
    for ( int i = 0; i < data.size(); ++i )
    {
        m_cache.set( start + i, data[i] );
    }
}

TagLib::ByteVector
CloudStream::GetCached( int start, int end )
{
    const uint size = end - start + 1;
    TagLib::ByteVector ret( size );
    for ( int i = 0; i < size; ++i )
    {
        ret[i] = m_cache.get( start + i );
    }
    return ret;
}

void
CloudStream::Precache()
{
    // For reading the tags of an MP3, TagLib tends to request:
    // 1. The first 1024 bytes
    // 2. Somewhere between the first 2KB and first 60KB
    // 3. The last KB or two.
    // 4. Somewhere in the first 64KB again
    //
    // OGG Vorbis may read the last 4KB.
    //
    // So, if we precache the first 64KB and the last 8KB we should be sorted :-)
    // Ideally, we would use bytes=0-655364,-8096 but Google Drive does not seem
    // to support multipart byte ranges yet so we have to make do with two
    // requests.
    tDebug( LOGINFO ) << "#### CloudStream : Precaching from :" << m_filename;
    seek( 0, TagLib::IOStream::Beginning );
    readBlock( kTaglibPrefixCacheBytes );
    seek( kTaglibSuffixCacheBytes, TagLib::IOStream::End );
    readBlock( kTaglibSuffixCacheBytes );
    clear();
    tDebug( LOGINFO ) << "#### CloudStream : Precaching end for :" << m_filename;
}

TagLib::ByteVector
CloudStream::readBlock( ulong length )
{

    const uint start = m_cursor;
    const uint end = qMin( m_cursor + length - 1, m_length - 1 );

    tDebug( LOGINFO ) << "#### CloudStream : parsing from " << m_url.toString();
    tDebug( LOGINFO ) << "#### CloudStream : parsing from (encoded) " << m_url.toEncoded().constData();
    if ( end < start )
    {
        return TagLib::ByteVector();
    }

    if ( CheckCache( start, end ) )
    {
        TagLib::ByteVector cached = GetCached( start, end );
        m_cursor += cached.size();
        return cached;
    }

    if ( m_num_requests_in_error > MAX_ALLOW_ERROR_QUERY )
    {
        return TagLib::ByteVector();
    }

    if ( m_refreshUrlEachTime )
    {
        if( !refreshStreamUrl() )
        {
            tDebug( LOGINFO ) << "#### CloudStream : cannot refresh streamUrl for " << m_filename;
        }
    }

    QNetworkRequest request = QNetworkRequest( m_url );

    //setings of specials OAuth (1 or 2) headers
    foreach ( const QString& headerName, m_headers.keys() )
    {
        request.setRawHeader( headerName.toLocal8Bit(), m_headers[headerName].toString().toLocal8Bit() );

    }

    request.setRawHeader( "Range", QString( "bytes=%1-%2" ).arg( start ).arg( end ).toUtf8() );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
    // The Ubuntu One server applies the byte range to the gzipped data, rather
    // than the raw data so we must disable compression.
    if ( m_url.host() == "files.one.ubuntu.com" )
    {
        request.setRawHeader( "Accept-Encoding", "identity" );
    }

    //tDebug() << request.rawHeader("Authorization");
    tDebug() << "######## CloudStream : HTTP request : ";
    foreach ( const QByteArray& header, request.rawHeaderList() )
    {
        tDebug() << "#### CloudStream : header request : " << header << " = " << request.rawHeader(header);
    }

    QNetworkReply* reply = m_network->get( request );
    connect( reply, SIGNAL( sslErrors( QList<QSslError> ) ), SLOT( SSLErrors( QList<QSslError> ) ) );
    ++m_num_requests;

    QEventLoop loop;
    QObject::connect( reply, SIGNAL( finished() ), &loop, SLOT( quit() ) );
    loop.exec();
    reply->deleteLater();

    int code = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    tDebug() << "######### CloudStream : HTTP reply : #########";
    tDebug( LOGINFO ) << "#### Cloudstream : HttpStatusCode : " << code;
    foreach ( const QNetworkReply::RawHeaderPair& pair, reply->rawHeaderPairs() )
    {
        tDebug( LOGINFO ) << "#### Cloudstream : header reply " << pair;
    }

    QByteArray data = reply->readAll();

    if ( code != 206 )
    {
        m_num_requests_in_error++;
        tDebug( LOGINFO ) << "#### Cloudstream : Error " << code << " retrieving url to tag for " << m_filename;
        tDebug() << "#### CloudStream : body response : " << data;

        if ( refreshStreamUrl() )
        {
            TagLib::ByteVector bytes = readBlock( length );
            //if we have datas, let another chance to parse ID3Tags (especially for Dropbox)
            if( bytes.size() > 0 ){
                tDebug( LOGINFO ) << "#### Cloudstream : we have datas response with the refreshUrl !";
                m_num_requests_in_error--;
            }
            return bytes;
        }
        else
        {
            return TagLib::ByteVector();
        }
    }


    TagLib::ByteVector bytes( data.data(), data.size() );
    m_cursor += data.size();

    FillCache( start, bytes );
    return bytes;
}

bool
CloudStream::refreshStreamUrl()
{
    if ( m_javascriptRefreshUrlFunction.isEmpty() )
    {
        return false;
    }
    tDebug( LOGINFO ) << "####### Cloudstream : refreshing streamUrl for " << m_filename;
    QString refreshUrl = QString( "resolver.%1( \"%2\" );" ).arg( m_javascriptRefreshUrlFunction )
            .arg( m_fileId );
    tDebug( LOGINFO ) << "####### Cloudstream : refresh request : " << refreshUrl;
    QVariant response = m_scriptResolver->executeJavascript( refreshUrl );

    if ( response.isNull() )
    {
        tDebug( LOGINFO ) << "####### Cloudstream : refreshUrl response is empty, returning";
        return false;
    }

    QVariantMap request;
    QString urlString;

    if ( response.type() == QVariant::Map )
    {
        request = response.toMap();

        urlString = request["url"].toString();

        m_headers = request["headers"].toMap();
    }
    else
    {
        urlString = response.toString();
    }

    m_url.setUrl( urlString );
    tDebug( LOGINFO ) << "####### Cloudstream : streamUrl refreshed for " << m_filename;
    return true;
}

void
CloudStream::writeBlock( const TagLib::ByteVector& )
{
    tDebug( LOGINFO ) << "writeBlock not implemented";
}

void
CloudStream::insert( const TagLib::ByteVector&, ulong, ulong )
{
    tDebug( LOGINFO ) << "insert not implemented";
}

void
CloudStream::removeBlock( ulong, ulong )
{
    tDebug( LOGINFO ) << "removeBlock not implemented";
}

bool
CloudStream::readOnly() const
{
    tDebug( LOGINFO ) << "readOnly not implemented";
    return true;
}

bool
CloudStream::isOpen() const
{
    return true;
}

void
CloudStream::seek( long offset, TagLib::IOStream::Position p )
{
    switch ( p )
    {
    case TagLib::IOStream::Beginning:
        m_cursor = offset;
        break;

    case TagLib::IOStream::Current:
        m_cursor = qMin( ulong( m_cursor + offset ), m_length );
        break;

    case TagLib::IOStream::End:
        // This should really not have qAbs(), but OGG reading needs it.
        m_cursor = qMax( 0UL, m_length - qAbs( offset ) );
        break;
    }
}

void
CloudStream::clear()
{
    m_cursor = 0;
}

long
CloudStream::tell() const
{
    return m_cursor;
}

long
CloudStream::length()
{
    return m_length;
}

void
CloudStream::truncate( long )
{
    tDebug( LOGINFO ) << "not implemented";
}

void
CloudStream::SSLErrors( const QList<QSslError>& errors )
{
    foreach ( const QSslError& error, errors )
    {
        tDebug( LOGINFO ) << "#### Cloudstream : Error for " << m_filename << " : ";
        tDebug( LOGINFO ) << error.error() << error.errorString();
        tDebug( LOGINFO ) << error.certificate();
    }
}
