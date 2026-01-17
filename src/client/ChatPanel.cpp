/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ChatPanel - Client UI for text chat implementation
*/

#include "ChatPanel.hpp"
#include <iostream>
#include <algorithm>

ChatPanel::ChatPanel(Registry &reg) : _reg(reg), _unreadCount(0) {
}

void ChatPanel::init() {
    // Chat panel positioned on the right side of the screen
    const float panelX = 750.f;
    const float panelY = 400.f;
    const float panelHeight = 300.f;

    // Toggle button (bottom left corner)
    const float toggleX = 10.f;
    const float toggleY = 680.f;
    
    _toggleButton = _reg.createEntity();
    _reg.addComponent<Position>(_toggleButton, toggleX, toggleY);
    _reg.addComponent<Sprite>(_toggleButton, std::string("textures/button/square_button.png"),
                              std::string("square_button"), 100, 80, 70, -7.5f, -26.f, false);
    _reg.addComponent<Button>(_toggleButton, std::string("chat_toggle"), 100, false);
    _toggleEntities.push_back(_toggleButton);

    _toggleButtonLabel = _reg.createEntity();
    _reg.addComponent<Position>(_toggleButtonLabel, toggleX + 25.f, toggleY + 5.f);
    _reg.addComponent<Label>(_toggleButtonLabel, std::string("Chat"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_tiny"), Color(255, 255, 255), 301, false);
    _toggleEntities.push_back(_toggleButtonLabel);

    // Unread count label (top right of toggle button)
    _unreadCountLabel = _reg.createEntity();
    _reg.addComponent<Position>(_unreadCountLabel, toggleX + 55.f, toggleY - 5.f);
    _reg.addComponent<Label>(_unreadCountLabel, std::string(""),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_tiny"), Color(255, 100, 100), 302, false);
    _toggleEntities.push_back(_unreadCountLabel);

    // Title label
    _titleLabel = _reg.createEntity();
    _reg.addComponent<Position>(_titleLabel, panelX + 10.f, panelY);
    _reg.addComponent<Label>(_titleLabel, std::string("Chat"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(255, 255, 100), 250, false);
    _allEntities.push_back(_titleLabel);

    // Create message label entities (pool)
    for (size_t i = 0; i < MAX_VISIBLE_MESSAGES; ++i) {
        Entity msgLabel = _reg.createEntity();
        _reg.addComponent<Position>(msgLabel, panelX + 10.f, panelY + 30.f + (i * 25.f));
        _reg.addComponent<Label>(msgLabel, std::string(""),
                                 std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                                 std::string("default_font_small"), Color(200, 200, 200), 251, false);
        _messageLabels.push_back(msgLabel);
        _allEntities.push_back(msgLabel);
    }

    // Input text
    _inputTextLabel = _reg.createEntity();
    _reg.addComponent<Position>(_inputTextLabel, panelX + 10.f, panelY + panelHeight - 30.f);
    _reg.addComponent<Label>(_inputTextLabel, std::string("> (Press T to chat)"),
                             std::string("font/josefin-sans/JosefinSans-Regular.ttf"),
                             std::string("default_font_small"), Color(150, 150, 150), 252, false);
    _allEntities.push_back(_inputTextLabel);

    // Start hidden
    hide();
}

void ChatPanel::setup(ButtonSystem &buttonsys) {
    buttonsys.registerHandler("chat_toggle", [this](Registry &, Entity) {
        toggle();
    });
}

void ChatPanel::show() {
    if (!_enabled) return;
    _visible = true;
    for (Entity e : _allEntities) {
        if (_reg.hasComponent<Label>(e)) {
            _reg.getComponent<Label>(e).visible = true;
        }
        if (_reg.hasComponent<Sprite>(e)) {
            _reg.getComponent<Sprite>(e).visible = true;
        }
        if (_reg.hasComponent<Button>(e)) {
            _reg.getComponent<Button>(e).visible = true;
        }
    }
    // Update toggle button label
    if (_reg.hasComponent<Label>(_toggleButtonLabel)) {
        _reg.getComponent<Label>(_toggleButtonLabel).text = "Hide";
    }
    // Reset unread count when showing
    _unreadCount = 0;
    updateUnreadDisplay();
    updateMessageDisplay();
    updateInputDisplay();
}

void ChatPanel::hide() {
    _visible = false;
    _inputFocused = false;
    for (Entity e : _allEntities) {
        if (_reg.hasComponent<Label>(e)) {
            _reg.getComponent<Label>(e).visible = false;
        }
        if (_reg.hasComponent<Sprite>(e)) {
            _reg.getComponent<Sprite>(e).visible = false;
        }
        if (_reg.hasComponent<Button>(e)) {
            _reg.getComponent<Button>(e).visible = false;
        }
    }
    // Update toggle button label
    if (_reg.hasComponent<Label>(_toggleButtonLabel)) {
        _reg.getComponent<Label>(_toggleButtonLabel).text = "Chat";
    }
}

void ChatPanel::toggle() {
    std::cout << "[ChatPanel] toggle() called, _visible=" << _visible << " _enabled=" << _enabled << std::endl;
    if (_visible) {
        hide();
    } else {
        show();
    }
}

void ChatPanel::setEnabled(bool enabled) {
    _enabled = enabled;
    
    // Show/hide toggle button and enable/disable it
    for (Entity e : _toggleEntities) {
        if (_reg.hasComponent<Label>(e)) {
            _reg.getComponent<Label>(e).visible = enabled;
        }
        if (_reg.hasComponent<Sprite>(e)) {
            _reg.getComponent<Sprite>(e).visible = enabled;
        }
        if (_reg.hasComponent<Button>(e)) {
            _reg.getComponent<Button>(e).visible = enabled;
            _reg.getComponent<Button>(e).enabled = enabled;
        }
    }
    
    if (!enabled) {
        hide();
    }
}

void ChatPanel::handleTextInput(char c) {
    if (!_visible || !_inputFocused) return;
    
    // Skip the T character that was used to focus the input
    if (_skipNextTextInput) {
        _skipNextTextInput = false;
        return;
    }
    
    if (_inputBuffer.length() >= MAX_INPUT_LENGTH) return;
    
    // Only allow printable characters
    if (c >= 32 && c < 127) {
        _inputBuffer += c;
        updateInputDisplay();
    }
}

void ChatPanel::handleBackspace() {
    if (!_visible || !_inputFocused) return;
    if (!_inputBuffer.empty()) {
        _inputBuffer.pop_back();
        updateInputDisplay();
    }
}

void ChatPanel::handleEnter() {
    if (!_visible || !_inputFocused) return;
    
    // If empty, just unfocus
    if (_inputBuffer.empty()) {
        unfocusInput();
        return;
    }

    // Send the message
    if (_onSend) {
        _onSend(_inputBuffer);
    }

    // Clear input and unfocus
    _inputBuffer.clear();
    unfocusInput();
}

void ChatPanel::focusInput() {
    _inputFocused = true;
    _skipNextTextInput = true;  // Skip the 'T' character that triggered focus
    updateInputDisplay();
}

void ChatPanel::unfocusInput() {
    _inputFocused = false;
    updateInputDisplay();
}

void ChatPanel::addMessage(const ChatMessage& msg) {
    _messages.push_back(msg);
    
    // Limit history
    while (_messages.size() > MAX_HISTORY) {
        _messages.pop_front();
    }
    
    if (_visible) {
        updateMessageDisplay();
    } else {
        _unreadCount++;
        updateUnreadDisplay();
    }
}

void ChatPanel::addSystemMessage(const std::string& message) {
    ChatMessage sysMsg;
    sysMsg.senderId = 0;
    sysMsg.senderName = "System";
    sysMsg.message = message;
    sysMsg.isSystem = true;
    addMessage(sysMsg);
}

void ChatPanel::setContext(const std::string& context) {
    _context = context;
    
    // Update the title label to show context
    if (_reg.hasComponent<Label>(_titleLabel)) {
        auto& label = _reg.getComponent<Label>(_titleLabel);
        label.text = "Chat - " + _context;
    }
    
    // Clear messages and show context change
    clearMessages();
    addSystemMessage("Entered " + _context);
}

void ChatPanel::clearMessages() {
    _messages.clear();
    if (_visible) {
        updateMessageDisplay();
    }
}

void ChatPanel::updateMessageDisplay() {
    // Show most recent messages
    size_t startIdx = 0;
    if (_messages.size() > MAX_VISIBLE_MESSAGES) {
        startIdx = _messages.size() - MAX_VISIBLE_MESSAGES;
    }

    for (size_t i = 0; i < MAX_VISIBLE_MESSAGES; ++i) {
        if (i + startIdx < _messages.size()) {
            const ChatMessage& msg = _messages[i + startIdx];
            
            // Format: "username: message" or "[System] message"
            std::string displayText;
            Color textColor(200, 200, 200);  // Default gray
            
            if (msg.isSystem) {
                displayText = "[" + msg.message + "]";
                textColor = Color(255, 200, 100);  // Yellow for system
            } else {
                // Truncate if too long
                std::string truncatedMsg = msg.message;
                if (truncatedMsg.length() > 35) {
                    truncatedMsg = truncatedMsg.substr(0, 32) + "...";
                }
                displayText = msg.senderName + ": " + truncatedMsg;
            }
            
            if (_reg.hasComponent<Label>(_messageLabels[i])) {
                auto& label = _reg.getComponent<Label>(_messageLabels[i]);
                label.text = displayText;
                label.color = textColor;
                label.visible = _visible;
            }
        } else {
            // Clear unused slots
            if (_reg.hasComponent<Label>(_messageLabels[i])) {
                auto& label = _reg.getComponent<Label>(_messageLabels[i]);
                label.text = "";
                label.visible = _visible;
            }
        }
    }
}

void ChatPanel::updateInputDisplay() {
    if (!_reg.hasComponent<Label>(_inputTextLabel)) return;
    
    auto& label = _reg.getComponent<Label>(_inputTextLabel);
    
    if (_inputFocused) {
        // Show cursor when focused
        std::string displayText = "> " + _inputBuffer + "_";
        // Truncate if too long for display
        if (displayText.length() > 40) {
            displayText = "> ..." + _inputBuffer.substr(_inputBuffer.length() - 35) + "_";
        }
        label.text = displayText;
        label.color = Color(255, 255, 255);
    } else {
        // Dimmed when not focused
        label.text = "> (Press T to chat)";
        label.color = Color(150, 150, 150);
    }
}

void ChatPanel::updateUnreadDisplay() {
    if (!_reg.hasComponent<Label>(_unreadCountLabel)) return;
    
    auto& label = _reg.getComponent<Label>(_unreadCountLabel);
    
    if (_unreadCount == 0 || _visible) {
        label.text = "";
    } else if (_unreadCount >= 99) {
        label.text = "99+";
    } else {
        label.text = std::to_string(_unreadCount);
    }
}
