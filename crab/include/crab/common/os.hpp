#pragma once

#include <iosfwd>
#include <memory>
#include <sstream>

namespace crab {

// An adaptor for std::ostream that avoids polluting all crab header
// files with iostream stuff
class crab_os {

  static std::unique_ptr<crab_os> m_cout;
  static std::unique_ptr<crab_os> m_cerr;

public:
  static crab_os &cout();
  static crab_os &cerr();

private:
  std::ostream *m_os;

protected:
  crab_os();

public:
  crab_os(std::ostream *os);

  crab_os(const crab_os &o) = delete;

  crab_os &operator=(const crab_os &o) = delete;

  virtual ~crab_os();

  virtual crab_os &operator<<(char C);
  virtual crab_os &operator<<(unsigned char C);
  virtual crab_os &operator<<(signed char C);
  virtual crab_os &operator<<(const char *Str);
  virtual crab_os &operator<<(const std::string &Str);
  virtual crab_os &operator<<(unsigned long N);
  virtual crab_os &operator<<(long N);
  virtual crab_os &operator<<(unsigned long long N);
  virtual crab_os &operator<<(long long N);
  virtual crab_os &operator<<(const void *P);
  virtual crab_os &operator<<(unsigned int N);
  virtual crab_os &operator<<(int N);
  virtual crab_os &operator<<(double N);
};

extern crab_os &outs();
extern crab_os &errs();

// An adaptor for std::ostringstream
class crab_string_os : public crab_os {

  std::ostringstream *m_string_os;

public:
  crab_string_os();

  ~crab_string_os();

  std::string str();

  crab_os &operator<<(char C);
  crab_os &operator<<(unsigned char C);
  crab_os &operator<<(signed char C);
  crab_os &operator<<(const char *Str);
  crab_os &operator<<(const std::string &Str);
  crab_os &operator<<(unsigned long N);
  crab_os &operator<<(long N);
  crab_os &operator<<(unsigned long long N);
  crab_os &operator<<(long long N);
  crab_os &operator<<(const void *P);
  crab_os &operator<<(unsigned int N);
  crab_os &operator<<(int N);
  crab_os &operator<<(double N);
};

} // namespace crab
