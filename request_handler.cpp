#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

std::optional<RequestHandler::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	json::Dict dict;
	const auto bus = db_.FindBusByNumber(bus_name);
	if (bus == nullptr) {
		return std::nullopt;
	}
	else {
		const auto route_info = db_.GetRouteInfromation(bus_name);
		dict["curvature"] = route_info.first.first / route_info.first.second;
		dict["route_length"] = static_cast<int>(route_info.first.first);
		dict["stop_count"] = static_cast<int>(route_info.second->stops_number);
		dict["unique_stop_count"] = static_cast<int>(route_info.second->unique_stops);
	}

	return dict;
}

const std::optional<std::set<std::string_view>> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	std::set<std::string_view> buses;
	const auto stop = db_.FindStopByName(stop_name);
	if (stop == nullptr) {
		return std::nullopt;
	}
	else {
		buses = db_.FindBusesByStop(stop_name).second;
	}
	return buses;
}

svg::Document RequestHandler::RenderMap() const {
	return renderer_.Render();
}