
#include <QPainter>
#include <stdio.h>

#include "window.h"
#include <thread>


#define DEFAULT_A -10
#define DEFAULT_B 10
#define DEFAULT_N 8
#define DEFAULT_M 8192
#define DEFAULT_DSR 0.

static
double f_0 (double x)
{
  return x;
}

static
double df_0 (double )
{
  return 1;
}

static
double f_1 (double x)
{
  return x * x * x;
}

static
double df_1 (double x)
{
  return 3* x * x;
}

static
double f_2 (double x)
{
  return x*x*x*x*x;
}

static
double df_2 (double x)
{
  return 5*x*x*x*x;
}

static
double f_3 (double x)
{
  return sin (x);
}



static
double df_3 (double x)
{
  return cos (x);
}

static
double f_4 (double x)
{
  double a = fabs(tan(x*x*x)) + 1;
  return 1 / a;
}

static
double df_4 (double x)
{
  double a = 3*x*x*tan(x*x*x);
  double c = fabs(tan(x*x*x));
  double cx4 = cos(x*x*x);
  double b = c*(c+1)*(c+1) * cx4 * cx4;
  return - a / b;
}

Window::Window (QWidget *parent)
  : QWidget (parent)
{
  a = DEFAULT_A;
  b = DEFAULT_B;
  n = DEFAULT_N;
  m = DEFAULT_M;
  dsr = DEFAULT_DSR;
  func_id = 0;
  method = GraphMethod::simple;

  change_func ();
}

QSize Window::minimumSizeHint () const
{
  return QSize (100, 100);
}

QSize Window::sizeHint () const
{
  return QSize (1000, 1000);
}

int Window::parse_command_line (int argc, char *argv[])
{
  if (argc == 1)
    return 0;

  if (argc == 2)
    return -1;

  if (   sscanf (argv[1], "%lf", &a) != 1
      || sscanf (argv[2], "%lf", &b) != 1
      || b - a < 1.e-6
      || (argc > 3 && sscanf (argv[3], "%d", &n) != 1)
      || n <= 0)
    return -2;

  return 0;
}

/// change current function for drawing
void Window::change_func ()
{
  func_id = (func_id + 1) % 5;

  switch (func_id)
    {
  case 0:
    f_name = "f (x) = x";
    f = f_0;
    df = df_0;
    break;
  case 1:
    f_name = "f (x) = x * x * x";
    f = f_1;
    df = df_1;
    break;
  case 2:
    f_name = "f (x) = x * x * x *x *x";
    f = f_2;
    df = df_2;
    break;
  case 3:
    f_name = "f (x) = sin(x)";
    f = f_3;
    df = df_3;
    break;
  case 4:
    f_name = "f (x) = 1 / (|tanx^4| + 1)";
    f = f_4;
    df = df_4;
    break;
    }
  update ();
}

void Window::change_method()
{
    method = (GraphMethod)(((int)method +1)%3);
    if(method == GraphMethod::newton){
        n = 64;
    }
    update ();
}

void Window::double_n()
{
    if (n < (2<<25))
    {
        n*=2;
        update ();
    }
}

void Window::half_n()
{

    if (n > 1)
    {
        n /= 2;
        update ();
    }
}

void Window::double_m()
{
    if (m < (2<<20))
    {
        m*=2;
        update ();
    }
}

void Window::half_m()
{
    if (m > 1)
    {
        m /= 2;
        update ();
    }
}

/// render graph
void Window::paintEvent (QPaintEvent * /* event */)
{  
  QPainter painter (this);
//  painter.setRenderHint(QPainter::Antialiasing, true);
//  painter.setRenderHint(QPainter::HighQualityAntialiasing, true);
  double x1, y1;
  double max_y, min_y;
  double delta_y, delta_x = (b - a) / m;
  QPen pen_black(Qt::black, 0, Qt::SolidLine); 
  QPen pen_red(Qt::black, 1, Qt::SolidLine);

  painter.setPen (pen_black);

  // calculate min and max for current function
  max_y = min_y = 0;
  for (x1 = a; x1 - b < 1.e-6; x1 += delta_x)
    {
      y1 = f (x1);
      if (y1 < min_y)
        min_y = y1;
      if (y1 > max_y)
        max_y = y1;
    }

  delta_y = 0.01 * (max_y - min_y);
  min_y -= delta_y;
  max_y += delta_y;

  // save current Coordinate System
  painter.save ();

  // make Coordinate Transformations
  painter.translate (0.5 * width (), 0.5 * height ());
  auto scaling = cmp(max_y, min_y)?max_y-min_y: max_y-min_y;
  painter.scale (width () / (b - a), -height () / (scaling));
  painter.translate (-0.5 * (a + b), -0.5 * (min_y + max_y));

  // draw axis
  pen_red.setCosmetic(1);
  painter.setPen (pen_red);
  painter.drawLine(QPointF(a-1, 0), QPointF(b+1, 0));
  painter.drawLine(QPointF(0, max_y+10), QPointF(0, min_y-10));
//  painter.drawLine (a-1, 0, b+1, 0);
//  painter.drawLine (0, max_y+10, 0, min_y-10);


  // restore previously saved Coordinate System
  paintGraph(painter, Qt::red);
  painter.restore ();
  // render function name
  painter.setPen ("green");
  painter.drawText (0, 20, f_name);
  painter.drawText (0, 35, "n = " + QString::number(n));
  QString method_name;
  switch(method)
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
  painter.drawText (0, 50, "m = " + QString::number(m));
  painter.drawText (0, 65, "method = " + method_name);

  painter.drawText (0, 80, "a = " + QString::number(a) + " b = " + QString::number(b));
  painter.drawText (0, 95, "disrep = " + QString::number(dsr));

}

/// simple approximation
void Window::simpleApprox (QPainter& painter, double delta_x)
{
    // draw approximated line for graph
    double x1, x2, y1, y2;
    x1 = a;
    y1 = f (x1);
    for (x2 = x1 + delta_x; x2 - b < 1.e-6; x2 += delta_x)
      {
        y2 = f (x2);
        painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));
        x1 = x2; y1 = y2;
      }
    x2 = b;
    y2 = f (x2);
    painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));
}

void Window::initVector(std::vector<double>& x, std::vector<double>& y){
    auto n = x.size()-1;
    auto delta = (b-a)/n;
    double xx = a;
    for (size_t i = 0; i<n+1; i++){
        x[i] = xx;
        y[i] = f(xx);
        xx+= delta;
    }
}

void Window::initInVector(std::vector<double>& in){
    auto n = in.size()-1;
    double B = b<0?b*1.01:b*0.99;
    double A = a<0?a*0.99:a*1.01;
    auto delta = (B-A)/n;
    double xx = A;
    for (size_t i = 0; i<n+1; i++){
        in[i] = xx;
        xx+= delta;
    }
}

double Window::findDsr (std::vector<double>& out, std::vector<double>& in){
    double dsr = 0.;
    auto n = out.size();
    for (size_t i = 0; i < n; i++) {
        dsr += (fabs(out[i] - f(in[i])))/n;
    }
    return dsr;
}

double Window::findDsrK (std::vector<double>& out, std::vector<double>& in, size_t k){
    double dsr = 0.;
    auto n = out.size();
    k = n / 2 + 1;
    if (k==0) k = 1;
    for (size_t i = k/2; i < n; i+=k) {
        printf("dsr computed %lu\n", i);
        dsr += (fabs(fabs(out[i]) - fabs(f(in[i]))));
    }
    printf("dsr computed in %lu points\n", n/k);
    return dsr/(n/k);
}

double Window::findDsrKk (std::vector<double>& out, std::vector<double>& in, size_t k){
    double dsr = 0.;
    auto n = out.size();
    k = n / 10;
    if (k==0) k = 1;
    for (size_t i = k; i < n; i+=k) {
        dsr += (fabs(fabs(out[i]) - fabs(f(in[i]))));
    }
    return dsr/(n/k);
}


void Window::paintGraph (QPainter& painter, Qt::GlobalColor color){
//    printf("paintGraph called\n");
    fflush(stdout);
    QPen pen(color, 0.5, Qt::SolidLine);
     pen.setCosmetic(1);
    painter.setPen(pen);
    auto x = std::vector<double> (n+1);
    auto y = std::vector<double> (n+1);
    auto in = std::vector<double> (m+1);
    initVector(x, y);
    initInVector(in);

//    for(double i:x){
//        printf("x = %lf", i);
//    }
//    for(double i:y){
//        printf("y = %lf", i);
//    }
    auto appr = Approximator(x, y, in, df(x[0]), df(x[n]), method);
    appr.computeOut();
//    printf("deb p1\n");
//    fflush(stdout);
    auto out = appr.get_out();
//    for(double i:out){
//        printf("out = %lf", i);
//    }
    double x1, x2, y1, y2;
    auto p = QPainterPath();
    x1 = in[0];
    y1 = out[0];
    p.moveTo(QPointF(x1, y1));
//    printf("deb p1 m=%d\n", m);
    fflush(stdout);
    dsr = findDsrK(out, in, 0);
    auto dsrK = findDsrKk(out, in, 0);
    if(method == GraphMethod::newton && dsrK > 1e8){
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        printf("disrep > 1e8; newton restarts with default settings\n");
        fflush(stdout);
        n = 64;
        m = 1024;
        change_method();

        paintGraph(painter, color);
        return;
    }
    for (int i = 1;i < m+1;i++)
      {
        x2 = in[i];
        y2 = out[i];
//        printf("deb p i%d, x = %lf, y = %lf\n", i, x1, y1);
//        fflush(stdout);
//        painter.drawLine (QPointF (x1, y1), QPointF (x2, y2));
        p.lineTo(QPointF (x2, y2));
        x1 = x2; y1 = y2;
      }
    painter.drawPath(p);

}
