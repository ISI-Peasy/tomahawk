#ifndef SESSION_H
#define SESSION_H

#include <QtCore/QObject>

#include "Query.h"
#include "utils/TomahawkUtils.h"


class Session : public QObject
{
    Q_OBJECT
public:
    explicit Session();
    //Session* operator<<( Tomahawk::query_ptr query );
    void addQuery( Tomahawk::query_ptr q );
    QString getSessionOwner();
    QString getPredominantArtist();
    QString getPredominantAlbum();
    int getStartTime();
    int getEndTime();
    bool operator<(const Session s);
private:
    QList< Tomahawk::query_ptr > m_queries;
signals:
    
public slots:
    
};

class SessionGreatThan
{
public:
    explicit SessionGreatThan();
    bool operator()(Session *s1, Session *s2 );

private:

};
#endif // SESSION_H
