/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LoginMenu - Client UI for authentication implementation
*/

#include "LoginMenu.hpp"
#include <iostream>

LoginMenu::LoginMenu(Registry &reg) : _reg(reg) {
}

void LoginMenu::init() {
    const float screenCenterX = 540.f;
    const float startY = 100.f;

    // Title
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, screenCenterX - 80.f, startY);
    _reg.addComponent<Label>(_titleLabel, std::string("R-Type Login"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font"), Color(255, 255, 255), 200, false);
    _allEntities.push_back(_titleLabel);

    // Username label
    _usernameLabel = _reg.createEntity();
    _reg.addComponent<Position>(_usernameLabel, screenCenterX - 150.f, startY + 80.f);
    _reg.addComponent<Label>(_usernameLabel, std::string("Username:"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 200, false);
    _allEntities.push_back(_usernameLabel);

    // Username input background (no sprite, just position reference)
    _usernameInputBg = _reg.createEntity();
    _reg.addComponent<Position>(_usernameInputBg, screenCenterX - 150.f, startY + 110.f);
    _allEntities.push_back(_usernameInputBg);

    // Username input text
    _usernameInputText = _reg.createEntity();
    _reg.addComponent<Position>(_usernameInputText, screenCenterX - 140.f, startY + 118.f);
    _reg.addComponent<Label>(_usernameInputText, std::string("_"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 201, false);
    _allEntities.push_back(_usernameInputText);

    // Password label
    _passwordLabel = _reg.createEntity();
    _reg.addComponent<Position>(_passwordLabel, screenCenterX - 150.f, startY + 160.f);
    _reg.addComponent<Label>(_passwordLabel, std::string("Password:"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 200, false);
    _allEntities.push_back(_passwordLabel);

    // Password input background (no sprite, just position reference)
    _passwordInputBg = _reg.createEntity();
    _reg.addComponent<Position>(_passwordInputBg, screenCenterX - 150.f, startY + 190.f);
    _allEntities.push_back(_passwordInputBg);

    // Password input text (shows asterisks)
    _passwordInputText = _reg.createEntity();
    _reg.addComponent<Position>(_passwordInputText, screenCenterX - 140.f, startY + 198.f);
    _reg.addComponent<Label>(_passwordInputText, std::string(""),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 255), 201, false);
    _allEntities.push_back(_passwordInputText);

    // Focus indicator (shows which field is active)
    _focusIndicator = _reg.createEntity();
    _reg.addComponent<Position>(_focusIndicator, screenCenterX - 160.f, startY + 118.f);
    _reg.addComponent<Label>(_focusIndicator, std::string(">"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 0), 202, false);
    _allEntities.push_back(_focusIndicator);

    // Login button
    float buttonY = startY + 260.f;
    _loginButton = _reg.createEntity();
    _reg.addComponent<Position>(_loginButton, screenCenterX - 250.f, buttonY - 50.f);
    _reg.addComponent<Sprite>(_loginButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 200, 160, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_loginButton, std::string("login_submit"), 200, false);
    _allEntities.push_back(_loginButton);

    _loginButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_loginButtonLabel, screenCenterX - 185.f, buttonY + 15.f);
    _reg.addComponent<Label>(_loginButtonLabel, std::string("Login"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(100, 255, 100), 201, false);
    _allEntities.push_back(_loginButtonLabel);

    // Register button
    _registerButton = _reg.createEntity();
    _reg.addComponent<Position>(_registerButton, screenCenterX - 95.f, buttonY - 50.f);
    _reg.addComponent<Sprite>(_registerButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 200, 160, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_registerButton, std::string("login_register"), 200, false);
    _allEntities.push_back(_registerButton);

    _registerButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_registerButtonLabel, screenCenterX - 45.f, buttonY + 15.f);
    _reg.addComponent<Label>(_registerButtonLabel, std::string("Register"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(100, 100, 255), 201, false);
    _allEntities.push_back(_registerButtonLabel);

    // Guest button
    _guestButton = _reg.createEntity();
    _reg.addComponent<Position>(_guestButton, screenCenterX + 70.f, buttonY - 50.f);
    _reg.addComponent<Sprite>(_guestButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 200, 160, 200, 0.f, 0.f, false);
    _reg.addComponent<Button>(_guestButton, std::string("login_guest"), 200, false);
    _allEntities.push_back(_guestButton);

    _guestButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_guestButtonLabel, screenCenterX + 130.f, buttonY + 15.f);
    _reg.addComponent<Label>(_guestButtonLabel, std::string("Guest"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(200, 200, 200), 201, false);
    _allEntities.push_back(_guestButtonLabel);

    // Error label (hidden by default)
    _errorLabel = _reg.createEntity();
    _reg.addComponent<Position>(_errorLabel, screenCenterX - 150.f, startY + 330.f);
    _reg.addComponent<Label>(_errorLabel, std::string(""),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 100, 100), 202, false);
    _allEntities.push_back(_errorLabel);

    std::cout << "[LoginMenu] Initialized with " << _allEntities.size() << " entities" << std::endl;
}

void LoginMenu::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("login_submit", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_visible && _onLogin && !_username.empty() && !_password.empty()) {
            clearError();
            _onLogin(_username, _password);
        } else if (_username.empty() || _password.empty()) {
            showError("Please enter username and password");
        }
    });

    buttonsys.registerHandler("login_register", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_visible && _onRegister && !_username.empty() && !_password.empty()) {
            clearError();
            _onRegister(_username, _password);
        } else if (_username.empty() || _password.empty()) {
            showError("Please enter username and password");
        }
    });

    buttonsys.registerHandler("login_guest", [this]([[maybe_unused]] Registry &r, [[maybe_unused]] Entity e) {
        if (_visible && _onGuest) {
            clearError();
            _onGuest();
        }
    });
}

void LoginMenu::show() {
    _visible = true;
    _username.clear();
    _password.clear();
    _focusOnPassword = false;
    clearError();

    for (auto &entity : _allEntities) {
        if (_reg.hasComponent<Sprite>(entity)) {
            _reg.getComponent<Sprite>(entity).visible = true;
        }
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = true;
        }
        if (_reg.hasComponent<Button>(entity)) {
            _reg.getComponent<Button>(entity).enabled = true;
        }
    }

    updateUsernameDisplay();
    updatePasswordDisplay();

    // Update focus indicator position
    if (_reg.hasComponent<Position>(_focusIndicator)) {
        _reg.getComponent<Position>(_focusIndicator).y = _focusOnPassword ? 298.f : 218.f;
    }

    std::cout << "[LoginMenu] Shown" << std::endl;
}

void LoginMenu::hide() {
    _visible = false;

    for (auto &entity : _allEntities) {
        if (_reg.hasComponent<Sprite>(entity)) {
            _reg.getComponent<Sprite>(entity).visible = false;
        }
        if (_reg.hasComponent<Label>(entity)) {
            _reg.getComponent<Label>(entity).visible = false;
        }
        if (_reg.hasComponent<Button>(entity)) {
            _reg.getComponent<Button>(entity).enabled = false;
        }
    }

    std::cout << "[LoginMenu] Hidden" << std::endl;
}

void LoginMenu::handleTextInput(char c) {
    if (!_visible) return;

    // Only allow printable ASCII characters
    if (c < 32 || c > 126) return;

    if (_focusOnPassword) {
        if (_password.length() < 32) {
            _password += c;
            updatePasswordDisplay();
        }
    } else {
        if (_username.length() < 20) {
            _username += c;
            updateUsernameDisplay();
        }
    }
}

void LoginMenu::handleBackspace() {
    if (!_visible) return;

    if (_focusOnPassword) {
        if (!_password.empty()) {
            _password.pop_back();
            updatePasswordDisplay();
        }
    } else {
        if (!_username.empty()) {
            _username.pop_back();
            updateUsernameDisplay();
        }
    }
}

void LoginMenu::switchInputField() {
    if (!_visible) return;

    _focusOnPassword = !_focusOnPassword;

    // Update focus indicator position
    if (_reg.hasComponent<Position>(_focusIndicator)) {
        _reg.getComponent<Position>(_focusIndicator).y = _focusOnPassword ? 298.f : 218.f;
    }

    // Update display to show/hide cursors
    updateUsernameDisplay();
    updatePasswordDisplay();
}

void LoginMenu::updateUsernameDisplay() {
    if (_reg.hasComponent<Label>(_usernameInputText)) {
        Label &label = _reg.getComponent<Label>(_usernameInputText);
        if (_username.empty() && !_focusOnPassword) {
            label.text = "_";  // Cursor when empty and focused
        } else if (_username.empty()) {
            label.text = "";
        } else if (!_focusOnPassword) {
            label.text = _username + "_";  // Show cursor
        } else {
            label.text = _username;
        }
    }
}

void LoginMenu::updatePasswordDisplay() {
    if (_reg.hasComponent<Label>(_passwordInputText)) {
        Label &label = _reg.getComponent<Label>(_passwordInputText);
        std::string masked(_password.length(), '*');
        if (_password.empty() && _focusOnPassword) {
            label.text = "_";  // Cursor when empty and focused
        } else if (_password.empty()) {
            label.text = "";
        } else if (_focusOnPassword) {
            label.text = masked + "_";  // Show cursor
        } else {
            label.text = masked;
        }
    }
}

void LoginMenu::showError(const std::string& message) {
    if (_reg.hasComponent<Label>(_errorLabel)) {
        _reg.getComponent<Label>(_errorLabel).text = message;
    }
}

void LoginMenu::clearError() {
    if (_reg.hasComponent<Label>(_errorLabel)) {
        _reg.getComponent<Label>(_errorLabel).text = "";
    }
}
