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


%module ops
%{
#include "OpsHeader.hxx"
  %}

%include "typemaps.i"
%include "std_string.i"
%include "std_vector.i"
using namespace std;

namespace std
{
  %template(VectBool) vector<bool>;
  %template(VectInt) vector<int>;
  %template(VectFloat) vector<float>;
  %template(VectDouble) vector<double>;
  %template(VectString) vector<string>;
}

%include "Error.hxx"
%exception
{
  try
    {
      $action
	}
  catch(Ops::Error& e)
    {
      PyErr_SetString(PyExc_Exception, e.What().c_str());
      return NULL;
    }
  catch(std::exception& e)
    {
      PyErr_SetString(PyExc_Exception, e.what());
      return NULL;
    }
  catch(std::string& s)
    {
      PyErr_SetString(PyExc_Exception, s.c_str());
      return NULL;
    }
  catch(const char* s)
    {
      PyErr_SetString(PyExc_Exception, s);
      return NULL;
    }
  catch(...)
    {
      PyErr_SetString(PyExc_Exception, "Unknown exception...");
      return NULL;
    }
}

%include "OpsHeader.hxx"
%include "ClassOps.hxx"


%define OPS_INSTANTIATE_ALL(suffix, type)
%template(Get ## suffix) Get<type >;
%template(Is ## suffix) Is<type >;
%enddef

namespace Ops
{

  %extend Ops
  {
    OPS_INSTANTIATE_ALL(Bool, bool);
    OPS_INSTANTIATE_ALL(Int, int);
    OPS_INSTANTIATE_ALL(Float, float);
    OPS_INSTANTIATE_ALL(Double, double);
    OPS_INSTANTIATE_ALL(String, string);
    OPS_INSTANTIATE_ALL(VectBool, std::vector<bool>);
    OPS_INSTANTIATE_ALL(VectInt, std::vector<int>);
    OPS_INSTANTIATE_ALL(VectFloat, std::vector<float>);
    OPS_INSTANTIATE_ALL(VectDouble, std::vector<double>);
    OPS_INSTANTIATE_ALL(VectString, std::vector<string>);
  };

}
