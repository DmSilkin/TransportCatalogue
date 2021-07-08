#include "map_renderer.h"


namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Color RenderSettings::GetColor(json::Node color) {
        if (color.IsString()) {
            return color.AsString();
        }
        else {
            const auto arr = color.AsArray();
            if (arr.size() == 3) {
                return svg::Rgb{
                    static_cast<uint8_t>(arr[0].AsInt()),
                    static_cast<uint8_t>(arr[1].AsInt()),
                    static_cast<uint8_t>(arr[2].AsInt())
                };
            }
            else if (arr.size() == 4) {
                return svg::Rgba{
                    static_cast<uint8_t>(arr[0].AsInt()),
                    static_cast<uint8_t>(arr[1].AsInt()),
                    static_cast<uint8_t>(arr[2].AsInt()),
                    arr[3].AsDouble()
                };
            }
        }
        return svg::Color{};
    }

    void RouteRenderer::Draw(svg::ObjectContainer& container) const {
        svg::Polyline polyline;
        for (const auto& coordinates : route_settings_.stops_coordinates) {
            polyline.AddPoint(coordinates);
        }
        polyline.SetStrokeColor(route_settings_.stroke_color).SetFillColor(svg::NoneColor);
        polyline.SetStrokeWidth(route_settings_.line_width);
        polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        container.Add(std::move(polyline));
    }

    void TextRenderer::Draw(svg::ObjectContainer& container) const {
        container.Add(CreateUnderlayer());
        container.Add(CreateText());
    }

    void StopRenderer::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Circle().SetCenter(center_).SetRadius(stop_radius_).SetFillColor("white"));
    }

    svg::Text TextRenderer::CreateUnderlayer() const {
        svg::Text text{ CreateText() };
        text.SetFillColor(underlayer_color_).SetStrokeColor(underlayer_color_);
        text.SetStrokeWidth(underlayer_width_);
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        return text;
    }

    svg::Text TextRenderer::CreateText() const {
        svg::Text text;
        text.SetPosition(coordinates_).SetOffset(bus_label_offset_);
        text.SetFontSize(bus_label_font_size_).SetFontFamily(font_family_);
        if (font_weight_) {
            text.SetFontWeight(*font_weight_);
        }
        text.SetData(bus_name_);
        return text.SetFillColor(label_color_);
    }


    void MapRenderer::SetRenderSettings(const RenderSettings& render_settings) {
        render_settings_ = render_settings;
    }

    void MapRenderer::SetRoutes(const std::vector<RouteRenderer>&& routes) {
        routes_ = std::move(routes);
    }

    void MapRenderer::SetText(const std::vector<TextRenderer>&& text) {
        text_ = std::move(text);
    }

    void MapRenderer::SetStops(const std::vector<StopRenderer>&& stops) {
        stops_ = std::move(stops);
    }

    void MapRenderer::SetStopNames(const std::vector<TextRenderer>&& stop_names) {
        stop_names_ = std::move(stop_names);
    }

    void MapRenderer::CreateRender(const catalogue::TransportCatalogue& transport_catalogue) {
        const auto& stops = transport_catalogue.GetStops();
        std::vector<geo::Coordinates> stops_to_draw;
        for (const auto& stop : stops) {
            if (transport_catalogue.FindBusesByStop(stop.first).second.size() != 0) {
                stops_to_draw.push_back(stop.second->coordinates);
            }
        }

        renderer::SphereProjector sphere_projector(stops_to_draw.begin(), stops_to_draw.end(), render_settings_.width, render_settings_.height, render_settings_.padding);
        SetRoutes(RoutesLineRenderer(transport_catalogue, sphere_projector));
        SetText(RoutesNameRenderer(transport_catalogue, sphere_projector));
        SetStops(StopsRenderer(transport_catalogue, sphere_projector));
        SetStopNames(StopsNameRenderer(transport_catalogue, sphere_projector));
    }

    std::vector<RouteRenderer> MapRenderer::RoutesLineRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const {
        const auto& routes = transport_catalogue.GetRoutes();
        std::vector<RouteRenderer> routes_render;
        routes_render.reserve(routes.size());
        const size_t color_size = render_settings_.color_palette.size();
        size_t number = 0;
        for (const auto& [bus_name, stops_at_route] : routes) {
            std::vector<svg::Point> points;
            if (!stops_at_route->route.empty()) {
                if (number >= color_size) {
                    number = 0;
                }
                for (const auto& stop : stops_at_route->route) {
                    points.push_back(sphere_projector(transport_catalogue.FindStopByName(stop->name)->coordinates));
                }
                routes_render.emplace_back(points, render_settings_.color_palette[number], render_settings_.line_width);
                ++number;
            }
        }
        return routes_render;
    }

    std::vector<TextRenderer> MapRenderer::RoutesNameRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const {
        std::vector<renderer::TextRenderer> routes_names;
        const auto& routes = transport_catalogue.GetRoutes();
        const size_t color_size = render_settings_.color_palette.size();
        size_t number = 0;
        for (const auto& [bus_name, stops_at_route] : routes) {

            if (!stops_at_route->route.empty()) {
                if (number >= color_size) {
                    number = 0;
                }
                const auto first_stop = transport_catalogue.FindStopByName(stops_at_route->route.front()->name);
                const auto end_stop = transport_catalogue.FindStopByName(stops_at_route->route[stops_at_route->route.size() / 2]->name);
                const svg::Point position = sphere_projector(transport_catalogue.FindStopByName(stops_at_route->route.front()->name)->coordinates);
                routes_names.emplace_back(position, render_settings_.bus_label_offset, static_cast<uint32_t>(render_settings_.bus_label_font_size),
                    "bold", std::string(bus_name), render_settings_.underlayer_color, render_settings_.underlayer_width,
                    render_settings_.color_palette[number]);
                if (!transport_catalogue.FindBusByNumber(bus_name)->is_round_trip && first_stop != end_stop) {
                    const svg::Point back_position = sphere_projector(transport_catalogue.FindStopByName(stops_at_route->route[stops_at_route->route.size() / 2]->name)->coordinates);
                    routes_names.emplace_back(back_position, render_settings_.bus_label_offset, static_cast<uint32_t>(render_settings_.bus_label_font_size),
                        "bold", std::string(bus_name), render_settings_.underlayer_color, render_settings_.underlayer_width,
                        render_settings_.color_palette[number]);
                }
                ++number;
            }

        }
        return routes_names;
    }

    std::vector<StopRenderer> MapRenderer::StopsRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const {
        const auto& stops = transport_catalogue.GetStops();

        std::vector<renderer::StopRenderer> stops_for_draw;
        stops_for_draw.reserve(stops.size());
        for (const auto& stop : stops) {
            if (transport_catalogue.FindBusesByStop(stop.first).second.size() != 0) {
                stops_for_draw.emplace_back(sphere_projector(transport_catalogue.FindStopByName(stop.second->name)->coordinates), render_settings_.stop_radius);
            }
        }
        return stops_for_draw;
    }

    std::vector<TextRenderer> MapRenderer::StopsNameRenderer(const catalogue::TransportCatalogue& transport_catalogue, const SphereProjector& sphere_projector) const {
        const auto& stops = transport_catalogue.GetStops();
        std::vector<renderer::TextRenderer> stops_for_draw;
        stops_for_draw.reserve(stops.size());
        for (const auto& stop : stops) {
            if (transport_catalogue.FindBusesByStop(stop.first).second.size() != 0) {
                stops_for_draw.emplace_back(sphere_projector(transport_catalogue.FindStopByName(stop.second->name)->coordinates), render_settings_.stop_label_offset,
                    render_settings_.stop_label_font_size, std::nullopt, std::string{ stop.second->name },
                    render_settings_.underlayer_color, render_settings_.underlayer_width, "black");
            }

        }
        return stops_for_draw;
    }

    svg::Document MapRenderer::Render() const {
        svg::Document doc;

        for (const auto route : routes_) {
            route.Draw(doc);
        }

        for (const auto text : text_) {
            text.Draw(doc);
        }

        for (const auto stop : stops_) {
            stop.Draw(doc);
        }

        for (const auto stop_name : stop_names_) {
            stop_name.Draw(doc);
        }

        return doc;
    }

    RenderSettings MapRenderer::GetRenderSettings() const {
        return render_settings_;
    }
}
/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */