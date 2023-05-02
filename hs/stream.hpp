#pragma once

#include <vector>

namespace hs {
    template <class T> class stream_t {
        typedef std::vector<T> vector_t;

        T m_dummy;

        vector_t m_buf;

        size_t m_pos = 0;

    public:
        vector_t* data() {
            return &m_buf;
        }

        typename std::vector<T>::iterator begin() {
            return m_buf.begin();
        }

        typename std::vector<T>::iterator end() {
            return m_buf.end();
        }

        bool eof() {
            return m_pos == m_buf.size();
        }

        T& peek() {
            if ((m_pos + 1) == m_buf.size()) return m_dummy;

            return m_buf[m_pos + 1];
        }

        void put(const T& data) {
            m_buf.push_back(data);
        }

        size_t tellg() {
            return m_pos;
        }

        void seekg(size_t pos) {
            m_pos = pos;
        }

        T& get() {
            if (this->eof()) return m_dummy;

            return m_buf[m_pos++];
        }
    };
}