#pragma once

#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"

#include <iostream>
#include <vector>
#include <unordered_set>
#include <sstream>

namespace json_reader {
	struct RoutingSettings {
		int busWaitTime;
		double busVelocity;
	};
		
	void JsonReader(catalogue::TransportCatalogue& transport_catalogue, std::istream& in, std::ostream& out);
	void JsonSerialize(std::istream& in);
	void JsonDeserialize(std::istream& in, std::ostream& out);
	
	void ProcessRequests(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict, std::ostream& out);

	json::Document StatRequests(RequestHandler& request_handler, const json::Array& arr, router::TransportRouter& router);
	json::Node ProcessStatRequestsStop(RequestHandler& request_handler, const json::Dict& dict);
	json::Node ProcessStatRequestsBus(RequestHandler& request_handler, const json::Dict& dict);
	json::Node ProcessRenderSettingsRequests(RequestHandler& request_handler, const json::Dict& dict);
	json::Node ProcessRouteRequests(router::TransportRouter& router, const json::Dict& dict);
	
	void ProcessBaseRequests(catalogue::TransportCatalogue& transport_catalogue, const json::Array& arr);
	void ProcessBaseRequestStop(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict);
	void ProcessBaseRequestBus(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict);
	void ProcessBaseRequestDistance(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict);
	void ProcessRoutingSettings(catalogue::TransportCatalogue& transport_catalogue, const json::Dict& dict);

	renderer::RenderSettings ProcessRenderSettings(const json::Dict& dict);
	void RenderSettingsRequests(catalogue::TransportCatalogue& transport_catalogue, renderer::MapRenderer& map_renderer, const json::Dict& dict);

}