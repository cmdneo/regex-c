A basic RegEx library
===

Project structure
---

    .
    |---.gitignore
    |---CMakeLists.txt
    |---DOoxyfile
    |---gentests.py
    |---README.md
    |
    |---docs
    |---include
    |   |---<module1_name>
    |   |    |---internal
    |   |---<module2_name>
    |   |    |---internal
    |   |---..............
    |
    |---<module1_name>
    |---<module2_name>
    |---..............
    |
    |---tests

---

Module str
--

Basic string operations library, has two string types:
- str: string slice type, just refers to data.
- strbuf: A string builder which which does allocate memory and manages its own data

Module re (**WIP**)
---
Basic RegEx module. WORK IN PROGRESS.

