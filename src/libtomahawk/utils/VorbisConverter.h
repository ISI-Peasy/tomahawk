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
#include <QBuffer>
#include <QDataStream>

#include <phonon/AudioDataOutput>
#include <phonon/AudioOutput>
#include <phonon/MediaObject>

#include "utils/TomahawkUtils.h"
#include "utils/vorbiswriter/VorbisWriter.h"

class QFile;
class QByteArray;

class VorbisConverter : public QIODevice
{
    Q_OBJECT

public:
    VorbisConverter( const Tomahawk::result_ptr& result, QObject* parent = 0 );
    //virtual ~VorbisConverter() {}

    void startConversion();

    qint64 readData ( char * data, qint64 maxSize );
    qint64 writeData ( const char * data, qint64 maxSize );

    qint64 bytesAvailable() const;
    bool atEnd() const;

    QIODevice* getStream() { return this; }
    bool seek(qint64 pos);
    qint64 pos() const;
public slots:
    void receiveData( const QMap< Phonon::AudioDataOutput::Channel, QVector< qint16 > > &  	data );
    void onEndOfMedia(int remainingSamples);
    void onStateChanged( Phonon::State, Phonon::State );

private:
    Phonon::MediaObject* m_mediaObject;
    Phonon::AudioOutput* m_audioOutput;
    Phonon::AudioDataOutput* m_audioDataOutput;

    VorbisWriter* m_vorbisWriter;

    QByteArray* m_buffer;
    bool m_atEnd;
    int m_remainingSamples;

    QBuffer* m_stream;

    QFile* test;
};

#endif // VORBISCONVERTER_H
