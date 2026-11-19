#pragma once

#include <cstddef>
#include <string>
#include <vector>

const int DEFAULT_VERTEX_ID = -1;
const long long INITIAL_ARC_WEIGHT = 1;

/*
 * Represents a directed edge between a source IP vertex and a destination
 * server vertex, stored once and linked into both incidence lists.
 */
struct Arc {
	int origin = DEFAULT_VERTEX_ID;
	int destination = DEFAULT_VERTEX_ID;
	long long weight = 0;
	Arc* nextOut = nullptr;
	Arc* nextIn = nullptr;
};

/*
 * Represents a graph vertex for either a source IP or a destination server,
 * holding its name, role flag, and outgoing/incoming arc list heads.
 */
struct Vertex {
	std::string name = "";
	bool isServer = false;
	Arc* outHead = nullptr;
	Arc* inHead = nullptr;

	/*
	 * Creates an empty vertex with default values.
	 * @return Initialized Vertex instance.
	 */
	Vertex() = default;

	/*
	 * Creates a vertex with the given name and role flag.
	 * @param vertexName IP address or server hostname.
	 * @param serverFlag True if the vertex is a destination server, false if IP.
	 * @return Initialized Vertex instance.
	 */
	Vertex(const std::string& vertexName, bool serverFlag)
		: name(vertexName), isServer(serverFlag), outHead(nullptr), inHead(nullptr) {
	}
};

/*
 * Directed graph stored as an arc list with dynamic vertex allocation and
 * dual incidence lists for outgoing and incoming degree/weight queries.
 */
class ArcListGraph {
private:
	std::vector<Vertex> vertices;
	std::vector<Arc*> allArcs;

	/*
	 * Searches for an existing arc from origin to destination in outgoing list.
	 * @param originId Source vertex index.
	 * @param destinationId Destination vertex index.
	 * @return Pointer to matching Arc if found, or nullptr otherwise.
	 */
	Arc* findOutgoingArc(int originId, int destinationId) const {
		Arc* found = nullptr;
		Arc* current = vertices[originId].outHead;

		while (current != nullptr && found == nullptr) {
			if (current->destination == destinationId) {
				found = current;
			} else {
				current = current->nextOut;
			}
		}
		return found;
	}

public:
	/*
	 * Creates an empty directed access graph.
	 * @return Initialized ArcListGraph instance.
	 */
	ArcListGraph() = default;

	ArcListGraph(const ArcListGraph&) = delete;
	ArcListGraph& operator=(const ArcListGraph&) = delete;

	/*
	 * Releases all dynamically allocated arcs owned by the graph.
	 * @return Nothing.
	 */
	~ArcListGraph() {
		std::size_t arcCount = allArcs.size();

		for (std::size_t i = 0; i < arcCount; ++i) {
			delete allArcs[i];
			allArcs[i] = nullptr;
		}
	}

	/*
	 * Returns the vertex index for name and role, creating it if not present.
	 * @param vertexName IP address or server hostname.
	 * @param serverFlag True for destination server, false for source IP.
	 * @return Index of the existing or newly created vertex.
	 */
	int getId(const std::string& vertexName, bool serverFlag) {
		int vertexCount = static_cast<int>(vertices.size());
		int foundId = DEFAULT_VERTEX_ID;
		int index = 0;

		while (index < vertexCount && foundId == DEFAULT_VERTEX_ID) {
			if (vertices[index].name == vertexName && vertices[index].isServer == serverFlag) {
				foundId = index;
			}
			index += 1;
		}

		if (foundId == DEFAULT_VERTEX_ID) {
			vertices.push_back(Vertex(vertexName, serverFlag));
			foundId = static_cast<int>(vertices.size()) - 1;
		}
		return foundId;
	}

	/*
	 * Returns the total number of vertices currently stored in the graph.
	 * @return Number of vertices.
	 */
	int size() const {
		return static_cast<int>(vertices.size());
	}

	/*
	 * Returns the name associated with a given vertex index.
	 * @param vertexId Index of the target vertex.
	 * @return Constant reference to the vertex name.
	 */
	const std::string& name(int vertexId) const {
		return vertices[vertexId].name;
	}

	/*
	 * Indicates whether a vertex represents a destination server.
	 * @param vertexId Index of the target vertex.
	 * @return True if the vertex is a server, false if it is a source IP.
	 */
	bool isServer(int vertexId) const {
		return vertices[vertexId].isServer;
	}

	/*
	 * Registers one access from originId to destinationId, incrementing weight
	 * if the arc exists or prepending a new arc to both incidence lists.
	 * @param originId Source IP vertex index.
	 * @param destinationId Destination server vertex index.
	 * @return Nothing.
	 */
	void addAccess(int originId, int destinationId) {
		Arc* existingArc = findOutgoingArc(originId, destinationId);

		if (existingArc != nullptr) {
			existingArc->weight += INITIAL_ARC_WEIGHT;
		} else {
			Arc* newArc = new Arc;

			newArc->origin = originId;
			newArc->destination = destinationId;
			newArc->weight = INITIAL_ARC_WEIGHT;
			newArc->nextOut = vertices[originId].outHead;
			newArc->nextIn = vertices[destinationId].inHead;

			vertices[originId].outHead = newArc;
			vertices[destinationId].inHead = newArc;
			allArcs.push_back(newArc);
		}
	}

	/*
	 * Computes the number of distinct destination servers reached by a source IP.
	 * @param originId Source IP vertex index.
	 * @return Out-degree of the vertex.
	 */
	int outDegree(int originId) const {
		int degree = 0;

		for (Arc* current = vertices[originId].outHead; current != nullptr; current = current->nextOut) {
			degree += 1;
		}
		return degree;
	}

	/*
	 * Computes the number of distinct source IPs reaching a destination server.
	 * @param destinationId Destination server vertex index.
	 * @return In-degree of the vertex.
	 */
	int inDegree(int destinationId) const {
		int degree = 0;

		for (Arc* current = vertices[destinationId].inHead; current != nullptr; current = current->nextIn) {
			degree += 1;
		}
		return degree;
	}

	/*
	 * Computes the total number of accesses sent from a source IP vertex.
	 * @param originId Source IP vertex index.
	 * @return Sum of weights across all outgoing arcs.
	 */
	long long totalOutWeight(int originId) const {
		long long total = 0;

		for (Arc* current = vertices[originId].outHead; current != nullptr; current = current->nextOut) {
			total += current->weight;
		}
		return total;
	}

	/*
	 * Computes the total number of accesses received by a destination server vertex.
	 * @param destinationId Destination server vertex index.
	 * @return Sum of weights across all incoming arcs.
	 */
	long long totalInWeight(int destinationId) const {
		long long total = 0;

		for (Arc* current = vertices[destinationId].inHead; current != nullptr; current = current->nextIn) {
			total += current->weight;
		}
		return total;
	}
};