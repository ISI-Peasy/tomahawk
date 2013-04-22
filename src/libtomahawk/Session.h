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
    QString getPredominantArtist();
private:
    QList< Tomahawk::query_ptr > m_queries;
signals:
    
public slots:
    
};

#endif // SESSION_H
