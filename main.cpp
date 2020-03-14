
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>

#include "window.h"

int main (int argc, char *argv[])
{
  QApplication app (argc, argv);

  QMainWindow *window = new QMainWindow;
  QMenuBar *tool_bar = new QMenuBar (window);
  Window *graph_area = new Window (window);
  QAction *action;

  if (graph_area->parse_command_line (argc, argv))
    {
      QMessageBox::warning (0, "Wrong input arguments!", 
                            "Wrong input arguments!");
      return -1;
    }

  action = tool_bar->addAction ("Change function", graph_area, SLOT (change_func ()));
  action->setShortcut (QString ("Ctrl+C"));

  action = tool_bar->addAction ("Double n", graph_area, SLOT (double_n ()));
  action->setShortcut (QString ("Ctrl++"));
  action = tool_bar->addAction ("Half n", graph_area, SLOT (half_n ()));
  action->setShortcut (QString ("Ctrl+-"));

  action = tool_bar->addAction ("Double m", graph_area, SLOT (double_m ()));
  action = tool_bar->addAction ("Half m", graph_area, SLOT (half_m ()));

  action = tool_bar->addAction ("Change method", graph_area, SLOT (change_method ()));
  action = tool_bar->addAction ("Exit", window, SLOT (close ()));
  action->setShortcut (QString ("Ctrl+X"));

  tool_bar->setMaximumHeight (30);

  window->setMenuBar (tool_bar);
  window->setCentralWidget (graph_area);
  window->setWindowTitle ("Graph");

  window->show ();
  app.exec ();
  delete graph_area;
  delete tool_bar;
  delete window;
  return 0;
}
