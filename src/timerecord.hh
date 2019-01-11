#ifndef INC_yXHsmviJddi
#define INC_yXHsmviJddi

#include <QDateTime>
#include <QDate>
#include <QString>
#include <vector>
#include <QTextStream>
#include <math.h>
#include <QMap>

class TimeRecord {
public:
  TimeRecord(QDateTime &start, QString &project);
  QDateTime start;
  QString project;
  int worked; // in accounted seconds
  int remaining; // in seconds
  int billed; // half hours billed
  static TimeRecord * read(QString &line);
};

class TimeOverview {
  public:
  TimeOverview();
  int weekNumber;
  float hours[7];
};

extern QMap<int,QMap<QString,TimeOverview>> timeOverview;
extern QList<int> weekNumbers;

const float scale_up=16.0/15.0;  // 8 hours for 7.5 actual work

extern std::vector<TimeRecord *> records;

QTextStream & operator<<(QTextStream &stream, const TimeRecord *tr);
int readRecords(QTextStream &ts);
void writeRecords(QTextStream &ts);
void recalculate();

#endif
