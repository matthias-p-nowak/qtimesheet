#include <qtimesheet.hh>

#define DATEFORMAT "yyyy:MM:dd:HH:mm:ss"

using namespace std;

vector<TimeRecord *> records;

TimeRecord::TimeRecord(QDateTime &_start, QString &_proj):
  start(_start),project(_proj)
{
  
}

TimeRecord * TimeRecord::read(QString &l){
  auto parts=l.split(" ");
  auto start=QDateTime::fromString(parts[0],DATEFORMAT);
  if(parts.size()<2){
    QString n("");
    return new TimeRecord(start,n);
  }
  auto project=parts[1];
  auto tr= new TimeRecord(start,project);
  if(parts.size()<3)
    return tr;
  tr->worked=parts[2].toInt();
  return tr;
}

QTextStream & operator<<(QTextStream &stream, const TimeRecord *tr){
  stream << tr->start.toString(DATEFORMAT) << " "  << tr->project 
    << " " << tr->worked << "\n";
  return stream;
}

void writeRecords(QTextStream &ts){
  for(auto tr :records)
    ts << tr;
}

int readRecords(QTextStream &ts){
  while(! ts.atEnd()){
    auto line=ts.readLine();
    if(!line.isEmpty()){
      auto tr=TimeRecord::read(line);
      records.push_back(tr);
    }
  }
}

void recalculate(){
 for(int i=0; i<records.size()-1; ++i) {
    records[i]->worked=records[i]->start.secsTo(records[i+1]->start);
    for(int j=i-1; j>=0; --j) {
      auto tr=records[i];
      auto tr2=records[j];
      if(tr->project != tr2->project)
        continue;
      // found last
      break;
    }
  }
}
