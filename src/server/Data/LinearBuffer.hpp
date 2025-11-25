/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** LinearBuffer
*/

#ifndef LINEARBUFFER_HPP_
#define LINEARBUFFER_HPP_

#include <vector>
#include <cstdint>
#include <cstring>

constexpr size_t MAX_BUFFER_SIZE = 8192;

class LinearBuffer {
    public:
        LinearBuffer() {}

        bool write(const char* data, size_t len) {
            if (_buffer.size() + len > MAX_BUFFER_SIZE) return false;
            _buffer.insert(_buffer.end(), data, data + len);
            return true;
        }

        uint8_t peek(size_t offset) const {
            if (offset >= _buffer.size()) return 0;
            return _buffer[offset];
        }

        void consume(size_t len) {
            if (len >= _buffer.size()) {
                _buffer.clear();
            } else {
                _buffer.erase(_buffer.begin(), _buffer.begin() + len);
            }
        }

        void read(std::vector<uint8_t>& out, size_t len) {
            if (len > _buffer.size()) len = _buffer.size();
            out.assign(_buffer.begin(), _buffer.begin() + len);
            consume(len);
        }

        size_t size() const { return _buffer.size(); }
        void clear() { _buffer.clear(); }

    private:
        std::vector<uint8_t> _buffer;
};

#endif /* !LINEARBUFFER_HPP_ */
