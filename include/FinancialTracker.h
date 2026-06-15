#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * @brief Represents a single expense record.
 *
 * @var Expense::amount The amount of money spent.
 * @var Expense::category The category of the expense.
 * @var Expense::date The date of the expense in YYYY-MM-DD format.
 */
struct Expense {
  double amount;
  std::string category;
  std::string date;
};

/**
 * @brief Represents a financial summary.
 *
 * @var FinancialSummary::budget The total budget set by the user.
 * @var FinancialSummary::totalSpent The total amount spent.
 * @var FinancialSummary::remaining The remaining budget.
 */
struct FinancialSummary {
  double budget;
  double totalSpent;
  double remaining;
};

/**
 * @brief Represents statistics for a specific time period.
 *
 * @var PeriodStats::totalSpent Total money spent in the period.
 * @var PeriodStats::expenseCount Number of expenses recorded in the period.
 */
struct PeriodStats {
  double totalSpent;
  int expenseCount;
};

/**
 * @brief Manages personal finance tracking including budget and expenses.
 *
 * Provides functionality to set a budget, record expenses by category,
 * and generate various financial statistics. All data is persisted to a
 * JSON-like plain-text file.
 */
class FinancialTracker {
public:
  /**
   * @brief Constructs a FinancialTracker with the given data file path.
   *
   * @param dataFile Path to the file used for persisting data.
   *                 Defaults to "financial_data.json".
   */
  explicit FinancialTracker(const std::string &dataFile = "financial_data.json");

  /**
   * @brief Sets the user's monthly budget.
   *
   * @param budget The budget amount. Must be non-negative.
   * @throws std::invalid_argument if budget is negative.
   */
  void setBudget(double budget);

  /**
   * @brief Adds a new expense record.
   *
   * @param amount The expense amount. Must be positive.
   * @param category The expense category. Must not be empty.
   * @param date The date in YYYY-MM-DD format. Defaults to today if empty.
   * @throws std::invalid_argument if amount is not positive or category is
   * empty.
   * @throws std::runtime_error if the date format is invalid.
   */
  void addExpense(double amount, const std::string &category,
                  const std::string &date = "");

  /**
   * @brief Calculates the total amount spent across all expenses.
   *
   * @return Total amount spent as a double.
   */
  double getTotalSpent() const;

  /**
   * @brief Returns spending totals grouped by category.
   *
   * @return A map from category name to total amount spent in that category.
   */
  std::map<std::string, double> getCategoryStats() const;

  /**
   * @brief Returns spending statistics for a given date range.
   *
   * @param startDate Start of the period in YYYY-MM-DD format.
   * @param endDate End of the period in YYYY-MM-DD format.
   * @return A PeriodStats struct with total spent and expense count.
   * @throws std::invalid_argument if the date format is invalid or start is
   * after end.
   */
  PeriodStats getPeriodStats(const std::string &startDate,
                             const std::string &endDate) const;

  /**
   * @brief Returns a complete financial summary.
   *
   * @return A FinancialSummary struct with budget, total spent, and remaining.
   */
  FinancialSummary getFinancialSummary() const;

  /**
   * @brief Returns the current list of all expenses.
   *
   * @return A const reference to the vector of Expense records.
   */
  const std::vector<Expense> &getExpenses() const;

  /**
   * @brief Returns the current budget.
   *
   * @return The budget value as a double.
   */
  double getBudget() const;

private:
  std::string dataFile_;
  double budget_;
  std::vector<Expense> expenses_;

  void loadData();
  void saveData() const;
};

/**
 * @brief Validates a date string in YYYY-MM-DD format.
 *
 * @param dateStr The date string to validate.
 * @return True if the date is valid, false otherwise.
 */
bool validateDate(const std::string &dateStr);

/**
 * @brief Returns today's date as a string in YYYY-MM-DD format.
 *
 * @return Today's date string.
 */
std::string getTodayDate();

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * @param str The string to trim.
 * @return A new string with whitespace removed from both ends.
 */
std::string trim(const std::string &str);
