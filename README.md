A RegEx library
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
    |   |---regex
    |   |---strlx
    |
    |---regex
    |---strlx
    |
    |---tests

---

Module str
--

String library ([strlx/strlx.h](include/strlx/strlx.h)), has two string types:
- str: string slice type, just refers to the data.
- strbuf: A dynamic string builder.

Module re (**WIP**)
---
Basic RegEx module. WORK IN PROGRESS.

