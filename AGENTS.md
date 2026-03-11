# Agent Guidelines - GitGUIApp

This document provides essential information for autonomous agents working on the GitGUIApp project.

## 1. Build and Development Commands

This project uses CMake and Qt6.

### Build the Project
To perform a clean build:
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Run the Application
```bash
./build/GitGUIApp
```

### Installation/Uninstallation
The build process generates helper scripts in the `build/` directory:
```bash
# Install to /usr/local/bin
sudo ./build/install.sh

# Uninstall
sudo ./build/uninstall.sh
```

### Tests
Currently, there are no automated tests (e.g., CTest or QtTest) integrated into the `CMakeLists.txt`. When adding new features, consider adding corresponding tests.

### Linting/Formatting
No explicit linting configuration (like `.clang-format`) was found. Follow existing patterns or use `clang-format` with default LLVM/Qt style if necessary.

---

## 2. Code Style and Conventions

### C++ Standard
- **Standard:** C++17 (as specified in `CMakeLists.txt`).
- **Required:** Yes.

### Naming Conventions
- **Classes:** `PascalCase` (e.g., `MainWindow`, `GitManager`).
- **Methods/Functions:** `camelCase` (e.g., `setupUi()`, `runGitCommand()`).
- **Variables:** `camelCase` (e.g., `centralWidget`, `stagedList`).
- **Private Member Variables:** `m_` prefix for some (e.g., `m_repositoryPath`) or just `camelCase` in others. Prefer `m_` for consistency in new logic.
- **Files:** `PascalCase` for source files matching class names (e.g., `GitManager.cpp`, `GitManager.h`).

### Header Files
- Use `#ifndef` / `#define` / `#endif` guards named after the file (e.g., `MAINWINDOW_H`).
- Order of includes:
    1. Local header (for .cpp files).
    2. Qt headers (e.g., `<QMainWindow>`).
    3. Standard Library headers.
    4. Other local headers.

### Qt Specifics
- Always include `Q_OBJECT` macro in classes inheriting from `QObject`.
- Use the modern `connect()` syntax:
  ```cpp
  connect(sender, &SenderClass::signal, receiver, &ReceiverClass::slot);
  ```
- Manage memory using Qt's parent-child hierarchy where possible to avoid manual `delete`.

### Error Handling
- Current pattern: Return default-constructed objects (empty `QString`, empty `QList`) or `false` on failure.
- UI: Use `QMessageBox::warning()` or `QMessageBox::critical()` for user-facing errors.
- Logging: Use `qDebug()` for developer-level logging.

### Formatting
- **Indentation:** 4 spaces.
- **Braces:** Open brace on the same line as the statement (e.g., `void func() {`).
- **Pointer/Reference Alignment:** Space before the asterisk/ampersand (e.g., `QWidget *parent`).

### Project Structure
- `src/`: Contains all header and source files.
- `build/`: Build artifacts (ignored by git).
- `CMakeLists.txt`: Root build configuration.

---

## 3. UI Design Guidelines
- The app uses `QSplitter` to separate the sidebar from the main content.
- Sidebar width is fixed at 300px.
- Use `QVBoxLayout` for sidebar organization and `QHBoxLayout` for button groups.
- Favor `QLineEdit::setPlaceholderText()` for user inputs.

---

## 4. Git Operations
- Git commands are executed via `QProcess` in `GitManager`.
- Always use the `--porcelain` flag for parsing `git status` output.
- Use `-U0` for diffs when parsing hunks to avoid context line overhead unless context is needed.

---

## 5. External Rules
No `.cursorrules` or `.github/copilot-instructions.md` were found in this repository. Follow the guidelines in this file as the primary source of truth for agent behavior.
