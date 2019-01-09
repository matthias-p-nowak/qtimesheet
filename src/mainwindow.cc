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

  auto fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction("&Open",this,&MainWindow::openFile,QKeySequence("Ctrl+O"));
  fileMenu->addSeparator();
  fileMenu->addAction("&Quit",this,&MainWindow::closeMain,QKeySequence("Ctrl+Q"));

  auto projMenu = menuBar()->addMenu("&Projects");
  projMenu->addAction("&New one",this,&MainWindow::newProject);
  projMenu->addSeparator();

  if(QSystemTrayIcon::isSystemTrayAvailable()) {
    fromHere("setting tray icon");
    trayIcon=new QSystemTrayIcon(icon,this);
    trayIcon->show();
    connect(trayIcon,&QSystemTrayIcon::activated,this,&MainWindow::sysTrayMenuActivated);
  }
  connect(&_rec,&QThread::finished,this,&MainWindow::updateResults);
  readRecordsFromFile();
  _rec.start();
}

MainWindow::~MainWindow() {
  fromHere("gone");
}

void MainWindow::sysTrayMenuActivated(QSystemTrayIcon::ActivationReason reason) {
  show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // ----- saving for restore -----
  QSettings settings(this);
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState",saveState());
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

void MainWindow::updateResults(){
  fromHere("writing to files");
  writeRecordsToFile();
}

