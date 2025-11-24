/*
** EPITECH PROJECT, 2025
** local
** File description:
** Color
*/

#include <cstdint>

#pragma once

class Color
{
public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;

    Color(uint8_t r, uint8_t g, uint8_t b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        a = 255;
    }

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

};