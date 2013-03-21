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

#include "Query.h"
#include "Result.h"
#include "Typedefs.h"
#include "collection/Collection.h"
#include "Source.h"

#include "utils/Logger.h"
#include "utils/TomahawkUtils.h"

using namespace Tomahawk;


VorbisConverter::VorbisConverter(const Tomahawk::query_ptr& query, QObject *parent) : QObject(parent)
{
    if ( ! query->numResults() )
    {
        tDebug() << "No results to convert";
        return;
    }

    m_mediaObject = new Phonon::MediaObject( this );
    m_audioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
    m_audioDataOutput = new Phonon::AudioDataOutput( m_mediaObject );

    Phonon::createPath( m_mediaObject, m_audioDataOutput );
    Phonon::createPath( m_audioDataOutput, m_audioOutput );

//    Phonon::createPath( m_mediaObject, m_audioOutput );
//    Phonon::createPath( m_mediaObject, m_audioDataOutput );



    Tomahawk::result_ptr result = query->results().first();

    if ( result->collection() && result->collection()->source()->isLocal() )
    {
        m_mediaObject->setCurrentSource( Phonon::Mrl::fromUserInput(result->url() ));

        connect(m_audioDataOutput,
             SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >&)),
             this,
             SLOT(receiveData(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >&)));
        connect( m_audioDataOutput, SIGNAL( endOfMedia(int) ), this, SLOT( onEndOfMedia(int) ) );
    }
    showDataSize();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showDataSize()));
    timer->start(1000);
}


VorbisConverter::VorbisConverter(Phonon::AudioDataOutput* ao, QObject *parent) : QObject(parent)
{
    m_audioDataOutput = ao;

    connect(m_audioDataOutput,
         SIGNAL(dataReady(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >&)),
         this,
         SLOT(receiveData(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >&)));
}


void
VorbisConverter::startConversion()
{
    tDebug() << "Playing : " << m_mediaObject->currentSource().url() << "with bitrate : " << m_audioDataOutput->sampleRate() ;
    m_mediaObject->play();
}


void
VorbisConverter::receiveData(const QMap<Phonon::AudioDataOutput::Channel , QVector<qint16> > &data)
{
    tDebug() <<"Received Data from audio";
    tDebug() << data;
}


void VorbisConverter::onEndOfMedia(int remainingSamples)
{
    tDebug() << "End of media, with : " << remainingSamples ;
}


void VorbisConverter::showDataSize()
{
    tDebug() << "Data size : " << m_audioDataOutput->dataSize() << " with bitrate : " << m_audioDataOutput->sampleRate();
}

