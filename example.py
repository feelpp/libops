### This file is a Python example. It is a translation of example.cpp.

# Below, only the key differences with C++ are commented. See example.cpp for
# explanations about Ops itself.

# There are two major differences with C++: there is no method 'Set'. Only
# 'Get' is available. Since Python does not understand overloads, the type is
# explicitely included in the method name: 'GetInt', 'GetString', ...

import ops

ops = ops.Ops("example.lua")

### Basic access

string = ops.GetString("last_name")
print "Last name:", string
print "Full name:", ops.GetString("full_name")

print "Birth year:", ops.GetInt("birth_year")

# Vectors are perfectly handled and they are returned as tuples.
print "Nationality:", ops.GetVectString("nationality")

### List of entries

entry_list = ops.GetEntryList("name")
print "Entries:", entry_list

print "Middle name:", ops.GetString("name." + entry_list[1])
ops.SetPrefix("name.")
print "Middle name:", ops.GetString(entry_list[1])

### Constraints

ops.ClearPrefix()

ops.GetInt("death_age", "v >= 0 and v < 150")

ops.GetString("one_composition", "ops_in(v, {'Messiah', 'Water Music'})")

ops.GetVectInt("compositions.concerti_grossi_op_6",
               "(v % 2 == 0 or v % 2 == 1) and v < 13")

### Default values

# Python Boolean are manipulated here.
ops.GetBool("Show_compositions", "", True)
