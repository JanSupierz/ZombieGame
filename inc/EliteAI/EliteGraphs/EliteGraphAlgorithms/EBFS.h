#pragma once

namespace Elite 
{
	template <class T_NodeType, class T_ConnectionType>
	class BFS
	{
	public:
		BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& visitedNodes);
	private:
		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template <class T_NodeType, class T_ConnectionType>
	BFS<T_NodeType, T_ConnectionType>::BFS(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{

	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> BFS<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode, std::vector<T_NodeType*>& visitedNodes)
	{
		visitedNodes.clear();

		std::queue<T_NodeType*> openList;
		std::map<T_NodeType*, T_NodeType*> closedList;

		openList.push(pStartNode);

		while (openList.empty() == false)
		{
			T_NodeType* pCurrentNode{ openList.front() };
			visitedNodes.push_back(pCurrentNode);
			openList.pop();

			if (pCurrentNode == pDestinationNode)
			{
				break;
			}

			for (auto& connection : m_pGraph->GetNodeConnections(pCurrentNode))
			{
				T_NodeType* pNextNode{ m_pGraph->GetNode(connection->GetTo()) };

				if (closedList.find(pNextNode) == closedList.end())
				{
					openList.push(pNextNode);
					closedList[pNextNode] = pCurrentNode;
				}
			}
		}

		if (closedList.find(pDestinationNode) == closedList.end())
		{
			float lowestCost{ 1000000.f };
			T_NodeType* closestNode{ pStartNode };

			Elite::Vector2 destinationPosition{ m_pGraph->GetNodeWorldPos(pDestinationNode) };

			for (auto& currentNode : visitedNodes)
			{
				const float currentCost{ (m_pGraph->GetNodeWorldPos(currentNode) - destinationPosition).Magnitude() };
				if (currentCost < lowestCost)
				{
					lowestCost = currentCost;
					closestNode = currentNode;
				}
			}

			std::vector<T_NodeType*> tempVisitedNodes;

			return FindPath(pStartNode, closestNode, tempVisitedNodes);
		}

		//Backtracking
		std::vector<T_NodeType*> path;
		
		T_NodeType* pCurrentNode{ pDestinationNode };

		while (pCurrentNode != pStartNode)
		{
			path.push_back(pCurrentNode);
			pCurrentNode = closedList[pCurrentNode];
		}

		path.push_back(pStartNode);

		std::reverse(path.begin(), path.end());

		return path;
	}
}

