#ifndef IOUTILS_HPP
#define IOUTILS_HPP

#include <ostream>
#include <string_view>

class RightPad
{
public:
    RightPad(const std::string_view text_, const std::size_t width_) : text{text_}, width{width_} {}
    friend std::ostream& operator<<(std::ostream& out, const RightPad& p);

private:
    std::string text;
    std::size_t width;
};

class LeftPad
{
public:
    LeftPad(const std::string_view text_, const std::size_t width_) : text{text_}, width{width_} {}
    friend std::ostream& operator<<(std::ostream& out, const LeftPad& p);

private:
    std::string text;
    std::size_t width;
};

inline std::ostream& operator<<(std::ostream& out, const RightPad& p)
{
    if (p.text.size() > p.width)
    {
        out << std::string_view{p.text.data(), p.width};
    }
    else
    {
        out << p.text;
        for (auto i = p.text.size(); i < p.width; ++i)
        {
            out << ' ';
        }
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const LeftPad& p)
{
    if (p.text.size() > p.width)
    {
        out << std::string_view{p.text.data(), p.width};
    }
    else
    {
        for (auto i = p.text.size(); i < p.width; ++i)
        {
            out << ' ';
        }
        out << p.text;
    }
    return out;
}

#endif // IOUTILS_HPP