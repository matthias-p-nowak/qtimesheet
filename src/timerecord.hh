
#include <QDateTime>
#include <QString>
#include <vector>
#include <QTextStream>

class TimeRecord {
public:
  TimeRecord(QDateTime &start, QString &project);
  QDateTime start;
  QString project;
  int worked;
  static TimeRecord * read(QString &line);

};

extern std::vector<TimeRecord *> records;

QTextStream & operator<<(QTextStream &stream, const TimeRecord *tr);
int readRecords(QTextStream &ts);
void writeRecords(QTextStream &ts);
void recalculate();
