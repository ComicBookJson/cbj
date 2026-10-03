%module cbj
%{
#include "Cbj.hpp"
#include "CbjBuilder.hpp"
#include "StringImageHelper.hpp"
%}

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
