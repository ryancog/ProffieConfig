#include "core/fileparse.h"
#include <algorithm>

std::vector<std::string> FileParse::extractSection(std::string sectionName, std::vector<std::string>& search) {
  std::vector<std::string> section;

  auto begin = search.begin();
  auto end = search.end();
  int32_t depth{0};
  bool copy{false};
  for (auto line = search.begin(); line < search.end(); line++) {
    if (copy) {
      section.push_back(*line);
      if ((*line).find("{") != std::string::npos) depth++;
      else if ((*line).find("}") != std::string::npos) depth--;

      if (depth < 0) {
        end = line;
        break;
      }
    }
    if ((*line).find(sectionName) != std::string::npos) {
      copy = true;
      section.push_back(*line);
      begin = line;
    }
  }
  if (copy == false || depth >= 0) return {};

  search.erase(begin, end);

  return section;
}
std::string FileParse::parseEntry(std::string entry, std::vector<std::string>& search) {
  for (auto line = search.begin(); line < search.end(); line++) {
    if ((*line).find(entry) == std::string::npos) continue;

    size_t index = (*line).find(":");
    if (index == std::string::npos) continue;

    std::string output = (*line).substr(index);
    if (output.find_first_of("\"") == std::string::npos || output.find_last_of("\"") == std::string::npos) output = "";
    else output = output.substr(output.find_first_of("\"") + 1, output.find_last_of("\"") - output.find_first_of("\"") - 1);

    search.erase(line);

    return output;
  }

  return {};
}
std::vector<std::string> FileParse::parseListEntry(std::string entry, std::vector<std::string>& search) {
  std::vector<std::string> parsedList;
  std::string stringList = parseEntry(entry, search);
  const std::string removeChars = " \"";

  if (stringList.empty()) return {};

  int32_t runs = std::count(stringList.begin(), stringList.end(), ',');
  for (int32_t run = 0; run <= runs; run++) {
    std::string tmp = stringList.substr(0, stringList.find(",") - 1);
    tmp.erase(std::remove_if(tmp.begin(), tmp.end(), [&removeChars](char curr) { return removeChars.find(curr) != std::string::npos; }), tmp.end());
    parsedList.push_back(tmp);
    stringList = stringList.substr(stringList.find(",") + 1);
  }

  return parsedList;
}
std::string FileParse::parseLabel(const std::string& entry) {
  if (entry.find("(\"") == std::string::npos || entry.find("\")") == std::string::npos) return {};
  return entry.substr(entry.find("(\"") + 2, entry.find("\")") - entry.find("(\"") - 2);
}
