// Copyright (C) 2010, Vivien Mallet
//
// This file is part Ops, a library for parsing Lua configuration files.
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


#ifndef OPS_FILE_CLASSOPS_CXX


#include "ClassOps.hxx"


namespace Ops
{


  /////////////////////////////////
  // CONSTRUCTORS AND DESTRUCTOR //
  /////////////////////////////////


  //! Default constructor.
  /*! Nothing is performed. The Lua state is set to NULL.
   */
  Ops::Ops():
    state_(NULL)
  {
  }


  //! Main constructor.
  /*! The Lua configuration file is loaded and run. An exception may be raised
    during this evaluation.
    \param[in] file_path path to the configuration file.
  */
  Ops::Ops(string file_path):
    file_path_(file_path), state_(NULL)
  {
    Open(file_path_);
  }


  //! Destructor.
  /*! Destroys the Lua state object.
   */
  Ops::~Ops()
  {
    Close();
  }


  //////////////////
  // MAIN METHODS //
  //////////////////


  //! Opens a new configuration file.
  /*! The previous configuration file (if any) is closed. The prefix is
    cleared.
    \param[in] file_path path to the configuration file.
  */
  void Ops::Open(string file_path)
  {
    Close();
    ClearPrefix();
    file_path_ = file_path;
    state_ = lua_open();
    luaL_openlibs(state_);
    if (luaL_dofile(state_, file_path_.c_str()))
      Error("Open(string)", lua_tostring(state_, -1));

    // Defines 'ops_in' for the user. It checks whether an element is in a
    // table.
    string code = "function ops_in(v, table)\
    for _, value in ipairs(table) do        \
        if v == value then                  \
            return true                     \
        end                                 \
    end                                     \
    return false                            \
    end";
    if (luaL_dostring(state_, code.c_str()))
      Error("Open(string)", lua_tostring(state_, -1));
  }


  //! Closes the configuration file (if any is open).
  /*! Destroys the Lua state object. The prefix is cleared.
    \param[in] file_path path to the configuration file.
  */
  void Ops::Close()
  {
    ClearPrefix();
    if (state_ != NULL)
      lua_close(state_);
  }


  template<class TD, class T>
  void
  Ops::Get(string name, string constraint, const TD& default_value, T& value)
  {
    GetValue(name, constraint, default_value, true, value);
  }


  template<class T>
  void Ops::Get(string name, string constraint, T& value)
  {
    GetValue(name, constraint, value, false, value);
  }


  template <class T>
  void Ops::Get(string name, T& value)
  {
    GetValue(name, "", value, false, value);
  }


  //! Checks that a certain entry satisfies a constraint.
  /*!
    \param[in] name the name of the entry whose consistency with \a constraint
    is to be checked.
    \param[in] constraint the constraint to be satisfied.
    \return True if the constraint is satisfied, false otherwise.
  */
  bool Ops::CheckConstraint(string name, string constraint)
  {
    if (constraint == "")
      return true;

    string code;
    code = "function ops_check_constraint(v)\nreturn " + constraint \
      + "\nend\nops_result = ops_check_constraint(" + Name(name) + ")";
    if (luaL_dostring(state_, code.c_str()))
      Error("CheckConstraint",
            "While checking " + Entry(name) + ":\n  "
            + string(lua_tostring(state_, -1)));

    PutOnStack("ops_result");
    if (!lua_isboolean(state_, -1))
      throw Error("CheckConstraint",
                  "For " + Entry(name) + ", the following constraint did "
                  "not return a Boolean:\n" + Constraint(constraint));

    return static_cast<bool>(lua_toboolean(state_, -1));
  }


  //! Puts \a name on top of the stack.
  /*! If \a name is a simple variable, it calls 'lua_getglobal' once. But if
    \a name is encapsulated in a table, this method iterates until it finds
    the variable.
    \param[in] name the name of the entry to be put on top of the stack.
    \note The prefix is not prepended to \a name.
  */
  void Ops::PutOnStack(string name)
  {
    vector<string> tree = Split(name, ".");

    lua_getglobal(state_, tree[0].c_str());
    if (tree.size() == 1)
      // No table involved here.
      return;

    for (int i = 1; i < int(tree.size()); i++)
      {
        if (lua_isnil(state_, -1))
          return;
        lua_pushstring(state_, tree[i].c_str());
        lua_gettable(state_, -2);
      }
  }


  //! Clears the stack.
  void Ops::ClearStack()
  {
    lua_pop(state_, lua_gettop(state_));
  }


  ////////////////////
  // ACCESS METHODS //
  ////////////////////


  //! Returns the path to the configuration file.
  /*!
    \return The path to the configuration file.
  */
  string Ops::GetFilePath() const
  {
    return file_path_;
  }


  //! Returns the Lua state object.
  /*!
    \return The Lua state object.
  */
  lua_State* Ops::GetState()
  {
    return state_;
  }


  //! Returns the Lua state object.
  /*!
    \return The Lua state object.
  */
  const lua_State* Ops::GetState() const
  {
    return state_;
  }


  //! Returns the current prefix.
  /*!
    \return The current prefix.
  */
  string Ops::GetPrefix() const
  {
    return prefix_;
  }


  //! Sets the prefix.
  /*!
    \param[in] prefix the new prefix.
  */
  void Ops::SetPrefix(string prefix)
  {
    prefix_ = prefix;
  }


  //! Clears the current prefix.
  void Ops::ClearPrefix()
  {
    prefix_ = "";
  }


  ///////////////////////
  // PROTECTED METHODS //
  ///////////////////////


  //! Retrieves a value and checks if it satisfies given constraints.
  /*! If the entry is not found, the default value is returned (if any).
    \param[in] name name of the entry.
    \param[in] constraint constraint to be satisfied.
    \param[in] default_value default value.
    \param[in] with_default is there a default value? If not, \a default_value
    is ignored.
    \param[out] value the value of the entry named \a name.
    \note The default value may not satisfy the constraint.
  */
  void Ops::GetValue(string name, string constraint,
                     const int& default_value, bool with_default,
                     int& value)
  {
    PutOnStack(Name(name));

    if (lua_isnil(state_, -1))
      if (with_default)
        {
          value = default_value;
          ClearStack();
          return;
        }
      else
        throw Error("GetValue(int&)",
                    "The " + Entry(name) + " was not found.");

    if (!lua_isnumber(state_, -1))
      throw Error("GetValue(int&)",
                  "The " + Entry(name) + " is not an integer.");
    double number = static_cast<double>(lua_tonumber(state_, -1));
    value = static_cast<int>(number);
    if (static_cast<double>(value) != number)
      throw Error("GetValue(int&)",
                  "The " + Entry(name) + " is not an integer.");

    if (!CheckConstraint(name, constraint))
      throw Error("GetValue(int&)",
                  "The " + Entry(name) + " does not satisfy "
                  + "the constraint:\n" + Constraint(constraint));

    ClearStack();
  }


  //! Retrieves a value and checks if it satisfies given constraints.
  /*! If the entry is not found, the default value is returned (if any).
    \param[in] name name of the entry.
    \param[in] constraint constraint to be satisfied.
    \param[in] default_value default value.
    \param[in] with_default is there a default value? If not, \a default_value
    is ignored.
    \param[out] value the value of the entry named \a name.
    \note The default value may not satisfy the constraint.
  */
  void Ops::GetValue(string name, string constraint,
                     const float& default_value, bool with_default,
                     float& value)
  {
    PutOnStack(Name(name));

    if (lua_isnil(state_, -1))
      if (with_default)
        {
          value = default_value;
          ClearStack();
          return;
        }
      else
        throw Error("GetValue(float&)",
                    "The " + Entry(name) + " was not found.");

    if (!lua_isnumber(state_, -1))
      throw Error("GetValue(float&)",
                  "The " + Entry(name) + " is not a float.");

    value = static_cast<float>(lua_tonumber(state_, -1));

    if (!CheckConstraint(name, constraint))
      throw Error("GetValue(float&)",
                  "The " + Entry(name) + " does not satisfy "
                  + "the constraint:\n" + Constraint(constraint));

    ClearStack();
  }


  //! Retrieves a value and checks if it satisfies given constraints.
  /*! If the entry is not found, the default value is returned (if any).
    \param[in] name name of the entry.
    \param[in] constraint constraint to be satisfied.
    \param[in] default_value default value.
    \param[in] with_default is there a default value? If not, \a default_value
    is ignored.
    \param[out] value the value of the entry named \a name.
    \note The default value may not satisfy the constraint.
  */
  void Ops::GetValue(string name, string constraint,
                     const double& default_value, bool with_default,
                     double& value)
  {
    PutOnStack(Name(name));

    if (lua_isnil(state_, -1))
      if (with_default)
        {
          value = default_value;
          ClearStack();
          return;
        }
      else
        throw Error("GetValue(double&)",
                    "The " + Entry(name) + " was not found.");

    if (!lua_isnumber(state_, -1))
      throw Error("GetValue(double&)",
                  "The " + Entry(name) + " is not a double.");

    value = static_cast<double>(lua_tonumber(state_, -1));

    if (!CheckConstraint(name, constraint))
      throw Error("GetValue(double&)",
                  "The " + Entry(name) + " does not satisfy "
                  + "the constraint:\n" + Constraint(constraint));

    ClearStack();
  }


  //! Retrieves a value and checks if it satisfies given constraints.
  /*! If the entry is not found, the default value is returned (if any).
    \param[in] name name of the entry.
    \param[in] constraint constraint to be satisfied.
    \param[in] default_value default value.
    \param[in] with_default is there a default value? If not, \a default_value
    is ignored.
    \param[out] value the value of the entry named \a name.
    \note The default value may not satisfy the constraint.
  */
  void Ops::GetValue(string name, string constraint,
                     const string& default_value, bool with_default,
                     string& value)
  {
    PutOnStack(Name(name));

    if (lua_isnil(state_, -1))
      if (with_default)
        {
          value = default_value;
          ClearStack();
          return;
        }
      else
        throw Error("GetValue(string&)",
                    "The " + Entry(name) + " was not found.");

    if (!lua_isstring(state_, -1))
      throw Error("GetValue(string&)",
                  "The " + Entry(name) + " is not a string.");

    value = static_cast<string>(lua_tostring(state_, -1));

    if (!CheckConstraint(name, constraint))
      throw Error("GetValue(string&)",
                  "The " + Entry(name) + " does not satisfy "
                  + "the constraint:\n" + Constraint(constraint));

    ClearStack();
  }


  //! Prepends the prefix to an entry name.
  /*!
    \param[in] name name of the entry.
    \return The entry name with the prefix prepended.
  */
  string Ops::Name(const string& name) const
  {
    return prefix_ + name;
  }


  //! Formats the description of an entry.
  /*!
    \param[in] name name of the entry.
    \return A string with the entry name and the path to the configuration
    file, both quoted.
  */
  string Ops::Entry(const string& name) const
  {
    return "entry \"" + Name(name) + "\" in \"" + file_path_ + "\"";
  }


  //! Formats the description of a constraint.
  /*!
    \param[in] constraint the constraint to be formatted.
    \return A string with the constraint properly formatted.
  */
  string Ops::Constraint(string constraint) const
  {
    constraint = "      " + constraint;
    if (constraint.find("ops_in", 0) != string::npos)
      constraint += "\n      Note: 'ops_in(v, array)' checks whether 'v' is "
        "part of the list 'array'.";
    return constraint;
  }


  //! Splits a string.
  /*!  The string is split according to delimiters and elements are stored in
    the vector \a vect.
    \param[in] str string to be split.
    \param[out] vect (output) vector containing the elements of the string.
    \param[in] delimiters (optional) delimiters. Default: " \n\t".
  */
  void Ops::Split(string str, vector<string>& vect, string delimiters) const
  {
    vect.clear();

    string tmp;
    string::size_type index_beg, index_end;

    index_beg = str.find_first_not_of(delimiters);

    while (index_beg != string::npos)
      {
	index_end = str.find_first_of(delimiters, index_beg);
	tmp = str.substr(index_beg, index_end == string::npos ?
                         string::npos : (index_end - index_beg));
	vect.push_back(tmp);
	index_beg = str.find_first_not_of(delimiters, index_end);
      }
  }


  //! Splits a string.
  /*!
    The string is split according to delimiters.
    \param[in] str string to be split.
    \param[in] delimiters (optional) delimiters. Default: " \n\t".
    \return A vector containing elements of the string.
  */
  vector<string> Ops::Split(string str, string delimiters)
  {
    vector<string> vect;
    Split(str, vect, delimiters);
    return vect;
  }


}


#define OPS_FILE_CLASSOPS_CXX
#endif
