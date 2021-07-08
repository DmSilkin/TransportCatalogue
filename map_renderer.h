#pragma once


#include "domain.h"
#include "geo.h"
#include "svg.h"
#include "json.h"
#include "transport_catalogue.h"
#include <algorithm>



/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */



namespace renderer {

    inline const double EPSILON = 1e-6;
    bool IsZero(double value);

    struct RenderSettings {
        double width = 0.0;
        double height = 0.0;
        double padding = 0.0;
        double line_width;
        double stop_radius = 0.0;
        int bus_label_font_size = 0;
        svg::Point bus_label_offset = { 0.0, 0.0 };
        int stop_label_font_size = 0;
        svg::Point stop_label_offset = { 0.0, 0.0 };
        svg::Color underlayer_color;
        double underlayer_width = 0.0;
        std::vector<svg::Color> color_palette;

        svg::Color GetColor(json::Node color);
    };

    struct RouteSettings {
        std::vector<svg::Point> stops_coordinates;
        svg::Color stroke_color;
        double line_width;
    };

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
            double max_height, double padding)
            : padding_(padding) {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lng < rhs.lng;
                    });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            const auto [bottom_it, top_it]
                = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                return lhs.lat < rhs.lat;
                    });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    class TextRenderer : svg::Drawable {
    public:
        TextRenderer(const svg::Point& coordinates, const svg::Point& bus_label_offset, const uint32_t bus_label_font_size, const std::optional<std::string>& font_weight,
            const std::string& bus_name, const svg::Color& underlayer_color, const double underlayer_width, const svg::Color& label_color)
            :coordinates_(coordinates),
             bus_label_offset_(bus_label_offset),
             bus_label_font_size_(bus_label_font_size),
             font_weight_(font_weight),
             bus_name_(bus_name),
             underlayer_color_(underlayer_color),
             underlayer_width_(underlayer_width),
             label_color_(label_color)
        {

        }

        void Draw(svg::ObjectContainer& container) const override;

    private:
        svg::Text CreateUnderlayer() const;
        svg::Text CreateText() const;

        svg::Point coordinates_;
        svg::Point bus_label_offset_;
        uint32_t bus_label_font_size_;
        std::string font_family_ = "Verdana";
        std::optional<std::string> font_weight_;
        std::string bus_name_;
        svg::Color underlayer_color_;
        double underlayer_width_;
        svg::Color label_color_;
    };

    class RouteRenderer : svg::Drawable {
    public:
        RouteRenderer(std::vector<svg::Point> stops_coordinates, svg::Color stroke_color, double line_width) {
            route_settings_.stops_coordinates = std::move(stops_coordinates);
            route_settings_.stroke_color = std::move(stroke_color);
            route_settings_.line_width = line_width;
        }

        void Draw(svg::ObjectContainer& container) const override;

    private:
        RouteSettings route_settings_;
    };

    class StopRenderer : svg::Drawable {
    public:
        StopRenderer(const svg::Point& center, double stop_radius)
            :center_(center),
             stop_radius_(stop_radius)
        {

        }

        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point center_;
        double stop_radius_ = 0.0;
    };

    class MapRenderer {
    public:
        void SetRenderSettings(const RenderSettings& render_settings);
        void SetRoutes(const std::vector<RouteRenderer>&& routes);
        void SetText(const std::vector<TextRenderer>&& text);
        void SetStops(const std::vector<StopRenderer>&& stops);
        void SetStopNames(const std::vector<TextRenderer>&& stop_names);
        void CreateRender(const catalogue::TransportCatalogue& transport_catalogue);
        svg::Document Render() const;

        RenderSettings GetRenderSettings() const;
    private:

        std::vector<RouteRenderer> RoutesLineRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const;
        std::vector<TextRenderer> RoutesNameRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const;
        std::vector<StopRenderer> StopsRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const;
        std::vector<TextRenderer> StopsNameRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const;
        RenderSettings render_settings_;
        std::vector<RouteRenderer> routes_;
        std::vector<TextRenderer> text_;
        std::vector<StopRenderer> stops_;
        std::vector<TextRenderer> stop_names_;
        
    };


}

