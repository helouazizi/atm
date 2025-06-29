CREATE TABLE
    IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT, -- internal unique id
        username TEXT UNIQUE NOT NULL, -- unique username
        password TEXT NOT NULL
    );

CREATE TABLE IF NOT EXISTS records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    owner TEXT NOT NULL,                    -- ✅ You forgot this in your SQL
    country TEXT NOT NULL,
    phone TEXT NOT NULL,                   -- ✅ was INTEGER; make it TEXT if phone has leading 0s or "+"
    accountType TEXT NOT NULL,             -- ✅ was `acount_type` with typo
    accountNbr INTEGER UNIQUE NOT NULL,
    amount REAL DEFAULT 0,
    deposit DATE,                          -- ✅ was "diposit" (typo)
    withdraw DATE,
    FOREIGN KEY (user_id) REFERENCES users(id)
);
