/**
 * @file financial_tracker.h
 * @brief Financial tracking application header file
 */

#ifndef FINANCIAL_TRACKER_H
#define FINANCIAL_TRACKER_H

#include <string>
#include <vector>
#include <map>

/**
 * @struct Expense
 * @brief Represents a single expense entry
 */
struct Expense {
    double amount;      ///< Amount spent
    std::string category; ///< Expense category
    std::string date;   ///< Date in YYYY-MM-DD format
};

/**
 * @class FinancialTracker
 * @brief Main class for tracking expenses and budget
 */
class FinancialTracker {
private:
    double m_budget;                    ///< Current budget
    std::vector<Expense> m_expenses;    ///< List of all expenses
    std::string m_dataFile;              ///< Data file path

    /**
     * @brief Load data from JSON file
     * @throws std::runtime_error if file cannot be read
     */
    void loadData();

    /**
     * @brief Save data to JSON file
     * @throws std::runtime_error if file cannot be written
     */
    void saveData() const;

public:
    /**
     * @brief Constructor
     * @param dataFile Path to data file (default: "financial_data.json")
     */
    explicit FinancialTracker(const std::string& dataFile = "financial_data.json");

    /**
     * @brief Set user's budget
     * @param budget Budget amount (must be non-negative)
     * @return true if successful
     * @throws std::invalid_argument if budget is invalid
     */
    bool setBudget(double budget);

    /**
     * @brief Add a new expense
     * @param amount Amount spent (must be positive)
     * @param category Expense category (cannot be empty)
     * @param date Date in YYYY-MM-DD format (default: today)
     * @return true if successful
     * @throws std::invalid_argument if expense data is invalid
     */
    bool addExpense(double amount, const std::string& category, 
                    const std::string& date = "");

    /**
     * @brief Calculate total amount spent
     * @return Total spent amount
     */
    double getTotalSpent() const;

    /**
     * @brief Get spending statistics by category
     * @return Map of category to total spent
     */
    std::map<std::string, double> getCategoryStats() const;

    /**
     * @brief Get spending statistics for a period
     * @param startDate Start date (YYYY-MM-DD)
     * @param endDate End date (YYYY-MM-DD)
     * @return Map with "total_spent" and "expense_count"
     * @throws std::invalid_argument if dates are invalid
     */
    std::map<std::string, double> getPeriodStats(const std::string& startDate, 
                                                 const std::string& endDate) const;

    /**
     * @brief Get complete financial summary
     * @return Map with budget, total_spent, remaining
     */
    std::map<std::string, double> getFinancialSummary() const;

    /**
     * @brief Get current date in YYYY-MM-DD format
     * @return Current date string
     */
    static std::string getCurrentDate();

    /**
     * @brief Validate date format
     * @param date Date string to validate
     * @return true if date is valid
     */
    static bool isValidDate(const std::string& date);
};

/**
 * @brief Validate and parse amount string
 * @param amountStr Amount string to validate
 * @param[out] amount Parsed amount if valid
 * @return true if amount is valid and non-negative
 */
bool validateAmount(const std::string& amountStr, double& amount);

#endif // FINANCIAL_TRACKER_H