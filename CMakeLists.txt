cmake_minimum_required(VERSION 3.16)
project(GitGUIApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Core Network REQUIRED)

add_executable(GitGUIApp
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/GitManager.cpp
    src/GitManager.h
    src/DiffView.cpp
    src/DiffView.h
    src/SyntaxHighlighter.cpp
    src/SyntaxHighlighter.h
    src/CommitItemWidget.cpp
    src/CommitItemWidget.h
    src/AIHandler.cpp
    src/AIHandler.h
)

target_link_libraries(GitGUIApp PRIVATE Qt6::Widgets Qt6::Core Qt6::Network)

# Generate install and uninstall scripts in the build folder
set(INSTALL_SCRIPT "${CMAKE_BINARY_DIR}/install.sh")
set(UNINSTALL_SCRIPT "${CMAKE_BINARY_DIR}/uninstall.sh")

file(WRITE ${INSTALL_SCRIPT}
"#!/bin/bash
# Install script for GitGUIApp
DEST_DIR=\"/usr/local/bin\"
echo \"Installing GitGUIApp to $DEST_DIR...\"
sudo cp \"\$(dirname \"\$0\")/GitGUIApp\" \"\$DEST_DIR/\"
sudo chmod +x \"\$DEST_DIR/GitGUIApp\"
echo \"Installation complete.\"
")

file(WRITE ${UNINSTALL_SCRIPT}
"#!/bin/bash
# Uninstall script for GitGUIApp
DEST_DIR=\"/usr/local/bin\"
echo \"Uninstalling GitGUIApp from $DEST_DIR...\"
sudo rm \"$DEST_DIR/GitGUIApp\"
echo \"Uninstallation complete.\"
")

# Make scripts executable
execute_process(COMMAND chmod +x ${INSTALL_SCRIPT} ${UNINSTALL_SCRIPT})
