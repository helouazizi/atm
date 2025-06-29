CREATE TABLE
    IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT, -- internal unique id
        username TEXT UNIQUE NOT NULL, -- unique username
        password TEXT NOT NULL
    );

CREATE TABLE
    IF NOT EXISTS records (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        country TEXT NOT NULL,
        phone TEXT NOT NULL,
        acount_type TEXT NOT NULL,
        accountNbr INTEGER UNIQUE NOT NULL,
        amount REAL DEFAULT 0,
        diposit DATE,
        withdraw DATE,
        FOREIGN KEY (user_id) REFERENCES users (id)
    );