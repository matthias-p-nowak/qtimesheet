#include <qtimesheet.hh>

using namespace std;

int main(int argc, char *argv[]) {
  fromHere("starting");
  // alarm(3);
  QApplication app(argc, argv);
  app.setOrganizationName("TimeSheet");
  app.setApplicationName("timesheet");
  MainWindow::create();
  fromHere("entering message loop");
  app.exec();
  fromHere("all done, exiting");
  exit(EXIT_SUCCESS);
}
