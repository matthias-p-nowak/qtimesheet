#ifndef INC_IZ0RSEfv698
#define INC_IZ0RSEfv698

#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QKeyEvent>
#include <QMenuBar>
#include <QTimer>
#include <QFileDialog>
#include <QInputDialog>


class MainWindow: public QMainWindow {

private:
  MainWindow();
  QSystemTrayIcon *trayIcon;
  void readRecordsFromFile();
  void writeRecordsToFile();
  
public:
  ~MainWindow();
  static void create();
  void keyPressEvent(QKeyEvent *event) override;
  void closeMain(bool _ignored);
  void openFile(bool _ignored);
  void newProject(bool _ignored);
  void updateResults();

public slots:
  void closeEvent(QCloseEvent *event);
  void sysTrayMenuActivated(QSystemTrayIcon::ActivationReason reason);

private slots:
  void initialize();
};


#endif
