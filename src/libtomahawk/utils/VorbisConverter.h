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

#ifndef VORBISCONVERTER_H
#define VORBISCONVERTER_H

#include <QMap>
#include <QObject>

#include <phonon/AudioDataOutput>
#include <phonon/AudioOutput>
#include <phonon/MediaObject>

#include "utils/TomahawkUtils.h"

class QFile;

class VorbisConverter : public QObject
{
    Q_OBJECT

public:
    VorbisConverter( const Tomahawk::query_ptr& query, QObject* parent = 0 );
    VorbisConverter( Phonon::AudioDataOutput*, QObject* parent = 0 );
    virtual ~VorbisConverter() {}

    void startConversion();

public slots:
    void receiveData( const QMap< Phonon::AudioDataOutput::Channel, QVector< qint16 > > &  	data );
    void onEndOfMedia(int remainingSamples);
    void showDataSize();

private:
    Phonon::MediaObject* m_mediaObject;
    Phonon::AudioOutput* m_audioOutput;
    Phonon::AudioDataOutput* m_audioDataOutput;
};

#endif // VORBISCONVERTER_H
