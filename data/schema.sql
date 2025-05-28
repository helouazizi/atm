CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- internal unique id
    username TEXT UNIQUE NOT NULL,           -- unique username
    password TEXT NOT NULL
);

CREATE TABLE records (
    id INTEGER PRIMARY KEY AUTOINCREMENT,   -- internal unique id for accounts
    accountNbr INTEGER UNIQUE NOT NULL,     -- account number chosen by user
    ownerUsername TEXT NOT NULL,
    balance REAL DEFAULT 0,
    FOREIGN KEY(ownerUsername) REFERENCES users(username)
);
