#include "json_reader.h"


namespace json_reader {

	void JsonReader(catalogue::TransportCatalogue& transport_catalogue, std::istream& in, std::ostream& out) {
		const auto dict = json::Load(in).GetRoot().AsDict();
		ProcessRequests(transport_catalogue, dict, out);	
	}

	void JsonSerialize(std::istream& in) {
		catalogue::TransportCatalogue transport_catalogue;
		const auto dict = json::Load(in).GetRoot().AsDict();
		const auto base_requests = dict.find("base_requests");
		if (base_requests != dict.end()) {
			ProcessBaseRequests(transport_catalogue, base_requests->second.AsArray());
		}

		renderer::MapRenderer map_renderer;
		const auto render_settings = dict.find("render_settings");
		if (render_settings != dict.end()) {
			RenderSettingsRequests(transport_catalogue, map_renderer, render_settings->second.AsDict());
		}

		const auto routing_settings = dict.find("routing_settings");
		if (routing_settings != dict.end()) {
			ProcessRoutingSettings(transport_catalogue, routing_settings->second.AsDict());
		}
		serialize::Serializator serializator(transport_catalogue, map_renderer);

		router::TransportRouter router(transport_catalogue);


		const auto serialization_settings = dict.find("serialization_settings");
		if (serialization_settings != dict.end()) {
			serializator.Serialize(serialization_settings->second.AsDict().at("file").AsString());
		}
	}

	void JsonDeserialize(std::istream& in, std::ostream& out) {
		catalogue::TransportCatalogue transport_catalogue;
		renderer::MapRenderer map_renderer;
		router::TransportRouter router(transport_catalogue);
		const auto dict = json::Load(in).GetRoot().AsDict();
		const auto serialization_settings = dict.find("serialization_settings");
		serialize::Deserializator deserializator;
		deserializator.Deserialize(transport_catalogue, map_renderer, router, serialization_settings->second.AsDict().at("file").AsString());
		const auto stat_requests = dict.find("stat_requests");
		if (stat_requests != dict.end()) {
			RequestHandler request_handler(transport_catalogue, map_renderer);
			//router::TransportRouter router(transport_catalogue); // Здесь надо десериализовать роутер не из справочника, а из базы
			const auto doc = StatRequests(request_handler, stat_requests->second.AsArray(), router);
			json::Print(doc, out);
		}

	}

	void ProcessRequests(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict, std::ostream& out) {
		
		const auto base_requests = dict.find("base_requests");
		if (base_requests != dict.end()) {
			ProcessBaseRequests(transport_catalogue, base_requests->second.AsArray());
		}


		renderer::MapRenderer map_renderer;
		const auto render_settings = dict.find("render_settings");
		if (render_settings != dict.end()) {
			RenderSettingsRequests(transport_catalogue, map_renderer, render_settings->second.AsDict());
		}

		const auto routing_settings = dict.find("routing_settings");
		if (routing_settings != dict.end()) {
			ProcessRoutingSettings(transport_catalogue, routing_settings->second.AsDict());
		}
		
		const auto stat_requests = dict.find("stat_requests");
		/*if (stat_requests != dict.end()) {
			RequestHandler request_handler(transport_catalogue, map_renderer);
			router::TransportRouter router(transport_catalogue);
			const auto doc = StatRequests(request_handler, stat_requests->second.AsArray(), router);
			json::Print(doc, out);
		}*/
		
	}

	void ProcessBaseRequests(catalogue::TransportCatalogue& transport_catalogue, const json::Array& arr) {
		for (const auto& request : arr) {
			const auto base_request_type = request.AsDict().find("type");
			if (base_request_type != request.AsDict().end()) {
				if (base_request_type->second.AsString() == "Stop") {
					ProcessBaseRequestStop(transport_catalogue, request.AsDict());
				}
			}
		}

		for (const auto& request : arr) {
			const auto base_request_type = request.AsDict().find("type");
			if (base_request_type != request.AsDict().end()) {
				if (base_request_type->second.AsString() == "Stop") {
					ProcessBaseRequestDistance(transport_catalogue, request.AsDict());
				}
			}
		}

		for (const auto& request : arr) {
			const auto base_request_type = request.AsDict().find("type");
			if (base_request_type != request.AsDict().end()) {
				if (base_request_type->second.AsString() == "Bus") {
					ProcessBaseRequestBus(transport_catalogue, request.AsDict());
				}
			}
		}


	}

	void ProcessBaseRequestStop(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict) {
		std::string stop_name = dict.find("name")->second.AsString();
		double latitude = dict.find("latitude")->second.AsDouble();
		double longitude = dict.find("longitude")->second.AsDouble();	
		transport_catalogue.AddStop({ move(stop_name), {latitude, longitude} });
	}

	void ProcessBaseRequestDistance(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict) {
		std::string_view stop_name_from = dict.find("name")->second.AsString();
		detail::StopPtr stop_from = transport_catalogue.FindStopByName(stop_name_from);
		const auto stops_name_to = dict.find("road_distances")->second.AsDict();
		std::vector<std::pair<detail::StopPtr, double>> stops_to;
		if (stops_name_to.size()) {		
			stops_to.reserve(stops_name_to.size());
			for (const auto& stop : stops_name_to) {
				detail::StopPtr stop_to = transport_catalogue.FindStopByName(move(stop.first));
				stops_to.push_back({ stop_to, stop.second.AsInt() });
				
			}
		}
		transport_catalogue.AddDistance(stop_from, stops_to);
	}

	void ProcessBaseRequestBus(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict) {
		std::string bus_number = dict.at("name").AsString();
		bool is_round_trip = dict.at("is_roundtrip").AsBool();
		const auto stops_on_route = dict.at("stops").AsArray();
		std::unordered_set<std::string> unique_stops;
		std::vector<std::string> raw_route;
		for (const auto& stop : stops_on_route) {
			raw_route.push_back(stop.AsString());
			unique_stops.insert(stop.AsString());
		}
		detail::Bus bus{ unique_stops.size(), stops_on_route.size(), bus_number, raw_route, {}, is_round_trip };
		transport_catalogue.AddBus(bus);		
	}

	void ProcessRoutingSettings(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict) {
		int waitTime = dict.at("bus_wait_time").AsInt();
		double velocity = dict.at("bus_velocity").AsDouble();
		transport_catalogue.SetRoutingSettings(waitTime, velocity);
	}

	json::Document StatRequests(RequestHandler& request_handler, const json::Array& arr, router::TransportRouter& router) {
		json::Array stat;
		for (const auto& request : arr) {
			const auto stat_request_type = request.AsDict().find("type");
			if (stat_request_type != request.AsDict().end()) {
				if (stat_request_type->second.AsString() == "Bus") {
					stat.push_back(ProcessStatRequestsBus(request_handler, request.AsDict()));
				}
				else if (stat_request_type->second.AsString() == "Stop") {
					stat.push_back(ProcessStatRequestsStop(request_handler, request.AsDict()));
				}
				else if (stat_request_type->second.AsString() == "Map") {
					stat.push_back(ProcessRenderSettingsRequests(request_handler, request.AsDict()));
				}
				else if (stat_request_type->second.AsString() == "Route") {
					stat.push_back(ProcessRouteRequests(router, request.AsDict()));
				}
			}
		}

		return json::Document(json::Builder{}.Value(stat).Build());

		//return json::Document{ json::Node{stat} };
	}

	json::Node ProcessRenderSettingsRequests(RequestHandler& request_handler, const json::Dict& dict) {
		svg::Document doc = request_handler.RenderMap();
		int id = dict.at("id").AsInt();
		std::ostringstream strm;
		doc.Render(strm);
		return json::Builder{}.StartDict()
								.Key("request_id").Value(id)
								.Key("map").Value(strm.str())
							  .EndDict()
							  .Build();
		//return json::Node{ json::Dict{{"request_id", id}, {"map", strm.str()}} };

	}

	json::Node ProcessStatRequestsStop(RequestHandler& request_handler, const json::Dict& dict) {
		int request_id = dict.at("id").AsInt();
		std::string stop_name = dict.at("name").AsString();
		const auto buses = request_handler.GetBusesByStop(move(stop_name));
		if (!buses) {
			std::string tmp = "not found";

			return json::Builder{}.StartDict()
									.Key("request_id").Value(request_id)
									.Key("error_message").Value(tmp)
								  .EndDict()
								  .Build();
			//return json::Node{ json::Dict{{"request_id", request_id}, {"error_message", "not found"}} };
		}
		else {
			json::Array arr;
			if (buses.has_value()) {
				arr.reserve(buses.value().size());
				for (const auto bus : buses.value()) {
					arr.push_back(move(std::string(bus)));
				}
			}
			
			return json::Builder{}.StartDict()
									.Key("request_id").Value(request_id)
									.Key("buses").Value(arr)
								  .EndDict()
								  .Build();

			//return json::Node{ json::Dict{{"request_id", request_id}, {"buses", arr}} };
		}
	}

	json::Node ProcessStatRequestsBus(RequestHandler& request_handler, const json::Dict& dict) {
		int request_id = dict.at("id").AsInt();
		auto bus_stat = request_handler.GetBusStat(dict.at("name").AsString());
		if (!bus_stat) {
			std::string tmp = "not found";

			return json::Builder{}.StartDict()
				.Key("request_id").Value(request_id)
				.Key("error_message").Value(tmp)
				.EndDict()
				.Build();
			//return json::Node{ json::Dict{{"request_id", request_id}, {"error_message", "not found"}} };
		}
		else {
			bus_stat->insert({ "request_id", request_id });
		}
		return json::Node{ bus_stat.value() };
	}

	renderer::RenderSettings ProcessRenderSettings(const json::Dict& dict) {
		renderer::RenderSettings render_settings;
		render_settings.width = dict.at("width").AsDouble();
		render_settings.height = dict.at("height").AsDouble();
		render_settings.padding = dict.at("padding").AsDouble();
		render_settings.line_width = dict.at("line_width").AsDouble();
		render_settings.stop_radius = dict.at("stop_radius").AsDouble();
		render_settings.bus_label_font_size = dict.at("bus_label_font_size").AsInt();
		render_settings.bus_label_offset = { dict.at("bus_label_offset").AsArray()[0].AsDouble(), dict.at("bus_label_offset").AsArray()[1].AsDouble() };
		render_settings.stop_label_font_size = dict.at("stop_label_font_size").AsInt();
		render_settings.stop_label_offset = { dict.at("stop_label_offset").AsArray()[0].AsDouble(), dict.at("stop_label_offset").AsArray()[1].AsDouble() };
		render_settings.underlayer_color = render_settings.GetColor(dict.at("underlayer_color"));
		render_settings.underlayer_width = dict.at("underlayer_width").AsDouble();

		for (const auto& color : dict.at("color_palette").AsArray()) {
			render_settings.color_palette.push_back(render_settings.GetColor(color));
		}

		return render_settings;
	}

	void RenderSettingsRequests(catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, const json::Dict& dict) {
		map_renderer.SetRenderSettings(ProcessRenderSettings(dict));
		map_renderer.CreateRender(transport_catalogue);
	}

	json::Node ProcessRouteRequests(router::TransportRouter& router, const json::Dict& dict) {
		int request_id = dict.at("id").AsInt();
		std::string_view stop_from = dict.at("from").AsString();
		std::string_view stop_to = dict.at("to").AsString();
		auto found_route = router.FindRouteInGraph(stop_from, stop_to);
		
		if (!found_route) {
			std::string tmp = "not found";

			return json::Builder{}
				.StartDict()
					.Key("request_id").Value(request_id)
					.Key("error_message").Value(tmp)
				.EndDict()
				.Build();
		}

		json::Builder builder;
		builder.StartArray();

		for (auto& item : found_route.value().edges_info) {
			if (item.item_type == router::ItemType::WAIT_ITEM) {
				std::string tmp = "Wait";
				builder
					.StartDict()
						.Key("stop_name").Value(std::string(item.name))
						.Key("time").Value(item.total_time)
						.Key("type").Value(tmp)
					.EndDict();
			}
			else if (item.item_type == router::ItemType::BUS_ITEM) {
				std::string tmp = "Bus";
				builder
					.StartDict()
						.Key("bus").Value(std::string(item.name))
						.Key("span_count").Value(item.span_count)
						.Key("time").Value(item.total_time)
						.Key("type").Value(tmp)
					.EndDict();
			}
		}

		return json::Builder{}
			.StartDict()
				.Key("items").Value(builder.EndArray().Build().AsArray())
				.Key("request_id").Value(request_id)
				.Key("total_time").Value(found_route.value().route_total_time)
			.EndDict()
			.Build();	
	}
}