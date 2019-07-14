// fromhttps://gist.github.com/comargo/f7fe244e539325cef0f66704b58dc35a
#pragma once
#include <sstream>

namespace dbg {
  template<class CharT, class TraitsT = std::char_traits<CharT> >
  class basic_dostream :
    public std::basic_ostream<CharT, TraitsT> {
  public:
    basic_dostream();
    ~basic_dostream();
  };

  typedef basic_dostream<char> dostream;
  typedef basic_dostream<wchar_t> wdostream;
  extern dostream cout;
  extern wdostream wcout;
}