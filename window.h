
#ifndef WINDOW_H
#define WINDOW_H

#include "approximator.h"
#include "graphMethod.h"
#include <QtWidgets/QtWidgets>

enum class content_type
{
  newton,
  cubic_spline,
  both,
  both_error
};

class Window : public QWidget
{
  Q_OBJECT
public:
  using vector = std::vector<double>;

private:
  int func_id;
  content_type content;
  const char *f_name;
  double a;
  double b;
  double dsr;
  double elapsed = 0;
  int n;
  int p;
  size_t m;
  bool m_show_disrep;
  bool m_need_to_recalc;
  double (*f) (double);
  double (*df) (double);
  std::unique_ptr<Approximator> m_approximator;
  std::unique_ptr<Approximator> m_approximator_newton;

  void
  simpleApprox (QPainter &painter, double delta_x);
  void
  paintGraph (QPainter &painter, Qt::GlobalColor color, double delta_x,
              Approximator &appr);
  void
  initInVector (std::vector<double> &in);
  void
  initVector (std::vector<double> &x, std::vector<double> &y);
  void
  paintGraph (QPainter &painter, Qt::GlobalColor color);
  double
  findDsr (std::vector<double> &out, std::vector<double> &in);
  double
  findDsrK (std::vector<double> &out, std::vector<double> &in, size_t k);
  double
  findDsrKk (std::vector<double> &out, std::vector<double> &in, size_t k);
  void
  scale (QPainter &painter, double mod_max_y, double mod_min_y);
  void
  printMiscInfo (QPainter &painter, double min_y, double max_y);
  void
  paintAxis (QPainter &painter, double max_y, double min_y);
  void
  zoom (double coef);
  void
  calc_out (vector &x, vector &y, vector &in, vector &out, bool to_recalc);
  double
  check_disrep (const vector &x, const vector &y, double (*f) (double));

  void
  change_delta (int p);
  void
  calc_out (Approximator &approx, bool to_recalc);

public:
  Window (QWidget *parent);

  QSize
  minimumSizeHint () const;
  QSize
  sizeHint () const;

  int
  parse_command_line (int argc, char *argv[]);

  void
  paintMap (vector &x, vector &y, QPainter &painter, QPen &pen);
  void
  calc_disrep (const vector &x, const vector &y, vector &disrep,
               double (*f) (double));
public slots:
  void
  change_func ();
  void
  double_n ();
  void
  half_n ();
  //  void double_m ();
  //  void half_m ();

  void
  show_disrep ();
  void
  zoom_in ();
  void
  zoom_out ();

  void
  set_big_n ();
  void
  add_delta ();
  void
  subtract_delta ();
  void
  change_content ();

protected:
  void
  paintEvent (QPaintEvent *event);
};

#endif
