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
    explicit Session(const Session& copy);
    virtual ~Session();

    //Session* operator<<( Tomahawk::query_ptr query );
    bool operator<(const Session s);
    void addQuery( Tomahawk::query_ptr q );

    QString getSessionOwner();
    QString getPredominantArtist();
    QString getPredominantAlbum();
    QPixmap getCover(QSize & size);
    Tomahawk::source_ptr getSessionSource();
    QList< Tomahawk::query_ptr > getTracks() const;

    int getStartTime();
    int getEndTime();
    int count();

private:
    QList< Tomahawk::query_ptr > m_queries;
signals:
    
public slots:
    
};

Q_DECLARE_METATYPE ( Session* ) ;
Q_DECLARE_METATYPE ( Session ) ;

class SessionGreatThan
{
public:
    explicit SessionGreatThan();
    bool operator()(Session *s1, Session *s2 );

private:

};
#endif // SESSION_H
