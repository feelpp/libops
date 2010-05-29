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
    Set(string name, string constraint, const TD& default_value, T& value);
    template<class T>
    void Set(string name, string constraint, T& value);
    template<class T>
    void Set(string name, T& value);
    template<class T>
    T Get(string name);
    template<class T>
    T Get(string name, string constraint);
    template<class T>
    T Get(string name, string constraint, const T& default_value);
    std::vector<string> GetEntryList(string name = "");
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
    bool Convert(int index, bool& output, string name = "");
    bool Convert(int index, int& output, string name = "");
    bool Convert(int index, float& output, string name = "");
    bool Convert(int index, double& output, string name = "");
    bool Convert(int index, string& output, string name = "");
    template<class TD, class T>
    void SetValue(string name, string constraint,
                  const TD& default_value, bool with_default, T& value);
    template<class T>
    void SetValue(string name, string constraint,
                  const std::vector<T>& default_value, bool with_default,
                  std::vector<T>& value);
    string Constraint(string constraint) const;
    string Name(const string& name) const;
    string Entry(const string& name) const;
    void Split(string str, std::vector<string>& vect,
               string delimiters = " \n\t") const;
    std::vector<string> Split(string str, string delimiters = " \n\t");
  };

}


#define OPS_FILE_CLASSOPS_HXX
#endif
