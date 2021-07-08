#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << R"(<circle cx=")" << center_.x << R"(" cy=")" << center_.y << R"(" )";
        out << R"(r=")" << radius_ << R"(" )";
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // -----------Polyline-----------------

    Polyline& Polyline::AddPoint(Point point) {
        vertices_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        size_t size = vertices_.size();
        out << R"(<polyline points=")";
        for (const auto& point : vertices_) {
            if (size > 1) {
                out << point.x << ',' << point.y << ' ';
            }
            else {
                out << point.x << ',' << point.y;
            }
            --size;

        }

        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // --------------Text------------------

    Text& Text::SetPosition(Point pos) {
        position_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data) {
        data_ = move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;

        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(context.out);
        out << " >"sv;

        std::string result;

        for (const char c : data_) {
            switch (c) {
            case '"':
                result += "&quot;"s;
                break;
            case '\'':
                result += "&apos;"s;
                break;
            case '>':
                result += "&gt;"s;
                break;
            case '<':
                result += "&lt;"s;
                break;
            case '&':
                result += "&amp;"s;
                break;
            default:
                result += c;
                break;
            }
        }
        out << result;
        out << "</text>"sv;
    }

    // ------------Document----------------


    void Document::AddPtr(std::shared_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }

    void Document::Render(const RenderContext& context) const {
        auto& out = context.out;

        out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
        out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;
        for (const auto& obj : objects_) {
            obj.get()->Render(out);
        }
        out << "</svg>"sv;
        out << std::endl;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        default:
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        default:
            break;
        }
        return out;
    }


    //------------OstreamColor-------------
    void OstreamColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }

    void OstreamColorPrinter::operator()(std::string color) const {
        out << color;
    }

    void OstreamColorPrinter::operator()(Rgb rgb) const {
        out << "rgb("sv << static_cast<int>(rgb.red) << ","sv << static_cast<int>(rgb.green) << ","sv << static_cast<int>(rgb.blue) << ")"sv;
    }

    void OstreamColorPrinter::operator()(Rgba rgba) const {
        out << "rgba("sv << static_cast<int>(rgba.red) << ","sv << static_cast<int>(rgba.green) << ","sv << static_cast<int>(rgba.blue) << ","sv << rgba.opacity << ")"sv;
    }
}  // namespace svg
