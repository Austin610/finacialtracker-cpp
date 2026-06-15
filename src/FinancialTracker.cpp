#include "FinancialTracker.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

// ── Helpers ──────────────────────────────────────────────────────────────────

std::string trim(const std::string &str) {
  auto start = str.begin();
  while (start != str.end() && std::isspace(*start)) {
    ++start;
  }
  auto end = str.end();
  while (end != start && std::isspace(*(end - 1))) {
    --end;
  }
  return {start, end};
}

bool validateDate(const std::string &dateStr) {
  if (dateStr.size() != 10 || dateStr[4] != '-' || dateStr[7] != '-') {
    return false;
  }

  // Check all other characters are digits
  for (int i = 0; i < 10; ++i) {
    if (i == 4 || i == 7) {
      continue;
    }
    if (!std::isdigit(dateStr[i])) {
      return false;
    }
  }

  int month = std::stoi(dateStr.substr(5, 2));
  int day = std::stoi(dateStr.substr(8, 2));

  return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

std::string getTodayDate() {
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm *local = std::localtime(&t);
  std::ostringstream oss;
  oss << std::put_time(local, "%Y-%m-%d");
  return oss.str();
}

// ── Minimal JSON helpers ──────────────────────────────────────────────────────

static std::string escapeJson(const std::string &s) {
  std::string out;
  for (char c : s) {
    if (c == '"') {
      out += "\\\"";
    } else if (c == '\\') {
      out += "\\\\";
    } else {
      out += c;
    }
  }
  return out;
}

// Extract value for a top-level key like "budget": 3000.0
static std::string extractJsonValue(const std::string &json,
                                    const std::string &key) {
  std::string search = "\"" + key + "\"";
  auto pos = json.find(search);
  if (pos == std::string::npos) {
    return "";
  }
  auto colon = json.find(':', pos + search.size());
  if (colon == std::string::npos) {
    return "";
  }
  auto valStart = json.find_first_not_of(" \t\n\r", colon + 1);
  if (valStart == std::string::npos) {
    return "";
  }
  // Find end of value (comma, } or ])
  auto valEnd = json.find_first_of(",}\n", valStart);
  return trim(json.substr(valStart, valEnd - valStart));
}

// ── FinancialTracker ─────────────────────────────────────────────────────────

FinancialTracker::FinancialTracker(const std::string &dataFile)
    : dataFile_(dataFile), budget_(0.0) {
  loadData();
}

void FinancialTracker::loadData() {
  std::ifstream file(dataFile_);
  if (!file.is_open()) {
    return;
  }

  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  file.close();

  if (content.empty()) {
    return;
  }

  try {
    // Parse budget
    std::string budgetStr = extractJsonValue(content, "budget");
    if (!budgetStr.empty()) {
      budget_ = std::stod(budgetStr);
    }

    // Parse expenses array - find each object block
    auto arrayStart = content.find("\"expenses\"");
    if (arrayStart == std::string::npos) {
      return;
    }
    auto bracketOpen = content.find('[', arrayStart);
    auto bracketClose = content.rfind(']');
    if (bracketOpen == std::string::npos || bracketClose == std::string::npos) {
      return;
    }

    std::string arrayContent =
        content.substr(bracketOpen + 1, bracketClose - bracketOpen - 1);

    // Iterate over { } objects
    std::size_t pos = 0;
    while (pos < arrayContent.size()) {
      auto objStart = arrayContent.find('{', pos);
      if (objStart == std::string::npos) {
        break;
      }
      auto objEnd = arrayContent.find('}', objStart);
      if (objEnd == std::string::npos) {
        break;
      }

      std::string obj = arrayContent.substr(objStart, objEnd - objStart + 1);

      std::string amountStr = extractJsonValue(obj, "amount");
      std::string category = extractJsonValue(obj, "category");
      std::string date = extractJsonValue(obj, "date");

      // Strip quotes from string values
      auto stripQuotes = [](std::string s) -> std::string {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
          return s.substr(1, s.size() - 2);
        }
        return s;
      };

      category = stripQuotes(category);
      date = stripQuotes(date);

      if (!amountStr.empty() && !category.empty() && !date.empty()) {
        Expense e;
        e.amount = std::stod(amountStr);
        e.category = category;
        e.date = date;
        expenses_.push_back(e);
      }

      pos = objEnd + 1;
    }
  } catch (...) {
    // Corrupted file – reset to defaults
    budget_ = 0.0;
    expenses_.clear();
  }
}

void FinancialTracker::saveData() const {
  std::ofstream file(dataFile_);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open data file for writing: " + dataFile_);
  }

  file << std::fixed << std::setprecision(2);
  file << "{\n";
  file << "  \"budget\": " << budget_ << ",\n";
  file << "  \"expenses\": [\n";

  for (std::size_t i = 0; i < expenses_.size(); ++i) {
    const auto &e = expenses_[i];
    file << "    {\n";
    file << "      \"amount\": " << e.amount << ",\n";
    file << "      \"category\": \"" << escapeJson(e.category) << "\",\n";
    file << "      \"date\": \"" << escapeJson(e.date) << "\"\n";
    file << "    }";
    if (i + 1 < expenses_.size()) {
      file << ",";
    }
    file << "\n";
  }

  file << "  ]\n}\n";
}

void FinancialTracker::setBudget(double budget) {
  if (budget < 0) {
    throw std::invalid_argument("Budget cannot be negative");
  }
  budget_ = budget;
  saveData();
}

void FinancialTracker::addExpense(double amount, const std::string &category,
                                  const std::string &date) {
  if (amount <= 0) {
    throw std::invalid_argument("Amount must be positive");
  }

  std::string trimmedCategory = trim(category);
  if (trimmedCategory.empty()) {
    throw std::invalid_argument("Category cannot be empty");
  }

  std::string expenseDate = date.empty() ? getTodayDate() : date;

  if (!validateDate(expenseDate)) {
    throw std::runtime_error("Invalid date format. Use YYYY-MM-DD.");
  }

  Expense e;
  e.amount = amount;
  e.category = trimmedCategory;
  e.date = expenseDate;

  expenses_.push_back(e);
  saveData();
}

double FinancialTracker::getTotalSpent() const {
  double total = 0.0;
  for (const auto &e : expenses_) {
    total += e.amount;
  }
  return total;
}

std::map<std::string, double> FinancialTracker::getCategoryStats() const {
  std::map<std::string, double> stats;
  for (const auto &e : expenses_) {
    stats[e.category] += e.amount;
  }
  return stats;
}

PeriodStats FinancialTracker::getPeriodStats(const std::string &startDate,
                                             const std::string &endDate) const {
  if (!validateDate(startDate) || !validateDate(endDate)) {
    throw std::invalid_argument("Invalid date format. Use YYYY-MM-DD.");
  }

  if (startDate > endDate) {
    throw std::invalid_argument("Start date cannot be after end date");
  }

  PeriodStats stats{0.0, 0};
  for (const auto &e : expenses_) {
    if (e.date >= startDate && e.date <= endDate) {
      stats.totalSpent += e.amount;
      stats.expenseCount += 1;
    }
  }
  return stats;
}

FinancialSummary FinancialTracker::getFinancialSummary() const {
  double spent = getTotalSpent();
  return {budget_, spent, budget_ - spent};
}

const std::vector<Expense> &FinancialTracker::getExpenses() const {
  return expenses_;
}

double FinancialTracker::getBudget() const { return budget_; }
