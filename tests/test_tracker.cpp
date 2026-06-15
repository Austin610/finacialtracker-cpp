#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "FinancialTracker.h"

#include <cstdio>
#include <stdexcept>
#include <string>

// ── Helper ────────────────────────────────────────────────────────────────────

static std::string tempFile() {
  return "test_data_" + std::to_string(std::rand()) + ".json";
}

static void removeFile(const std::string &path) { std::remove(path.c_str()); }

// ── trim ──────────────────────────────────────────────────────────────────────

TEST_CASE("trim removes leading and trailing spaces") {
  CHECK(trim("  hello  ") == "hello");
  CHECK(trim("hello") == "hello");
  CHECK(trim("  ") == "");
  CHECK(trim("") == "");
}

TEST_CASE("trim does not modify internal spaces") {
  CHECK(trim("  hello world  ") == "hello world");
}

// ── validateDate ──────────────────────────────────────────────────────────────

TEST_CASE("validateDate accepts valid dates") {
  CHECK(validateDate("2024-01-15") == true);
  CHECK(validateDate("2000-12-31") == true);
  CHECK(validateDate("1999-06-01") == true);
}

TEST_CASE("validateDate rejects invalid dates") {
  CHECK(validateDate("") == false);
  CHECK(validateDate("2024-13-01") == false); // month 13
  CHECK(validateDate("2024-00-10") == false); // month 0
  CHECK(validateDate("2024-01-00") == false); // day 0
  CHECK(validateDate("24-01-15") == false);   // wrong format
  CHECK(validateDate("not-a-date") == false);
  CHECK(validateDate("2024/01/15") == false); // wrong separator
}

// ── getTodayDate ──────────────────────────────────────────────────────────────

TEST_CASE("getTodayDate returns a valid date string") {
  std::string today = getTodayDate();
  CHECK(today.size() == 10);
  CHECK(validateDate(today) == true);
}

// ── FinancialTracker::setBudget ───────────────────────────────────────────────

TEST_CASE("setBudget sets a valid budget") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.setBudget(1500.0);
  CHECK(t.getBudget() == doctest::Approx(1500.0));

  removeFile(f);
}

TEST_CASE("setBudget accepts zero") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.setBudget(0.0);
  CHECK(t.getBudget() == doctest::Approx(0.0));

  removeFile(f);
}

TEST_CASE("setBudget throws on negative value") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK_THROWS_AS(t.setBudget(-100.0), std::invalid_argument);

  removeFile(f);
}

// ── FinancialTracker::addExpense ──────────────────────────────────────────────

TEST_CASE("addExpense adds a valid expense") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.addExpense(50.0, "Food", "2024-03-10");
  CHECK(t.getExpenses().size() == 1);
  CHECK(t.getExpenses()[0].amount == doctest::Approx(50.0));
  CHECK(t.getExpenses()[0].category == "Food");
  CHECK(t.getExpenses()[0].date == "2024-03-10");

  removeFile(f);
}

TEST_CASE("addExpense uses today when date is empty") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.addExpense(20.0, "Transport");
  CHECK(t.getExpenses().size() == 1);
  CHECK(validateDate(t.getExpenses()[0].date) == true);

  removeFile(f);
}

TEST_CASE("addExpense throws on non-positive amount") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK_THROWS_AS(t.addExpense(0.0, "Food"), std::invalid_argument);
  CHECK_THROWS_AS(t.addExpense(-10.0, "Food"), std::invalid_argument);

  removeFile(f);
}

TEST_CASE("addExpense throws on empty category") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK_THROWS_AS(t.addExpense(10.0, ""), std::invalid_argument);
  CHECK_THROWS_AS(t.addExpense(10.0, "   "), std::invalid_argument);

  removeFile(f);
}

TEST_CASE("addExpense throws on invalid date format") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK_THROWS_AS(t.addExpense(10.0, "Food", "01-15-2024"), std::runtime_error);

  removeFile(f);
}

// ── FinancialTracker::getTotalSpent ───────────────────────────────────────────

TEST_CASE("getTotalSpent returns 0 with no expenses") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK(t.getTotalSpent() == doctest::Approx(0.0));

  removeFile(f);
}

TEST_CASE("getTotalSpent sums all expenses") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.addExpense(100.0, "Food", "2024-01-01");
  t.addExpense(50.0, "Transport", "2024-01-02");
  t.addExpense(25.5, "Entertainment", "2024-01-03");

  CHECK(t.getTotalSpent() == doctest::Approx(175.5));

  removeFile(f);
}

// ── FinancialTracker::getCategoryStats ────────────────────────────────────────

TEST_CASE("getCategoryStats groups expenses by category") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.addExpense(100.0, "Food", "2024-01-01");
  t.addExpense(40.0, "Food", "2024-01-02");
  t.addExpense(60.0, "Transport", "2024-01-03");

  auto stats = t.getCategoryStats();
  CHECK(stats["Food"] == doctest::Approx(140.0));
  CHECK(stats["Transport"] == doctest::Approx(60.0));

  removeFile(f);
}

TEST_CASE("getCategoryStats returns empty map when no expenses") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK(t.getCategoryStats().empty() == true);

  removeFile(f);
}

// ── FinancialTracker::getPeriodStats ──────────────────────────────────────────

TEST_CASE("getPeriodStats returns correct stats for a period") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.addExpense(100.0, "Food", "2024-01-05");
  t.addExpense(50.0, "Transport", "2024-01-15");
  t.addExpense(200.0, "Rent", "2024-02-01"); // outside range

  PeriodStats ps = t.getPeriodStats("2024-01-01", "2024-01-31");
  CHECK(ps.totalSpent == doctest::Approx(150.0));
  CHECK(ps.expenseCount == 2);

  removeFile(f);
}

TEST_CASE("getPeriodStats returns zeros when no expenses in range") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.addExpense(100.0, "Food", "2024-03-10");

  PeriodStats ps = t.getPeriodStats("2024-01-01", "2024-01-31");
  CHECK(ps.totalSpent == doctest::Approx(0.0));
  CHECK(ps.expenseCount == 0);

  removeFile(f);
}

TEST_CASE("getPeriodStats throws when start date is after end date") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK_THROWS_AS(t.getPeriodStats("2024-12-01", "2024-01-01"),
                  std::invalid_argument);

  removeFile(f);
}

TEST_CASE("getPeriodStats throws on invalid date format") {
  std::string f = tempFile();
  FinancialTracker t(f);

  CHECK_THROWS_AS(t.getPeriodStats("not-a-date", "2024-01-31"),
                  std::invalid_argument);

  removeFile(f);
}

// ── FinancialTracker::getFinancialSummary ─────────────────────────────────────

TEST_CASE("getFinancialSummary calculates correct remaining budget") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.setBudget(500.0);
  t.addExpense(150.0, "Food", "2024-01-10");
  t.addExpense(80.0, "Transport", "2024-01-11");

  FinancialSummary s = t.getFinancialSummary();
  CHECK(s.budget == doctest::Approx(500.0));
  CHECK(s.totalSpent == doctest::Approx(230.0));
  CHECK(s.remaining == doctest::Approx(270.0));

  removeFile(f);
}

TEST_CASE("getFinancialSummary shows negative remaining when over budget") {
  std::string f = tempFile();
  FinancialTracker t(f);

  t.setBudget(100.0);
  t.addExpense(150.0, "Rent", "2024-01-01");

  FinancialSummary s = t.getFinancialSummary();
  CHECK(s.remaining == doctest::Approx(-50.0));

  removeFile(f);
}

// ── Persistence ───────────────────────────────────────────────────────────────

TEST_CASE("data persists between FinancialTracker instances") {
  std::string f = tempFile();

  {
    FinancialTracker t(f);
    t.setBudget(1000.0);
    t.addExpense(200.0, "Food", "2024-05-01");
  }

  {
    FinancialTracker t(f);
    CHECK(t.getBudget() == doctest::Approx(1000.0));
    CHECK(t.getExpenses().size() == 1);
    CHECK(t.getExpenses()[0].amount == doctest::Approx(200.0));
    CHECK(t.getExpenses()[0].category == "Food");
  }

  removeFile(f);
}
