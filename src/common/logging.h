//
// Created by Yihang Yang on 11/20/20.
//

#ifndef DALI_SRC_COMMON_LOGGING_H_
#define DALI_SRC_COMMON_LOGGING_H_

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;

void init_logging_and_formatting(boost::log::trivial::severity_level sl = boost::log::trivial::info);

inline boost::log::record_ostream &operator<<(boost::log::record_ostream &p, std::vector<double> &v) {
  p << "[";
  for (size_t i = 0; i < v.size(); ++i) {
    p << v[i];
    if (i != v.size() - 1)
      p << ", ";
  }
  p << "]";
  return p;
}

inline void Assert(bool e, const std::string &error_message) {
  if (!e) {
    BOOST_LOG_TRIVIAL(fatal) << "\033[0;31m" << "FATAL ERROR:" << "\n"
                             << "    " << error_message << "\033[0m" << std::endl;
    exit(1);
  }
}

inline void Warning(bool e, const std::string &warning_message) {
  if (e) {
    BOOST_LOG_TRIVIAL(warning) << "WARNING:" << "\n"
                               << "    " << warning_message << std::endl;
  }
}

inline void PrintSoftwareStatement() {
  BOOST_LOG_TRIVIAL(info) << "\n"
                          << "+----------------------------------------------+\n"
                          << "|                                              |\n"
                          << "|     Dali: gridded cell placement flow        |\n"
                          << "|                                              |\n"
                          << "|     Department of Electrical Engineering     |\n"
                          << "|     Yale University                          |\n"
                          << "|                                              |\n"
                          << "|     Developed by                             |\n"
                          << "|     Yihang Yang, Rajit Manohar               |\n"
                          << "|                                              |\n"
                          << "|     This program is for academic use and     |\n"
                          << "|     testing only                             |\n"
                          << "|     THERE IS NO WARRANTY                     |\n"
                          << "|                                              |\n"
                          << "|     build time: " << __DATE__ << " " << __TIME__ << "         |\n"
                          << "|                                              |\n"
                          << "+----------------------------------------------+\n\n";
}

#endif //DALI_SRC_COMMON_LOGGING_H_