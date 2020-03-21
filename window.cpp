
#include <QPainter>
#include <stdio.h>

#include "window.h"
#include <thread>

#define DEFAULT_A -10
#define DEFAULT_B 10
#define DEFAULT_N 32
#define DEFAULT_M 1080
#define DEFAULT_DSR 0.

static double
f_0 (double x)
{
  double a = fabs (tan (x * x * x)) + 1;
  return 1 / a;
}

static double
df_0 (double x)
{
  double a = 3 * x * x * tan (x * x * x);
  double c = fabs (tan (x * x * x));
  double cx4 = cos (x * x * x);
  double b = c * (c + 1) * (c + 1) * cx4 * cx4;
  return -a / b;
}

static double
f_1 (double)
{
  return 1;
}

static double
df_1 (double)
{
  return 0;
}

static double
f_2 (double x)
{
  return x;
}

static double
df_2 (double)
{
  return 1;
}

static double
f_3 (double x)
{
  return 20 * x - .1 * x * x * x;
}

static double
df_3 (double x)
{
  return 20 - .3 * x * x;
}

static double
f_4 (double x)
{
  return sin (x);
}

static double
df_4 (double x)
{
  return cos (x);
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

Window::Window (QWidget *parent) : QWidget (parent)
{
  a = DEFAULT_A;
  b = DEFAULT_B;
  n = DEFAULT_N;
  m = DEFAULT_M;
  dsr = DEFAULT_DSR;
  m_show_disrep = 0;
  m_need_to_recalc = 0;
  func_id = 0;
  method = GraphMethod::simple;

  m_approximator = new Approximator (method);

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

  if (argc == 2)
    return -1;

  if (sscanf (argv[1], "%lf", &a) != 1 || sscanf (argv[2], "%lf", &b) != 1 ||
      b - a < 1.e-6 || (argc > 3 && sscanf (argv[3], "%d", &n) != 1) || n <= 0)
    return -2;

  return 0;
}

/// change current function for drawing
void
Window::change_func ()
{
  func_id = (func_id + 1) % 6;

  switch (func_id)
    {
    case 0:
      f_name = "f (x) = 1 / (|tanx^3| + 1)";
      f = f_0;
      df = df_0;
      break;
    case 1:
      f_name = "f (x) = 1";
      f = f_1;
      df = df_1;
      break;
    case 2:
      f_name = "f (x) = x";
      f = f_2;
      df = df_2;
      break;
    case 3:
      f_name = "f (x) = 20x - 0.1x^3";
      f = f_3;
      df = df_3;
      break;
    case 4:
      f_name = "f (x) = sin(x)";
      f = f_4;
      df = df_4;
      break;
    case 5:
      f_name = "f (x) = exp (x)";
      f = f_5;
      df = df_5;
      if (std::max (std::abs (a), std::abs (b)) > 39)
        {
          printf ("too big numbers for exp, a, b reseted\n");
          a = -10;
          b = 10;
        }
      break;
    }
  m_need_to_recalc = 1;
  update ();
}

void
Window::change_method ()
{
  method = (GraphMethod) (((int)method + 1) % 3);
  if (method == GraphMethod::newton)
    {
      n = 64;
    }

  m_need_to_recalc = 1;
  update ();
}

void
Window::double_n ()
{
  if (n < (2 << 25))
    {
      n *= 2;
      if (n >= 128 && method == GraphMethod::newton)
        {
          n = 64;
        }
      m_need_to_recalc = 1;
      update ();
    }
}

void
Window::set_big_n ()
{
  if (method != GraphMethod::newton)
    {
      n = 2097152;
      m_need_to_recalc = 1;
      update ();
    }
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
      m_need_to_recalc = 1;
    }
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
  QColor c = QColor(150, 130, 130);
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
  sprintf (buf, "%d/6: ", func_id + 1);
  QString func_name = buf;
  if (m_show_disrep)
    {
      func_name += "disrep for ";
    }
  int shift = 10;
  painter.fillRect (5, 5, 280, 100, brush);
  painter.drawText (shift, 20, func_name + f_name);
  painter.drawText (shift, 35, "n = " + QString::number (n));
  QString method_name;
  switch (method)
    {
    case GraphMethod::simple:
      method_name = "simple";
      break;
    case GraphMethod::newton:
      method_name = "newton";
      break;
    case GraphMethod::cubic_spline:
      method_name = "cubic_spline";
      break;
    }

  painter.drawText (shift, 50, "m = " + QString::number (m));
  painter.drawText (shift, 65, "method = " + method_name);

  painter.drawText (
      shift, 80, "a = " + QString::number (a) + " b = " + QString::number (b));
  sprintf (buf, "min, max = %.3e, %.3e", min_y, max_y);
  painter.drawText (shift, 95, buf);
}

/// render graph
void
Window::paintEvent (QPaintEvent * /* event */)
{
  QPainter painter (this);
  m = width ();
  QColor c = QColor(38, 38, 38);
  QBrush brush (c);
  painter.fillRect (5, 5, width (), height (), brush);
  //  painter.setRenderHint(QPainter::Antialiasing, true);

  //  QPen pen_black(Qt::black, 0, Qt::SolidLine);

  //  painter.setPen (pen_black);
  //  auto x = vector (n+1);
  //  auto y = vector (n+1);

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
  if (m_need_to_recalc)
    {
      initVector (x, y);
    }
  initInVector (in);
  calc_out (m_need_to_recalc);
  if (m_show_disrep)
    {
      calc_disrep (in, out, out, f);
    }
  if (method == GraphMethod::newton)
    {
      double max_disrep = check_disrep (in, out, f);
      if (max_disrep > 1e100)
        {
          printf ("Newton failed, max disrep = %e, half n\n", max_disrep);
          fflush (stdout);
          n /= 2;
          if (n == 0)
            {
              n = 1;
            }
          update ();
          return;
        }
    }
  out = m_approximator->get_out ();
  m_need_to_recalc = 0;
  auto [min_y_it, max_y_it] = std::minmax_element (begin (out), end (out));
  auto [min_y, max_y] = std::pair{*min_y_it, *max_y_it};
  double delta_y = 0.01 * (max_y - min_y);
  auto mod_min_y = min_y - delta_y;
  auto mod_max_y = max_y + delta_y;

  painter.save ();
  scale (painter, mod_max_y, mod_min_y);

  paintMap (in, out, painter, Qt::red);
  paintAxis (painter, mod_max_y, mod_min_y);
  painter.restore ();

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
Window::calc_out (bool to_recalc)
{
  double d1 = df (m_approximator->_x[0]);
  double dn = df (m_approximator->_x[n]);
  m_approximator->update (d1, dn, method);
  m_approximator->computeOut (to_recalc);
}

void
Window::paintMap (vector &x, vector &y, QPainter &painter,
                  Qt::GlobalColor color)
{
  auto p = QPainterPath ();
  QPen pen (color);
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
