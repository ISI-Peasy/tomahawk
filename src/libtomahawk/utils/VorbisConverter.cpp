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

#include <QFile>
#include <vorbis/vorbisenc.h>

#include "Query.h"
#include "Result.h"
#include "Typedefs.h"
#include "collection/Collection.h"
#include "Source.h"

#include "utils/Logger.h"
#include "utils/TomahawkUtils.h"

using namespace Tomahawk;

#define STANDARD_SAMPLE_RATE 44100
#define DEFAULT_IS_STEREO true


VorbisConverter::VorbisConverter(const Tomahawk::query_ptr& query, QObject *parent) : QObject(parent), m_remainingSamples(-1)
{
    if ( ! query->numResults() )
    {
        tDebug() << "No results to convert";
        return;
    }

    m_mediaObject = new Phonon::MediaObject( this );
    m_audioOutput = new Phonon::AudioOutput( Phonon::MusicCategory, this );
    m_audioDataOutput = new Phonon::AudioDataOutput( m_mediaObject );
    m_vorbisWriter = new VorbisWriter();

    Phonon::createPath( m_mediaObject, m_audioOutput );
    Phonon::createPath( m_mediaObject, m_audioDataOutput );

    Tomahawk::result_ptr result = query->results().first();

    if ( result->collection() && result->collection()->source()->isLocal() )
    {
        m_mediaObject->setCurrentSource( Phonon::Mrl::fromUserInput(result->url() ));

        connect( m_audioDataOutput, SIGNAL( dataReady( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& ) ),
             this,
             SLOT( receiveData( const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& ) ));
        connect( m_audioDataOutput, SIGNAL( endOfMedia(int) ), this, SLOT( onEndOfMedia(int) ) );
        connect( m_mediaObject, SIGNAL( stateChanged( Phonon::State, Phonon::State ) ), SLOT( onStateChanged( Phonon::State, Phonon::State ) ) );

        tDebug() << "Creating new file : " << m_vorbisWriter->open(m_mediaObject->currentSource().mrl().path() + ".ogg", m_audioDataOutput->sampleRate(), DEFAULT_IS_STEREO);
    }

    tDebug() << "Data size : " << m_audioDataOutput->dataSize() << " with bitrate : " << m_audioDataOutput->sampleRate();
}


VorbisConverter::VorbisConverter(Phonon::AudioDataOutput* ao, QObject *parent) : QObject(parent)
{
    m_audioDataOutput = ao;

    connect(m_audioDataOutput,
         SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >&)),
         this,
         SLOT(receiveData(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >&)));
    connect( m_audioDataOutput, SIGNAL( endOfMedia(int) ), this, SLOT( onEndOfMedia(int) ) );

}


void
VorbisConverter::startConversion()
{
    tDebug() << "Playing : " << m_mediaObject->currentSource().url() << "with bitrate : " << m_audioDataOutput->sampleRate() ;

    //m_audioOutput->setVolume(0);
    m_mediaObject->play();
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
        m_vorbisWriter->write(left.constData(), right.constData(), sampleSize, flush);
    }
}


void
VorbisConverter::onEndOfMedia(int remainingSamples)
{
    tDebug() << "End of media, with : " << remainingSamples ;
    m_remainingSamples = remainingSamples;
}


void
VorbisConverter::onStateChanged( Phonon::State newState, Phonon::State oldState )
{
    tDebug() << "State changed : " << newState << " was " << oldState;

    if(oldState == Phonon::PlayingState && newState == Phonon::StoppedState)
    {
        m_vorbisWriter->close();
    }

}


