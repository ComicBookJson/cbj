%module cbj

%{
#include "Cbj.hpp"
#include "CbjBuilder.hpp"
#include "StringImageHelper.hpp"
#include "CbjJsonStream.hpp"
#include "CbjLog.hpp"
%}

%pragma(java) jniclasscode=%{
  static {
    System.loadLibrary("cbz");
  }
%}

%include <std_string.i>
%include <std_vector.i>
%include "../core/include/CbjSchema.hpp"
%include "../core/include/Cbj.hpp"
%include "../core/include/CbjBuilder.hpp"
%include "../core/include/StringImageHelper.hpp"
%include "../core/include/CbjJsonStream.hpp"
%include "../core/include/CbjLog.hpp"
