# Source to HTML Converter

## 📌 Project Overview

The **Source to HTML Converter** is a C-based project that converts a C source file into an HTML file with syntax highlighting.

The program reads the source code character by character, identifies different elements of the C program, generates parser events for them, and converts those events into HTML `<span>` tags with appropriate CSS classes.

The generated HTML file can then be viewed as a formatted and syntax-highlighted version of the original C source code.

---

## ✨ Features

* Converts C source code into an HTML file
* Identifies and highlights:

  * Reserved keywords
  * Operators
  * Symbols
  * Numeric constants
  * Strings
  * ASCII character constants
  * Single-line comments
  * Multi-line comments
  * Preprocessor directives
  * Header files
* Preserves spaces and line breaks using the `<pre>` tag
* Uses CSS classes for syntax highlighting
* Generates the output file automatically
* Supports compilation using a Makefile

---

## 🛠️ Technologies Used

* **C Programming**
* **File Handling**
* **Finite State Machine / State-based Parsing**
* **HTML**
* **CSS**
* **Makefile**
* **GCC Compiler**

---

## 📂 Project Structure

```text
s2html/
│
├── s2html_main.c       # Main program
├── s2html_conv.c       # Converts parser events into HTML
├── s2html_conv.h
├── s2html_event.c      # Parser and event generation
├── s2html_event.h      # Event definitions and structures
├── styles.css          # CSS styles for syntax highlighting
├── test.c              # Sample C source file
├── test.c.html         # Generated HTML output
├── Makefile             # Project compilation
└── screenshots/
    ├── source-to-html-output.png
    └── program-execution.png
```

---

## ⚙️ How It Works

The project follows these main steps:

```text
C Source File
      ↓
Parser
      ↓
Identify Source Elements
      ↓
Generate Parser Events
      ↓
HTML Conversion
      ↓
CSS Styling
      ↓
Generated HTML File
```

The parser uses different states to identify source code elements such as keywords, strings, comments, numeric constants, operators, and symbols.

Each recognized element is converted into an event and passed to the HTML conversion function.

For example:

```c
int a = 10;
```

is converted into HTML containing elements such as:

```html
<span class="reserved_key1">int</span>
<span class="operator">=</span>
<span class="numeric_constant">10</span>
<span class="symbol">;</span>
```

---

## 🔨 Compilation Using Makefile

The project uses a Makefile to simplify compilation.

To compile the project:

```bash
make
```

To remove generated object files and the executable:

```bash
make clean
```

---

## ▶️ Running the Project

After compilation, run the converter by providing the C source file as an argument:

```bash
./s2html test.c
```

The program generates:

```text
test.c.html
```

with the message:

```text
Output file test.c.html generated

Source to html conversion is done successfully
```

---

## 📸 Output Demonstration

### Source Code and Generated HTML

The input C source code is converted into HTML with different CSS classes for syntax highlighting.

![Source code and generated HTML](screenshots/source-to-html-output.png)

### Program Compilation and Execution

The project is compiled using the Makefile and successfully generates the HTML output file.

![Program execution](screenshots/program-execution.png)

---

## 🎯 Key Learning Outcomes

Through this project, I learned:

* How to process a source file character by character
* How state-based parsing can be used to identify different source code elements
* How to generate and handle parser events
* File handling in C
* Working with structures and enumerations
* Converting parsed data into HTML
* Using CSS classes for syntax highlighting
* Creating and using a Makefile for project compilation
* Debugging and testing a multi-file C project

---

## 🚀 Future Improvements

Some possible improvements are:

* Support for more C operators and compound operators
* Improved handling of different numeric formats
* Support for additional programming languages
* More advanced syntax highlighting
* Improved error handling for invalid source code

---

## 👩‍💻 Author

**Preethi T M**

A C programming project developed as part of my learning in Embedded C and system-level programming.
