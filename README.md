# Financial Tracker

A command-line personal finance tracker written in C++17.  
It lets you set a monthly budget, record categorised expenses, and view spending summaries — all stored in a human-readable JSON file that persists between sessions.

---

## Features

- Set and update a monthly budget
- Add expenses with an amount, category, and date
- View a full financial summary (budget / spent / remaining)
- View spending grouped by category
- View statistics for any date range (YYYY-MM-DD)
- Data stored in a plain-text JSON file (path configurable at runtime)
- Full exception-based error handling — no silent failures

---

## Requirements

| Tool | Minimum version |
|------|----------------|
| C++ compiler | C++17 (GCC 7+, Clang 5+, MSVC 2017+) |
| CMake | 3.14+ |
| doctest | v2.4+ (single header bundled in `tests/`) |

---

## Building

```bash
git clone <your-repo-url>
cd financial_tracker

mkdir build && cd build
cmake ..
cmake --build .
```

### Running the app

```bash
# Interactive — prompts for data file path
./financial_tracker

# Non-interactive — pass data file as argument
./financial_tracker /path/to/my_budget.json
```

### Running the tests

```bash
cd build
ctest --output-on-failure
# or run the binary directly for detailed output:
./tracker_tests
```

---

## Usage walkthrough

```
=== Financial Tracker ===
Enter data file path (press Enter for default): 
Data file: financial_data.json

=== Main Menu ===
1. Set Budget
2. Add Expense
3. View Statistics
4. Exit
Choose an option (1-4): 1

Enter your budget: 3000
Budget set to: 3000.00

Choose an option (1-4): 2
Enter amount spent: 450.50
Enter category: Groceries
Enter date (YYYY-MM-DD) or press Enter for today: 
Expense added successfully!

Choose an option (1-4): 3

=== Financial Summary ===
Budget:       3000.00
Total Spent:  450.50
Remaining:    2549.50

=== Spending by Category ===
Groceries: 450.50

=== Period Statistics ===
Enter start date (YYYY-MM-DD) or press Enter to skip: 2024-01-01
Enter end date (YYYY-MM-DD) or press Enter for today: 
From 2024-01-01 to 2024-03-22:
Total spent:       450.50
Number of expenses:1
```

---

## Project structure

```
financial_tracker/
├── CMakeLists.txt              # Build + test configuration
├── README.md
├── include/
│   └── FinancialTracker.h      # Class declaration + Doxygen docs
├── src/
│   ├── FinancialTracker.cpp    # Core logic implementation
│   └── main.cpp               # Interactive CLI entry point
└── tests/
    ├── doctest.h              # Bundled doctest single-header
    └── test_tracker.cpp       # 24 unit tests
```

---

## Data file format

Data is stored as plain JSON and is human-readable:

```json
{
  "budget": 3000.00,
  "expenses": [
    {
      "amount": 450.50,
      "category": "Groceries",
      "date": "2024-03-22"
    }
  ]
}
```

---



### Key methods

```cpp
FinancialTracker tracker("my_data.json");

tracker.setBudget(3000.0);
tracker.addExpense(45.99, "Food", "2024-03-15");

double total = tracker.getTotalSpent();
auto   cats  = tracker.getCategoryStats();          // map<string, double>
auto   ps    = tracker.getPeriodStats("2024-01-01", "2024-03-31");
auto   sum   = tracker.getFinancialSummary();       // {budget, totalSpent, remaining}
```

### Free functions

```cpp
bool        validateDate(const std::string &dateStr);  // "2024-03-15" → true
std::string getTodayDate();                            // → "2024-03-22"
std::string trim(const std::string &str);              // strips whitespace
```

### Error handling

All errors are thrown as standard exceptions and must be caught by the caller:

```cpp
try {
    tracker.setBudget(-100);           // throws std::invalid_argument
    tracker.addExpense(0, "Food");     // throws std::invalid_argument
    tracker.addExpense(10, "");        // throws std::invalid_argument
    tracker.addExpense(10, "X", "bad-date"); // throws std::runtime_error
    tracker.getPeriodStats("2024-12-01", "2024-01-01"); // throws std::invalid_argument
} catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
}
```
