#ifndef INC_6XBESwi7bDi
#define INC_6XBESwi7bDi

#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QKeyEvent>
#include <QMenuBar>
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QSet>


class MainWindow: public QMainWindow {

private:
  MainWindow();
  QSystemTrayIcon *trayIcon;
  void readRecordsFromFile();
  void writeRecordsToFile();
  QStringList projectNames;
  void updateProjects();
  QMenu *projMenu;
  
public:
  ~MainWindow();
  static void create();
  void keyPressEvent(QKeyEvent *event) override;
  void closeMain(bool _ignored);
  void openFile(bool _ignored);
  void newProject(bool _ignored);
  void updateResults();
  void nextProject(bool _ignored);

public slots:
  void closeEvent(QCloseEvent *event);
  void sysTrayMenuActivated(QSystemTrayIcon::ActivationReason reason);

private slots:
  void initialize();
};


#endif
