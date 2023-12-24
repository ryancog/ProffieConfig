// ProffieConfig, All-In-One GUI Proffieboard Configuration Utility
// Copyright (C) 2023 Ryan Ogurek

#pragma once

#include <string>
#include <vector>

namespace FileParse {
  [[nodiscard]] std::vector<std::string> extractSection(std::string, std::vector<std::string>&);
  [[nodiscard]] std::string parseEntry(std::string, std::vector<std::string>&);
  [[nodiscard]] std::string parseEntry(std::string, std::vector<std::string>&, std::string&);
  [[nodiscard]] double parseNumEntry(std::string, std::vector<std::string>&);
  [[nodiscard]] bool parseBoolEntry(std::string, std::vector<std::string>&);
  [[nodiscard]] std::vector<std::string> parseListEntry(std::string, std::vector<std::string>&);
  [[nodiscard]] std::string parseLabel(const std::string&);
  }
