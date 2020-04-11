
#include <QPainter>
#include <stdio.h>

#include "window.h"
#include <chrono>
#include <iostream>
#include <thread>

#define DEFAULT_A -10
#define DEFAULT_B 10
#define DEFAULT_N 32
#define DEFAULT_M 1080
#define DEFAULT_DSR 0.

static double
f_0 (double)
{
  return 1;
}

static double
df_0 (double)
{
  return 0;
}

static double
f_1 (double x)
{
  return x;
}

static double
df_1 (double)
{
  return 1;
}

static double
f_2 (double x)
{
  return x * x;
}

static double
df_2 (double x)
{
  return 2 * x;
}

static double
f_3 (double x)
{
  return x * x * x;
}

static double
df_3 (double x)
{
  return 3 * x * x;
}

static double
f_4 (double x)
{
  return x * x * x * x;
}

static double
df_4 (double x)
{
  return 4 * x * x * x;
}

static double
f_5 (double x)
{
  return exp (x);
}

static double
df_5 (double x)
{
  return exp (x);
}

static double
f_6 (double x)
{
  return 1 / (25 * x * x + 1);
}

static double
df_6 (double x)
{
  return -50 * x / ((25 * x * x + 1) * (25 * x * x + 1));
}

Window::Window (QWidget *parent) : QWidget (parent)
{
  a = DEFAULT_A;
  b = DEFAULT_B;
  n = DEFAULT_N;
  m = DEFAULT_M;
  p = 0;
  dsr = DEFAULT_DSR;
  m_show_disrep = 0;
  m_need_to_recalc = 0;
  func_id = 6;
  this->content = content_type::newton;

  m_approximator = new Approximator (GraphMethod::cubic_spline);
  m_approximator_newton = new Approximator (GraphMethod::newton);
  m_approximator->_x.resize (100000000);
  m_approximator->_y.resize (100000000);
  m_approximator->_x.resize (0);
  m_approximator->_y.resize (0);

  change_func ();
}

QSize
Window::minimumSizeHint () const
{
  return QSize (100, 100);
}

QSize
Window::sizeHint () const
{
  return QSize (1000, 1000);
}

int
Window::parse_command_line (int argc, char *argv[])
{
  if (argc == 1)
    return 0;

  if (argc != 5)
    return -1;

  if (sscanf (argv[1], "%lf", &a) != 1 || sscanf (argv[2], "%lf", &b) != 1 ||
      b - a < 1.e-6 || (argc > 3 && sscanf (argv[3], "%d", &n) != 1) ||
      n <= 0 || (sscanf (argv[4], "%d", &this->func_id) != 1))
    return -2;
  for (int i = 0; i < 7; i++)
    {
      change_func ();
    }

  return 0;
}

/// change current function for drawing
void
Window::change_func ()
{
  func_id = (func_id + 1) % 7;

  switch (func_id)
    {
    case 0:
      f_name = "f = 1";
      f = f_0;
      df = df_0;
      break;
    case 1:
      f_name = "f = x";
      f = f_1;
      df = df_1;
      break;
    case 2:
      f_name = "f = x^2";
      f = f_2;
      df = df_2;
      break;
    case 3:
      f_name = "f = x^3";
      f = f_3;
      df = df_3;
      break;
    case 4:
      f_name = "f = x^4";
      f = f_4;
      df = df_4;
      break;
    case 5:
      f_name = "f = exp(x)";
      f = f_5;
      df = df_5;
      if (std::max (std::abs (a), std::abs (b)) > 39)
        {
          printf ("too big numbers for exp, a, b reseted\n");
          a = -10;
          b = 10;
        }
      break;
    case 6:
      f_name = "f = 1/(25*x^2 + 1)";
      f = f_6;
      df = df_6;
      break;
    }
  m_need_to_recalc = 1;
  update ();
}

void
Window::change_content ()
{
  this->content = (content_type) (((int)this->content + 1) % 4);
  update ();
}

void
Window::double_n ()
{
  if (n < (2 << 25))
    {
      n *= 2;
      m_need_to_recalc = 1;
      update ();
    }
}

void
Window::set_big_n ()
{
  n = 2 << 25;
  m_need_to_recalc = 1;
  update ();
}

void
Window::half_n ()
{

  if (n > 1)
    {
      n /= 2;
      m_need_to_recalc = 1;
      update ();
    }
}

void
Window::zoom_in ()
{
  zoom (0.5);
  m_need_to_recalc = 1;
  update ();
}

void
Window::change_delta (int p)
{
  this->p = p;
  m_need_to_recalc = 1;
  update ();
}

void
Window::add_delta ()
{
  this->change_delta (this->p + 1);
}

void
Window::subtract_delta ()
{
  this->change_delta (this->p - 1);
}

void
Window::zoom_out ()
{
  if (std::max (std::abs (a), std::abs (b)) > 39 && func_id == 5)
    {
      printf ("too big numbers for exp, a, b reseted\n");
      a = -10;
      b = 10;
    }
  else
    {
      zoom (2);
    }
  m_need_to_recalc = 1;
  update ();
}

void
Window::zoom (double coef)
{
  double middle = (a + b) / 2;
  double half_length = b - middle;
  half_length *= coef;
  a = middle - half_length;
  b = middle + half_length;
}

void
Window::show_disrep ()
{
  if (m_show_disrep == 0)
    {
      m_show_disrep = 1;
    }
  else
    {
      m_show_disrep = 0;
    }
  update ();
}

void
Window::scale (QPainter &painter, double max_y, double min_y)
{
  max_y *= 1.1;
  min_y *= 1.1;
  double aa = std::max (std::abs (max_y), std::abs (min_y)) * 0.05;
  if (cmp (aa, 0))
    {
      aa = 1;
    }
  if (std::abs (max_y) < 1e-20)
    {
      max_y = 1;
      min_y = -1;
    }

  if (max_y < 0)
    max_y = aa;
  if (min_y > 0)
    {
      min_y = -aa;
    }

  painter.translate (0.5 * width (), 0.5 * height ());
  auto scaling = std::abs (max_y - min_y);
  if (cmp (scaling, 0))
    {
      scaling = std::abs (max_y);
    }
  //  printf ("scale(%lf, %lf), tr(%lf, %lf)\n", width () / (b - a),
  //          -height () / scaling, -0.5 * (a + b), -0.5 * (min_y + max_y));
  //  fflush (stdout);
  painter.scale (width () / (b - a), -height () / (scaling));
  painter.translate (-0.5 * (a + b), -0.5 * (min_y + max_y));
}

void
Window::paintAxis (QPainter &painter, double max_y, double min_y)
{
  max_y *= 1.1;
  min_y *= 1.1;
  double aa = std::max (std::abs (max_y), std::abs (min_y)) * 0.05;
  if (cmp (aa, 0))
    {
      aa = 1;
    }
  if (std::abs (max_y) < 1e-20)
    {
      max_y = 1e-14;
      min_y = -1e-14;
    }

  if (max_y < 1e-12)
    max_y = aa;
  if (min_y > -1e-12)
    {
      min_y = -aa;
    }
  QColor c = QColor (150, 130, 130);
  QPen pen_red (c, 1, Qt::DotLine);
  pen_red.setCosmetic (1);
  painter.setPen (pen_red);
  painter.drawLine (QPointF (a - 1, 0), QPointF (b + 1, 0));

  painter.drawLine (QPointF (0, max_y * 1.5), QPointF (0, min_y - 1));
}

void
Window::printMiscInfo (QPainter &painter, double min_y, double max_y)
{
  QPen pen (Qt::green, 1, Qt::SolidLine);
  QBrush brush (Qt::black);
  pen.setCosmetic (1);
  painter.setPen (pen);
  char buf[60];
  sprintf (buf, "%d/7: ", func_id + 1);
  QString func_name = buf;
  int shift = 10;
  painter.fillRect (5, 5, 250, 95, brush);
  painter.drawText (shift, 20, func_name + f_name);
  painter.drawText (150, 20, "n = " + QString::number (n));
  QString content_name;
  switch (this->content)
    {
    case content_type::newton:
      content_name = "newton";
      break;
    case content_type::cubic_spline:
      content_name = "cubic spline";
      break;
    case content_type::both:
      content_name = "both";
      break;
    case content_type::both_error:
      content_name = "both errors";
      break;
    }

  painter.drawText (shift, 35, "content = " + content_name);
  painter.drawText (150, 35, "width = " + QString::number (m));

  painter.drawText (
      shift, 50, "a = " + QString::number (a) + " b = " + QString::number (b));
  sprintf (buf, "min, max = %.3e, %.3e", min_y, max_y);
  painter.drawText (shift, 65, buf);
  sprintf (buf, "p=%d", this->p);
  painter.drawText (160, 50, buf);
  brush.setColor (Qt::white);
  brush.setStyle (Qt::BrushStyle::SolidPattern);
  painter.fillRect (10, 70, 12, 12, brush);
  painter.drawText (30, 80, "exact");
  brush.setColor (Qt::red);
  painter.fillRect (90, 70, 12, 12, brush);
  painter.drawText (110, 80, "spline");
  brush.setColor (Qt::green);
  painter.fillRect (180, 70, 12, 12, brush);
  painter.drawText (200, 80, "newton");
  sprintf (buf, "elapsed = %.3lf", this->elapsed);
  painter.drawText (shift, 95, buf);
}

// render graph
void
Window::paintEvent (QPaintEvent * /* event */)
{
  auto t0 = std::chrono::high_resolution_clock::now ();
  QPainter painter (this);
  m = width ();
  QColor c = QColor (38, 38, 38);
  QBrush brush (c);
  painter.fillRect (5, 5, width (), height (), brush);

  auto &x = m_approximator->_x;
  auto &y = m_approximator->_y;
  auto &in = m_approximator->_in;
  auto &out = m_approximator->_out;
  auto &diffs = m_approximator->_diffs;

  x.resize (n + 1);
  y.resize (n + 1);
  diffs.resize (n + 1);
  in.resize (m + 1);
  out.resize (m + 1);
  auto t4 = std::chrono::high_resolution_clock::now ();
  if (m_need_to_recalc)
    {
      initVector (x, y);
    }
  auto t5 = std::chrono::high_resolution_clock::now ();
  vector x_real;
  vector y_real;
  x_real.resize (m + 1);
  y_real.resize (m + 1);
  initVector (x_real, y_real);
  double max = *std::max_element (
      y_real.cbegin (), y_real.cend (),
      [] (double x, double y) { return std::abs (x) < std::abs (y); });
  y[n / 2] += this->p * max;
  initInVector (in);

  auto t1 = std::chrono::high_resolution_clock::now ();
  calc_out (*this->m_approximator, m_need_to_recalc);
  auto t2 = std::chrono::high_resolution_clock::now ();
  std::cout << "spline time: " << (t2 - t1).count () / 1.e9 << "s" << std::endl;

  if (this->content == content_type::both_error)
    {
      calc_disrep (in, out, out, f);
    }

  out = m_approximator->get_out ();

  auto [min_y_it, max_y_it] = std::minmax_element (begin (out), end (out));

  auto [min_y, max_y] = std::pair{*min_y_it, *max_y_it};

  // newton
  if (this->n <= 50 && this->content != content_type::cubic_spline)
    {
      auto &x = m_approximator_newton->_x;
      auto &y = m_approximator_newton->_y;
      auto &in = m_approximator_newton->_in;
      auto &out = m_approximator_newton->_out;
      auto &diffs = m_approximator_newton->_diffs;
      x.resize (n + 1);
      y.resize (n + 1);
      diffs.resize (n + 1);
      in.resize (m + 1);
      out.resize (m + 1);
      if (m_need_to_recalc)
        {
          initVector (x, y);
        }
      // double max = *std::max_element(out.cbegin(), out.cend(), [](double
      // x){std::abs(x);});
      y[n / 2] += this->p * max;
      initInVector (in);
      calc_out (*this->m_approximator_newton, m_need_to_recalc);
      if (this->content == content_type::both_error)
        {
          calc_disrep (in, out, out, f);
        }

      out = m_approximator_newton->get_out ();
      auto [min_y_it, max_y_it] = std::minmax_element (begin (out), end (out));
      auto [min_y_n, max_y_n] = std::pair{*min_y_it, *max_y_it};
      if (min_y_n < min_y)
        min_y = min_y_n;
      if (max_y_n > max_y)
        max_y = max_y_n;
      if (this->content == content_type::newton)
        {
          min_y = min_y_n;
          max_y = max_y_n;
        }

      fflush (stdout);
    }
  m_need_to_recalc = 0;

  painter.save ();
  double delta_y = 0.01 * (max_y - min_y);
  auto mod_min_y = min_y - delta_y;
  auto mod_max_y = max_y + delta_y;
  scale (painter, mod_max_y, mod_min_y);
  QPen pen (Qt::red);

  if (this->content != content_type::newton)
    paintMap (in, out, painter, pen);

  pen = QPen (Qt::green);
  if (this->content != content_type::cubic_spline && this->n <= 50)
    paintMap (m_approximator_newton->_in, m_approximator_newton->_out, painter,
              pen);
  if (this->content != content_type::both_error)
    {
      vector x;
      vector y;
      x.resize (m + 1);
      y.resize (m + 1);
      initVector (x, y);
      pen.setColor (Qt::white);
      QVector<qreal> v;
      qreal space = 8;
      v << 8 << space;
      pen.setDashPattern (v);
      paintMap (x, y, painter, pen);
    }

  paintAxis (painter, mod_max_y, mod_min_y);
  painter.restore ();

  std::cout << "paint time: " << (t5 - t4).count () / 1.e9 << "s" << std::endl;
  auto t3 = std::chrono::high_resolution_clock::now ();
  this->elapsed = (t3 - t0).count () / 1.e9;
  printMiscInfo (painter, min_y, max_y);
}

void
Window::calc_disrep (const vector &x, const vector &y, vector &disrep,
                     double (*f) (double))
{
  auto len = x.size ();
  for (size_t i = 0; i < len; i++)
    {
      disrep[i] = std::abs (f (x[i]) - y[i]);
    }
}

double
Window::check_disrep (const vector &x, const vector &y, double (*f) (double))
{
  auto len = x.size ();
  double max = 0;
  for (size_t i = 0; i < len; i++)
    {
      auto disrep = std::abs (f (x[i]) - y[i]);
      if (disrep > max)
        max = disrep;
    }
  return max;
}

/// simple approximation
void
Window::simpleApprox (QPainter &painter, double delta_x)
{
  // draw approximated line for graph
  double x1, x2, y1, y2;
  x1 = a;
  y1 = f (x1);
  for (x2 = x1 + delta_x; x2 - b < 1.e-6; x2 += delta_x)
    {
      y2 = f (x2);
      painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));
      x1 = x2;
      y1 = y2;
    }
  x2 = b;
  y2 = f (x2);
  painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));
}

void
Window::initVector (std::vector<double> &x, std::vector<double> &y)
{
  auto n = x.size () - 1;
  auto delta = (b - a) / n;
  double xx = a;
  for (size_t i = 0; i < n + 1; i++)
    {
      x[i] = xx;
      y[i] = f (xx);
      xx += delta;
    }
}

void
Window::initInVector (std::vector<double> &in)
{
  auto n = in.size () - 1;
  double B = b < 0 ? b * 1.01 : b * 0.99;
  double A = a < 0 ? a * 0.99 : a * 1.01;
  auto delta = (B - A) / n;
  double xx = A;
  for (size_t i = 0; i < n + 1; i++)
    {
      in[i] = xx;
      xx += delta;
    }
}

double
Window::findDsr (std::vector<double> &out, std::vector<double> &in)
{
  double dsr = 0.;
  auto n = out.size ();
  for (size_t i = 0; i < n; i++)
    {
      dsr += (fabs (out[i] - f (in[i]))) / n;
    }
  return dsr;
}

double
Window::findDsrK (std::vector<double> &out, std::vector<double> &in, size_t k)
{
  double dsr = 0.;
  auto n = out.size ();
  k = n / 2 + 1;
  if (k == 0)
    k = 1;
  for (size_t i = k / 2; i < n; i += k)
    {
      printf ("dsr computed %lu\n", i);
      dsr += (fabs (fabs (out[i]) - fabs (f (in[i]))));
    }
  printf ("dsr computed in %lu points\n", n / k);
  return dsr / (n / k);
}

double
Window::findDsrKk (std::vector<double> &out, std::vector<double> &in, size_t k)
{
  double dsr = 0.;
  auto n = out.size ();
  k = n / 10;
  if (k == 0)
    k = 1;
  for (size_t i = k; i < n; i += k)
    {
      dsr += (fabs (fabs (out[i]) - fabs (f (in[i]))));
    }
  return dsr / (n / k);
}

void
Window::calc_out (Approximator &approx, bool to_recalc)
{
  double d1 = df (approx._x[0]);
  double dn = df (approx._x[n]);
  approx.update (d1, dn, GraphMethod::newton);
  approx.computeOut (to_recalc);
}

void
Window::paintMap (vector &x, vector &y, QPainter &painter, QPen &pen)
{
  auto p = QPainterPath ();
  pen.setCosmetic (1);
  painter.setPen (pen);
  p.moveTo (QPointF (x[0], y[0]));
  auto len = x.size ();
  for (size_t i = 1; i < len; i++)
    {
      p.lineTo (QPointF (x[i], y[i]));
    }
  painter.drawPath (p);
}
