#include "text_table.h"

#include "absl/algorithm/container.h"
#include "absl/container/flat_hash_set.h"
#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "vlog.h"

#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#endif

enum ColorType {
  kAuto = 0,
  kNone = 1,
  kForce = 2,
};

ABSL_FLAG(ColorType, color, ColorType::kAuto, "Color mode");

bool AbslParseFlag(std::string_view text, ColorType* c, std::string* error) {
  if (text == "auto") {
    *c = kAuto;
    return true;
  }
  if (text == "none") {
    *c = kNone;
    return true;
  }
  if (text == "force") {
    *c = kForce;
    return true;
  }
  *error = "Color mode must be one of 'auto', 'none', or 'force'";
  return false;
}

std::string AbslUnparseFlag(ColorType c) {
  switch (c) {
    case kAuto:
      return "auto";
    case kNone:
      return "none";
    case kForce:
      return "force";
  }
  return "<Bad ColorType>";
}

namespace {

bool IsColorTerminal() {
  // This list of supported TERM values is copied from Google Test:
  // <https://github.com/google/googletest/blob/v1.13.0/googletest/src/gtest.cc#L3225-L3259>.
  const char* const SUPPORTED_TERM_VALUES[] = {
      "xterm",
      "xterm-color",
      "xterm-256color",
      "screen",
      "screen-256color",
      "tmux",
      "tmux-256color",
      "rxvt-unicode",
      "rxvt-unicode-256color",
      "linux",
      "cygwin",
      "xterm-kitty",
      "alacritty",
      "foot",
      "foot-extra",
      "wezterm",
  };

  if (isatty(fileno(stdout)) == 0) {
    return false;
  }

#ifdef _WIN32
  return getenv("POWERSHELL_DISTRIBUTION_CHANNEL") != nullptr;
#endif

  const char* const term = getenv("TERM");

  bool term_supports_color = false;
  for (const char* candidate : SUPPORTED_TERM_VALUES) {
    if (term && 0 == strcmp(term, candidate)) {
      term_supports_color = true;
      break;
    }
  }

  return term_supports_color;
}

}  // namespace

TextTable::TextTable() {
  switch (absl::GetFlag(FLAGS_color)) {
    case kNone:
      use_color_ = false;
      break;
    case kForce:
      use_color_ = true;
      break;
    case kAuto:
      use_color_ = IsColorTerminal();
      break;
  }
}

std::vector<int> TextTable::Layout() const {
  absl::flat_hash_set<int> spans;
  int max_span = 0;
  for (const Row& row : rows_) {
    if (!std::holds_alternative<std::vector<Cell>>(row)) {
      continue;
    }
    const auto& cells = std::get<std::vector<Cell>>(row);
    int row_span = 0;
    for (const Cell& cell : cells) {
      spans.insert(cell.span);
      row_span += cell.span;
    }
    max_span = std::max(max_span, row_span);
  }
  std::vector<int> col_widths(max_span, 0);
  std::vector<int> spans_vec(spans.begin(), spans.end());
  absl::c_sort(spans_vec);
  for (int cur_span : spans_vec) {
    for (const Row& row : rows_) {
      if (!std::holds_alternative<std::vector<Cell>>(row)) {
        continue;
      }
      const auto& cells = std::get<std::vector<Cell>>(row);
      int col_idx = 0;
      for (const Cell& cell : cells) {
        if (cell.span == cur_span) {
          int need = cell.entry.size();
          int have = 0;
          for (int i = col_idx; i < col_idx + cell.span; ++i) {
            if (i != col_idx) have += 3;  // Margin.
            have += col_widths[i];
          }
          if (need > have) {
            // TODO(@monkeynova): This takes a budget and adds it to each
            // column evenly. When we have different lengthts to start with
            // we should prefer evening those lenghts out first.
            int delta = need - have;
            int add_per_col = delta / cell.span;
            int change_at = col_idx + cell.span;
            if (delta % cell.span != 0) {
              change_at = col_idx + delta % cell.span;
              ++add_per_col;
            }
            for (int i = col_idx; i < col_idx + cell.span; ++i) {
              col_widths[i] += add_per_col;
              if (i == change_at) {
                --add_per_col;
              }
            }
          }
        }
        col_idx += cell.span;
      }
    }
  }
  return col_widths;
}

std::string TextTable::Render() const {
  std::string ret;

  std::vector<int> col_widths = Layout();
  for (int i = 0; i < rows_.size(); ++i) {
    if (std::holds_alternative<Breaker>(rows_[i])) {
      RenderBreaker(&ret, col_widths, i);
    } else if (std::holds_alternative<std::vector<Cell>>(rows_[i])) {
      ret.append(1, '|');
      const std::vector<Cell>& cells = std::get<std::vector<Cell>>(rows_[i]);
      int col_idx = 0;
      for (const Cell& cell : cells) {
        int width = 0;
        for (int i = col_idx; i < col_idx + cell.span; ++i) {
          if (i > col_idx) width += 3;  // Margin.
          width += col_widths[i];
        }
        ret.append(1, ' ');
        cell.Render(&ret, width, use_color_);
        ret.append(" |");
        col_idx += cell.span;
      }
    }
    ret.append("\n");
  }
  return ret;
}

void TextTable::RenderBreaker(std::string* out,
                              const std::vector<int> col_widths,
                              int row_num) const {
  std::vector<int> prev;
  if (row_num > 0 &&
      std::holds_alternative<std::vector<Cell>>(rows_[row_num - 1])) {
    int col_idx = 0;
    for (const Cell& cell : std::get<std::vector<Cell>>(rows_[row_num - 1])) {
      prev.push_back(col_idx);
      col_idx += cell.span;
    }
    prev.push_back(col_idx);
  }
  std::vector<int> next;
  if (row_num < rows_.size() - 1 &&
      std::holds_alternative<std::vector<Cell>>(rows_[row_num + 1])) {
    int col_idx = 0;
    for (const Cell& cell : std::get<std::vector<Cell>>(rows_[row_num + 1])) {
      next.push_back(col_idx);
      col_idx += cell.span;
    }
    next.push_back(col_idx);
  }

  auto prev_it = prev.begin();
  auto next_it = next.begin();
  for (int i = 0; i < col_widths.size(); ++i) {
    bool col_match = false;
    if (prev_it != prev.end() && *prev_it == i) {
      col_match = true;
      ++prev_it;
    }
    if (next_it != next.end() && *next_it == i) {
      col_match = true;
      ++next_it;
    }
    out->append(1, col_match ? '+' : '-');
    out->append(col_widths[i] + 2, '-');
  }
  bool col_match = false;
  if (prev_it != prev.end() && *prev_it == col_widths.size()) {
    col_match = true;
    ++prev_it;
  }
  if (next_it != next.end() && *next_it == col_widths.size()) {
    col_match = true;
    ++next_it;
  }
  CHECK(prev_it == prev.end());
  CHECK(next_it == next.end());
  out->append(1, col_match ? '+' : '-');
}

void TextTable::Cell::Render(std::string* out, int width,
                             bool enable_color) const {
  bool need_reset = false;
  if (enable_color) {
    if (bold) {
      out->append("\u001b[1m");
      need_reset = true;
    }
    switch (color) {
      case kWhite:
        break;
      case kRed: {
        out->append("\u001b[31m");
        need_reset = true;
        break;
      }
      case kGreen: {
        out->append("\u001b[32m");
        need_reset = true;
        break;
      }
      case kYellow: {
        out->append("\u001b[33m");
        need_reset = true;
        break;
      }
    }
  }
  switch (justify) {
    case kRight: {
      out->append(width - entry.size(), ' ');
      out->append(entry);
      break;
    }
    case kLeft: {
      out->append(entry);
      out->append(width - entry.size(), ' ');
      break;
    }
    case kCenter: {
      int left = (width - entry.size()) / 2;
      int right = width - entry.size() - left;
      out->append(left, ' ');
      out->append(entry);
      out->append(right, ' ');
      break;
    }
  }
  if (need_reset) {
    out->append("\u001b[0m");
  }
}
