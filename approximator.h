#ifndef APPROXIMATOR_H
#define APPROXIMATOR_H
#include <vector>
#include <variant>
#include <string>
#include "graphMethod.h"
using result = std::variant<double, std::string>;

bool cmp(double x, double y);
template <typename T>
int
sgn (T val)
{
  return (T (0) < val) - (val < T (0));
}

int
cholesky_solve (double *a, double *b, double *result, int n);
int
cholesky_decomp (double *a, double *d, int n);

int
cholesky_compute_y (double *a, double *y, double *b, int n);

int
cholesky_compute_x (double *a, double *x, double *y, double *d, int n);

int solve(double* a, double* b, double* c, double* d, int n);

class Approximator
{
    // given
    std::vector<double>& _x;
    std::vector<double>& _y;
    std::vector<double>& _in;
    double _d1;
    double _dn;
    // to find
    std::vector<double> _out;

    std::vector<double> _diffs;

    GraphMethod _method;
    result approxSimple(double x);
    result approxNewton(double x, bool);
    result approxCubicSpline(double x);
    bool initCubicSpline(double, double);
    result approximate(double x);
public:

    explicit Approximator(std::vector<double> &x, std::vector<double> &y, std::vector<double> &in, double d1, double dn, GraphMethod method = GraphMethod::newton);
    std::vector<double> & get_out();
    bool computeOut();
};

#endif // APPROXIMATOR_H
