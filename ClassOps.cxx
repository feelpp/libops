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


  //! Retrieves a value from the configuration file.
  /*!
    \param[in] name name of the entry.
    \param[in] constraint constraint that the entry value must satisfy.
    \param[in] default_value default value for the entry in case it is not
    found in the configuration file.
    \param[out] value value of the entry.
  */
  template<class TD, class T>
  void
  Ops::Set(string name, string constraint, const TD& default_value, T& value)
  {
    SetValue(name, constraint, default_value, true, value);
  }


  //! Retrieves a value from the configuration file.
  /*!
    \param[in] name name of the entry.
    \param[in] constraint constraint that the entry value must satisfy.
    \param[out] value value of the entry.
  */
  template<class T>
  void Ops::Set(string name, string constraint, T& value)
  {
    SetValue(name, constraint, value, false, value);
  }


  //! Retrieves a value from the configuration file.
  /*!
    \param[in] name name of the entry.
    \param[out] value value of the entry.
  */
  template <class T>
  void Ops::Set(string name, T& value)
  {
    SetValue(name, "", value, false, value);
  }


  //! Retrieves a value from the configuration file.
  /*!
    \param[in] name name of the entry.
    \param[in] constraint constraint that the entry value must satisfy.
    \param[in] default_value default value for the entry in case it is not
    found in the configuration file.
    \return The value of the entry.
  */
  template<class T>
  T Ops::Get(string name, string constraint, const T& default_value)
  {
    T value;
    SetValue(name, constraint, default_value, true, value);
    return value;
  }


  //! Retrieves a value from the configuration file.
  /*!
    \param[in] name name of the entry.
    \param[in] constraint constraint that the entry value must satisfy.
    \return The value of the entry.
  */
  template<class T>
  T Ops::Get(string name, string constraint)
  {
    T value;
    SetValue(name, constraint, value, false, value);
    return value;
  }


  //! Retrieves a value from the configuration file.
  /*!
    \param[in] name name of the entry.
    \return The value of the entry.
  */
  template <class T>
  T Ops::Get(string name)
  {
    T value;
    SetValue(name, "", value, false, value);
    return value;
  }


  //! Returns the list of entries inside an entry.
  /*!
    \param[in] name name of the entry to search in.
    \return The list of entries under \a name, sorted in alphabetical order
    (with numbers coming first).
    \note The prefix is prepended to \a name before the search. If the entry
    \a name does not exist or does not contain other entries, an exception is
    raised.
  */
  std::vector<string> Ops::GetEntryList(string name)
  {
    PutOnStack(Name(name));

    if (lua_isnil(state_, -1))
      throw Error("GetEntryList",
                  "The " + Entry(name) + " was not found.");

    if (!lua_istable(state_, -1))
      throw Error("GetEntryList",
                  "The " + Entry(name) + " does not contain other entries.");

    std::vector<string> key_list;
    string key;
    // Now loops over all elements of the table.
    lua_pushnil(state_);
    while (lua_next(state_, -2) != 0)
      {
        // Duplicates the key so that 'lua_tostring' (applied to it) should
        // not interfere with 'lua_next'.
        lua_pushvalue(state_, -2);

        if (!Convert(-1, key))
          throw Error("GetEntryList",
                      "Unable to read the keys of " + Entry(name) + ".");
        key_list.push_back(key);

        lua_pop(state_, 2);
      }

    // Sorts the keys.
    std::sort(key_list.begin(), key_list.end());

    ClearStack();

    return key_list;
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
    if (name.empty())
      {
        lua_pushvalue(state_, LUA_GLOBALSINDEX);
        return;
      }

    size_t end = name.find_first_of(".[");
    if (end == 0)
      // The name starts with '.' or '[': wrong syntax.
      {
        lua_pushnil(state_);
        return;
      }
    if (end == string::npos)
      {
        lua_getglobal(state_, name.c_str());
        return;
      }

    lua_getglobal(state_, name.substr(0, end).c_str());

    if (name[end] == '.')
      WalkDown(name.substr(end + 1).c_str());
    else
      WalkDown(name.substr(end).c_str());
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


  //! Converts an element of the stack to a Boolean.
  /*!
    \param[in] index index in the stack.
    \param[out] output converted value.
    \param[in] name name of the entry.
    \return True if the conversion was successful, false otherwise.
    \note If \a name is not empty and if the conversion fails, an exception is
    raised by this method. This exception gives the name of the entry and
    states that it could not be converted. If \a name is empty, no exception
    is raised.
  */
  bool Ops::Convert(int index, bool& output, string name)
  {
    if (!lua_isboolean(state_, index))
      if (name.empty())
        return false;
      else
        throw Error("Convert(bool&)",
                    "The " + Entry(name) + " is not a Boolean.");

    output = static_cast<bool>(lua_toboolean(state_, index));
    return true;
  }


  //! Converts an element of the stack to an integer.
  /*!
    \param[in] index index in the stack.
    \param[out] output converted value.
    \param[in] name name of the entry.
    \return True if the conversion was successful, false otherwise.
    \note If \a name is not empty and if the conversion fails, an exception is
    raised by this method. This exception gives the name of the entry and
    states that it could not be converted. If \a name is empty, no exception
    is raised.
  */
  bool Ops::Convert(int index, int& output, string name)
  {
    if (!lua_isnumber(state_, index))
      if (name.empty())
        return false;
      else
        throw Error("Convert(int&)",
                    "The " + Entry(name) + " is not an integer.");

    double number = static_cast<double>(lua_tonumber(state_, index));
    int value = static_cast<int>(number);
    if (static_cast<double>(value) != number)
      if (name.empty())
        return false;
      else
        throw Error("Convert(int&)",
                    "The " + Entry(name) + " is not an integer.");

    output = value;
    return true;
  }


  //! Converts an element of the stack to a float.
  /*!
    \param[in] index index in the stack.
    \param[out] output converted value.
    \param[in] name name of the entry.
    \return True if the conversion was successful, false otherwise.
    \note If \a name is not empty and if the conversion fails, an exception is
    raised by this method. This exception gives the name of the entry and
    states that it could not be converted. If \a name is empty, no exception
    is raised.
  */
  bool Ops::Convert(int index, float& output, string name)
  {
    if (!lua_isnumber(state_, index))
      if (name.empty())
        return false;
      else
        throw Error("Convert(float&)",
                    "The " + Entry(name) + " is not a float.");

    output = static_cast<float>(lua_tonumber(state_, index));
    return true;
  }


  //! Converts an element of the stack to a double.
  /*!
    \param[in] index index in the stack.
    \param[out] output converted value.
    \param[in] name name of the entry.
    \return True if the conversion was successful, false otherwise.
    \note If \a name is not empty and if the conversion fails, an exception is
    raised by this method. This exception gives the name of the entry and
    states that it could not be converted. If \a name is empty, no exception
    is raised.
  */
  bool Ops::Convert(int index, double& output, string name)
  {
    if (!lua_isnumber(state_, index))
      if (name.empty())
        return false;
      else
        throw Error("Convert(double&)",
                    "The " + Entry(name) + " is not a double.");

    output = static_cast<double>(lua_tonumber(state_, index));
    return true;
  }


  //! Converts an element of the stack to a string.
  /*!
    \param[in] index index in the stack.
    \param[out] output converted value.
    \param[in] name name of the entry.
    \return True if the conversion was successful, false otherwise.
    \note If \a name is not empty and if the conversion fails, an exception is
    raised by this method. This exception gives the name of the entry and
    states that it could not be converted. If \a name is empty, no exception
    is raised.
  */
  bool Ops::Convert(int index, string& output, string name)
  {
    if (!lua_isstring(state_, index))
      if (name.empty())
        return false;
      else
        throw Error("Convert(string&)",
                    "The " + Entry(name) + " is not a string.");

    output = static_cast<string>(lua_tostring(state_, index));
    return true;
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
  template<class TD, class T>
  void Ops::SetValue(string name, string constraint,
                     const TD& default_value, bool with_default,
                     T& value)
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
        throw Error("SetValue",
                    "The " + Entry(name) + " was not found.");

    Convert(-1, value, name);

    if (!CheckConstraint(name, constraint))
      throw Error("SetValue",
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
  template<class T>
  void Ops::SetValue(string name, string constraint,
                     const std::vector<T>& default_value, bool with_default,
                     std::vector<T>& value)
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
        throw Error("SetValue",
                    "The " + Entry(name) + " was not found.");

    if (!lua_istable(state_, -1))
      throw Error("SetValue",
                  "The " + Entry(name) + " is not a table.");

    std::vector<T> element_list;
    T element;
    std::vector<string> key_list;
    string key;
    // Now loops over all elements of the table.
    lua_pushnil(state_);
    while (lua_next(state_, -2) != 0)
      {
        // Duplicates the key and value so that 'lua_tostring' (applied to
        // them) should not interfere with 'lua_next'.
        lua_pushvalue(state_, -2);
        lua_pushvalue(state_, -2);

        if (!Convert(-2, key))
          throw Error("SetValue",
                      "Unable to read the keys of " + Entry(name) + ".");
        key_list.push_back(key);

        Convert(-1, element, name + "[" + key + "]");
        element_list.push_back(element);

        lua_pop(state_, 3);
      }

    for (int i = 0; i < int(key_list.size()); i++)
      if (!CheckConstraint(name + "[" + key_list[i] + "]", constraint))
        throw Error("SetValue",
                    "The " + Entry(name + "[" + key_list[i] + "]")
                    + " does not satisfy the constraint:\n"
                    + Constraint(constraint));

    value = element_list;

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


  //! Iterates over the elements of \a name to put it on stack.
  /*! For instance, if "table.subtable.subsubtable[2]" is to be accessed, one
    should put "table" on top of the stack and call
    'WalkDown(subtable.subsubtable[2])'. This method will put the value of
    "table.subtable.subsubtable[2]" on top of the stack, or nil is any error
    occurred.
    \param[in] name the name of an entry that is accessible from the entry
    currently on top of the stack.
  */
  void Ops::WalkDown(string name)
  {
    if (name.empty() || lua_isnil(state_, -1))
      return;

    // The sub-entries are introduced with "." or "[i]".
    size_t end = name.find_first_of(".[");

    if (end == string::npos)
      // No more sub-entry here.
      {
        lua_pushstring(state_, name.c_str());
        lua_gettable(state_, -2);
        return;
      }

    if (name[end] == '.')
      // One step down.
      {
        lua_pushstring(state_, name.substr(0, end).c_str());
        lua_gettable(state_, -2);
        WalkDown(name.substr(end + 1).c_str());
        return;
      }

    if (name[end] == '[')
      if (end == 0)
        // Access to an element through "[i]".
        {
          // First getting the index.
          size_t end_index = name.find_first_of("]");
          if (end_index <= end + 1 || end_index == string::npos)
            // Syntax error: "]" was not found or is misplaced.
            {
              lua_pushnil(state_);
              return;
            }
          string index_str = name.substr(end + 1, end_index - end - 1);
          // Checks whether 'index_str' is an integer.
          for (int i = 0; i < int(index_str.size()); i++)
            if (!isdigit(index_str[i]))
              {
                lua_pushnil(state_);
                return;
              }
          std::istringstream str(index_str);
          int index;
          str >> index;
          // Now getting the element of index 'index'.
          lua_rawgeti(state_, -1, index);
          // And preparing for the next step down.
          string next_name = name.substr(end_index + 1).c_str();
          if (!next_name.empty() && next_name[0] == '.')
            // Removes the dot in the first place.
            next_name = next_name.substr(1);
          WalkDown(next_name);
          return;
        }
      else
        // One step down before addressing "[i]" in the next call to
        // 'WalkDown'.
        {
          lua_pushstring(state_, name.substr(0, end).c_str());
          lua_gettable(state_, -2);
          if (name[end] == '.')
            WalkDown(name.substr(end + 1).c_str());
          else
            WalkDown(name.substr(end).c_str());
          return;
        }
  }


  //! Splits a string.
  /*!  The string is split according to delimiters and elements are stored in
    the vector \a vect.
    \param[in] str string to be split.
    \param[out] vect (output) vector containing the elements of the string.
    \param[in] delimiters (optional) delimiters. Default: " \n\t".
  */
  void Ops::Split(string str, std::vector<string>& vect, string delimiters)
    const
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
  std::vector<string> Ops::Split(string str, string delimiters)
  {
    std::vector<string> vect;
    Split(str, vect, delimiters);
    return vect;
  }


}


#define OPS_FILE_CLASSOPS_CXX
#endif
