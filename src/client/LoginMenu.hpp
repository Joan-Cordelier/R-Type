/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** LoginMenu - Client UI for authentication (login, register, guest)
*/

#ifndef LOGINMENU_HPP_
#define LOGINMENU_HPP_

#include "../common/ecs/registry.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include <functional>
#include <string>
#include <vector>

class LoginMenu {
public:
    using LoginCallback = std::function<void(const std::string& username, const std::string& password)>;
    using RegisterCallback = std::function<void(const std::string& username, const std::string& password)>;
    using GuestCallback = std::function<void()>;

    LoginMenu(Registry &reg);

    void init();
    void setup(ButtonSystem &buttonsys);
    void show();
    void hide();
    bool isVisible() const { return _visible; }

    void setOnLogin(LoginCallback cb) { _onLogin = cb; }
    void setOnRegister(RegisterCallback cb) { _onRegister = cb; }
    void setOnGuest(GuestCallback cb) { _onGuest = cb; }

    // Text input handling
    void handleTextInput(char c);
    void handleBackspace();
    void switchInputField();  // Tab to switch between username/password

    void showError(const std::string& message);
    void clearError();

    const std::string& getUsername() const { return _username; }
    const std::string& getPassword() const { return _password; }

private:
    void updateUsernameDisplay();
    void updatePasswordDisplay();
    void createInputField(float x, float y, const std::string& label, bool isPassword);

    Registry &_reg;
    bool _visible = false;

    // Input state
    std::string _username;
    std::string _password;
    bool _focusOnPassword = false;  // false = username, true = password

    // Entities
    Entity _titleLabel;
    Entity _usernameLabel;
    Entity _usernameInputBg;
    Entity _usernameInputText;
    Entity _passwordLabel;
    Entity _passwordInputBg;
    Entity _passwordInputText;
    Entity _loginButton;
    Entity _loginButtonLabel;
    Entity _registerButton;
    Entity _registerButtonLabel;
    Entity _guestButton;
    Entity _guestButtonLabel;
    Entity _errorLabel;
    Entity _focusIndicator;

    std::vector<Entity> _allEntities;

    // Callbacks
    LoginCallback _onLogin;
    RegisterCallback _onRegister;
    GuestCallback _onGuest;
};

#endif // LOGINMENU_HPP_
