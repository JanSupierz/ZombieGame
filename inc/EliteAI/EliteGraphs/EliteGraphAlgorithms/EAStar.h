#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& visitedNodes);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode, std::vector<T_NodeType*>& visitedNodes)
	{
		visitedNodes.clear();

		std::vector<T_NodeType*> path;

		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;

		std::vector<NodeRecord> visitedList;

		NodeRecord currentRecord{ pStartNode };
		currentRecord.pConnection = nullptr;
		currentRecord.costSoFar = 0.f;
		currentRecord.estimatedTotalCost = GetHeuristicCost(pStartNode, pGoalNode);

		openList.push_back(currentRecord);

		while (openList.empty() == false)
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());

			if (currentRecord.pNode == pGoalNode)
			{
				break;
			}

			for (auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				const float costSoFar{ currentRecord.costSoFar + connection->GetCost() };

				bool isInClosedList{ false };
				bool isInOpenList{ false };

				NodeRecord existingRecord{};

				for(auto& element:closedList)
				{
					if(element.pNode == m_pGraph->GetNode(connection->GetTo()))
					{
						isInClosedList = true;
						existingRecord = element;
						break;
					}
				}

				if (!isInClosedList)
				{
					for (auto& element : openList)
					{
						if (element.pNode == m_pGraph->GetNode(connection->GetTo()))
						{
							isInOpenList = true;
							existingRecord = element;
							break;
						}
					}
				}

				if (isInClosedList)
				{
					if (costSoFar < existingRecord.costSoFar)
					{
						closedList.erase(std::remove(closedList.begin(), closedList.end(), existingRecord));
					}
					else
					{
						continue;
					}
				}
				else if (isInOpenList)
				{
					if (costSoFar < existingRecord.costSoFar)
					{
						openList.erase(std::remove(openList.begin(), openList.end(), existingRecord));
					}
					else
					{
						continue;
					}
				}

				NodeRecord newRecord{ m_pGraph->GetNode(connection->GetTo()) };
				newRecord.pConnection = connection;
				newRecord.costSoFar = costSoFar;
				newRecord.estimatedTotalCost = newRecord.costSoFar + GetHeuristicCost(newRecord.pNode, pGoalNode);

				openList.push_back(newRecord);
			}

			closedList.push_back(currentRecord);

			visitedList.push_back(currentRecord);
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
		}

		for (auto& record : visitedList)
		{
			//Debug visualization
			visitedNodes.push_back(record.pNode);
		}

		if(currentRecord.pNode != pGoalNode)
		{
			//Fallback path to closest node if end node is unreachable
			for (auto& record : visitedList)
			{
				record.estimatedTotalCost -= record.costSoFar;
			}

			currentRecord = *std::min_element(visitedList.begin(), visitedList.end());
		}

		while(currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);
			
			for (auto& element : closedList)
			{
				if (element.pNode == m_pGraph->GetNode(currentRecord.pConnection->GetFrom()))
				{
					currentRecord = element;
					break;
				}
			}

		}

		path.push_back(pStartNode);

		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}