#pragma once
#include <sqlite3.h>
#include <string>
class Database
{
  public:
    explicit Database(const std::string &path);
    ~Database();

    bool isOpen() const;
    sqlite3 *handle() const;

  private:
    sqlite3 *db = nullptr;
};
