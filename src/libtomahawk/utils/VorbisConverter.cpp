/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2013, Rémi Benoit <r3m1.benoit@gmail.com>
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

#include "VorbisConverter.h"

#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <vorbis/vorbisenc.h>

#include "Album.h"
#include "Artist.h"
#include "Query.h"
#include "Result.h"
#include "Typedefs.h"
#include "collection/Collection.h"
#include "Source.h"

#include "utils/Logger.h"
#include "utils/TomahawkUtils.h"
#include "Typedefs.h"

using namespace Tomahawk;

#define STANDARD_SAMPLE_RATE 44100
#define DEFAULT_IS_STEREO true


VorbisConverter::VorbisConverter(const Tomahawk::result_ptr& result, QObject *parent) :
    QIODevice(parent),
    m_remainingSamples(-1),
    m_buffer( new QByteArray() ),
    m_atEnd( false )
{

    m_mediaObject = new Phonon::MediaObject( this );
    m_audioOutput = new Phonon::AudioOutput( Phonon::MusicCategory, this );
    m_audioDataOutput = new Phonon::AudioDataOutput( m_mediaObject );
    m_stream =  new QBuffer(m_buffer);
    m_vorbisWriter = new VorbisWriter(this);

    Phonon::createPath( m_mediaObject, m_audioDataOutput );

    if ( result->collection() && result->collection()->source()->isLocal() )
    {
        m_mediaObject->setCurrentSource( Phonon::Mrl::fromUserInput(result->url() ));

        connect( m_audioDataOutput, SIGNAL( dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& ) ),
             this,
             SLOT( receiveData( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& ) ));
        connect( m_audioDataOutput, SIGNAL( endOfMedia(int) ), this, SLOT( onEndOfMedia(int) ) );
        connect( m_mediaObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ), SLOT( onStateChanged( Phonon::State, Phonon::State ) ) );

        m_vorbisWriter->addTag("ARTIST", result->artist()->name());
        m_vorbisWriter->addTag("ALBUM", result->album()->name());
        m_vorbisWriter->addTag("TITLE", result->track());

        tDebug() << "opening : " << m_mediaObject->currentSource().mrl().path() ;
        test = new QFile(m_mediaObject->currentSource().mrl().path() + ".ogg");
        test->open(ReadWrite);
        m_stream->open(QBuffer::ReadWrite);
        m_vorbisWriter->open( m_audioDataOutput->sampleRate(), DEFAULT_IS_STEREO , -0.9);
    }

    tDebug() << "Data size : " << m_audioDataOutput->dataSize() << " with bitrate : " << m_audioDataOutput->sampleRate();
    this->startConversion();
}


void
VorbisConverter::startConversion()
{
    tDebug() << "Playing : " << m_mediaObject->currentSource().url() << "with bitrate : " << m_audioDataOutput->sampleRate() ;

    //m_audioOutput->setVolume(0);
    m_mediaObject->play();
}


//qint64 VorbisConverter::readData(char *data, qint64 maxSize)
//{
//    int length = qMin(m_buffer.length(), (int) maxSize);
//    memcpy(data, m_buffer.constData(), length);
//    m_buffer.remove(0, length);
//    test->write(data, length);
//    return length;
//}


//qint64 VorbisConverter::writeData(const char *data, qint64 maxSize)
//{
//    //test->write(data, maxSize);
//    m_buffer.append(data, maxSize);
//    return maxSize;
//}


qint64 VorbisConverter::readData(char *data, qint64 maxSize)
{
    return m_stream->read(data, maxSize);
}


qint64 VorbisConverter::writeData(const char *data, qint64 maxSize)
{
//    char* tmp = new char[maxSize];
//    memcpy(tmp, data, maxSize);
//    test->write(tmp, maxSize);
//    return m_stream->write(tmp, maxSize);

    m_buffer->append(data, maxSize);
    return maxSize;
}


bool VorbisConverter::seek ( qint64 pos )
{
    return m_stream->seek(pos);
}


qint64 VorbisConverter::pos () const
{
    return m_stream->pos();
}


void
VorbisConverter::receiveData(const QMap<Phonon::AudioDataOutput::Channel , QVector<qint16> > &data)
{
    //tDebug() <<"Received Data from audio";

    QVector<qint16> left = data[Phonon::AudioDataOutput::LeftChannel];
    QVector<qint16> right = data[Phonon::AudioDataOutput::RightChannel];
    long sampleSize;
    bool flush = false;

    if(m_remainingSamples == -1)
        sampleSize = left.size();
    else
    {
        sampleSize = m_remainingSamples;
        flush = true;
        m_remainingSamples = -1;
    }

    if( left.isEmpty() || right.isEmpty() || left.size() != right.size() )
    {
        tDebug() << "Error with sample : [" << left.size() << "," << right.size() << "]";
    }
    else
    {
        qint64 streamSize = m_stream->size();
        int bufferSize = m_buffer->size();
        m_vorbisWriter->write(left.constData(), right.constData(), sampleSize, flush);
        if(streamSize != m_stream->size() && bufferSize != m_buffer->size())
        {
            tDebug() << "AFTER : array size : "<<m_buffer->size()<<" stream size : " <<m_stream->size()<<" pos : "<<m_stream->pos();
        }
    }
}


void
VorbisConverter::onEndOfMedia(int remainingSamples)
{
    tDebug() << "End of media, with : " << remainingSamples ;
    m_remainingSamples = remainingSamples;
}


qint64
VorbisConverter::bytesAvailable() const
{
    return m_buffer->size() + QIODevice::bytesAvailable();
}


bool
VorbisConverter::atEnd () const
{
    return m_atEnd;
}


void
VorbisConverter::onStateChanged( Phonon::State newState, Phonon::State oldState )
{
    tDebug() << "State changed : " << newState << " was " << oldState;

    if(oldState == Phonon::PlayingState && newState == Phonon::StoppedState)
    {
        m_atEnd = true;
        m_vorbisWriter->close();
        test->close();
    }

}


