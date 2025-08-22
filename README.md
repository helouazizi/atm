<!-- README.md -->
# 🏦 ATM Management System

A C-based **ATM Simulation System** designed to manage user accounts with operations like registration, login, transactions, ownership transfers, interest calculations, and more. This project uses **SQLite** for data persistence and provides a clean Terminal UI experience with robust input validation.

---

## 🚀 Features

### 1. 👤 User Registration
- Users can **register**  or **login**


---

### 2. 📝 Update Account Information
- Users can update:
  - **Country**
  - **Phone Number**
- The update requires:
  - Account ID input
  - Field selection
  - New value input (validated)
- Changes are persisted into the database.

---

### 3. 📋 View Account Details
- Users can **view individual account** information by inputting the account ID.
- Account types and interest rules:
  - `savings` → 💰 7% interest
  - `fixed01` → 💰 4% interest (1-year)
  - `fixed02` → 💰 5% interest (2-year)
  - `fixed03` → 💰 8% interest (3-year)
  - `current` → ❌ No interest
- Interest display is shown in a human-readable format, e.g.:
  > “You will get $5.97 as interest on day 10 of every month”

---

### 4. 💳 Transactions (Deposit & Withdraw)
- Only allowed for:
  - `savings`
  - `current`
- Prohibited for:
  - `fixed01`, `fixed02`, `fixed03`
- Updated balances are saved after each transaction.
- Invalid inputs or insufficient balances are handled gracefully.

---

### 5. ❌ Remove Account
- Users can remove an account **they own**.
- Requires confirmation: `Are you sure? (y/n)`
- Validates ownership and existence before deletion.

---

### 6. 🔄 Transfer Ownership
- Allows users to transfer an account to **another existing user**.
- Requires:
  - Account ID
  - New owner’s username
- Ownership change is reflected in the database.
- Validates:
  - Ownership
  - New username existence

#### ⚡ Bonus: Real-time Transfer Notification
> Using **pipes and child processes**, when one user transfers an account, the recipient is **instantly notified** in their terminal if logged in.

---

### 🧪 Additional Features 
- 🖼️ **Terminal UI Enhancements**
- 🧰 **Makefile** support for build automation
- 🗃️ **SQLite Database Integration** (instead of plain text files)
- 🧼 Proper **input validation**, **error handling**, and **memory management**
- 🧼 Proper **Pipes and child processes**

---

## 🧱 Technology Stack

| Tech        | Purpose                            |
|-------------|------------------------------------|
| **C**       | Core language                      |
| **SQLite**  | Database for storing accounts      |
| **Makefile**| Build automation                   |
| **ANSI Escape Codes** | Terminal styling & color |
| **Pipes**   | Bonus real-time inter-process alerts |

---

## 🛠️ Build & Run Instructions

### 🔧 Requirements
- GCC compiler
- SQLite development libraries
- Unix-based system (Linux/macOS)



### ⚙️ Build the Project


```bash
make run

````



## Author

[El ouazizi Hassan]

👉 [GitHub](https://github.com/helouazizi/)

