/*
 **************************************************************
 *         C++ Mathematical Expression Toolkit Library        *
 *                                                            *
 * ExprTk REPL Interface                                      *
 * Author: Arash Partow (1999-2015)                           *
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
#include <deque>
#include <fstream>
#include <iostream>
#include <string>

#include "exprtk.hpp"


template <typename T>
struct putch : public exprtk::ifunction<T>
{
   putch() : exprtk::ifunction<T>(1) {}

   inline T operator()(const T& v)
   {
      printf("%c",static_cast<int>(v));
      return T(0);
   }
};

template <typename T>
struct putint : public exprtk::ifunction<T>
{
   putint() : exprtk::ifunction<T>(1) {}

   inline T operator()(const T& v)
   {
      printf("%d",static_cast<int>(v));
      return T(0);
   }
};

template <typename T>
class expression_processor
{
public:

   typedef exprtk::symbol_table<T>    symbol_table_t;
   typedef exprtk::expression<T>        expression_t;
   typedef exprtk::parser<T>                parser_t;
   typedef exprtk::parser_error::type        error_t;

   typedef typename parser_t::dependent_entity_collector::symbol_t symbol_t;
   typedef std::vector<symbol_t> symbol_list_t;

   expression_processor()
   : persist_symbol_table_      (false),
     symbol_dump_               (false),
     assignment_dump_           (false),
     display_total_time_        (false),
     display_total_compile_time_(false),
     enable_usr_                (false)
   {
      symbol_table_.add_constants();

      symbol_table_.add_function("putch"  ,putch_  );
      symbol_table_.add_function("putint" ,putint_ );
      symbol_table_.add_function("print"  ,print_  );
      symbol_table_.add_function("println",println_);

      symbol_table_.add_function("poly01", poly01_);
      symbol_table_.add_function("poly02", poly02_);
      symbol_table_.add_function("poly03", poly03_);
      symbol_table_.add_function("poly04", poly04_);
      symbol_table_.add_function("poly05", poly05_);
      symbol_table_.add_function("poly06", poly06_);
      symbol_table_.add_function("poly07", poly07_);
      symbol_table_.add_function("poly08", poly08_);
      symbol_table_.add_function("poly09", poly09_);
      symbol_table_.add_function("poly10", poly10_);
      symbol_table_.add_function("poly11", poly11_);
      symbol_table_.add_function("poly12", poly12_);

      symbol_table_.add_stringvar("s0",local_str_[0]);
      symbol_table_.add_stringvar("s1",local_str_[1]);
      symbol_table_.add_stringvar("s2",local_str_[2]);
      symbol_table_.add_stringvar("s3",local_str_[3]);
   }

   bool& persist_symbol_table()
   {
      return persist_symbol_table_;
   }

   bool& symbol_dump()
   {
      return symbol_dump_;
   }

   bool& assignment_dump()
   {
      return assignment_dump_;
   }

   bool& display_total_time()
   {
      return display_total_time_;
   }

   bool& display_total_compile_time()
   {
      return display_total_compile_time_;
   }

   bool& enable_usr()
   {
      return enable_usr_;
   }

   void setup_symbol_table()
   {
      if (!persist_symbol_table_)
      {
         symbol_table_.clear_variables();
         symbol_table_.add_constants();
         symbol_table_.add_constant ("e",exprtk::details::numeric::constant::e);
         symbol_table_.add_stringvar("s0",local_str_[0]);
         symbol_table_.add_stringvar("s1",local_str_[1]);
         symbol_table_.add_stringvar("s2",local_str_[2]);
         symbol_table_.add_stringvar("s3",local_str_[3]);
      }
   }

   void process(const std::string& program)
   {
      setup_symbol_table();

      expression_t expression;
      expression.register_symbol_table(symbol_table_);

      exprtk::timer compile_timer;
      compile_timer.start();

      if (enable_usr_)
         parser_.enable_unknown_symbol_resolver();
      else
         parser_.disable_unknown_symbol_resolver();

      parser_.dec().collect_variables() = symbol_dump_;
      parser_.dec().collect_functions() = symbol_dump_;

      parser_.dec().collect_assignments() = assignment_dump_;

      if (!parser_.compile(program,expression))
      {
         printf("Error: %s\tExpression:%c%s\n",
                parser_.error().c_str(),
                ((std::string::npos != program.find_first_of('\n')) ? '\n' : ' '),
                ((program.size() < 200) ? program.c_str() : "....."));

         for (std::size_t i = 0; i < parser_.error_count(); ++i)
         {
            error_t error = parser_.get_error(i);

            printf("Err No.: %02d  Pos: %02d  Type: [%14s] Msg: %s\n",
                   static_cast<unsigned int>(i),
                   static_cast<unsigned int>(error.token.position),
                   exprtk::parser_error::to_str(error.mode).c_str(),
                   error.diagnostic.c_str());

            if (
                 (0 == i) &&
                 exprtk::parser_error::update_error(error,program)
               )
            {
               printf("Error (line: %d column: %d)\n",
                      static_cast<unsigned int>(error.line_no),
                      static_cast<unsigned int>(error.column_no));

               printf("%s \n",error.error_line.c_str());
               printf("%s^\n",std::string(error.column_no,'~').c_str());
            }
         }

         return;
      }

      compile_timer.stop();

      if (display_total_compile_time_)
      {
         printf("\nCompile time: %6.3fms\n",compile_timer.time() * 1000.0);
      }

      exprtk::timer timer;
      timer.start();

      T result = expression.value();

      timer.stop();

      printf("\nresult: %10.5f\n",result);

      if (display_total_time_)
      {
         printf("\nTotal time: %6.3fms\n",timer.time() * 1000.0);
      }

      if (symbol_dump_)
      {
         symbol_list_t symbol_list;

         parser_.dec().symbols(symbol_list);

         printf("---- Symbols ----\n");
         perform_symbol_dump(symbol_list);
         printf("-----------------\n");
      }

      if (assignment_dump_)
      {
         symbol_list_t assignment_list;

         parser_.dec().assignment_symbols(assignment_list);

         printf("---- Assignments ----\n");
         perform_symbol_dump(assignment_list);
         printf("---------------------\n");
      }
   }

   void perform_symbol_dump(const symbol_list_t& variable_list) const
   {
      for (std::size_t i = 0; i < variable_list.size(); ++i)
      {
         const symbol_t& symbol = variable_list[i];

         switch (symbol.second)
         {
            case parser_t::e_st_variable : printf("[%02d] Variable  %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            case parser_t::e_st_vector   : printf("[%02d] Vector    %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            case parser_t::e_st_string   : printf("[%02d] String    %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            case parser_t::e_st_function : printf("[%02d] Function  %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            case parser_t::e_st_local_variable
                                         : printf("[%02d] LocalVar  %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            case parser_t::e_st_local_vector
                                         : printf("[%02d] LocalVec  %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            case parser_t::e_st_local_string
                                         : printf("[%02d] LocalStr  %s\n",
                                                  static_cast<int>(i),symbol.first.c_str());
                                           break;

            default                      : break;
         }
      }
   }

   void process_from_file(const std::string& file_name)
   {
      if (file_name.empty())
         return;

      std::ifstream stream(file_name.c_str());

      if (!stream)
      {
         printf("ERROR: Failed to open file: %s\n\n",file_name.c_str());
         return;
      }

      std::string program(
                           (std::istreambuf_iterator<char>(stream)),
                           (std::istreambuf_iterator<char>())
                         );

      process(program);
   }

   void process_multiline()
   {
      std::string program;

      for ( ; ; )
      {
         std::string line;

         std::cout << ">> ";
         std::getline(std::cin,line);

         if (line.empty())
            continue;
         else if ("$end" == line)
            break;
         else
            program += (line + "\n");
      }

      process(program);
   }

   void list_symbols()
   {
      std::deque<std::pair<std::string,T> > variable_list;
      symbol_table_.get_variable_list(variable_list);

      std::size_t max_varname_length = 0;

      for (std::size_t i = 0; i < variable_list.size(); ++i)
      {
         max_varname_length = std::max(max_varname_length,variable_list[i].first.size());
      }

      for (std::size_t i = 0; i < variable_list.size(); ++i)
      {
         int pad_length = 0;

         if (max_varname_length > variable_list[i].first.size())
         {
            pad_length = max_varname_length - variable_list[i].first.size();
         }

         printf("%02d %s%*.*s %25.10f\n",
                static_cast<unsigned int>(i),
                variable_list[i].first.c_str(),
                pad_length,
                pad_length,
                std::string(max_varname_length,' ').c_str(),
                variable_list[i].second);
      }
   }

   void process_directive(const std::string expression)
   {
      if ('$' != expression[0])
         return;
      if ("$enable_cache" == expression)
         persist_symbol_table() = true;
      else if ("$disable_cache" == expression)
         persist_symbol_table() = false;
      else if ("$enable_symbol_dump" == expression)
         symbol_dump() = true;
      else if ("$disable_symbol_dump" == expression)
         symbol_dump() = false;
      else if ("$enable_assignment_dump" == expression)
         assignment_dump() = true;
      else if ("$disable_assignment_dump" == expression)
         assignment_dump() = false;
      else if ("$enable_timer" == expression)
         display_total_time() = true;
      else if ("$enable_compile_timer" == expression)
         display_total_compile_time() = true;
      else if ("$disable_timer" == expression)
         display_total_time() = false;
      else if ("$disable_compile_timer" == expression)
         display_total_compile_time() = false;
      else if ("$enable_usr" == expression)
         enable_usr() = true;
      else if ("$disable_usr" == expression)
         enable_usr() = false;
      else if ("$list_vars" == expression)
         list_symbols();
      else if (expression.find("$load ") == 0 && (expression.size() > 7))
         process_from_file(expression.substr(6,expression.size() - 6));
      else if ("$begin" == expression)
         process_multiline();
      else
         printf("\nERROR - Invalid directive: %s\n",expression.c_str());
   }

private:

   bool persist_symbol_table_;
   bool symbol_dump_;
   bool assignment_dump_;
   bool display_total_time_;
   bool display_total_compile_time_;
   bool enable_usr_;
   symbol_table_t symbol_table_;
   parser_t parser_;
   putch  <T> putch_;
   putint <T> putint_;
   exprtk::helper::print<T>   print_;
   exprtk::helper::println<T> println_;
   exprtk::polynomial<T, 1> poly01_;
   exprtk::polynomial<T, 2> poly02_;
   exprtk::polynomial<T, 3> poly03_;
   exprtk::polynomial<T, 4> poly04_;
   exprtk::polynomial<T, 5> poly05_;
   exprtk::polynomial<T, 6> poly06_;
   exprtk::polynomial<T, 7> poly07_;
   exprtk::polynomial<T, 8> poly08_;
   exprtk::polynomial<T, 9> poly09_;
   exprtk::polynomial<T,10> poly10_;
   exprtk::polynomial<T,11> poly11_;
   exprtk::polynomial<T,12> poly12_;
   std::string local_str_[4];
};

template <typename T>
void repl(int argc, char* argv[])
{
   expression_processor<T> processor;

   if (argc > 1)
   {
      for (int i = 1; i < argc; ++i)
      {
         processor.process_from_file(argv[i]);
      }
   }
   else
   {
      for ( ; ; )
      {
         std::string expression;

         std::cout << ">> ";
         std::getline(std::cin,expression);

         if (expression.empty())
            continue;
         else if ("exit" == expression)
            break;
         else if ("quit" == expression)
            break;
         else if ('$' == expression[0])
            processor.process_directive(expression);
         else
            processor.process(expression);
      }
   }
}

int main(int argc, char* argv[])
{
   repl<double>(argc,argv);
   return 0;
}


/*

Example REPL Instructions:
 Step 1.1 Enter: var x := 3; var y := 4; var z := x + y; println(z / 2);
 Step 1.2 Enter: var x := 3; var y := 4; var z := x - y; putint(z / 2);


 Step 2.Enter the following multi-line program:
 --- snip ---
 $begin
 x := 3;
 y := 4;
 z := sin(x / pi) * cos(y * pi);
 println(result := z / 2);
 result;
 $end
 --- snip ---


 Step 3.1 Create a new file called mandelbrot.txt
 Step 3.2 Copy the contents between the snip lines into the file
 Step 3.3 Execute: ./exprtk_repl mandelbrot.txt


 Step 4.1 Execute: ./exprtk_repl
 Step 4.2 Enter: $load mandelbrot.txt

---- snip ----
var width    := 118;
var height   := 41;
var imag_max := +1;
var imag_min := -1;
var real_max := +1;
var real_min := -2.5;
var x_step   := (real_max - real_min) / width;
var y_step   := (imag_max - imag_min) / height;
for (var y := 0; y < height; y += 1)
{
  var imag := imag_min + (y_step * y);
  for (var x := 0; x < width; x += 1)
  {
    var real   := real_min + x_step * x;
    var z_real := real;
    var z_imag := imag;
    var plot_value := 0;
    for (var n := 0; n < 30; n += 1)
    {
      var a := z_real^2;
      var b := z_imag^2;
      plot_value := n;
      if ((a + b) < 4)
      {
        z_imag := 2 * z_real * z_imag + imag;
        z_real := a - b + real;
      }
      else
        break;
    };
    putch(61 - plot_value);
  };
  println()
}
---- snip ----

*/