#pragma once

#include <string>
#include <vector>

namespace FileParse {
  [[nodiscard]] std::vector<std::string> extractSection(std::string, std::vector<std::string>&);
  [[nodiscard]] std::string parseEntry(std::string, std::vector<std::string>&);
  [[nodiscard]] std::vector<std::string> parseListEntry(std::string, std::vector<std::string>&);
  [[nodiscard]] std::string parseLabel(const std::string&);
  }
