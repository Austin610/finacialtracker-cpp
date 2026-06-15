/**
 * @file test_financial_tracker.cpp
 * @brief Unit tests for FinancialTracker
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "financial_tracker.h"
#include <fstream>
#include <cstdio>

// Helper function to remove file (cross-platform)
void removeFile(const std::string& filename) {
    std::remove(filename.c_str());
}

TEST_CASE("Testing validateAmount function") {
    double amount;
    
    SUBCASE("Valid positive amounts") {
        CHECK(validateAmount("50.5", amount) == true);
        CHECK(amount == 50.5);
        
        CHECK(validateAmount("0", amount) == true);
        CHECK(amount == 0);
        
        CHECK(validateAmount("100", amount) == true);
        CHECK(amount == 100);
    }
    
    SUBCASE("Negative amounts") {
        CHECK(validateAmount("-50.5", amount) == false);
        CHECK(validateAmount("-1", amount) == false);
    }
    
    SUBCASE("Invalid strings") {
        CHECK(validateAmount("invalid", amount) == false);
        CHECK(validateAmount("50.5.5", amount) == false);
        CHECK(validateAmount("", amount) == false);
        CHECK(validateAmount("abc", amount) == false);
    }
}

TEST_CASE("Testing isValidDate function") {
    SUBCASE("Valid dates") {
        CHECK(FinancialTracker::isValidDate("2024-01-15") == true);
        CHECK(FinancialTracker::isValidDate("2024-12-31") == true);
        CHECK(FinancialTracker::isValidDate("2024-02-29") == true); // Leap year
        CHECK(FinancialTracker::isValidDate("2023-02-28") == true); // Non-leap year
        CHECK(FinancialTracker::isValidDate("2024-04-30") == true);
    }
    
    SUBCASE("Invalid dates") {
        CHECK(FinancialTracker::isValidDate("2024-01-32") == false);
        CHECK(FinancialTracker::isValidDate("2024-13-01") == false);
        CHECK(FinancialTracker::isValidDate("2024-02-30") == false);
        CHECK(FinancialTracker::isValidDate("2023-02-29") == false); // Non-leap year
        CHECK(FinancialTracker::isValidDate("invalid") == false);
        CHECK(FinancialTracker::isValidDate("2024/01/15") == false);
        CHECK(FinancialTracker::isValidDate("24-01-15") == false);
        CHECK(FinancialTracker::isValidDate("2024-1-15") == false);
        CHECK(FinancialTracker::isValidDate("") == false);
    }
}

TEST_CASE("Testing FinancialTracker class") {
    // Use a temporary file for testing
    std::string testFile = "test_financial_data.json";
    
    // Clean up any existing test file
    removeFile(testFile);
    
    FinancialTracker tracker(testFile);
    
    SUBCASE("Set budget - positive") {
        CHECK(tracker.setBudget(1000.0) == true);
        auto summary = tracker.getFinancialSummary();
        CHECK(summary["budget"] == 1000.0);
    }
    
    SUBCASE("Set budget - zero") {
        CHECK(tracker.setBudget(0.0) == true);
        auto summary = tracker.getFinancialSummary();
        CHECK(summary["budget"] == 0.0);
    }
    
    SUBCASE("Set budget - negative") {
        CHECK_THROWS_AS(tracker.setBudget(-100.0), std::invalid_argument);
    }
    
    SUBCASE("Add expense - valid with date") {
        CHECK(tracker.addExpense(50.0, "Food", "2024-01-15") == true);
        CHECK(tracker.getTotalSpent() == 50.0);
    }
    
    SUBCASE("Add expense - valid without date") {
        CHECK(tracker.addExpense(50.0, "Food") == true);
        CHECK(tracker.getTotalSpent() == 50.0);
    }
    
    SUBCASE("Add expense - negative amount") {
        CHECK_THROWS_AS(tracker.addExpense(-50.0, "Food"), std::invalid_argument);
    }
    
    SUBCASE("Add expense - zero amount") {
        CHECK_THROWS_AS(tracker.addExpense(0.0, "Food"), std::invalid_argument);
    }
    
    SUBCASE("Add expense - empty category") {
        CHECK_THROWS_AS(tracker.addExpense(50.0, ""), std::invalid_argument);
    }
    
    SUBCASE("Add expense - spaces only category") {
        CHECK_THROWS_AS(tracker.addExpense(50.0, "   "), std::invalid_argument);
    }
    
    SUBCASE("Add expense - invalid date") {
        CHECK_THROWS_AS(tracker.addExpense(50.0, "Food", "2024-01-32"), 
                       std::invalid_argument);
    }
    
    SUBCASE("Get total spent - multiple expenses") {
        tracker.addExpense(50.0, "Food");
        tracker.addExpense(30.0, "Transport");
        tracker.addExpense(20.0, "Food");
        CHECK(tracker.getTotalSpent() == 100.0);
    }
    
    SUBCASE("Get total spent - no expenses") {
        CHECK(tracker.getTotalSpent() == 0.0);
    }
    
    SUBCASE("Get category stats - multiple categories") {
        tracker.addExpense(50.0, "Food");
        tracker.addExpense(30.0, "Food");
        tracker.addExpense(20.0, "Transport");
        tracker.addExpense(15.0, "Entertainment");
        
        auto stats = tracker.getCategoryStats();
        CHECK(stats["Food"] == 80.0);
        CHECK(stats["Transport"] == 20.0);
        CHECK(stats["Entertainment"] == 15.0);
        CHECK(stats.size() == 3);
    }
    
    SUBCASE("Get category stats - empty") {
        auto stats = tracker.getCategoryStats();
        CHECK(stats.empty());
    }
    
    SUBCASE("Get period stats - exact matches") {
        tracker.addExpense(50.0, "Food", "2024-01-15");
        tracker.addExpense(30.0, "Transport", "2024-01-16");
        tracker.addExpense(20.0, "Food", "2024-01-17");
        
        auto stats = tracker.getPeriodStats("2024-01-15", "2024-01-17");
        CHECK(stats["total_spent"] == 100.0);
        CHECK(stats["expense_count"] == 3.0);
    }
    
    SUBCASE("Get period stats - partial range") {
        tracker.addExpense(50.0, "Food", "2024-01-15");
        tracker.addExpense(30.0, "Transport", "2024-01-16");
        tracker.addExpense(20.0, "Food", "2024-01-17");
        
        auto stats = tracker.getPeriodStats("2024-01-16", "2024-01-17");
        CHECK(stats["total_spent"] == 50.0);
        CHECK(stats["expense_count"] == 2.0);
    }
    
    SUBCASE("Get period stats - no expenses in range") {
        tracker.addExpense(50.0, "Food", "2024-01-15");
        
        auto stats = tracker.getPeriodStats("2024-02-01", "2024-02-28");
        CHECK(stats["total_spent"] == 0.0);
        CHECK(stats["expense_count"] == 0.0);
    }
    
    SUBCASE("Get period stats - invalid dates") {
        tracker.addExpense(50.0, "Food", "2024-01-15");
        
        CHECK_THROWS_AS(tracker.getPeriodStats("invalid", "2024-01-16"), 
                       std::invalid_argument);
        CHECK_THROWS_AS(tracker.getPeriodStats("2024-01-15", "invalid"), 
                       std::invalid_argument);
        CHECK_THROWS_AS(tracker.getPeriodStats("2024-01-16", "2024-01-15"), 
                       std::invalid_argument);
    }
    
    SUBCASE("Get financial summary") {
        tracker.setBudget(1000.0);
        tracker.addExpense(50.0, "Food");
        tracker.addExpense(30.0, "Transport");
        
        auto summary = tracker.getFinancialSummary();
        CHECK(summary["budget"] == 1000.0);
        CHECK(summary["total_spent"] == 80.0);
        CHECK(summary["remaining"] == 920.0);
    }
    
    SUBCASE("Get financial summary - no budget set") {
        tracker.addExpense(50.0, "Food");
        
        auto summary = tracker.getFinancialSummary();
        CHECK(summary["budget"] == 0.0);
        CHECK(summary["total_spent"] == 50.0);
        CHECK(summary["remaining"] == -50.0);
    }
    
    SUBCASE("Data persistence") {
        tracker.setBudget(500.0);
        tracker.addExpense(25.0, "Coffee", "2024-01-15");
        tracker.addExpense(15.0, "Snacks", "2024-01-16");
        
        // Create new tracker instance with same file
        FinancialTracker tracker2(testFile);
        auto summary = tracker2.getFinancialSummary();
        CHECK(summary["budget"] == 500.0);
        CHECK(summary["total_spent"] == 40.0);
        
        auto stats = tracker2.getCategoryStats();
        CHECK(stats["Coffee"] == 25.0);
        CHECK(stats["Snacks"] == 15.0);
    }
    
    // Clean up test file
    removeFile(testFile);
}

TEST_CASE("Testing edge cases") {
    std::string testFile = "test_financial_data_edge.json";
    removeFile(testFile);
    
    FinancialTracker tracker(testFile);
    
    SUBCASE("Empty tracker") {
        CHECK(tracker.getTotalSpent() == 0.0);
        CHECK(tracker.getCategoryStats().empty());
        
        auto summary = tracker.getFinancialSummary();
        CHECK(summary["budget"] == 0.0);
        CHECK(summary["total_spent"] == 0.0);
        CHECK(summary["remaining"] == 0.0);
    }
    
    SUBCASE("Multiple expenses same category") {
        tracker.addExpense(10.0, "Food");
        tracker.addExpense(20.0, "Food");
        tracker.addExpense(30.0, "Food");
        
        auto stats = tracker.getCategoryStats();
        CHECK(stats["Food"] == 60.0);
        CHECK(stats.size() == 1);
    }
    
    SUBCASE("Expenses with today's date") {
        std::string today = FinancialTracker::getCurrentDate();
        tracker.addExpense(100.0, "Shopping");
        
        auto stats = tracker.getPeriodStats(today, today);
        CHECK(stats["total_spent"] == 100.0);
        CHECK(stats["expense_count"] == 1.0);
    }
    
    SUBCASE("Large amount") {
        tracker.addExpense(999999.99, "Big Purchase");
        CHECK(tracker.getTotalSpent() == 999999.99);
    }
    
    SUBCASE("Category with special characters") {
        tracker.addExpense(50.0, "Food & Drinks");
        auto stats = tracker.getCategoryStats();
        CHECK(stats["Food & Drinks"] == 50.0);
    }
    
    removeFile(testFile);
}

TEST_CASE("Testing error handling") {
    std::string testFile = "test_financial_data_error.json";
    removeFile(testFile);
    
    FinancialTracker tracker(testFile);
    
    SUBCASE("Try to add expense with invalid category") {
        CHECK_THROWS_AS(tracker.addExpense(50.0, ""), std::invalid_argument);
        CHECK_THROWS_AS(tracker.addExpense(50.0, "   "), std::invalid_argument);
    }
    
    SUBCASE("Try to get period stats with invalid date format") {
        CHECK_THROWS_AS(tracker.getPeriodStats("2024/01/15", "2024-01-16"), 
                       std::invalid_argument);
    }
    
    SUBCASE("Try to set invalid budget") {
        CHECK_THROWS_AS(tracker.setBudget(-1), std::invalid_argument);
    }
    
    removeFile(testFile);
}