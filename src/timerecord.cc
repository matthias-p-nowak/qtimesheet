#include <qtimesheet.hh>

#define DATEFORMAT "yyyy:MM:dd:HH:mm:ss"

using namespace std;

vector<TimeRecord *> records;

TimeRecord::TimeRecord(QDateTime &_start, QString &_proj):
  start(_start),project(_proj)
{
  worked=0;
  remaining=0;
  billed=0;
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
  if(parts.size()<5)
    return tr;
  tr->remaining=parts[3].toInt();
  tr->billed=parts[4].toInt();
  return tr;
}

QTextStream & operator<<(QTextStream &stream, const TimeRecord *tr){
  stream << tr->start.toString(DATEFORMAT) << " "  << tr->project 
    << " " << tr->worked << " " << tr->remaining << " " << tr->billed <<"\n";
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
  int sz=records.size()-1;
 for(int i=0; i<sz; ++i) {
      TimeRecord* tr=records[i];
    tr->worked=tr->start.secsTo(records[i+1]->start);
    for(int j=i-1; j>=0; --j) {
      TimeRecord* tr2=records[j];
      if(tr->project != tr2->project)
        continue;
      // found last
      // new remaining = last remaining + worked
      tr->remaining=tr2->remaining+tr->worked;
      // test if day is the same
      QDate d=tr->start.date();
      QDate d2=tr2->start.date();
      if(d==d2){
	tr->remaining+= 1800*tr2->billed;
	tr2->billed=0;
      }
      if(tr->remaining>0){
	tr->billed=ceil(tr->remaining/1800.0);
	tr->remaining -= tr->billed*1800;
      }
      break;
    }
  }
}
