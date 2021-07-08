#pragma once
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "domain.h"
//#include "serialization.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <cmath>
#include <optional>
#include <string_view>

namespace router {
	using namespace catalogue;

	enum class ItemType {
		WAIT_ITEM,
		BUS_ITEM
	};

	struct EdgesInfo {
		//graph::EdgeId edge_id;
		std::string name;
		double total_time;
		ItemType item_type = ItemType::WAIT_ITEM;
		int span_count = 0;
	};

	struct RouteInfo {
		double route_total_time;
		std::vector<EdgesInfo> edges_info;
	};

	struct VertexInfo {
		graph::VertexId stop_in;
		graph::VertexId stop_out;
	};

	class TransportRouter {
	public:
		using Graph = graph::DirectedWeightedGraph<double>;
		using Router = graph::Router<double>;
		
		explicit TransportRouter(const TransportCatalogue& transport_catalogue)
			:transport_catalogue_(transport_catalogue),
			 bus_wait_time_(transport_catalogue.GetRoutingSettings().first),
			 bus_velocity_(transport_catalogue.GetRoutingSettings().second)
		{
		}

		std::optional<RouteInfo> FindRouteInGraph(std::string_view from, std::string_view to) const;
		
		//Геттеры для сериализации
		const std::unordered_map<detail::StopPtr, VertexInfo>& GetStopsToVertexIds() const;
		const std::vector<EdgesInfo>& GetEdgesInfo() const;
		const size_t GetGraphVertexCount() const;
		
		const std::vector<graph::Edge<double>> GetGraphEdges() const {
			return graph_.get()->GetEdges();
		}

		//Сеттеры для десериализации
		void AddEdgeInfo(const EdgesInfo& edge_info);
		void AddStopToVertexIds(const detail::StopPtr stop, const VertexInfo vertex_info);
		void AddEdgeToGraph(const graph::Edge<double> edge);
		void SetBusWaitTime(const int bus_wait_time);
		void SetBusVelocity(const double velocity);
		void SetRouter();
		void SetGraph(const size_t vertex_count);


	private:
		const TransportCatalogue& transport_catalogue_;
		std::unique_ptr<Graph> graph_;
		std::unique_ptr<Router> transport_router_;
		int bus_wait_time_ = 0;
		double bus_velocity_ = 0.0;
		std::unordered_map<detail::StopPtr, VertexInfo> stops_to_vertex_ids_;
		std::vector<EdgesInfo> edges_info_;		
	};
}