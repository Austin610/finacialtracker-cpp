/**
 * @file financial_tracker.cpp
 * @brief Implementation of financial tracking functions
 */

#include "financial_tracker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <cctype>
#include <chrono>

// Function to trim whitespace from string
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

FinancialTracker::FinancialTracker(const std::string& dataFile) 
    : m_budget(0.0), m_dataFile(dataFile) {
    loadData();
}

void FinancialTracker::loadData() {
    std::ifstream file(m_dataFile);
    if (!file.is_open()) {
        // File doesn't exist - start with empty data
        m_budget = 0.0;
        m_expenses.clear();
        return;
    }

    try {
        // Simple JSON parsing (for educational purposes)
        std::string line;
        bool inExpenses = false;
        Expense currentExpense;
        
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty()) continue;
            
            if (line.find("\"budget\"") != std::string::npos) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string valueStr = line.substr(colonPos + 1);
                    valueStr = trim(valueStr);
                    // Remove trailing comma if present
                    if (!valueStr.empty() && valueStr.back() == ',') {
                        valueStr.pop_back();
                    }
                    m_budget = std::stod(valueStr);
                }
            }
            else if (line.find("\"expenses\"") != std::string::npos) {
                inExpenses = true;
            }
            else if (inExpenses && line.find("{") != std::string::npos) {
                currentExpense = Expense{0.0, "", ""};
            }
            else if (inExpenses && line.find("\"amount\"") != std::string::npos) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string valueStr = line.substr(colonPos + 1);
                    valueStr = trim(valueStr);
                    if (!valueStr.empty() && valueStr.back() == ',') {
                        valueStr.pop_back();
                    }
                    currentExpense.amount = std::stod(valueStr);
                }
            }
            else if (inExpenses && line.find("\"category\"") != std::string::npos) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string valueStr = line.substr(colonPos + 1);
                    valueStr = trim(valueStr);
                    // Remove quotes and trailing comma
                    if (!valueStr.empty() && valueStr.front() == '"') {
                        valueStr = valueStr.substr(1);
                    }
                    if (!valueStr.empty() && valueStr.back() == '"') {
                        valueStr.pop_back();
                    }
                    if (!valueStr.empty() && valueStr.back() == ',') {
                        valueStr.pop_back();
                    }
                    currentExpense.category = valueStr;
                }
            }
            else if (inExpenses && line.find("\"date\"") != std::string::npos) {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string valueStr = line.substr(colonPos + 1);
                    valueStr = trim(valueStr);
                    // Remove quotes and trailing comma
                    if (!valueStr.empty() && valueStr.front() == '"') {
                        valueStr = valueStr.substr(1);
                    }
                    if (!valueStr.empty() && valueStr.back() == '"') {
                        valueStr.pop_back();
                    }
                    if (!valueStr.empty() && valueStr.back() == ',') {
                        valueStr.pop_back();
                    }
                    currentExpense.date = valueStr;
                }
            }
            else if (inExpenses && line.find("}") != std::string::npos) {
                if (currentExpense.amount > 0 && !currentExpense.category.empty() 
                    && !currentExpense.date.empty()) {
                    m_expenses.push_back(currentExpense);
                }
            }
            else if (line.find("]") != std::string::npos) {
                inExpenses = false;
            }
        }
        file.close();
    }
    catch (const std::exception& e) {
        // If file is corrupted, start fresh
        m_budget = 0.0;
        m_expenses.clear();
    }
}

void FinancialTracker::saveData() const {
    std::ofstream file(m_dataFile);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + m_dataFile);
    }

    file << "{\n";
    file << "  \"budget\": " << m_budget << ",\n";
    file << "  \"expenses\": [\n";
    
    for (size_t i = 0; i < m_expenses.size(); ++i) {
        file << "    {\n";
        file << "      \"amount\": " << m_expenses[i].amount << ",\n";
        file << "      \"category\": \"" << m_expenses[i].category << "\",\n";
        file << "      \"date\": \"" << m_expenses[i].date << "\"\n";
        file << "    }";
        if (i < m_expenses.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    file.close();
}

bool FinancialTracker::setBudget(double budget) {
    if (budget < 0) {
        throw std::invalid_argument("Budget cannot be negative");
    }
    m_budget = budget;
    saveData();
    return true;
}

bool FinancialTracker::addExpense(double amount, const std::string& category, 
                                  const std::string& date) {
    if (amount <= 0) {
        throw std::invalid_argument("Amount must be positive");
    }
    
    std::string trimmedCategory = trim(category);
    if (trimmedCategory.empty()) {
        throw std::invalid_argument("Category cannot be empty");
    }
    
    std::string expenseDate = date;
    if (expenseDate.empty()) {
        expenseDate = getCurrentDate();
    } else if (!isValidDate(expenseDate)) {
        throw std::invalid_argument("Invalid date format. Use YYYY-MM-DD");
    }
    
    Expense expense{amount, trimmedCategory, expenseDate};
    m_expenses.push_back(expense);
    saveData();
    return true;
}

double FinancialTracker::getTotalSpent() const {
    double total = 0.0;
    for (const auto& expense : m_expenses) {
        total += expense.amount;
    }
    return total;
}

std::map<std::string, double> FinancialTracker::getCategoryStats() const {
    std::map<std::string, double> stats;
    for (const auto& expense : m_expenses) {
        stats[expense.category] += expense.amount;
    }
    return stats;
}

std::map<std::string, double> FinancialTracker::getPeriodStats(
    const std::string& startDate, const std::string& endDate) const {
    
    if (!isValidDate(startDate) || !isValidDate(endDate)) {
        throw std::invalid_argument("Invalid date format");
    }
    
    if (startDate > endDate) {
        throw std::invalid_argument("Start date cannot be after end date");
    }
    
    double total = 0.0;
    int count = 0;
    
    for (const auto& expense : m_expenses) {
        if (expense.date >= startDate && expense.date <= endDate) {
            total += expense.amount;
            count++;
        }
    }
    
    std::map<std::string, double> result;
    result["total_spent"] = total;
    result["expense_count"] = static_cast<double>(count);
    return result;
}

std::map<std::string, double> FinancialTracker::getFinancialSummary() const {
    double totalSpent = getTotalSpent();
    std::map<std::string, double> summary;
    summary["budget"] = m_budget;
    summary["total_spent"] = totalSpent;
    summary["remaining"] = m_budget - totalSpent;
    return summary;
}

std::string FinancialTracker::getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&timeT);
    
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d");
    return oss.str();
}

bool FinancialTracker::isValidDate(const std::string& date) {
    if (date.length() != 10) return false;
    if (date[4] != '-' || date[7] != '-') return false;
    
    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;
        
        // Basic month-day validation
        if (month == 2) {
            bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
            if (day > (isLeap ? 29 : 28)) return false;
        } else if (month == 4 || month == 6 || month == 9 || month == 11) {
            if (day > 30) return false;
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool validateAmount(const std::string& amountStr, double& amount) {
    try {
        size_t pos;
        amount = std::stod(amountStr, &pos);
        if (pos != amountStr.length()) return false;
        return amount >= 0;
    } catch (...) {
        return false;
    }
}