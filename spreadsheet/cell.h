#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <optional>
#include <set>

class Sheet;

    class Cell : public CellInterface {
    public:
        class Impl;
        Cell(Sheet& sheet);
        ~Cell();

        void Set(std::string text);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;
    
        bool IsReferenced() const;
        void InvalidateAllCache(bool b);
    
    private:
        std::unique_ptr<Impl> impl_;
        Sheet& sheet_;
    
        std::set<Cell*> dependent_cells_;
        std::set<Cell*> referenced_cells_;
        
        std::unique_ptr<Cell::Impl> CreateImpl(std::string text);
        bool IsFormula(const std::string& text);
        void CheckCircularDependencies(Impl* temp_impl);
        void DetectCircularDependencies(const std::vector<Position>& temp_ref_cells);
        void UpdateDependencies(std::unique_ptr<Impl> new_impl);
    };
        
    class Cell::Impl {
    public:
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
        virtual std::vector<Position> GetReferencedCells() const;
        virtual bool HasCache();
        virtual void InvalidateCache();  
        virtual ~Impl() = default;
    };
    
    class EmptyImpl : public Cell::Impl {
    public:           
        Cell::Value GetValue() const override;
        std::string GetText() const override;      
    };
    
    class TextImpl : public Cell::Impl {
    public:    
        explicit TextImpl(std::string text); 
        Cell::Value GetValue() const override;       
        std::string GetText() const override;       
    private:
        std::string text_;        
    };
    
    class FormulaImpl : public Cell::Impl {
    public:   
        explicit FormulaImpl(std::string text, SheetInterface& sheet);   
        Cell::Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;     
        void InvalidateCache() override; 
        bool HasCache() override;    
    private:
        mutable std::optional<FormulaInterface::Value> cache_;
        std::unique_ptr<FormulaInterface> formula_ptr_;
        SheetInterface& sheet_;
    };
