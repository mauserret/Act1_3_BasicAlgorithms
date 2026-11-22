/**
 * Directed graph stored as an arc list. Every distinct (IP, server) pair is
 * one arc, kept once in memory and linked from two incidence lists.
 *
Authors: A01648241
         A01642638
         A01648221
         A01643073
Date: 19/11/2026
 */

#ifndef ARC_LIST_H
#define ARC_LIST_H

#include <string>
#include <vector>

/**
 * One arc per distinct (origin IP, destination server) pair. It lives once
 * in memory and is linked in the outgoing list of its origin and in the
 * incoming list of its destination.
 */
struct Arc {
	int origin;
	int destination;
	long long weight;
	Arc* nextOut;
	Arc* nextIn;
};

/**
 * One vertex per distinct name (an IP or a server host name). The name lives
 * here, not in a parallel vector.
 */
struct Vertex {
	std::string name;
	bool isServer;
	Arc* outHead;
	Arc* inHead;

	Vertex() : name(""), isServer(false), outHead(nullptr), inHead(nullptr) {
	}

	Vertex(const std::string& vertexName, bool vertexIsServer)
		: name(vertexName), isServer(vertexIsServer), outHead(nullptr), inHead(nullptr) {
	}
};

class ArcListGraph {
private:
	std::vector<Vertex> vertices;
	std::vector<Arc*> allArcs;

public:
	ArcListGraph() = default;

	// Copies would free the same raw Arc pointers twice.
	ArcListGraph(const ArcListGraph&) = delete;
	ArcListGraph& operator=(const ArcListGraph&) = delete;

	/**
	 * @brief Frees every arc owned by the graph.
	 * @note Time O(A), A = number of arcs. Space O(1).
	 */
	~ArcListGraph() {
		for (Arc* arc : allArcs) {
			delete arc;
		}
	}

	/**
	 * @brief Finds the vertex with the given name and kind, creating it the
	 *        first time it is seen.
	 * @param name IP text or server host name.
	 * @param isServer true for a server, false for an IP.
	 * @return Id (index) of the vertex.
	 * @note Time O(V) in the worst case: linear scan of the vertex vector.
	 *       Space O(1) amortized.
	 */
	int getOrCreateVertexId(const std::string& name, bool isServer) {
		int id = 0;
		int vertexCount = static_cast<int>(vertices.size());
		while (id < vertexCount && !(vertices[id].name == name && vertices[id].isServer == isServer)) {
			id++;
		}
		if (id == vertexCount) {
			vertices.push_back(Vertex(name, isServer));
		}
		return id;
	}

	/**
	 * @brief Registers one access origin -> destination. Adds 1 to the weight
	 *        of the arc if it exists, otherwise creates it and links it at
	 *        the front of both incidence lists.
	 * @param origin Id of the IP vertex.
	 * @param destination Id of the server vertex.
	 * @note Time O(outDegree(origin)) in the worst case: scan of the outgoing
	 *       list. Space O(1) amortized.
	 */
	void addAccess(int origin, int destination) {
		Arc* arc = vertices[origin].outHead;
		while (arc != nullptr && arc->destination != destination) {
			arc = arc->nextOut;
		}
		if (arc != nullptr) {
			arc->weight++;
		} else {
			arc = new Arc{origin, destination, 1, vertices[origin].outHead, vertices[destination].inHead};
			vertices[origin].outHead = arc;
			vertices[destination].inHead = arc;
			allArcs.push_back(arc);
		}
	}

	/**
	 * @brief Counts the vertices of one kind.
	 * @param isServer true to count servers, false to count IPs.
	 * @return Number of vertices of that kind.
	 * @note Time O(V). Space O(1).
	 */
	int countVertices(bool isServer) const {
		int count = 0;
		for (const Vertex& vertex : vertices) {
			if (vertex.isServer == isServer) {
				count++;
			}
		}
		return count;
	}

	/**
	 * @brief Gives the number of vertices of any kind.
	 * @return Total number of vertices.
	 * @note Time O(1). Space O(1).
	 */
	int size() const {
		return static_cast<int>(vertices.size());
	}

	/**
	 * @brief Gives the number of distinct arcs.
	 * @return Number of arcs.
	 * @note Time O(1). Space O(1).
	 */
	int arcCount() const {
		return static_cast<int>(allArcs.size());
	}

	/**
	 * @brief Adds the weight of every arc, i.e. the accesses in the log.
	 * @return Total number of accesses.
	 * @note Time O(A). Space O(1).
	 */
	long long totalWeight() const {
		long long total = 0;
		for (const Arc* arc : allArcs) {
			total += arc->weight;
		}
		return total;
	}

	/**
	 * @brief Gives the name of a vertex.
	 * @param vertexId Id of the vertex.
	 * @return IP text or server host name.
	 * @note Time O(1). Space O(1).
	 */
	const std::string& name(int vertexId) const {
		return vertices[vertexId].name;
	}

	/**
	 * @brief Tells if a vertex is a server or an IP.
	 * @param vertexId Id of the vertex.
	 * @return true for a server, false for an IP.
	 * @note Time O(1). Space O(1).
	 */
	bool isServer(int vertexId) const {
		return vertices[vertexId].isServer;
	}

	/**
	 * @brief Counts the arcs that leave an IP: distinct servers it reaches.
	 * @param vertexId Id of the IP vertex.
	 * @return Out-degree of the vertex.
	 * @note Time O(outDegree). Space O(1).
	 */
	int outDegree(int vertexId) const {
		int degree = 0;
		for (const Arc* arc = vertices[vertexId].outHead; arc != nullptr; arc = arc->nextOut) {
			degree++;
		}
		return degree;
	}

	/**
	 * @brief Counts the arcs that enter a server: distinct IPs that reach it.
	 * @param vertexId Id of the server vertex.
	 * @return In-degree of the vertex.
	 * @note Time O(inDegree). Space O(1).
	 */
	int inDegree(int vertexId) const {
		int degree = 0;
		for (const Arc* arc = vertices[vertexId].inHead; arc != nullptr; arc = arc->nextIn) {
			degree++;
		}
		return degree;
	}

	/**
	 * @brief Adds the weights of the arcs that leave an IP.
	 * @param vertexId Id of the IP vertex.
	 * @return Total accesses sent by the IP.
	 * @note Time O(outDegree). Space O(1).
	 */
	long long totalOutWeight(int vertexId) const {
		long long total = 0;
		for (const Arc* arc = vertices[vertexId].outHead; arc != nullptr; arc = arc->nextOut) {
			total += arc->weight;
		}
		return total;
	}

	/**
	 * @brief Adds the weights of the arcs that enter a server.
	 * @param vertexId Id of the server vertex.
	 * @return Total accesses received by the server.
	 * @note Time O(inDegree). Space O(1).
	 */
	long long totalInWeight(int vertexId) const {
		long long total = 0;
		for (const Arc* arc = vertices[vertexId].inHead; arc != nullptr; arc = arc->nextIn) {
			total += arc->weight;
		}
		return total;
	}
};

#endif
