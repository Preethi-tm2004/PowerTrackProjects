# TEXT EDITOR

## Project Description

This project implements a **console-based Text Editor in C** with basic text editing operations and advanced **Undo/Redo functionality**.

The editor manages text line-by-line using a **Doubly Linked List** and implements Undo/Redo operations using **dynamic array stacks**.

The project demonstrates important Data Structures and C programming concepts such as linked lists, stacks, dynamic memory allocation, pointers, structures, string handling, and modular programming.

---

## Features

The Text Editor provides the following operations:

* Insert Text
* Delete Text
* Move Cursor Up
* Move Cursor Down
* Move Cursor Left
* Move Cursor Right
* Undo
* Redo
* Display Text
* Exit

---

## Data Structures Used

### 1. Doubly Linked List

A doubly linked list is used to store the lines of text.

Each node contains:

* Text content of the line
* Pointer to the previous line
* Pointer to the next line

This allows movement between lines and efficient cursor navigation.

### 2. Dynamic Array Stack

Dynamic array stacks are used to implement:

* Undo operation
* Redo operation

Editing operations are stored so that they can be reversed using **Undo** or reapplied using **Redo**.

---

## Concepts Demonstrated

* Structures
* Pointers
* Doubly Linked Lists
* Stack
* Dynamic Memory Allocation
* String Handling
* Modular Programming
* Cursor Management
* Undo and Redo Mechanism
* Menu-driven Programming

---

## Project Structure

```text
Text_editor/
│
├── main.c
├── text_editor.c
├── text_editor.h
├── undo_redo.c
├── undo_redo.h
├── Makefile
└── README.md
```

---

## Compilation

The project can be compiled using the provided Makefile.

```bash
make
```

To remove the generated object files and executable:

```bash
make clean
```

---

## Running the Project

After compilation, run the executable:

```bash
./text_editor
```

---

## Main Menu

```text
==================TEXT EDITOR==================
1. Insert Text
2. Delete Text
3. Move Cursor Up
4. Move Cursor Down
5. Move Cursor Left
6. Move Cursor Right
7. Undo
8. Redo
9. Display Text
10. Exit
```

The user can select an operation by entering the corresponding menu option.

---

## Undo / Redo

The editor supports both **Undo** and **Redo** operations.

### Undo

Undo reverses the most recent editing operation.

```text
Insert Text
      ↓
Undo
      ↓
Inserted text is removed
```

### Redo

Redo reapplies an operation that was previously undone.

```text
Insert Text
      ↓
Undo
      ↓
Redo
      ↓
Inserted text is restored
```

The Undo and Redo stacks are maintained independently to manage the editing history.

---

## Working Principle

The text editor maintains the document as a collection of lines using a doubly linked list.

The cursor keeps track of the current position in the document.

When an editing operation is performed:

```text
User Input
    ↓
Text Editing Operation
    ↓
Update Doubly Linked List
    ↓
Store Operation in Undo Stack
```

When Undo is selected:

```text
Undo Stack
    ↓
Retrieve Previous Operation
    ↓
Reverse Operation
    ↓
Store Operation in Redo Stack
```

When Redo is selected:

```text
Redo Stack
    ↓
Retrieve Operation
    ↓
Reapply Operation
    ↓
Store Operation in Undo Stack
```

---

## Learning Outcomes

Through this project, the following concepts were practiced:

* Designing a data structure-based application
* Managing dynamically allocated memory
* Implementing a doubly linked list
* Implementing stack operations
* Managing cursor movement
* Maintaining editing history
* Implementing Undo/Redo functionality
* Writing modular and maintainable C code

---

## Author

**Preethi T M**

**Project:** Text Editor

**Language:** C
