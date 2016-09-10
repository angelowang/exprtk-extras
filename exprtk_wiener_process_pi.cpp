/*
 **************************************************************
 *         C++ Mathematical Expression Toolkit Library        *
 *                                                            *
 * Approximation of Pi via Wiener Process based Monte Carlo   *
 * Author: Arash Partow (1999-2016)                           *
 * URL: http://www.partow.net/programming/exprtk/index.html   *
 *                                                            *
 * Copyright notice:                                          *
 * Free use of the Mathematical Expression Toolkit Library is *
 * permitted under the guidelines and in accordance with the  *
 * most current version of the Common Public License.         *
 * http://www.opensource.org/licenses/cpl1.0.php              *
 *                                                            *
 **************************************************************
*/


#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

#include "exprtk.hpp"


template <typename T>
struct rnd_01 : public exprtk::ifunction<T>
{
   rnd_01() : exprtk::ifunction<T>(0)
   { ::srand(static_cast<unsigned int>(time(NULL))); }

   inline T operator()()
   {
      // Note: Do not use this in production
      // Result is in the interval [0,1)
      return T(::rand() / T(RAND_MAX + 1.0));
   }
};

template <typename T>
void wiener_process_pi()
{
   typedef exprtk::symbol_table<T> symbol_table_t;
   typedef exprtk::expression<T>     expression_t;
   typedef exprtk::parser<T>             parser_t;

   const std::string wiener_process_pi_program =
                        " var w[10^4] := [0];                         "
                        "                                             "
                        " for (var i := 0; i < w[]; i += 1)           "
                        " {                                           "
                        "   var x[10^4] := [(rnd_01 < 0.5) ? -1 : 1]; "
                        "   w[i]        := sum(x);                    "
                        " }                                           "
                        "                                             "
                        " (2 * w[]) / avg(abs(w))^2;                  ";

   rnd_01<T> rnd01;

   symbol_table_t symbol_table;
   symbol_table.add_function("rnd_01",rnd01);

   expression_t expression;
   expression.register_symbol_table(symbol_table);

   parser_t parser;
   parser.compile(wiener_process_pi_program,expression);

   const T approximate_pi = expression.value();

   const T real_pi = T(3.141592653589793238462643383279502); // or close enough...

   printf("pi ~ %20.17f\terror: %20.17f\n",
          approximate_pi,
          std::abs(real_pi - approximate_pi));
}

int main()
{
   wiener_process_pi<double>();
   return 0;
}