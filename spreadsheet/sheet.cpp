#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::variant<Ts...>& var) {
    std::visit([&os](const auto& val) {
        os << val;
    }, var);
    return os;
}

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    ValidatePosition(pos);
    auto cell = cells_.find(pos);

    if (cell == cells_.end()) {
        cells_.emplace(pos, std::make_unique<Cell>(*this));
    }
    cells_.at(pos)->Set(std::move(text));
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetCellPtr(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellPtr(pos);
}

void Sheet::ClearCell(Position pos) {
    ValidatePosition(pos);
    auto cell = cells_.find(pos);
    if (cell != cells_.end() && cell->second) {
        cell->second->Clear();
        if (!cell->second->IsReferenced()) {
            cells_.erase(cell);
        }
    }
}

Size Sheet::GetPrintableSize() const {
    Size size{ 0, 0 };
    for (const auto& [pos, cell_ptr] : cells_) {
        if (cell_ptr) {
            size.rows = std::max(size.rows, pos.row + 1);
            size.cols = std::max(size.cols, pos.col + 1);
        }
    }
    return size;
}

template<typename ValuePrinter>
void Sheet::Print(std::ostream& output, ValuePrinter printer) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) output << '\t';
            const auto& cell_it = cells_.find({ row, col });
            if (cell_it != cells_.end() && cell_it->second) {
                output << printer(*cell_it->second);
            }
        }
        output << '\n';
    }
}

void Sheet::PrintValues(std::ostream& output) const {
    Print(output, [](const Cell& cell) { return cell.GetValue(); });
}

void Sheet::PrintTexts(std::ostream& output) const {
    Print(output, [](const Cell& cell) { return cell.GetText(); });
}

const Cell* Sheet::GetCellPtr(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid cell position");
    }
    const auto cell = cells_.find(pos);
    if (cell == cells_.end()) {
        return nullptr;
    }
    return cells_.at(pos).get();
}

Cell* Sheet::GetCellPtr(Position pos) {
    return const_cast<Cell*>(
        static_cast<const Sheet&>(*this).GetCellPtr(pos));
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::ValidatePosition(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("invalid position");
    }
}
