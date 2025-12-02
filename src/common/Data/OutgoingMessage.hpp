/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** OutgoingMessage
*/

#ifndef OUTGOINGMESSAGE_HPP_
#define OUTGOINGMESSAGE_HPP_

#include "MessageFactory.hpp"

struct OutgoingMessage {
    int targetFd;           // -1 = broadcast to all
    MessageData data;
};

#endif /* !OUTGOINGMESSAGE_HPP_ */
