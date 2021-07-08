#include "transport_router.h"
#include <iostream>

namespace router {
	using namespace graph;
	using namespace catalogue;
	using namespace std;

	optional<RouteInfo> TransportRouter::FindRouteInGraph(string_view from, string_view to) const {
		detail::StopPtr stop_from = transport_catalogue_.FindStopByName(from);
		detail::StopPtr stop_to = transport_catalogue_.FindStopByName(to);

		if (stop_from == nullptr || stop_to == nullptr) {
			return nullopt;
		}

		VertexId from_vertex_id = stops_to_vertex_ids_.at(stop_from).stop_out;
		VertexId to_vertex_id = stops_to_vertex_ids_.at(stop_to).stop_out;

		auto route = transport_router_->BuildRoute(from_vertex_id, to_vertex_id);
		if (!route) {
			return nullopt;
		}

		RouteInfo result;
		result.route_total_time = route.value().weight;

		for (auto& edge_id : route->edges) {
			result.edges_info.push_back(edges_info_.at(edge_id));
		}
		return result;
	}

	const std::unordered_map<detail::StopPtr, VertexInfo>& TransportRouter::GetStopsToVertexIds() const {
		return stops_to_vertex_ids_;
	}

	const std::vector<EdgesInfo>& TransportRouter::GetEdgesInfo() const {
		return edges_info_;
	}

	const size_t TransportRouter::GetGraphVertexCount() const {
		return graph_.get()->GetVertexCount();
	}

	void TransportRouter::AddEdgeInfo(const EdgesInfo& edge_info) {
		edges_info_.push_back(edge_info);
	}

	void TransportRouter::AddEdgeToGraph(const graph::Edge<double> edge) {
		size_t edge_id = graph_->AddEdge(edge);
	}


	void TransportRouter::AddStopToVertexIds(const detail::StopPtr stop, const VertexInfo vertex_info) {
		stops_to_vertex_ids_[stop] = vertex_info;
	}

	void TransportRouter::SetBusWaitTime(const int bus_wait_time) {
		bus_wait_time_ = bus_wait_time;
	}

	void TransportRouter::SetBusVelocity(const double velocity) {
		bus_velocity_ = velocity;
	}

	void TransportRouter::SetRouter() {
		transport_router_ = make_unique<Router>(*graph_);
	}

	void TransportRouter::SetGraph(const size_t vertex_count) {
		graph_ = make_unique<Graph>(vertex_count * 2);
	}
}