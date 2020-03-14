
#ifndef WINDOW_H
#define WINDOW_H

#include <QtWidgets/QtWidgets>
#include "graphMethod.h"
#include "approximator.h"

class Window : public QWidget
{
  Q_OBJECT

private:
  int func_id;
  GraphMethod method;
  const char *f_name;
  double a;
  double b;
  double dsr;
  int n;
  int m;
  double (*f) (double);
  double (*df)(double);

  void simpleApprox(QPainter &painter, double delta_x);
  void paintGraph(QPainter &painter, Qt::GlobalColor color, double delta_x, Approximator & appr);
  void initInVector(std::vector<double>& in);
  void initVector(std::vector<double>& x, std::vector<double>& y);
  void paintGraph(QPainter &painter, Qt::GlobalColor color);
  double findDsr(std::vector<double> &out, std::vector<double> &in);
  double findDsrK(std::vector<double> &out, std::vector<double> &in, size_t k);
  double findDsrKk(std::vector<double> &out, std::vector<double> &in, size_t k);
public:
  Window (QWidget *parent);

  QSize minimumSizeHint () const;
  QSize sizeHint () const;

  int parse_command_line (int argc, char *argv[]);

public slots:
  void change_func ();
  void change_method ();
  void double_n ();
  void half_n ();
  void double_m ();
  void half_m ();

protected:
  void paintEvent (QPaintEvent *event);
};


#endif
