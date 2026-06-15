#include "FinancialTracker.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

static void showStatistics(FinancialTracker &tracker) {
  FinancialSummary summary = tracker.getFinancialSummary();
  auto categories = tracker.getCategoryStats();

  std::cout << std::fixed << std::setprecision(2);
  std::cout << "\n=== Financial Summary ===\n";
  std::cout << "Budget:       " << summary.budget << "\n";
  std::cout << "Total Spent:  " << summary.totalSpent << "\n";
  std::cout << "Remaining:    " << summary.remaining << "\n";

  std::cout << "\n=== Spending by Category ===\n";
  if (categories.empty()) {
    std::cout << "No expenses recorded.\n";
  } else {
    for (const auto &[cat, amount] : categories) {
      std::cout << cat << ": " << amount << "\n";
    }
  }

  std::cout << "\n=== Period Statistics ===\n";
  std::cout << "Enter start date (YYYY-MM-DD) or press Enter to skip: ";
  std::string startDate;
  std::getline(std::cin, startDate);
  startDate = trim(startDate);

  if (!startDate.empty()) {
    if (!validateDate(startDate)) {
      std::cout << "Invalid start date format!\n";
      return;
    }

    std::cout << "Enter end date (YYYY-MM-DD) or press Enter for today: ";
    std::string endDate;
    std::getline(std::cin, endDate);
    endDate = trim(endDate);

    if (endDate.empty()) {
      endDate = getTodayDate();
    }

    if (!validateDate(endDate)) {
      std::cout << "Invalid end date format!\n";
      return;
    }

    try {
      PeriodStats ps = tracker.getPeriodStats(startDate, endDate);
      std::cout << "\nFrom " << startDate << " to " << endDate << ":\n";
      std::cout << "Total spent:       " << ps.totalSpent << "\n";
      std::cout << "Number of expenses:" << ps.expenseCount << "\n";
    } catch (const std::exception &e) {
      std::cout << "Error: " << e.what() << "\n";
    }
  }
}

int main(int argc, char *argv[]) {
  std::cout << "=== Financial Tracker ===\n";

  std::string dataFile;

  // Allow passing the data file path as a command-line argument
  if (argc >= 2) {
    dataFile = argv[1];
  } else {
    std::cout << "Enter data file path (press Enter for default): ";
    std::getline(std::cin, dataFile);
    dataFile = trim(dataFile);
    if (dataFile.empty()) {
      dataFile = "financial_data.json";
    }
  }

  FinancialTracker tracker(dataFile);
  std::cout << "Data file: " << dataFile << "\n";

  while (true) {
    std::cout << "\n=== Main Menu ===\n";
    std::cout << "1. Set Budget\n";
    std::cout << "2. Add Expense\n";
    std::cout << "3. View Statistics\n";
    std::cout << "4. Exit\n";
    std::cout << "Choose an option (1-4): ";

    std::string choice;
    std::getline(std::cin, choice);
    choice = trim(choice);

    if (choice == "1") {
      std::cout << "Enter your budget: ";
      std::string amountStr;
      std::getline(std::cin, amountStr);

      try {
        double amount = std::stod(amountStr);
        tracker.setBudget(amount);
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Budget set to: " << amount << "\n";
      } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
      }

    } else if (choice == "2") {
      std::cout << "Enter amount spent: ";
      std::string amountStr;
      std::getline(std::cin, amountStr);

      double amount = 0.0;
      try {
        amount = std::stod(amountStr);
      } catch (...) {
        std::cout << "Invalid amount! Please enter a positive number.\n";
        continue;
      }

      if (amount <= 0) {
        std::cout << "Invalid amount! Please enter a positive number.\n";
        continue;
      }

      std::cout << "Enter category: ";
      std::string category;
      std::getline(std::cin, category);
      category = trim(category);

      if (category.empty()) {
        std::cout << "Category cannot be empty!\n";
        continue;
      }

      std::cout << "Enter date (YYYY-MM-DD) or press Enter for today: ";
      std::string date;
      std::getline(std::cin, date);
      date = trim(date);

      if (!date.empty() && !validateDate(date)) {
        std::cout << "Invalid date format. Using today's date.\n";
        date = "";
      }

      try {
        tracker.addExpense(amount, category, date);
        std::cout << "Expense added successfully!\n";
      } catch (const std::exception &e) {
        std::cout << "Error: " << e.what() << "\n";
      }

    } else if (choice == "3") {
      showStatistics(tracker);

    } else if (choice == "4") {
      std::cout << "Goodbye!\n";
      break;

    } else {
      std::cout << "Invalid choice! Please try again.\n";
    }
  }

  return 0;
}
