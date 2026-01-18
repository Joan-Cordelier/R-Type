/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ChatPanel - Client UI for text chat in lobby and game
*/

#ifndef CHATPANEL_HPP_
#define CHATPANEL_HPP_

#include "../common/ecs/registry.hpp"
#include "../common/ecs/ecs_components.hpp"
#include "../common/ecs/ecs_system.hpp"
#include <functional>
#include <string>
#include <vector>
#include <deque>

struct ChatMessage {
    uint32_t senderId;
    std::string senderName;
    std::string message;
    bool isSystem = false;  // For system messages like "Player joined"
};

class ChatPanel {
public:
    using SendCallback = std::function<void(const std::string& message)>;

    ChatPanel(Registry &reg);

    void init();
    void setup(ButtonSystem &buttonsys);  // Register button handlers
    void show();
    void hide();
    void toggle();  // Toggle chat visibility
    bool isVisible() const { return _visible; }
    bool isInputFocused() const { return _inputFocused; }
    bool isEnabled() const { return _enabled; }  // Whether chat is available

    void setEnabled(bool enabled);  // Enable/disable chat (shows/hides toggle button)
    void setOnSend(SendCallback cb) { _onSend = cb; }

    // Text input handling
    void handleTextInput(char c);
    void handleBackspace();
    void handleEnter();  // Send message
    void focusInput();
    void unfocusInput();

    // Add message to chat history
    void addMessage(const ChatMessage& msg);
    void addSystemMessage(const std::string& message);

    // Chat context management
    void setContext(const std::string& context);  // "Lobby", "Room 1", etc.
    void clearMessages();

    // Update display (call after adding messages)
    void updateMessageDisplay();

    static constexpr size_t MAX_INPUT_LENGTH = 128;
    static constexpr size_t MAX_VISIBLE_MESSAGES = 8;
    static constexpr size_t MAX_HISTORY = 50;

private:
    void updateInputDisplay();
    void updateUnreadDisplay();
    void createMessageEntry(size_t index, const ChatMessage& msg);

    Registry &_reg;
    bool _visible = false;
    bool _inputFocused = false;
    bool _enabled = false;  // Whether chat system is active
    bool _skipNextTextInput = false;

    std::string _inputBuffer;
    std::string _context = "Lobby";  // Current chat context
    std::deque<ChatMessage> _messages;

    // Entities
    Entity _panelBackground;
    Entity _inputBackground;
    Entity _inputTextLabel;
    Entity _inputCursor;
    Entity _titleLabel;
    Entity _toggleButton;       // Button to show/hide chat
    Entity _toggleButtonLabel;  // Label on toggle button
    Entity _unreadCountLabel;   // Shows unread message count

    size_t _unreadCount = 0;

    std::vector<Entity> _messageLabels;  // Pool of message display entities
    std::vector<Entity> _allEntities;
    std::vector<Entity> _toggleEntities;  // Toggle button entities (always visible when enabled)

    // Callback
    SendCallback _onSend;
};

#endif /* !CHATPANEL_HPP_ */
