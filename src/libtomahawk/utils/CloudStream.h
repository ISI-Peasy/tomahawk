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

//#ifndef GOOGLEDRIVESTREAM_H
//#define GOOGLEDRIVESTREAM_H

#include <QObject>
#include <QList>
#include <QSslError>
#include <QUrl>


#include <google/sparsetable>
#include <taglib/tiostream.h>

class QNetworkAccessManager;
class QtScriptResolver;

class CloudStream : public QObject, public TagLib::IOStream
{
    Q_OBJECT
public:
    CloudStream( QUrl& url,
                 const QString& filename,
                 const QString& fileId,
                 const long length,
                 QVariantMap& headers,
                 QtScriptResolver *scriptResolver,
                 const QString& javascriptRefreshUrlFunction,
                 const bool refreshUrlEachTime );

    //Taglib::IOStream;
    virtual TagLib::FileName name() const;
    virtual TagLib::ByteVector readBlock( ulong length );
    virtual void writeBlock( const TagLib::ByteVector& );
    virtual void insert( const TagLib::ByteVector&, ulong, ulong );
    virtual void removeBlock( ulong, ulong );
    virtual bool readOnly() const;
    virtual bool isOpen() const;
    virtual void seek( long offset, TagLib::IOStream::Position p );
    virtual void clear();
    virtual long tell() const;
    virtual long length();
    virtual void truncate( long );

    google::sparsetable<char>::size_type cached_bytes() const
    {
        return m_cache.num_nonempty();
    }

    int num_requests() const
    {
        return m_num_requests;
    }

    int num_requests_in_error() const
    {
        return m_num_requests_in_error;
    }

    // Use educated guess to request the bytes that TagLib will probably want.
    void Precache();

    bool refreshStreamUrl();

private:
    bool CheckCache( int start, int end );
    void FillCache( int start, TagLib::ByteVector data );
    TagLib::ByteVector GetCached( int start, int end );


private slots:
    void SSLErrors( const QList<QSslError>& errors );

private:
    QUrl m_url;
    const QString m_filename;
    const QString m_fileId;
    const QByteArray m_encoded_filename;
    const ulong m_length;
    QVariantMap m_headers;
    const QString m_javascriptRefreshUrlFunction;
    const bool m_refreshUrlEachTime;

    int m_cursor;
    QNetworkAccessManager* m_network;
    QtScriptResolver* m_scriptResolver;

    google::sparsetable<char> m_cache;
    int m_num_requests;
    int m_num_requests_in_error;
};

//#endif // GOOGLEDRIVESTREAM_H
