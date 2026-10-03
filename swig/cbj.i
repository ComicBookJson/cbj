%module cbj

%{
#include "Cbj.hpp"
#include "CbjBuilder.hpp"
#include "StringImageHelper.hpp"
#include "CbjLog.hpp"
%}

/*
 * CbjzSchema.hpp is a public C++ model, but its JSON serialization helpers
 * are implementation details and must not become language bindings.
 * In particular, exposing nlohmann::json causes SWIG to emit wrappers using
 * an unqualified "json" type which is only an alias inside namespace cbjz.
 */
%ignore cbjz::get_untyped;
%ignore cbjz::get_heap_optional;
%ignore cbjz::get_stack_optional;
%ignore cbjz::CheckConstraint;
%ignore cbjz::from_json;
%ignore cbjz::to_json;
%ignore cbjz::ClassMemberConstraints;
%ignore cbjz::ClassMemberConstraintException;
%ignore cbjz::ValueTooLowException;
%ignore cbjz::ValueTooHighException;
%ignore cbjz::ValueTooShortException;
%ignore cbjz::ValueTooLongException;
%ignore cbjz::InvalidPatternException;
%ignore cbj::CbjLog::SetCallback;

%pragma(java) jniclasscode=%{
  static {
    System.loadLibrary("cbz");
  }
%}

%include <std_string.i>
%include <std_vector.i>
%include "../core/include/CbjzSchema.hpp"
%include "../core/include/Cbj.hpp"
%include "../core/include/CbjBuilder.hpp"
%include "../core/include/StringImageHelper.hpp"
%include "../core/include/CbjLog.hpp"
