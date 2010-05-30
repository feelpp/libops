// Copyright (C) 2010, Vivien Mallet
//
// This file is part of Ops, a library for parsing Lua configuration files.
//
// Ops is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// Ops is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Ops. If not, see http://www.gnu.org/licenses/.


// This file is used to instantiate explicitely the template methods, so as to
// build a compiled library and the Python interface.


#ifndef OPS_FILE_OPS_CPP

#include "Ops.hxx"

#define OPS_INSTANTIATE_ALL(type)                             \
  template type Ops::Get(string);                             \
  template type Ops::Get(string, string);                     \
  template type Ops::Get(string, string, const type&);        \
  template bool Ops::Is<type >(string);                       \

namespace Ops
{
  OPS_INSTANTIATE_ALL(bool);
  OPS_INSTANTIATE_ALL(int);
  OPS_INSTANTIATE_ALL(float);
  OPS_INSTANTIATE_ALL(double);
  OPS_INSTANTIATE_ALL(string);
  OPS_INSTANTIATE_ALL(std::vector<bool>);
  OPS_INSTANTIATE_ALL(std::vector<int>);
  OPS_INSTANTIATE_ALL(std::vector<float>);
  OPS_INSTANTIATE_ALL(std::vector<double>);
  OPS_INSTANTIATE_ALL(std::vector<string>);
}


#define OPS_FILE_OPS_CPP
#endif
