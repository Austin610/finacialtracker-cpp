#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

class Expense {
public:
    double amount;
    std::string category;
    std::string date;
};

class FinancialTracker {
private:
    double budget;
    std::vector<Expense> expenses;
    std::string dataFile;

    std::string getCurrentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::string year = std::to_string(1900 + ltm->tm_year);
        std::string month = std::to_string(1 + ltm->tm_mon);
        std::string day = std::to_string(ltm->tm_mday);
        
        if (month.length() == 1) month = "0" + month;
        if (day.length() == 1) day = "0" + day;
        
        return year + "-" + month + "-" + day;
    }

    bool isValidDate(std::string date) {
        if (date.length() != 10) return false;
        if (date[4] != '-' || date[7] != '-') return false;
        
        try {
            int year = std::stoi(date.substr(0, 4));
            int month = std::stoi(date.substr(5, 2));
            int day = std::stoi(date.substr(8, 2));
            
            if (month < 1 || month > 12) return false;
            if (day < 1 || day > 31) return false;
            
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

    void loadData() {
        std::ifstream file(dataFile);
        if (!file.is_open()) {
            budget = 0.0;
            expenses.clear();
            return;
        }

        std::string line;
        bool inExpenses = false;
        Expense currentExpense;
        
        while (std::getline(file, line)) {
            if (line.find("\"budget\"") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string val = line.substr(colon + 1);
                    val.erase(remove_if(val.begin(), val.end(), isspace), val.end());
                    if (!val.empty() && val.back() == ',') val.pop_back();
                    budget = std::stod(val);
                }
            }
            else if (line.find("\"expenses\"") != std::string::npos) {
                inExpenses = true;
            }
            else if (inExpenses && line.find("{") != std::string::npos) {
                currentExpense = Expense{0.0, "", ""};
            }
            else if (inExpenses && line.find("\"amount\"") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string val = line.substr(colon + 1);
                    val.erase(remove_if(val.begin(), val.end(), isspace), val.end());
                    if (!val.empty() && val.back() == ',') val.pop_back();
                    currentExpense.amount = std::stod(val);
                }
            }
            else if (inExpenses && line.find("\"category\"") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string val = line.substr(colon + 1);
                    val.erase(remove_if(val.begin(), val.end(), isspace), val.end());
                    if (!val.empty() && val.front() == '"') val = val.substr(1);
                    if (!val.empty() && val.back() == '"') val.pop_back();
                    if (!val.empty() && val.back() == ',') val.pop_back();
                    currentExpense.category = val;
                }
            }
            else if (inExpenses && line.find("\"date\"") != std::string::npos) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string val = line.substr(colon + 1);
                    val.erase(remove_if(val.begin(), val.end(), isspace), val.end());
                    if (!val.empty() && val.front() == '"') val = val.substr(1);
                    if (!val.empty() && val.back() == '"') val.pop_back();
                    if (!val.empty() && val.back() == ',') val.pop_back();
                    currentExpense.date = val;
                }
            }
            else if (inExpenses && line.find("}") != std::string::npos) {
                if (currentExpense.amount > 0 && !currentExpense.category.empty() 
                    && !currentExpense.date.empty()) {
                    expenses.push_back(currentExpense);
                }
            }
            else if (line.find("]") != std::string::npos) {
                inExpenses = false;
            }
        }
        file.close();
    }

    void saveData() {
        std::ofstream file(dataFile);
        if (!file.is_open()) return;

        file << "{\n";
        file << "  \"budget\": " << budget << ",\n";
        file << "  \"expenses\": [\n";
        
        for (size_t i = 0; i < expenses.size(); i++) {
            file << "    {\n";
            file << "      \"amount\": " << expenses[i].amount << ",\n";
            file << "      \"category\": \"" << expenses[i].category << "\",\n";
            file << "      \"date\": \"" << expenses[i].date << "\"\n";
            file << "    }";
            if (i < expenses.size() - 1) file << ",";
            file << "\n";
        }
        
        file << "  ]\n";
        file << "}\n";
        file.close();
    }

public:
    FinancialTracker(std::string file = "financial_data.json") {
        dataFile = file;
        loadData();
    }

    bool setBudget(double amount) {
        if (amount < 0) {
            std::cout << "Budget cannot be negative!\n";
            return false;
        }
        budget = amount;
        saveData();
        std::cout << "Budget set to: " << budget << "\n";
        return true;
    }

    bool addExpense(double amount, std::string category, std::string date = "") {
        if (amount <= 0) {
            std::cout << "Amount must be positive!\n";
            return false;
        }
        
        if (category.empty()) {
            std::cout << "Category cannot be empty!\n";
            return false;
        }
        
        std::string expenseDate = date;
        if (expenseDate.empty()) {
            expenseDate = getCurrentDate();
        } else if (!isValidDate(expenseDate)) {
            std::cout << "Invalid date format! Use YYYY-MM-DD\n";
            return false;
        }
        
        Expense e;
        e.amount = amount;
        e.category = category;
        e.date = expenseDate;
        expenses.push_back(e);
        saveData();
        std::cout << "Expense added successfully!\n";
        return true;
    }

    double getTotalSpent() {
        double total = 0;
        for (Expense& e : expenses) {
            total += e.amount;
        }
        return total;
    }

    void showCategoryStats() {
        std::map<std::string, double> stats;
        for (Expense& e : expenses) {
            stats[e.category] += e.amount;
        }
        
        std::cout << "\n=== Spending by Category ===\n";
        if (stats.empty()) {
            std::cout << "No expenses recorded.\n";
        } else {
            for (auto& item : stats) {
                std::cout << item.first << ": " << item.second << "\n";
            }
        }
    }

    void showFinancialSummary() {
        double total = getTotalSpent();
        double remaining = budget - total;
        
        std::cout << "\n=== Financial Summary ===\n";
        std::cout << "Budget: " << budget << "\n";
        std::cout << "Total Spent: " << total << "\n";
        std::cout << "Remaining: " << remaining << "\n";
    }

    void showPeriodStats() {
        std::string startDate, endDate;
        std::cout << "Enter start date (YYYY-MM-DD): ";
        std::cin >> startDate;
        std::cout << "Enter end date (YYYY-MM-DD): ";
        std::cin >> endDate;
        
        if (!isValidDate(startDate) || !isValidDate(endDate)) {
            std::cout << "Invalid date format!\n";
            return;
        }
        
        if (startDate > endDate) {
            std::cout << "Start date cannot be after end date!\n";
            return;
        }
        
        double total = 0;
        int count = 0;
        
        for (Expense& e : expenses) {
            if (e.date >= startDate && e.date <= endDate) {
                total += e.amount;
                count++;
            }
        }
        
        std::cout << "\nFrom " << startDate << " to " << endDate << ":\n";
        std::cout << "Total spent: " << total << "\n";
        std::cout << "Number of expenses: " << count << "\n";
    }
};

int main() {
    std::cout << "=== Financial Tracker ===\n";
    
    std::string dataFile;
    std::cout << "Enter data file name (press Enter for default): ";
    std::getline(std::cin, dataFile);
    
    if (dataFile.empty()) {
        dataFile = "financial_data.json";
    }
    
    FinancialTracker tracker(dataFile);
    
    while (true) {
        std::cout << "\n=== Main Menu ===\n";
        std::cout << "1. Set Budget\n";
        std::cout << "2. Add Expense\n";
        std::cout << "3. View Summary\n";
        std::cout << "4. View Category Stats\n";
        std::cout << "5. View Period Stats\n";
        std::cout << "6. Exit\n";
        std::cout << "Choose option: ";
        
        std::string choice;
        std::getline(std::cin, choice);
        
        if (choice == "1") {
            std::cout << "Enter budget: ";
            double amount;
            std::cin >> amount;
            std::cin.ignore();
            tracker.setBudget(amount);
        }
        else if (choice == "2") {
            double amount;
            std::string category, date;
            
            std::cout << "Enter amount: ";
            std::cin >> amount;
            std::cin.ignore();
            
            std::cout << "Enter category: ";
            std::getline(std::cin, category);
            
            std::cout << "Enter date (YYYY-MM-DD) or press Enter for today: ";
            std::getline(std::cin, date);
            
            tracker.addExpense(amount, category, date);
        }
        else if (choice == "3") {
            tracker.showFinancialSummary();
        }
        else if (choice == "4") {
            tracker.showCategoryStats();
        }
        else if (choice == "5") {
            tracker.showPeriodStats();
        }
        else if (choice == "6") {
            std::cout << "Goodbye!\n";
            break;
        }
        else {
            std::cout << "Invalid choice!\n";
        }
    }
    
    return 0;
}