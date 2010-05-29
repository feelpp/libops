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


#ifndef OPS_FILE_CLASSOPS_HXX


namespace Ops
{

  class Ops
  {
  protected:
    //! Path to the configuration file.
    string file_path_;
    //! Lua state.
    lua_State* state_;
    //! Prefix to be prepended to the entries names.
    string prefix_;

  public:
    // Constructor and destructor.
    Ops();
    Ops(string file_path);
    ~Ops();

    // Main methods.
    void Open(string file_path);
    void Close();
    template<class TD, class T>
    void
    Get(string name, string constraint, const TD& default_value, T& value);
    template<class T>
    void Get(string name, string constraint, T& value);
    template<class T>
    void Get(string name, T& value);
    bool CheckConstraint(string name, string constraint);
    void PutOnStack(string name);
    void ClearStack();

    // Access methods.
    string GetFilePath() const;
    lua_State* GetState();
    const lua_State* GetState() const;
    string GetPrefix() const;
    void SetPrefix(string prefix);
    void ClearPrefix();

  protected:
    void GetValue(string name, string constraint,
                  const int& default_value, bool with_default,
                  int& value);
    void GetValue(string name, string constraint,
                  const float& default_value, bool with_default,
                  float& value);
    void GetValue(string name, string constraint,
                  const double& default_value, bool with_default,
                  double& value);
    void GetValue(string name, string constraint,
                  const string& default_value, bool with_default,
                  string& value);
    string Constraint(string constraint) const;
    string Name(const string& name) const;
    string Entry(const string& name) const;
    void Split(string str, vector<string>& vect, string delimiters = " \n\t")
      const;
    vector<string> Split(string str, string delimiters = " \n\t");
  };

}


#define OPS_FILE_CLASSOPS_HXX
#endif
