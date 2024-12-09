
/*
File for explicit testing functions from uni5_for.c file.
*/

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

namespace tt = boost::test_tools;

#include <adolc/adolc.h>

#include "const.h"

#include <array>
#include <numeric>
#include <vector>

BOOST_AUTO_TEST_SUITE(uni5_for)

BOOST_AUTO_TEST_CASE(Fmaxoperator_ZOS_PL_Forward) {

  enableMinMaxUsingAbs();

  const int16_t tag = 1;

  const int dim_out = 1;
  const int dim_in = 3;

  std::array<double, dim_in> in = {-2.0, 0.0, 1.5};
  std::array<adouble, dim_in> indep;
  double out[] = {0.0};

  // ---------------------- trace on ---------------------
  // function is given by fabs(in_2 + fabs(in_1 + fabs(in_0)))
  trace_on(tag);

  // init independents
  std::for_each(in.begin(), in.end(), [&, i = 0](int value) mutable {
    indep[i] <<= in[i];
    ++i;
  });

  // fabs(in_2 + fabs(in_1 + fabs(in_0)))
  adouble dep =
      std::accumulate(indep.begin(), indep.end(), adouble(0.0),
                      [](adouble sum, adouble val) { return fabs(sum + val); });

  dep >>= out[0];
  trace_off();

  // ---------------------- trace off ---------------------

  // test outout
  double test_out =
      std::accumulate(in.begin(), in.end(), 0.0, [](double sum, double val) {
        return std::fabs(sum + val);
      });
  BOOST_TEST(out[0] == test_out, tt::tolerance(tol));

  // test num switches
  int num_switches = get_num_switches(tag);
  BOOST_TEST(num_switches == dim_in, tt::tolerance(tol));

  const int keep = 0;
  std::vector<double> switching_vec(num_switches);
  zos_pl_forward(tag, dim_out, dim_in, keep, in.data(), out,
                 switching_vec.data());

  // test outout of zos_pl_forward
  BOOST_TEST(out[0] == test_out, tt::tolerance(tol));

  // test switching vector
  std::vector<double> test_switching_vec(num_switches);
  test_switching_vec[0] = in[0];
  for (std::size_t i = 1; i < num_switches; ++i) {
    test_switching_vec[i] = std::fabs(test_switching_vec[i - 1]) + in[i];
  }
  BOOST_CHECK_EQUAL_COLLECTIONS(switching_vec.begin(), switching_vec.end(),
                                test_switching_vec.begin(),
                                test_switching_vec.end());

  removeTape(tag, ADOLC_REMOVE_COMPLETELY);
}
BOOST_AUTO_TEST_SUITE_END()