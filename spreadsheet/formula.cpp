#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression) try : ast_(ParseFormulaAST(expression)) {}
        catch (...) {
            throw FormulaException("formula is incorrect");
    }

   Value Evaluate(const SheetInterface& sheet) const override {
    try {
        std::function<double(Position)> args = [&sheet](const Position pos)->double {
            if (!pos.IsValid()) {
                throw FormulaError(FormulaError::Category::Ref);
            }
            
            const auto* cell = sheet.GetCell(pos);
            if (!cell) {
                return 0.0;
            }
            
            const auto& value = cell->GetValue();
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
            
            if (std::holds_alternative<std::string>(value)) {
                const auto& str_value = std::get<std::string>(value);
                if (str_value.empty()) {
                    return 0.0;
                }
                
                std::istringstream input(str_value);
                double num = 0.0;
                if (!(input >> num) || !input.eof()) {
                    throw FormulaError(FormulaError::Category::Value);
                }
                return num;
            }
            
            throw std::get<FormulaError>(value);
        };
        
        return ast_.Execute(args);
        
    } catch (const FormulaError& evaluate_error) {
        return evaluate_error;
    }
}
    
    std::vector<Position> GetReferencedCells() const override {
        std::vector<Position> cells;
        for (auto cell : ast_.GetCells()) {
            if (cell.IsValid()) cells.push_back(cell);
        }
        cells.resize(std::unique(cells.begin(), cells.end()) - cells.begin());
        return cells;
    }

    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    }

private:
    const FormulaAST ast_;
};

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    }
    catch (...) {
        throw FormulaException("");
    }
}

FormulaError::FormulaError(Category category) 
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Ref:
            return "#REF!";
        case Category::Value:
            return "#VALUE!";
        case Category::Arithmetic:
            return "#ARITHM!";
        }
    return "";
}
