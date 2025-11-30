#ifndef TEXT_TABLE_H
#define TEXT_TABLE_H

#include <string>
#include <vector>

class TextTable {
 public:
  struct Cell {
    enum Justify { kLeft = 0, kRight = 1, kCenter = 2 };
    enum Color { kWhite = 0, kYellow = 1, kGreen = 2, kRed = 3 };
    std::string entry;
    Justify justify = kLeft;
    Color color = kWhite;
    bool bold = false;
    int span = 1;

    void Render(std::string* out, int width, bool enable_color) const;
  };

  TextTable();

  void AddBreaker() { rows_.emplace_back(Breaker()); }

  void AddRow(std::vector<Cell> row) { rows_.emplace_back(std::move(row)); }

  std::string Render() const;

 private:
  std::vector<int> Layout() const;
  void RenderBreaker(std::string* out, const std::vector<int> col_widths,
                     int row_num) const;

  struct Breaker {};
  using Row = std::variant<Breaker, std::vector<Cell>>;
  bool use_color_ = false;
  std::vector<Row> rows_;
};

#endif  // TEXT_TABLE_HH
