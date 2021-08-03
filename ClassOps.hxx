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


#ifndef OPS_FILE_CLASSOPS_HXX

#include <map>
#include <string>
#include <vector>

namespace Ops
{

  class Ops
  {
  protected:
    //! Path to the configuration file.
    std::string file_path_;
    //! Lua state.
    lua_State* state_;
    //! Prefix to be prepended to the entries names.
    std::string prefix_;

    //! Names and values of all Booleans read in the file.
    std::map<std::string, bool> read_bool;
    //! Names and values of all integer read in the file.
    std::map<std::string, int> read_int;
    //! Names and values of all floats read in the file.
    std::map<std::string, float> read_float;
    //! Names and values of all doubles read in the file.
    std::map<std::string, double> read_double;
    //! Names and values of all strings read in the file.
    std::map<std::string, std::string> read_string;
    //! Names and values of all vectors of Booleans read in the file.
    std::map<std::string, std::vector<bool> > read_vect_bool;
    //! Names and values of all vectors of integers read in the file.
    std::map<std::string, std::vector<int> > read_vect_int;
    //! Names and values of all vectors of floats read in the file.
    std::map<std::string, std::vector<float> > read_vect_float;
    //! Names and values of all vectors of doubles read in the file.
    std::map<std::string, std::vector<double> > read_vect_double;
    //! Names and values of all vectors of strings read in the file.
    std::map<std::string, std::vector<std::string> > read_vect_string;

  public:
    // Constructor and destructor.
    Ops();
    explicit Ops(std::string file_path);
    ~Ops();

    // Main methods.
    void Open(std::string file_path, bool close_state = true);
    void Reload(bool close_state = true);
    void Close();
    template<class TD, class T>
    void
    Set(std::string name, std::string constraint, const TD& default_value, T& value);
    template<class T>
    void Set(std::string name, std::string constraint, T& value);
    template<class T>
    void Set(std::string name, T& value);
    template<class T>
    T Get(std::string name);
    template<class T>
    T Get(std::string name, std::string constraint);
    template<class T>
    T Get(std::string name, std::string constraint, const T& default_value);
    template<class Tin, class Tout>
    void Apply(std::string name, const std::vector<Tin>& in,
               std::vector<Tout>& OUTPUT);
    template<class T>
    T Apply(std::string name, const T& arg0);
    template<class T>
    T Apply(std::string name, const T& arg0, const T& arg1);
    template<class T>
    T Apply(std::string name, const T& arg0, const T& arg1, const T& arg2);
    template<class T>
    T Apply(std::string name, const T& arg0, const T& arg1, const T& arg2,
            const T& arg3);
    template<class T>
    T Apply(std::string name, const T& arg0, const T& arg1, const T& arg2,
            const T& arg3, const T& arg4);
    std::vector<std::string> GetEntryList(std::string name = "");
    bool CheckConstraint(std::string name, std::string constraint);
    bool CheckConstraintOnValue(std::string value, std::string constraint);
    void PutOnStack(std::string name);
    bool Exists(std::string name);
    void PushOnStack(bool value);
    void PushOnStack(int value);
    void PushOnStack(float value);
    void PushOnStack(double value);
    void PushOnStack(std::string value);
    template<class T>
    void PushOnStack(const std::vector<T>& v);
    template<class T>
    bool Is(std::string name);
    bool IsTable(std::string name);
    bool IsFunction(std::string name);
    void ClearStack();

    void DoFile(std::string file_path);
    void DoString(std::string expression);

    // Access methods.
    [[nodiscard]] std::string GetFilePath() const;
    lua_State* GetState();
#ifndef SWIG
    [[nodiscard]] const lua_State* GetState() const;
#endif
    [[nodiscard]] std::string GetPrefix() const;
    void SetPrefix(std::string prefix);
    void ClearPrefix();
    std::vector<std::string> GetReadEntryList();
    void UpdateLuaDefinition();
    std::string LuaDefinition(std::string name);
    std::string LuaDefinition();
    void WriteLuaDefinition(std::string file_name);

  protected:
    bool Convert(int index, std::vector<bool>::reference output,
                 std::string name = "");
    bool Convert(int index, bool& output, std::string name = "");
    bool Convert(int index, int& output, std::string name = "");
    bool Convert(int index, float& output, std::string name = "");
    bool Convert(int index, double& output, std::string name = "");
    bool Convert(int index, std::string& output, std::string name = "");
    template<class TD, class T>
    void SetValue(std::string name, std::string constraint,
                  const TD& default_value, bool with_default, T& value);
    template<class T>
    void SetValue(std::string name, std::string constraint,
                  const std::vector<T>& default_value, bool with_default,
                  std::vector<T>& value);
    [[nodiscard]] std::string Constraint(std::string constraint) const;
    [[nodiscard]] std::string Name(const std::string& name) const;
    [[nodiscard]] std::string Entry(const std::string& name) const;
    [[nodiscard]] std::string Function(const std::string& name) const;
    void WalkDown(std::string name);
    template<class T>
    bool IsParam(std::string name, T& value);
    template<class T>
    bool IsParam(std::string name, std::vector<T>& value);
    void Push(std::string name, const bool& value);
    void Push(std::string name, const int& value);
    void Push(std::string name, const float& value);
    void Push(std::string name, const double& value);
    void Push(std::string name, const std::string& value);
    void Push(std::string name, const std::vector<bool>& value);
    void Push(std::string name, const std::vector<int>& value);
    void Push(std::string name, const std::vector<float>& value);
    void Push(std::string name, const std::vector<double>& value);
    void Push(std::string name, const std::vector<std::string>& value);
    template<class TK, class T>
    void AppendKey(const std::map<TK, T>& input, std::vector<TK>& vect);
  };

}

#include <ClassOps_impl.hxx>

#define OPS_FILE_CLASSOPS_HXX
#endif
