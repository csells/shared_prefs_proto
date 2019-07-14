#include "pch.h"
#include "basic_debugbuf.h"

namespace dbg {
  dostream cout;
  wdostream wcout;

  namespace {

    template <class CharT, class TraitsT = std::char_traits<CharT> >
    class basic_debugbuf :
      public std::basic_stringbuf<CharT, TraitsT> {
    public:
      virtual ~basic_debugbuf(void) {
        sync();
      }

    protected:
      int sync() {
        output_debug_string(this->str().c_str());
        this->str(std::basic_string<CharT>()); // Clear the string buffer

        return 0;
      }

      void output_debug_string(const CharT *text) {}
    };

    template<>
    void basic_debugbuf<char>::output_debug_string(const char *text) {
      ::OutputDebugStringA(text);
    }

    template<>
    void basic_debugbuf<wchar_t>::output_debug_string(const wchar_t *text) {
      ::OutputDebugStringW(text);
    }
  }

  template<class CharT, class TraitsT>
  basic_dostream<CharT, TraitsT>::basic_dostream()
    : std::basic_ostream<CharT, TraitsT>(new basic_debugbuf<CharT, TraitsT>()) {
  }

  template<class CharT, class TraitsT>
  basic_dostream<CharT, TraitsT>::~basic_dostream() {
    delete this->rdbuf();
  }

}