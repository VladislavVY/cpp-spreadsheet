# Spreadsheet
Электронная таблица с базовой функциональностью: возможностью хранения текста и формул с ссылками на другие ячейки.
# Особенности
- Поддержка чисел и строк в ячейках
- Возможность использования формул с числами, строками и ссылками на другие ячейки
- Обработка циклических зависимостей и ошибок в формулах
- Автоматическое обновление значений ячеек при изменении зависимых ячеек
- Использует ANTLR (ANother Tool for Language Recognition) (https://www.antlr.org/) для построения абстрактного синтаксического дерева.
# Инструкция по сборке проекта
1. Установить [Комплект разработки JDK Development Kit](https://www.oracle.com/java/technologies/downloads/)
2. Установить [ANTLR](https://www.antlr.org/)(ANother Tool for Language Recognition), выполнив все пункты в меню Quick Start.
3. Проверить в файлах FindANTLR.cmake и CMakeLists.txt название файла antlr-X.X.X-complete.jar на корректность версии. Вместо "X.X.X" указать свою версию antlr.
4. Создать папку с названием "antlr4_runtime" без кавычек и скачать в неё [файлы](https://github.com/antlr/antlr4/tree/master/runtime/Cpp).
5. Запустить cmake build с CMakeLists.txt.
