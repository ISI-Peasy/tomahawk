#ifndef SESSION_H
#define SESSION_H

#include <QtCore/QObject>

#include "Query.h"
#include "Track.h"
#include "utils/TomahawkUtils.h"


class Session : public QObject
{
Q_OBJECT
public:
    explicit Session();
    explicit Session(const Session& copy);
    virtual ~Session();

    bool operator<(const Session s);
    void addQuery( QPair<Tomahawk::track_ptr , Tomahawk::PlaybackLog>& track ) ;

    QString getSessionOwner();
    QString getPredominantArtist();
    QString getPredominantAlbum();
    Tomahawk::source_ptr getSessionSource();

    QList < QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> >getTracks() const;

    int getStartTime();
    int getEndTime();
    int count();

private:
    QList < QPair <Tomahawk::track_ptr , Tomahawk::PlaybackLog> > m_tracks ;
    QList< Tomahawk::query_ptr > m_queries;
signals:
    
public slots:
    
};

Q_DECLARE_METATYPE ( Session* )
Q_DECLARE_METATYPE ( Session )

class SessionGreatThan
{
public:
    explicit SessionGreatThan();
    bool operator()(Session *s1, Session *s2 );

private:

};
#endif // SESSION_H
