#include <iostream>
using namespace std;

#define OPS_WITH_ABORT
#include "Ops.hxx"


int main(int argc, char *argv[])
{
  int integer;
  double number;
  string str;
  vector<string> str_vector;
  vector<int> int_vector;

  Ops::Ops ops("example.lua");

  /*** Basic access ***/

  ops.Set("last_name", str);
  cout << "Last name: " << str << endl;
  // Alternatively, it is always possible to use the template method
  // 'Get'. 'Get' returns the value while 'Set' assigns it to the last
  // parameter.
  cout << "Full name: " << ops.Get<string>("full_name") << endl;

  ops.Set("birth_year", integer);
  cout << "Birth year: " << integer << endl;

  ops.Set("nationality", str_vector);
  cout << "Nationality: " << str_vector[0] << ", " << str_vector[1] << endl;

  /*** List of entries ***/

  str_vector = ops.GetEntryList("name");
  cout << "Entries in \"name\": " << str_vector[0] << ", " << str_vector[1]
       << ", " << str_vector[2] << endl;

  cout << "Middle name: " << ops.Get<string>("name." + str_vector[1]) << endl;
  // One may use a prefix.
  ops.SetPrefix("name.");
  cout << "Middle name: " << ops.Get<string>(str_vector[1]) << endl;

  /*** Constraints ***/

  ops.ClearPrefix(); // removes the prefix.

  // An age should be greater than 0 and less than, say, 150. It is possible
  // to check it with a logical expression (written in Lua). The expression
  // should be written with 'v' being the variable to be checked.
  ops.Set("death_age", "v >= 0 and v < 150", integer);

  // It is possible to check whether a variable is in a set of acceptable
  // value. This is performed with 'ops_in' (a Lua function defined by Ops).
  str = ops.Get<string>("one_composition",
                        "ops_in(v, {'Messiah', 'Water Music'})");

  // If a vector is retrieved, the constraint must be satisfied on every
  // element of the vector.
  ops.Set("compositions.concerti_grossi_op_6",
          "(v % 2 == 0 or v % 2 == 1) and v < 13", int_vector);

  /*** Default values ***/

  // It is possible to set a variable to a default value if it is not
  // available in the configuration file. In this case, it is necessary to
  // give a constraint as well (second paramater), but it may be empty.
  bool show = ops.Get<bool>("Show_compositions", "", true);

  return 0;
}
