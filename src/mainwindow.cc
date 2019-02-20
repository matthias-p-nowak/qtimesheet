#include <qtimesheet.hh>

MainWindow *mw=NULL;

class Recalculate: public QThread {
public:
  void run() {
    fromHere("running thread");
    recalculate();
  };
} _rec;

void MainWindow::create() {
  fromHere("creating mainwindow");
  if(!mw) {
    mw=new MainWindow();
  }
  mw->show();
}

MainWindow::MainWindow() {
  setMinimumSize(300,200); // just in case
  // restoring from previous runs
  QSettings settings(this);
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  QTimer::singleShot(0, this, &MainWindow::initialize);
}

void MainWindow::initialize() {
  fromHere("initializing");
  QIcon icon(":/Sandwatch.png");
  setWindowIcon(icon);

  tree=new QTreeWidget(this);
  setCentralWidget(tree);
  tree->setColumnCount(8);
  QStringList sl;
  sl << "week/project"<<"Man"<<"Tue"<<"Wed"<<"Thu"<<"Fri"<<"Sat"<<"Sun";
  tree->setHeaderLabels(sl);

  auto fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("&Open",this,&MainWindow::openFile,QKeySequence("Ctrl+O"));
  fileMenu->addSeparator();
  fileMenu->addAction("&Quit",this,&MainWindow::closeMain,QKeySequence("Ctrl+Q"));

  projMenu = menuBar()->addMenu("&Projects");


  if(QSystemTrayIcon::isSystemTrayAvailable()) {
    fromHere("setting tray icon");
    trayIcon=new QSystemTrayIcon(icon,this);
    trayIcon->show();
    auto cm=new QMenu(this);
    trayIcon->setContextMenu(cm);
    connect(trayIcon,&QSystemTrayIcon::activated,this,&MainWindow::sysTrayMenuActivated);
  }
  connect(&_rec,&QThread::finished,this,&MainWindow::updateResults);
  readRecordsFromFile();
  _rec.start();
}

MainWindow::~MainWindow() {
  fromHere("gone");
  if(trayIcon) {
    trayIcon->hide();
  }
}

void MainWindow::sysTrayMenuActivated(QSystemTrayIcon::ActivationReason reason) {
  switch(reason) {
  case QSystemTrayIcon::Trigger:
    show();
    break;
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // ----- saving for restore -----
  QSettings settings(this);
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState",saveState());
   if(trayIcon) {
    trayIcon->hide();
    trayIcon->deleteLater();
  }
  QMainWindow::closeEvent(event);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if(event->matches(QKeySequence::Cancel)) {
    hide();
    return;
  }
  QMainWindow::keyPressEvent(event);
}

// actions can't call closeEvent
void MainWindow::closeMain(bool _ignored) {
  fromHere("closing all windows");
  if(trayIcon) {
    trayIcon->hide();
  }
  qApp->quit();
}

void MainWindow::openFile(bool _ignored) {
  QSettings settings(this);
  QString fileName = settings.value("file").toString();
  fileName = QFileDialog::getSaveFileName(this,"Record file name",fileName);
  if(!fileName.isEmpty()) {
    settings.setValue("file",fileName);
    qDebug()<<"saving to file "<<fileName;
  }
}

void MainWindow::newProject(bool _ignored) {
  bool ok;
  QString name=QInputDialog::getText(this,"Add a new project name","name:",QLineEdit::Normal,"",&ok);
  if(ok) {
    qDebug()<<"adding " <<name;
    auto now = QDateTime::currentDateTime();
    auto tr=new TimeRecord(now,name);
    records.push_back(tr);
    _rec.start();
  }
}

void MainWindow::readRecordsFromFile() {
  for(auto r:records)
    delete r;
  records.clear();
  QSettings settings(this);
  auto fn=settings.value("file").toString();
  // qDebug() << "record file "<<fn;
  QFile f(fn);
  f.open(QIODevice::ReadOnly | QIODevice::Text);
  QTextStream ts(&f);
  auto r=readRecords(ts);
  qDebug()<< QString("read %1 records from %2").arg(r).arg(fn);
}

void MainWindow::writeRecordsToFile() {
  QSettings settings(this);
  auto fn=settings.value("file").toString();
  qDebug() << "record file "<<fn;
  QFile f(fn);
  f.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream ts(&f);
  writeRecords(ts);
  ts.flush();
}

void MainWindow::updateProjects() {
  projectNames.clear();
  QSet<QString> gotSoFar;
  projMenu->clear();
  projMenu->addAction("&New one",this,&MainWindow::newProject);
  projMenu->addSeparator();
  auto cm=trayIcon->contextMenu();
  cm->clear();
  for(auto r: records) {
    if(!gotSoFar.contains(r->project)) {
      gotSoFar.insert(r->project);
      projectNames << r->project;
      projMenu->addAction(r->project,this, &MainWindow::nextProject);
      cm->addAction(r->project,this, &MainWindow::nextProject);
    }
  }
}

void MainWindow::nextProject(bool _ignored) {
  QAction *pA=qobject_cast<QAction *>(sender());
  auto now = QDateTime::currentDateTime();
  auto name=pA->text();
  auto tr=new TimeRecord(now, name);
  auto title=QString("%1 [%2]").arg(qApp->applicationName()).arg(name);
  setWindowTitle(title);
  records.push_back(tr);
  _rec.start();
}

void MainWindow::startUpdate() {
  _rec.start();
}

void MainWindow::updateResults() {
  fromHere("writing to files");
  writeRecordsToFile();
  updateProjects();
  tree->clear();
  int w=0;
  for(auto wi=weekNumbers.rbegin(); wi!=weekNumbers.rend(); ++wi) {
    int wn=*wi;
    if(++w>6)
      break;
    QStringList sl;
    sl <<QString::number(wn);
    auto ti=new QTreeWidgetItem(sl);
    tree->addTopLevelItem(ti);
    auto entries=timeOverview[wn];
    TimeOverview weekSum;
    float ws=0;
    for(QString &prj: entries.keys()) {
      if(prj.isEmpty())
        continue;
      TimeOverview &ov=entries[prj];
      sl.clear();
      sl << prj;
      for(int i=0; i<7; ++i) {
        if(ov.hours[i]>0)
          sl<< QString::number(ov.hours[i]);
        else
          sl << "";
        weekSum.hours[i]+=ov.hours[i];
        ws+=ov.hours[i];
      }
      auto tr=new QTreeWidgetItem(ti,sl);
    }
    sl.clear();
    sl << QString("sum: %1").arg(ws);
    for(int i=0; i<7; ++i) {
      sl << QString::number(weekSum.hours[i]);
    }
    auto tr=new QTreeWidgetItem(ti,sl);
  }
  auto ti=tree->topLevelItem(0);
  if(ti)
    ti->setExpanded(true);
  for(int i=0; i<8; ++i)
    tree->resizeColumnToContents(i);
  if(!records.empty()) {
    auto lastRecord=records.back();
    auto name=lastRecord->project;
    auto wn=lastRecord->start.date().weekNumber();
    bool week[7];
    for(int i=0; i<7; ++i)
      week[i]=false;
    float billed=0;
    for(auto ri=records.rbegin(); ri!=records.rend(); ++ri) {
      auto rec=*ri;
      if(rec->project.isEmpty())
        continue;
      auto day=rec->start.date();
      if(wn!=day.weekNumber())
        break;
      auto d=day.dayOfWeek()-1;
      week[d]=true;
      billed+= rec->billed;
    }
    float days=0;
    for(auto w: week)
      if(w) days+=1.0;
    float todo= days*16.0 - billed; // half hours
    qDebug()<< "must work" << todo;
    todo= todo*1800 - lastRecord->remaining;
    if(todo > 0) {
      todo /= scale_up;
      auto now = QDateTime::currentDateTime();
      auto stop=now.addSecs(todo);
      auto stopTime=stop.time().toString("HH:mm:ss");
      qDebug()<<"stop working at"<<stopTime;
      if(trayIcon) {
        auto msg=QString("go home at %1").arg(stopTime);
        trayIcon->showMessage("daily work",msg);
      }
      auto title=QString("%1 [%2] %3").arg(qApp->applicationName()).arg(name).arg(stopTime);
      setWindowTitle(title);
      todo= - lastRecord->remaining;
      if(todo>0) {
        todo /= scale_up;
        todo+=2;
        QTimer::singleShot(todo*1000, this, &MainWindow::startUpdate);
      }
    } else {
      auto title=QString("%1 [%2] go home...").arg(qApp->applicationName()).arg(name);
      setWindowTitle(title);
      if(trayIcon) {
        trayIcon->showMessage("QTimesheet official statement","go home, you have worked enough");
      }
      todo=-lastRecord->remaining;
      todo+=2;
      if(todo>0)
        QTimer::singleShot(todo*1000, this, &MainWindow::startUpdate);
    }
  }
}

