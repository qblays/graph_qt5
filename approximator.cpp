#include "approximator.h"
#include "algorithm"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>

double
Approximator::approxSimple (double x)
{
  if (*_x.begin () > x || *(_x.end () - 1) < x)
    {
      printf ("out of bounds");
      fflush (stdout);
      return 0;
    }
  auto res_x = std::lower_bound (_x.cbegin (), _x.cend (), x);
  auto res_xi = res_x - _x.cbegin ();
  auto res_yi = res_xi;
  auto res_y = _y.cbegin () + res_yi;
  if (cmp (*res_x, x))
    {
      return _y[res_yi];
    }
  else if (res_x == _x.cbegin ())
    {
      return _y[res_yi];
    }
  else
    {
      double prev_x = *(res_x - 1);
      double prev_y = *(res_y - 1);
      double diff_x = *res_x - prev_x;
      double diff_y = *res_y - prev_y;
      return prev_y + diff_y / diff_x * (x - prev_x);
    }
  //    return std::variant<double, std::string> (1);
}

double
Approximator::approxNewton (double x, bool skip_diffs = 1)
{
  int n = _y.size ();
  int i, d = 1, k = n;
  int s = 0;
  if (!skip_diffs)
    {
      _diffs[0] = _y[0];
      while (k != 1)
        {
          // printf("---\n");
          for (i = 0; i < k - 1; i++)
            {
              if (cmp (_x[i + s + 1], _x[i]))
                {
                  printf ("Not correct data\n");
                  return 0;
                }
              else
                {
                  if (std::abs (_y[i]) < 1e-40 && std::abs (_y[i + 1]) < 1e-40)
                    {
                      _y[i] = 0;
                    }
                  else
                    _y[i] = (_y[i + 1] - _y[i]) / (_x[i + s + 1] - _x[i]);
                }
              if (i == 0)
                {
                  _diffs[d] = _y[i];
                  d++;
                }
            }
          k--;
          s++;
        }
    }
  double L = _diffs[n - 1]; //берем последнюю разность

  for (i = n - 2; i >= 0; i--)
    {
      L *= x - _x[i]; //домножаем на коэф
      L += _diffs[i]; //прибавляем предыдущую разность
                      // printf("L = %lf", L);
    }

  return L;
}

double
Approximator::approxCubicSpline (double x)
{
  if (*_x.begin () > x || *(_x.end () - 1) < x)
    {
      printf ("out of bounds");
      fflush (stdout);
      return 0;
    }

  auto res_x = std::upper_bound (_x.cbegin (), _x.cend (), x);
  auto res_xi = res_x - _x.cbegin ();
  res_xi--;
  if (res_xi < 0)
    {
      printf ("out of bound!!\n");
      fflush (stdout);
      abort ();
    }
  size_t i = res_xi;
  double c1 = _y[i];
  double c2 = _diffs[i];
  double diff = (_y[i + 1] - _y[i]) / (_x[i + 1] - _x[i]);
  double c3 = (3 * diff - 2 * _diffs[i] - _diffs[i + 1]) / (_x[i + 1] - _x[i]);
  double c4 = (_diffs[i] + _diffs[i + 1] - 2 * diff) /
              ((_x[i + 1] - _x[i]) * (_x[i + 1] - _x[i]));
  double k = x - _x[i];
  return c1 + c2 * k + c3 * k * k + c4 * k * k * k;
}

bool
Approximator::initCubicSpline (double d1, double dn)
{
  int n = _x.size ();
  _diffs.reserve (n);
  _a.reserve (n);
  _b.reserve (n);
  _c.reserve (n);
  auto &d = _diffs;
  _b[0] = 1;
  _c[0] = 0;
  d[0] = d1;
  _b[n - 1] = 1;
  _a[n - 1] = 0;
  d[n - 1] = dn;

  _c[0] /= _b[0];
  d[0] /= _b[0];
  for (int i = 1; i < n - 1; i++)
    {
      _a[i] = _x[i + 1] - _x[i];
      _b[i] = 2*(_x[i + 1] - _x[i - 1]);
      _c[i] = _x[i] - _x[i - 1];
      double diff1 = (_y[i] - _y[i - 1]) / (_x[i] - _x[i - 1]);
      double diff2 = (_y[i + 1] - _y[i]) / (_x[i + 1] - _x[i]);
      d[i] = 3 * (diff1 * (_x[i + 1] - _x[i]) + diff2 * (_x[i] - _x[i - 1]));
      _c[i] /= _b[i] - _a[i] * _c[i - 1];
      d[i] = (d[i] - _a[i] * d[i - 1]) / (_b[i] - _a[i] * _c[i - 1]);
    }

  auto res = solve (_a.data (), _b.data (), _c.data (), d.data (), n);
  if (res)
    {
      printf ("solve failes = %d\n", res);
      return res;
    }
  return 0;
}

double
Approximator::approximate (double x)
{
  switch (_method)
    {
    case GraphMethod::simple:
      return approxSimple (x);
    case GraphMethod::newton:
      return approxNewton (x);
    case GraphMethod::cubic_spline:
      return approxCubicSpline (x);
    default:
      return approxSimple (x);
    }
}

void
Approximator::update (double d1, double dn, GraphMethod)
{
  _d1 = d1;
  _dn = dn;
}

bool
Approximator::computeOut (bool to_recalc)
{
  auto n = _in.size ();
  if (to_recalc)
    {
      if (_method == GraphMethod::newton)
        {
          approxNewton (0, 0);
        }
      if (_method == GraphMethod::cubic_spline)
        {
          initCubicSpline (_d1, _dn);
        }
    }

  for (int i = 0; i < (int)n; i++)
    {
      auto res = approximate (_in[i]);
      _out[i] = res;
    }
  return 0;
}

Approximator::Approximator (std::vector<double> &x, std::vector<double> &y,
                            std::vector<double> &in, double d1, double dn,
                            GraphMethod method)
    : _x (x), _y (y), _in (in), _d1 (d1), _dn (dn), _method (method)
{
  _out = std::vector<double> (_in.size ());
  _diffs = std::vector<double> (_y.size ());
}

Approximator::Approximator (GraphMethod method) : _method (method)
{
  _x.reserve (1000000);
  _y.reserve (1000000);
  if (method == GraphMethod::cubic_spline)
    {
      _a.reserve (1000000);
      _b.reserve (1000000);
      _c.reserve (1000000);
    }
  _in.reserve (5000);
  _out.reserve (5000);
}

std::vector<double> &
Approximator::get_out ()
{
  return _out;
}

bool
cmp (double x, double y)
{
  if (std::abs (x - y) < 1e-12)
    {
      return 1;
    }
  else
    return 0;
}

// computes vector y
// int
// cholesky_compute_y (double *a, double *y, double *b, int n)
//{
//  for (int i = 0; i < n; i++)
//    {
//      double sum = 0;
//      int k;

//      for (k = 0; k < i; k += 1)
//        {
//          // printf ("Gnil'2 %e\n", a[get_el (k, i, u, bb, n)] * y[k]);
//          sum += a[k * n + i] * y[k];
//        }
//      if (fabs (a[i * n + i]) < 1e-10)
//        {
//          return -1;
//        }
//      /* printf ("sum %e\n", sum);
//       printf ("b %e\n", b[i]);
//       printf ("b-sum %e\n", b[i] - sum);
//       printf ("ii %e\n", a[get_el (i, i, u, bb, n)]);*/
//      y[i] = (b[i] - sum) / a[i * n + i];
//    }
//  return 0;
//}

// int
// cholesky_compute_x (double *a, double *x, double *y, double *d, int n)
//{
//  for (int i = n - 1; i >= 0; i--)
//    {
//      double sum = 0;
//      for (int k = i + 1; k < n; k++)
//        {
//          sum += a[i * n + k] * x[k];
//        }
//      if (fabs (a[i * n + i]) < 1e-10)
//        {
//          return -1;
//        }

//      x[i] = d[i] * (y[i] - d[i] * sum) / a[i * n + i];
//    }
//  return 0;
//}

//// solve system Ax = b by cholesky method
//// n -- dimension.
//// matrix A will be destroyed.
//// output result to result.
//// if cant solve return non zero.
// int
// cholesky_solve (double *a, double *b, double *result, int n)
//{
//   double *d = (double *)malloc (n * 2 * sizeof (double));
//   double *y = d + n; // temporary vector
//  if (cholesky_decomp (a, d, n) != 0)
//    {
//      return -1;
//    }
//  if (cholesky_compute_y (a, y, b, n) != 0)
//    {
//      printf ("cholesky_compute_y failed\n");
//      return -1;
//    }
//  if (cholesky_compute_x (a, result, y, d, n) != 0)
//    {
//      printf ("cholesky_compute_x failed\n");
//      return -1;
//    }
//   free (d);
//  return 0;
//}

// int
// cholesky_decomp (double *a, double *d, int n)
//{
//  for (int i = 0; i < n; i++)
//    {
//      // compute d_ii, r_ii:
//      double sum = a[i * n + i];
//      for (int k = 0; k < i; k++)
//        {
//          sum -= a[n * k + i] * a[n * k + i] * d[k];
//        }
//      d[i] = sgn (sum);
//      if (fabs (sum) < 1e-10)
//        {
//          printf ("Cholesky_decomp failed\n");
//          return -1;
//        }
//      a[i * n + i] = sqrt (fabs (sum));

//      // compute r_ij:
//      for (int j = i + 1; j < n; j++)
//        {
//          double sum = a[i * n + j];
//          for (int k = 0; k < i; k++)
//            {
//              sum -= a[n * k + i] * a[n * k + j] * d[k];
//            }
//          a[n * i + j] = sum / (a[n * i + i] * d[i]);
//        }
//    }
//  return 0;
//}

int
solve (double *a, double *b, double *c, double *d, int n)
{
  n--; // since we start from x0 (not x1)
       //  c[0] /= b[0];
       //  d[0] /= b[0];

  //  for (int i = 1; i < n; i++)
  //    {
  //      c[i] /= b[i] - a[i] * c[i - 1];
  //      d[i] = (d[i] - a[i] * d[i - 1]) / (b[i] - a[i] * c[i - 1]);
  //    }
  d[n] = (d[n] - a[n] * d[n - 1]) / (b[n] - a[n] * c[n - 1]);

  for (int i = n; i-- > 0;)
    {
      d[i] -= c[i] * d[i + 1];
    }
  return 0;
}
