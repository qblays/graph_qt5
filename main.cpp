
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>

#include "window.h"
#include <csignal>

int
main (int argc, char *argv[])
{
  QApplication app (argc, argv);

  QMainWindow *window = new QMainWindow;
  QMenuBar *tool_bar = new QMenuBar (window);
  tool_bar->move (100, 0);
  tool_bar->update ();
  Window *graph_area = new Window (window);
  QAction *action;

  if (graph_area->parse_command_line (argc, argv))
    {
      QMessageBox::warning (0, "Wrong input arguments!",
                            "Wrong input arguments!");
      printf ("usage: ./basic_graph <a> <b>");
      return -1;
    }
  printf ("Shortcuts: \n"
          "Change function 0\n"
          "Show disrep     1\n"
          "Zoom in         2\n"
          "Zoom out        3\n"
          "Double n        4\n"
          "Half n          5\n"
          "Add delta       6\n"
          "Subtract delta  7\n"
          "Exit            Ctrl+W\n"
          );

  action = tool_bar->addAction ("Change function", graph_area,
                                SLOT (change_func ()));
  action->setShortcut (QString ("0"));
  action =
      tool_bar->addAction ("Change content", graph_area, SLOT (change_content ()));
  action->setShortcut (QString ("1"));
  action = tool_bar->addAction ("Zoom in", graph_area, SLOT (zoom_in ()));
  action->setShortcut (QString ("2"));
  action = tool_bar->addAction ("Zoom out", graph_area, SLOT (zoom_out ()));
  action->setShortcut (QString ("3"));
  action = tool_bar->addAction ("Double n", graph_area, SLOT (double_n ()));
  action->setShortcut (QString ("4"));
  action = tool_bar->addAction ("Half n", graph_area, SLOT (half_n ()));
  action->setShortcut (QString ("5"));
  action = tool_bar->addAction ("Add delta", graph_area, SLOT (add_delta ()));
  action->setShortcut (QString ("6"));
  action = tool_bar->addAction ("Subtract delta", graph_area, SLOT (subtract_delta ()));
  action->setShortcut (QString ("7"));


  action = tool_bar->addAction ("Set big n", graph_area, SLOT (set_big_n ()));
  action = tool_bar->addAction ("Exit", window, SLOT (close ()));
  action->setShortcut (QString ("Ctrl+W"));

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
