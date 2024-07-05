#include "cell.h"
#include "sheet.h"
 
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
 
Cell::Cell(Sheet& sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    auto temp_impl = CreateImpl(std::move(text));
    CheckCircularDependencies(temp_impl.get());
    UpdateDependencies(std::move(temp_impl));
    InvalidateAllCache(true);
}

std::unique_ptr<Cell::Impl> Cell::CreateImpl(std::string text) {
    if (text.empty()) {
        return std::make_unique<EmptyImpl>();
    } else if (IsFormula(text)) {
        return std::make_unique<FormulaImpl>(std::move(text), sheet_);
    } else {
        return std::make_unique<TextImpl>(std::move(text));
    }
}

bool Cell::IsFormula(const std::string& text) {
    return text.size() >= 2 && text.at(0) == FORMULA_SIGN;
}

void Cell::CheckCircularDependencies(Impl* temp_impl) {
    const auto temp_cells = temp_impl->GetReferencedCells();
    if (!temp_cells.empty()) {
        DetectCircularDependencies(temp_cells);
    }
}

void Cell::DetectCircularDependencies(const std::vector<Position>& temp_cells) {
    std::set<const Cell*> ref_collect;
    std::set<const Cell*> enter_collect;
    std::vector<const Cell*> to_enter_collect;
    for (auto position : temp_cells) {
        ref_collect.insert(sheet_.GetCellPtr(position));
    }
    to_enter_collect.push_back(this);
    while (!to_enter_collect.empty()) {
        const Cell* ongoing = to_enter_collect.back();
        to_enter_collect.pop_back();
        enter_collect.insert(ongoing);

        if (ref_collect.find(ongoing) == ref_collect.end()) {
            for (const Cell* dependent : ongoing->dependent_cells_) {
                if (enter_collect.find(dependent) == enter_collect.end()) {
                    to_enter_collect.push_back(dependent);
                }
            }
        } else {
            throw CircularDependencyException("circular dependency detected");
        }
    }
}

void Cell::UpdateDependencies(std::unique_ptr<Impl> new_impl) {
    for (Cell* referenced : referenced_cells_) {
        referenced->dependent_cells_.erase(this);
    }
    referenced_cells_.clear();
    for (const auto& position : new_impl->GetReferencedCells()) {
        Cell* referenced = sheet_.GetCellPtr(position);
      
        if (!referenced) {
            sheet_.SetCell(position, "");
            referenced = sheet_.GetCellPtr(position);
        }
        referenced_cells_.insert(referenced);
        referenced->dependent_cells_.insert(this);
    }
    impl_ = std::move(new_impl);
}
 
void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}
 
Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

bool Cell::IsReferenced() const {
    return !dependent_cells_.empty();
}
 
void Cell::InvalidateAllCache(bool b = false) {
    if (impl_->HasCache() || b) {
        impl_->InvalidateCache();      
        for (Cell* dependent : dependent_cells_) {
            dependent->InvalidateAllCache();
        }
    }
}
 
std::vector<Position> Cell::Impl::GetReferencedCells() const {
    return {};
}

bool Cell::Impl::HasCache() {
    return true;
}

void Cell::Impl::InvalidateCache() {}
 
Cell::Value EmptyImpl::GetValue() const {
    return "";
}

std::string EmptyImpl::GetText() const {
    return "";
}
 
TextImpl::TextImpl(std::string text) : text_(std::move(text)) {}
 
Cell::Value TextImpl::GetValue() const { 
    if (text_.empty()) {
        throw FormulaException("its empty impl");       
    } else if (text_.at(0) == ESCAPE_SIGN) {
        return text_.substr(1);      
    } else {
        return text_;    
    }      
}
 
std::string TextImpl::GetText() const {
    return text_;
}
 
FormulaImpl::FormulaImpl(std::string text, SheetInterface& sheet) : formula_ptr_(ParseFormula(text.substr(1))), sheet_(sheet) {}
 
Cell::Value FormulaImpl::GetValue() const {             
    if (!cache_) {cache_ = formula_ptr_->Evaluate(sheet_);}     
    return std::visit([](auto& helper){return Cell::Value(helper);}, *cache_);        
}
 
std::string FormulaImpl::GetText() const {
    return FORMULA_SIGN + formula_ptr_->GetExpression();
}

std::vector<Position> FormulaImpl::GetReferencedCells() const {
    return formula_ptr_->GetReferencedCells();
}

bool FormulaImpl::HasCache() {
    return cache_.has_value();
}

void FormulaImpl::InvalidateCache() {
    cache_.reset();
}
