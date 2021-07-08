#include "serialization.h"

using namespace std;

namespace serialize {
	void Serializator::Serialize(const string& filename) {
		ofstream out(filename, ios::binary);
		//transport_catalogue_serialize::TransportCatalogue tc_serialize;

		SerializeStop(tc_serialize_);
		SerializeDistance(tc_serialize_);
		SerializeBus(tc_serialize_);
		SerializeRenderSettings(tc_serialize_);
		SerializeGraph();
		tc_serialize_.SerializeToOstream(&out);
	}

	void Serializator::SerializeStop(transport_catalogue_serialize::TransportCatalogue& tc_serialize) {
		const auto stops = db_.GetStops();
		for (auto stop : stops) {
			transport_catalogue_serialize::Stop buf_stop;
			transport_catalogue_serialize::Coordinates buf_coordinates;
			buf_coordinates.set_lat(stop.second->coordinates.lat);
			buf_coordinates.set_lng(stop.second->coordinates.lng);

			buf_stop.set_name(std::move(stop.second->name));
			*buf_stop.mutable_coordinates() = buf_coordinates;

			*tc_serialize.add_stops() = buf_stop;
		}
	}

	void Serializator::SerializeBus(transport_catalogue_serialize::TransportCatalogue& tc_serialize) {
		const auto buses = db_.GetRoutes();
		for (auto bus : buses) {
			transport_catalogue_serialize::Bus buf_bus;
			buf_bus.set_unique_stops(bus.second->unique_stops);
			buf_bus.set_stops_number(bus.second->stops_number);
			buf_bus.set_number(bus.second->number);
			buf_bus.set_is_round_trip(bus.second->is_round_trip);
			buf_bus.set_distance(bus.second->distance);

			for (auto raw_stop : bus.second->raw_route) {
				buf_bus.add_raw_route(raw_stop);
			}

			for (auto stop : bus.second->route) {
				transport_catalogue_serialize::Stop buf_stop;
				transport_catalogue_serialize::Coordinates buf_coordinates;
				buf_coordinates.set_lat(stop->coordinates.lat);
				buf_coordinates.set_lng(stop->coordinates.lng);

				buf_stop.set_name(std::move(stop->name));
				*buf_stop.mutable_coordinates() = buf_coordinates;

				*buf_bus.add_route() = buf_stop;
			}
			*tc_serialize.add_buses() = buf_bus;
		}
	}

	void Serializator::SerializeDistance(transport_catalogue_serialize::TransportCatalogue& tc_serialize) {
		const auto distances = db_.GetDistances();
		for (auto distance : distances) {
			transport_catalogue_serialize::Distance buf_distance;
			buf_distance.set_from(distance.first.first->name);
			buf_distance.set_to(distance.first.second->name);
			buf_distance.set_distance(distance.second);

			*tc_serialize.add_distances() = buf_distance;
		}
	}

	void Serializator::SerializeRenderSettings(transport_catalogue_serialize::TransportCatalogue& tc_serialize) {
		const auto render_settings = map_renderer_.GetRenderSettings();

		transport_catalogue_serialize::RenderSettings buf_render_settings;
		transport_catalogue_serialize::Point buf_point;

		buf_render_settings.set_width(render_settings.width);
		buf_render_settings.set_height(render_settings.height);
		buf_render_settings.set_padding(render_settings.padding);
		buf_render_settings.set_line_width(render_settings.line_width);
		buf_render_settings.set_stop_radius(render_settings.stop_radius);
		buf_render_settings.set_bus_label_font_size(render_settings.bus_label_font_size);
		buf_render_settings.set_stop_label_font_size(render_settings.stop_label_font_size);
		buf_render_settings.set_underlayer_width(render_settings.underlayer_width);

		buf_point.set_x(render_settings.bus_label_offset.x);
		buf_point.set_y(render_settings.bus_label_offset.y);
		*buf_render_settings.mutable_bus_label_offset() = buf_point;

		buf_point.set_x(render_settings.stop_label_offset.x);
		buf_point.set_y(render_settings.stop_label_offset.y);
		*buf_render_settings.mutable_stop_label_offset() = buf_point;

		*buf_render_settings.mutable_underlayer_color() = SerializeColor(buf_render_settings, render_settings.underlayer_color);

		for (auto color : render_settings.color_palette) {
			transport_catalogue_serialize::Color buf_color = SerializeColor(buf_render_settings, color);
			*buf_render_settings.add_color_palette() = buf_color;
		}

		*tc_serialize.mutable_render_settings() = buf_render_settings;
	}

	transport_catalogue_serialize::Color Serializator::SerializeColor(transport_catalogue_serialize::RenderSettings buf_render_settings, const svg::Color color) {
		transport_catalogue_serialize::Color buf_color;
		transport_catalogue_serialize::Rgba buf_rgba;
		
		if (holds_alternative<svg::Rgb>(color)) {
			svg::Rgb svg_rgb = get<svg::Rgb>(color);
			buf_rgba.set_red(svg_rgb.red);
			buf_rgba.set_blue(svg_rgb.blue);
			buf_rgba.set_green(svg_rgb.green);
			*buf_color.mutable_rgba() = buf_rgba;
		}
		else if (holds_alternative<svg::Rgba>(color)) {
			svg::Rgba svg_rgba = get<svg::Rgba>(color);
			buf_rgba.set_red(svg_rgba.red);
			buf_rgba.set_blue(svg_rgba.blue);
			buf_rgba.set_green(svg_rgba.green);
			buf_rgba.set_opacity(svg_rgba.opacity);
			buf_color.set_is_rgba(true);
			*buf_color.mutable_rgba() = buf_rgba;
		}
		else if (holds_alternative<string>(color)) {
			string svg_name = get<string>(color);
			buf_color.set_name(svg_name);
		}

		return buf_color;
	}


	void Serializator::SerializeRoutingSettings(transport_catalogue_serialize::Router& buf_router) {
		transport_catalogue_serialize::RoutingSettings buf_routing_settings;
		buf_routing_settings.set_wait_time(db_.GetRoutingSettings().first);
		buf_routing_settings.set_velocity(db_.GetRoutingSettings().second);
		*buf_router.mutable_routing_settings() = buf_routing_settings;
	}

	void Serializator::SerializeStopToVertexIds(transport_catalogue_serialize::Router& buf_router, const std::pair<detail::StopPtr, router::VertexInfo>& stop) {
		transport_catalogue_serialize::StopsToVertex buf_stops_to_vertex;
		buf_stops_to_vertex.set_stop_name(stop.first->name);
		buf_stops_to_vertex.set_stop_in_id(stop.second.stop_in);
		buf_stops_to_vertex.set_stop_out_id(stop.second.stop_out);

		*buf_router.add_stops_to_vertex() = buf_stops_to_vertex;
	}


	void Serializator::SerializeEdgeInfo(transport_catalogue_serialize::Router& buf_router, const router::EdgesInfo& edge_info) {
		transport_catalogue_serialize::EdgesInfo buf_edges_info;
		//buf_edges_info.set_edge_id(edge_info.edge_id);
		buf_edges_info.set_name(edge_info.name);
		buf_edges_info.set_total_time(edge_info.total_time);
		buf_edges_info.set_item_type(edge_info.item_type == router::ItemType::WAIT_ITEM ? false : true);
		buf_edges_info.set_span_count(edge_info.span_count);

		*buf_router.add_edges_info() = buf_edges_info;
	}

	void Serializator::SerializeGraphEdge(transport_catalogue_serialize::Graph& buf_graph, const graph::Edge<double>& edge) {
		transport_catalogue_serialize::Edge buf_edge;
		buf_edge.set_from(edge.from);
		buf_edge.set_to(edge.to);
		buf_edge.set_weight(edge.weight);

		*buf_graph.add_edge() = buf_edge;
	}

	void Serializator::SetRouterSerialize(const transport_catalogue_serialize::Router& buf_router) {
		*tc_serialize_.mutable_router() = buf_router;
	}

	void Serializator::SerializeWaitStop(const std::pair<std::string_view, detail::StopPtr>& stop, graph::VertexId& vertex_id) {
		stops_to_vertex_ids_.insert({ stop.second , {++vertex_id - 1, vertex_id++} });//Вставка остановки для ожидания A' и отъезда А

		SerializeStopToVertexIds(buf_router_, { stop.second , {vertex_id - 2, vertex_id - 1} });
		SerializeGraphEdge(buf_graph_, { vertex_id - 1, vertex_id - 2, static_cast<double>(bus_wait_time_) });
		SerializeEdgeInfo(buf_router_, router::EdgesInfo{ stop.second->name, static_cast<double>(bus_velocity_) });
	}

	void Serializator::SerializeRoute(const detail::BusPtr& route) {
		const auto& stops_in_route = route->route;
		for (size_t i = 0; i + 1 < stops_in_route.size(); ++i) {
			double distance = 0.0;
			const graph::VertexId vertex_to_start_from = stops_to_vertex_ids_[stops_in_route[i]].stop_in;

			for (size_t j = i + 1; j < stops_in_route.size(); ++j) {
				if (&stops_in_route[i] != &stops_in_route[j]) {
					distance += db_.GetDistanceBetweenStops(stops_in_route[j - 1], stops_in_route[j]);
					double weight = distance / METERS_TO_KM / bus_velocity_ * HOURS_TO_MINUTES;
					SerializeGraphEdge(buf_graph_, { vertex_to_start_from, stops_to_vertex_ids_[stops_in_route[j]].stop_out, weight });
					SerializeEdgeInfo(buf_router_, router::EdgesInfo{ route->number, weight, router::ItemType::BUS_ITEM, abs(static_cast<int>(i - j)) });
				}
			}
		}
	}

	void Serializator::SerializeGraph() {
		const auto& stops = db_.GetStops();
		bus_wait_time_ = db_.GetRoutingSettings().first;
		bus_velocity_ = db_.GetRoutingSettings().second;
		buf_graph_.set_vertex_count(stops.size() * 2);
		SerializeRoutingSettings(buf_router_);

		graph::VertexId vertex_id = 0;
		for (const auto& stop : stops) {
			SerializeWaitStop(stop, vertex_id);
		}

		for (const auto [_, route] : db_.GetRoutes()) {
			SerializeRoute(route);
		}

		*buf_router_.mutable_graph() = buf_graph_;
		*tc_serialize_.mutable_router() = buf_router_;
	}



	void Deserializator::Deserialize(catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, router::TransportRouter& router, const string& filename) {
		ifstream in(filename, ios::binary);
		transport_catalogue_serialize::TransportCatalogue tc_serialize;
		tc_serialize.ParseFromIstream(&in);
		
		for (int i = 0; i < tc_serialize.stops().size(); ++i) {
			detail::Stop stop = DeserializeStop(tc_serialize.stops(i));
			transport_catalogue.AddStop(stop);
		}

		for (int i = 0; i < tc_serialize.buses().size(); ++i) {
			detail::Bus bus = DeserializeBus(tc_serialize.buses(i));
			transport_catalogue.AddBus(bus);
		}

		for (int i = 0; i < tc_serialize.distances_size(); ++i) {
			const auto deserialized_distance = DeserializeDistance(tc_serialize.distances(i), transport_catalogue);
			transport_catalogue.AddDistance(deserialized_distance.first.first, { make_pair(deserialized_distance.first.second, deserialized_distance.second) });
		}

		map_renderer.SetRenderSettings(DeserializeRenderSettings(tc_serialize.render_settings()));
		map_renderer.CreateRender(transport_catalogue);

		DeserializeRouter(tc_serialize.router(), transport_catalogue, router);
	}

	detail::Stop Deserializator::DeserializeStop(const transport_catalogue_serialize::Stop& stop_serialize) {
		detail::Stop stop;
		stop.name = stop_serialize.name();
		stop.coordinates.lat = stop_serialize.coordinates().lat();
		stop.coordinates.lng = stop_serialize.coordinates().lng();

		return stop;
	}

	detail::Bus Deserializator::DeserializeBus(const transport_catalogue_serialize::Bus& bus_serialize) {
		detail::Bus bus;
		bus.unique_stops = bus_serialize.unique_stops();
		bus.distance = bus_serialize.distance();
		bus.stops_number = bus_serialize.stops_number();
		bus.is_round_trip = bus_serialize.is_round_trip();
		bus.number = bus_serialize.number();

		for (int i = 0; i < bus_serialize.raw_route_size(); ++i) {
			bus.raw_route.push_back(bus_serialize.raw_route(i));
		}
	
		return bus;
	}

	std::pair<Deserializator::StopPtrPair, uint64_t> Deserializator::DeserializeDistance(const transport_catalogue_serialize::Distance& distance_serialize, const catalogue::TransportCatalogue& transport_catalogue) {
		detail::StopPtr stop_from = transport_catalogue.FindStopByName(distance_serialize.from());
		detail::StopPtr stop_to = transport_catalogue.FindStopByName(distance_serialize.to());
		uint64_t distance = distance_serialize.distance();

		return make_pair(make_pair(stop_from, stop_to), distance);

	}


	renderer::RenderSettings Deserializator::DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& render_settings_serialize) {
		renderer::RenderSettings render_settings;
		render_settings.width = render_settings_serialize.width();
		render_settings.height = render_settings_serialize.height();
		render_settings.padding = render_settings_serialize.padding();
		render_settings.line_width = render_settings_serialize.line_width();
		render_settings.stop_radius = render_settings_serialize.stop_radius();
		render_settings.bus_label_font_size = render_settings_serialize.bus_label_font_size();
		render_settings.bus_label_offset = { render_settings_serialize.bus_label_offset().x(), render_settings_serialize.bus_label_offset().y() };
		render_settings.stop_label_font_size = render_settings_serialize.stop_label_font_size();
		render_settings.stop_label_offset = { render_settings_serialize.stop_label_offset().x(), render_settings_serialize.stop_label_offset().y() };
		render_settings.underlayer_width = render_settings_serialize.underlayer_width();
		render_settings.underlayer_color = DeserializeColor(render_settings_serialize.underlayer_color());

		for (int i = 0; i < render_settings_serialize.color_palette_size(); ++i) {
			render_settings.color_palette.push_back(DeserializeColor(render_settings_serialize.color_palette(i)));
		}


		
		return render_settings;
	}

	svg::Color Deserializator::DeserializeColor(const transport_catalogue_serialize::Color& color_serialize) {
		svg::Color color;
		if (!color_serialize.name().empty()) {
			color = color_serialize.name();
		}
		else if (color_serialize.is_rgba()) {
			color = svg::Rgba{ 
							static_cast<uint8_t>(color_serialize.rgba().red()),
							static_cast<uint8_t>(color_serialize.rgba().green()),
							static_cast<uint8_t>(color_serialize.rgba().blue()),
							color_serialize.rgba().opacity() 
			};
		}
		else {
			color = svg::Rgb{ 
							static_cast<uint8_t>(color_serialize.rgba().red()),
							static_cast<uint8_t>(color_serialize.rgba().green()),
							static_cast<uint8_t>(color_serialize.rgba().blue()) 
			};
		}
		return color;
	}

	void Deserializator::DeserializeRouter(const transport_catalogue_serialize::Router& router_serialize, const catalogue::TransportCatalogue& transport_catalogue, router::TransportRouter& route) {
		route.SetBusVelocity(router_serialize.routing_settings().velocity());
		route.SetBusWaitTime(router_serialize.routing_settings().wait_time());

		for (auto stop : router_serialize.stops_to_vertex()) {
			const auto found_stop = transport_catalogue.FindStopByName(stop.stop_name());
			router::VertexInfo vertex_info = { stop.stop_in_id(), stop.stop_out_id() };
			route.AddStopToVertexIds(found_stop, vertex_info);
		}

		for (auto edge_info_serialize : router_serialize.edges_info()) {
			router::EdgesInfo edge_info;
			//edge_info.edge_id = edge_info_serialize.edge_id();
			edge_info.item_type = edge_info_serialize.item_type() == 0 ? router::ItemType::WAIT_ITEM : router::ItemType::BUS_ITEM;
			edge_info.name = edge_info_serialize.name();
			edge_info.span_count = edge_info_serialize.span_count();
			edge_info.total_time = edge_info_serialize.total_time();
			route.AddEdgeInfo(edge_info);
		}

		route.SetGraph(router_serialize.graph().vertex_count());
		for (auto edge_serialize : router_serialize.graph().edge()) {
			graph::Edge<double> edge;
			edge.from = edge_serialize.from();
			edge.to = edge_serialize.to();
			edge.weight = edge_serialize.weight();
			route.AddEdgeToGraph(edge);
		}
		route.SetRouter();
	}


}