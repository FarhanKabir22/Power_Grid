#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>

using namespace std;

// ============================================================
// INPUT UTILITIES
// ============================================================

string getInput(const string& prompt) {
    cout << prompt << flush;

    string input;
    getline(cin, input);

    return input;
}

int getInt(const string& prompt, int minValue, int maxValue) {
    while (true) {
        string input = getInput(prompt);

        stringstream ss(input);

        int value;
        char extra;

        if (ss >> value && !(ss >> extra)) {
            if (value >= minValue && value <= maxValue)
                return value;
        }

        cout << "\nInvalid input. Please enter a number from "
             << minValue << " to " << maxValue << ".\n";
    }
}

double getDouble(
    const string& prompt,
    double minValue,
    double maxValue
) {
    while (true) {
        string input = getInput(prompt);

        stringstream ss(input);

        double value;
        char extra;

        if (ss >> value && !(ss >> extra)) {
            if (value >= minValue && value <= maxValue)
                return value;
        }

        cout << "\nInvalid input. Please enter a value between "
             << minValue << " and " << maxValue << ".\n";
    }
}

void waitForEnter() {
    cout << "\nPress ENTER to continue..." << flush;

    string input;
    getline(cin, input);
}

// ============================================================
// ENUMS
// ============================================================

enum class NodeType {
    POWER_PLANT,
    SUBSTATION,
    LOAD_CENTER
};

enum class LoadPriority {
    CRITICAL_HIGH = 1,
    MEDIUM = 2,
    LOW = 3,
    NONE = 99
};

// ============================================================
// UTILITY FUNCTIONS
// ============================================================

string nodeTypeToString(NodeType type) {
    if (type == NodeType::POWER_PLANT)
        return "POWER PLANT";

    if (type == NodeType::SUBSTATION)
        return "SUBSTATION";

    return "LOAD CENTER";
}

string priorityToString(LoadPriority p) {
    if (p == LoadPriority::CRITICAL_HIGH)
        return "HIGH";

    if (p == LoadPriority::MEDIUM)
        return "MEDIUM";

    if (p == LoadPriority::LOW)
        return "LOW";

    return "N/A";
}

// ============================================================
// NODE
// ============================================================

struct Node {
    int id;
    string name;

    NodeType type;
    LoadPriority priority;

    double demand;
    double supply;
    double received;

    bool reachable;
};

// ============================================================
// EDGE
// ============================================================

struct Edge {
    int u;
    int v;

    double capacity;
    double cost;

    bool failed;
    bool backup;
    bool active;
};

// ============================================================
// DSU
// ============================================================

class DSU {

private:

    vector<int> parent;
    vector<int> rankValue;

public:

    DSU(int n) {
        parent.resize(n);
        rankValue.assign(n, 0);

        iota(
            parent.begin(),
            parent.end(),
            0
        );
    }

    int find(int x) {
        if (parent[x] == x)
            return x;

        return parent[x] = find(parent[x]);
    }

    bool unite(int a, int b) {
        int rootA = find(a);
        int rootB = find(b);

        if (rootA == rootB)
            return false;

        if (rankValue[rootA] < rankValue[rootB])
            swap(rootA, rootB);

        parent[rootB] = rootA;

        if (rankValue[rootA] == rankValue[rootB])
            rankValue[rootA]++;

        return true;
    }
};

// ============================================================
// DINIC MAX FLOW
// ============================================================

struct FlowEdge {
    int to;

    double capacity;
    double flow;

    int reverseIndex;
};

class Dinic {

private:

    int n;

    vector<vector<FlowEdge>> graph;

    vector<int> level;
    vector<int> pointer;

    bool bfs(int source, int sink) {

        fill(
            level.begin(),
            level.end(),
            -1
        );

        queue<int> q;

        level[source] = 0;
        q.push(source);

        while (!q.empty()) {

            int u = q.front();
            q.pop();

            for (const auto& e : graph[u]) {

                double residual =
                    e.capacity - e.flow;

                if (
                    residual > 1e-9 &&
                    level[e.to] == -1
                ) {
                    level[e.to] =
                        level[u] + 1;

                    q.push(e.to);
                }
            }
        }

        return level[sink] != -1;
    }

    double dfs(
        int u,
        int sink,
        double pushed
    ) {
        if (pushed <= 1e-9)
            return 0.0;

        if (u == sink)
            return pushed;

        for (
            int& i = pointer[u];
            i < (int)graph[u].size();
            i++
        ) {

            FlowEdge& e =
                graph[u][i];

            double residual =
                e.capacity - e.flow;

            if (
                level[e.to] != level[u] + 1 ||
                residual <= 1e-9
            ) {
                continue;
            }

            double amount =
                dfs(
                    e.to,
                    sink,
                    min(
                        pushed,
                        residual
                    )
                );

            if (amount <= 1e-9)
                continue;

            e.flow += amount;

            graph[e.to]
                 [e.reverseIndex]
                .flow -= amount;

            return amount;
        }

        return 0.0;
    }

public:

    Dinic(int n)
        : n(n),
          graph(n),
          level(n),
          pointer(n) {
    }

    int addEdge(
        int from,
        int to,
        double capacity
    ) {
        int forwardIndex =
            (int)graph[from].size();

        int reverseIndex =
            (int)graph[to].size();

        FlowEdge forward{
            to,
            capacity,
            0.0,
            reverseIndex
        };

        FlowEdge backward{
            from,
            0.0,
            0.0,
            forwardIndex
        };

        graph[from].push_back(forward);
        graph[to].push_back(backward);

        return forwardIndex;
    }

    double maxFlow(
        int source,
        int sink
    ) {
        double total = 0.0;

        while (bfs(source, sink)) {

            fill(
                pointer.begin(),
                pointer.end(),
                0
            );

            while (true) {

                double pushed =
                    dfs(
                        source,
                        sink,
                        1e18
                    );

                if (pushed <= 1e-9)
                    break;

                total += pushed;
            }
        }

        return total;
    }

    double getFlow(
        int from,
        int index
    ) const {
        return graph[from][index].flow;
    }
};

// ============================================================
// POWER GRID
// ============================================================

class PowerGrid {

private:

    int n;

    vector<Node> nodes;
    vector<Edge> edges;

    int plantID;

public:

    PowerGrid(int n)
        : n(n),
          nodes(n),
          plantID(0) {
    }

    // ========================================================
    // ADD NODE
    // ========================================================

    void addNode(
        int id,
        const string& name,
        NodeType type,
        LoadPriority priority,
        double demand,
        double supply
    ) {
        nodes[id] = {
            id,
            name,
            type,
            priority,
            demand,
            supply,
            0.0,
            false
        };

        if (type == NodeType::POWER_PLANT)
            plantID = id;
    }

    // ========================================================
    // ADD EDGE
    // ========================================================

    void addEdge(
        int u,
        int v,
        double capacity,
        double cost,
        bool failed,
        bool backup
    ) {
        bool active =
            !failed && !backup;

        edges.push_back({
            u,
            v,
            capacity,
            cost,
            failed,
            backup,
            active
        });
    }

    // ========================================================
    // HEADER
    // ========================================================

    void header(const string& title) {

        cout << "\n";

        cout
            << "============================================================\n";

        cout
            << " "
            << title
            << "\n";

        cout
            << "============================================================\n";
    }

    // ========================================================
    // SHOW NODES
    // ========================================================

    void showNodes() {

        header("POWER GRID NODES");

        cout << left
             << setw(5) << "ID"
             << setw(28) << "NAME"
             << setw(18) << "TYPE"
             << setw(15) << "PRIORITY"
             << setw(12) << "DEMAND"
             << "SUPPLY\n";

        cout
            << string(100, '-')
            << "\n";

        for (const auto& node : nodes) {

            cout << left
                 << setw(5)
                 << node.id

                 << setw(28)
                 << node.name

                 << setw(18)
                 << nodeTypeToString(node.type)

                 << setw(15)
                 << priorityToString(node.priority)

                 << setw(12)
                 << fixed
                 << setprecision(1)
                 << node.demand

                 << node.supply
                 << " MW\n";
        }
    }

    // ========================================================
    // SHOW EDGES
    // ========================================================

    void showEdges() {

        header("TRANSMISSION LINES");

        cout << left
             << setw(5) << "ID"
             << setw(27) << "FROM"
             << setw(27) << "TO"
             << setw(12) << "CAPACITY"
             << setw(12) << "COST"
             << setw(12) << "STATUS"
             << "TYPE\n";

        cout
            << string(110, '-')
            << "\n";

        for (
            int i = 0;
            i < (int)edges.size();
            i++
        ) {

            const Edge& e = edges[i];

            string status;

            if (e.failed)
                status = "FAILED";
            else if (e.active)
                status = "ACTIVE";
            else
                status = "STANDBY";

            string type =
                e.backup
                ? "BACKUP"
                : "PRIMARY";

            cout << left
                 << setw(5)
                 << i

                 << setw(27)
                 << nodes[e.u].name

                 << setw(27)
                 << nodes[e.v].name

                 << setw(12)
                 << e.capacity

                 << setw(12)
                 << e.cost

                 << setw(12)
                 << status

                 << type
                 << "\n";
        }
    }

    // ========================================================
    // SHOW TOPOLOGY
    // ========================================================

    void showTopology() {

        header("CURRENT ACTIVE TOPOLOGY");

        for (const auto& node : nodes) {

            cout
                << "\n["
                << node.id
                << "] "
                << node.name
                << "\n";

            bool found = false;

            for (const auto& e : edges) {

                if (!e.active || e.failed)
                    continue;

                if (e.u == node.id) {

                    cout
                        << "   --> "
                        << nodes[e.v].name
                        << " ("
                        << e.capacity
                        << " MW)\n";

                    found = true;
                }
                else if (e.v == node.id) {

                    cout
                        << "   --> "
                        << nodes[e.u].name
                        << " ("
                        << e.capacity
                        << " MW)\n";

                    found = true;
                }
            }

            if (!found)
                cout
                    << "   No active connection\n";
        }
    }

    // ========================================================
    // STAGE 1 - BFS
    // ========================================================

    void bfsConnectivity() {

        header(
            "STAGE 1 - BFS CONNECTIVITY ANALYSIS"
        );

        for (auto& node : nodes)
            node.reachable = false;

        vector<vector<int>> graph(n);

        for (const auto& e : edges) {

            if (e.active && !e.failed) {

                graph[e.u].push_back(e.v);
                graph[e.v].push_back(e.u);
            }
        }

        queue<int> q;

        nodes[plantID].reachable = true;

        q.push(plantID);

        while (!q.empty()) {

            int u = q.front();
            q.pop();

            for (int v : graph[u]) {

                if (!nodes[v].reachable) {

                    nodes[v].reachable = true;

                    q.push(v);
                }
            }
        }

        int isolated = 0;
        double lostDemand = 0.0;

        cout << "\n";

        for (const auto& node : nodes) {

            if (
                node.type !=
                NodeType::LOAD_CENTER
            ) {
                continue;
            }

            cout << left
                 << setw(28)
                 << node.name;

            if (node.reachable) {

                cout
                    << "[CONNECTED]\n";
            }
            else {

                cout
                    << "[ISOLATED]\n";

                isolated++;

                lostDemand += node.demand;
            }
        }

        cout << "\n";

        cout
            << "Disconnected Loads : "
            << isolated
            << "\n";

        cout
            << "Lost Demand        : "
            << fixed
            << setprecision(1)
            << lostDemand
            << " MW\n";
    }

    // ========================================================
    // STAGE 2 - KRUSKAL
    // ========================================================

    void kruskalRestoration() {

        header(
            "STAGE 2 - KRUSKAL + DSU RESTORATION"
        );

        DSU dsu(n);

        for (const auto& e : edges) {

            if (e.active && !e.failed) {

                dsu.unite(
                    e.u,
                    e.v
                );
            }
        }

        vector<int> candidates;

        for (
            int i = 0;
            i < (int)edges.size();
            i++
        ) {

            if (
                edges[i].failed ||
                edges[i].backup
            ) {
                candidates.push_back(i);
            }
        }

        sort(
            candidates.begin(),
            candidates.end(),
            [&](int a, int b) {
                return edges[a].cost <
                       edges[b].cost;
            }
        );

        double totalCost = 0.0;
        int restored = 0;

        cout << "\nCandidate lines:\n\n";

        for (int id : candidates) {

            cout
                << "Line "
                << id
                << ": "
                << nodes[edges[id].u].name
                << " <--> "
                << nodes[edges[id].v].name
                << " | Cost = $"
                << edges[id].cost
                << "k\n";
        }

        cout << "\nRestoration process:\n\n";

        for (int id : candidates) {

            Edge& e = edges[id];

            if (dsu.unite(e.u, e.v)) {

                e.active = true;
                e.failed = false;

                totalCost += e.cost;
                restored++;

                cout
                    << "[ACTIVATED] "
                    << nodes[e.u].name
                    << " <--> "
                    << nodes[e.v].name
                    << "\n";
            }
        }

        cout << "\n";

        cout
            << "Lines Activated : "
            << restored
            << "\n";

        cout
            << "Total Cost      : $"
            << fixed
            << setprecision(1)
            << totalCost
            << "k\n";
    }

    // ========================================================
    // FLOW RESULT
    // ========================================================

    struct FlowResult {

        double totalFlow;

        vector<double> delivered;
    };

    // ========================================================
    // CALCULATE FLOW
    // ========================================================

    FlowResult calculateFlow(
        const vector<int>& selectedLoads
    ) {

        int source = n;
        int sink = n + 1;

        Dinic dinic(n + 2);

        dinic.addEdge(
            source,
            plantID,
            nodes[plantID].supply
        );

        for (const auto& e : edges) {

            if (e.active && !e.failed) {

                dinic.addEdge(
                    e.u,
                    e.v,
                    e.capacity
                );

                dinic.addEdge(
                    e.v,
                    e.u,
                    e.capacity
                );
            }
        }

        vector<int> sinkEdge(n, -1);

        for (int id : selectedLoads) {

            sinkEdge[id] =
                dinic.addEdge(
                    id,
                    sink,
                    nodes[id].demand
                );
        }

        double flow =
            dinic.maxFlow(
                source,
                sink
            );

        vector<double> delivered(
            n,
            0.0
        );

        for (int id : selectedLoads) {

            if (sinkEdge[id] != -1) {

                delivered[id] =
                    max(
                        0.0,
                        dinic.getFlow(
                            id,
                            sinkEdge[id]
                        )
                    );
            }
        }

        return {
            flow,
            delivered
        };
    }

    // ========================================================
    // STAGE 3 + 4
    // ========================================================

    void maxFlowGreedy() {

        header(
            "STAGE 3 & 4 - DINIC MAX-FLOW + GREEDY"
        );

        for (auto& node : nodes)
            node.received = 0.0;

        vector<int> loads;

        for (const auto& node : nodes) {

            if (
                node.type ==
                NodeType::LOAD_CENTER
            ) {
                loads.push_back(node.id);
            }
        }

        sort(
            loads.begin(),
            loads.end(),
            [&](int a, int b) {

                if (
                    nodes[a].priority !=
                    nodes[b].priority
                ) {
                    return
                        (int)nodes[a].priority <
                        (int)nodes[b].priority;
                }

                return
                    nodes[a].demand >
                    nodes[b].demand;
            }
        );

        cout
            << "\nGreedy restoration order:\n\n";

        for (
            int i = 0;
            i < (int)loads.size();
            i++
        ) {

            int id = loads[i];

            cout
                << i + 1
                << ". "
                << nodes[id].name
                << " | "
                << priorityToString(
                    nodes[id].priority
                )
                << " | "
                << nodes[id].demand
                << " MW\n";
        }

        vector<int> accepted;

        cout << "\n";

        for (int target : loads) {

            accepted.push_back(target);

            FlowResult result =
                calculateFlow(accepted);

            double totalDemand = 0.0;

            for (int id : accepted)
                totalDemand += nodes[id].demand;

            cout
                << "Testing: "
                << nodes[target].name
                << "\n";

            cout
                << "Required: "
                << totalDemand
                << " MW\n";

            cout
                << "Available: "
                << result.totalFlow
                << " MW\n";

            if (
                result.totalFlow >=
                totalDemand - 1e-6
            ) {

                cout
                    << "RESULT: FULLY RESTORABLE\n\n";
            }
            else {

                accepted.pop_back();

                cout
                    << "RESULT: LOAD SHED\n\n";
            }
        }

        if (!accepted.empty()) {

            FlowResult finalResult =
                calculateFlow(accepted);

            for (int id : accepted) {

                nodes[id].received =
                    min(
                        nodes[id].demand,
                        finalResult.delivered[id]
                    );
            }
        }
    }

    // ========================================================
    // FINAL REPORT
    // ========================================================

    void finalReport() {

        header(
            "FINAL POWER RESTORATION REPORT"
        );

        double totalDemand = 0.0;
        double totalReceived = 0.0;

        int criticalTotal = 0;
        int criticalRestored = 0;

        cout << left
             << setw(5) << "ID"
             << setw(28) << "LOAD"
             << setw(15) << "PRIORITY"
             << setw(12) << "DEMAND"
             << setw(12) << "RECEIVED"
             << "STATUS\n";

        cout
            << string(100, '-')
            << "\n";

        for (const auto& node : nodes) {

            if (
                node.type !=
                NodeType::LOAD_CENTER
            ) {
                continue;
            }

            totalDemand += node.demand;
            totalReceived += node.received;

            string status;

            if (
                node.received >=
                node.demand - 1e-6
            ) {
                status = "FULL";
            }
            else if (node.received > 1e-6) {
                status = "PARTIAL";
            }
            else {
                status = "SHED";
            }

            if (
                node.priority ==
                LoadPriority::CRITICAL_HIGH
            ) {

                criticalTotal++;

                if (
                    node.received >=
                    node.demand - 1e-6
                ) {
                    criticalRestored++;
                }
            }

            cout << left
                 << setw(5)
                 << node.id

                 << setw(28)
                 << node.name

                 << setw(15)
                 << priorityToString(node.priority)

                 << setw(12)
                 << fixed
                 << setprecision(1)
                 << node.demand

                 << setw(12)
                 << node.received

                 << status
                 << "\n";
        }

        double percentage = 0.0;

        if (totalDemand > 0.0) {
            percentage =
                totalReceived /
                totalDemand *
                100.0;
        }

        cout
            << string(100, '-')
            << "\n";

        cout
            << "Total Demand     : "
            << totalDemand
            << " MW\n";

        cout
            << "Total Delivered  : "
            << totalReceived
            << " MW\n";

        cout
            << "Delivery Rate    : "
            << percentage
            << "%\n";

        cout
            << "Critical Loads   : "
            << criticalRestored
            << " / "
            << criticalTotal
            << "\n";
    }

    // ========================================================
    // FAIL LINE
    // ========================================================

    void failLine() {

        showEdges();

        int id =
            getInt(
                "\nEnter line ID to fail: ",
                0,
                static_cast<int>(edges.size()) - 1
            );

        Edge& e = edges[id];

        if (e.failed) {

            cout
                << "\nThis line is already failed.\n";

            return;
        }

        e.failed = true;
        e.active = false;

        cout
            << "\nFAILURE CREATED\n";

        cout
            << nodes[e.u].name
            << " <--> "
            << nodes[e.v].name
            << " is now FAILED.\n";
    }

    // ========================================================
    // REPAIR LINE
    // ========================================================

    void repairLine() {

        showEdges();

        int id =
            getInt(
                "\nEnter line ID to activate: ",
                0,
                static_cast<int>(edges.size()) - 1
            );

        Edge& e = edges[id];

        if (e.active) {

            cout
                << "\nThis line is already active.\n";

            return;
        }

        e.failed = false;
        e.active = true;

        cout
            << "\nLINE ACTIVATED\n";

        cout
            << nodes[e.u].name
            << " <--> "
            << nodes[e.v].name
            << "\n";
    }

    // ========================================================
    // CHANGE PLANT CAPACITY
    // ========================================================

    void changePlantCapacity() {

        cout
            << "\nCurrent capacity: "
            << nodes[plantID].supply
            << " MW\n";

        double capacity =
            getDouble(
                "New capacity: ",
                0.0,
                10000.0
            );

        nodes[plantID].supply =
            capacity;

        cout
            << "\nPlant capacity changed to "
            << capacity
            << " MW.\n";
    }

    // ========================================================
    // RESET
    // ========================================================

    void reset() {

        for (auto& e : edges) {

            if (e.backup) {

                e.active = false;
                e.failed = false;
            }
            else {

                e.active = true;
                e.failed = false;
            }
        }

        // Original scenario starts with
        // line 1 failed.

        if (edges.size() > 1) {

            edges[1].active = false;
            edges[1].failed = true;
        }

        for (auto& node : nodes) {

            node.received = 0.0;
            node.reachable = false;
        }

        nodes[plantID].supply = 100.0;

        cout
            << "\nGrid reset successfully.\n";
    }

    // ========================================================
    // COMPLETE SIMULATION
    // ========================================================

    void completeSimulation() {

        cout << "\n";

        cout
            << "############################################################\n";

        cout
            << "#              COMPLETE GRID SIMULATION                    #\n";

        cout
            << "############################################################\n";

        bfsConnectivity();

        kruskalRestoration();

        maxFlowGreedy();

        finalReport();
    }

    // ========================================================
    // MENU
    // ========================================================

    void menu() {

        while (true) {

            cout << "\n";

            cout
                << "============================================================\n";

            cout
                << "              INTELLIGENT POWER GRID SIMULATOR\n";

            cout
                << "============================================================\n";

            cout << "\n";

            cout
                << " 1. View Nodes\n";

            cout
                << " 2. View Transmission Lines\n";

            cout
                << " 3. View Current Topology\n";

            cout
                << " 4. Create Line Failure\n";

            cout
                << " 5. Repair / Activate Line\n";

            cout
                << " 6. BFS Connectivity Analysis\n";

            cout
                << " 7. Kruskal + DSU Restoration\n";

            cout
                << " 8. Dinic Max-Flow + Greedy Restoration\n";

            cout
                << " 9. Run Complete Simulation\n";

            cout
                << "10. View Final Report\n";

            cout
                << "11. Change Plant Capacity\n";

            cout
                << "12. Reset Grid\n";

            cout
                << " 0. Exit\n";

            cout << "\n";

            cout.flush();

            int choice =
                getInt(
                    "Choose an option: ",
                    0,
                    12
                );

            switch (choice) {

                case 1:

                    showNodes();

                    waitForEnter();

                    break;

                case 2:

                    showEdges();

                    waitForEnter();

                    break;

                case 3:

                    showTopology();

                    waitForEnter();

                    break;

                case 4:

                    failLine();

                    waitForEnter();

                    break;

                case 5:

                    repairLine();

                    waitForEnter();

                    break;

                case 6:

                    bfsConnectivity();

                    waitForEnter();

                    break;

                case 7:

                    kruskalRestoration();

                    waitForEnter();

                    break;

                case 8:

                    maxFlowGreedy();

                    waitForEnter();

                    break;

                case 9:

                    completeSimulation();

                    waitForEnter();

                    break;

                case 10:

                    finalReport();

                    waitForEnter();

                    break;

                case 11:

                    changePlantCapacity();

                    waitForEnter();

                    break;

                case 12:

                    reset();

                    waitForEnter();

                    break;

                case 0:

                    cout
                        << "\nExiting simulator...\n";

                    return;
            }
        }
    }
};

// ============================================================
// CREATE DEFAULT GRID
// ============================================================

PowerGrid createGrid() {

    PowerGrid grid(8);

    // ========================================================
    // NODES
    // ========================================================

    grid.addNode(
        0,
        "Central Power Plant",
        NodeType::POWER_PLANT,
        LoadPriority::NONE,
        0,
        100
    );

    grid.addNode(
        1,
        "North Substation",
        NodeType::SUBSTATION,
        LoadPriority::NONE,
        0,
        0
    );

    grid.addNode(
        2,
        "Metro Substation",
        NodeType::SUBSTATION,
        LoadPriority::NONE,
        0,
        0
    );

    grid.addNode(
        3,
        "East Substation",
        NodeType::SUBSTATION,
        LoadPriority::NONE,
        0,
        0
    );

    grid.addNode(
        4,
        "General Hospital",
        NodeType::LOAD_CENTER,
        LoadPriority::CRITICAL_HIGH,
        25,
        0
    );

    grid.addNode(
        5,
        "Municipal Water Works",
        NodeType::LOAD_CENTER,
        LoadPriority::CRITICAL_HIGH,
        20,
        0
    );

    grid.addNode(
        6,
        "Metro Residential",
        NodeType::LOAD_CENTER,
        LoadPriority::MEDIUM,
        35,
        0
    );

    grid.addNode(
        7,
        "Heavy Industrial Park",
        NodeType::LOAD_CENTER,
        LoadPriority::LOW,
        30,
        0
    );

    // ========================================================
    // PRIMARY LINES
    // ========================================================

    grid.addEdge(
        0, 1,
        50,
        10,
        false,
        false
    );

    // Initially failed
    grid.addEdge(
        0, 2,
        60,
        15,
        true,
        false
    );

    grid.addEdge(
        0, 3,
        40,
        12,
        false,
        false
    );

    grid.addEdge(
        1, 4,
        30,
        8,
        false,
        false
    );

    grid.addEdge(
        1, 6,
        25,
        5,
        false,
        false
    );

    grid.addEdge(
        2, 5,
        25,
        7,
        false,
        false
    );

    grid.addEdge(
        2, 7,
        35,
        9,
        false,
        false
    );

    // ========================================================
    // BACKUP LINES
    // ========================================================

    grid.addEdge(
        1, 2,
        45,
        18,
        false,
        true
    );

    grid.addEdge(
        3, 2,
        40,
        14,
        false,
        true
    );

    grid.addEdge(
        3, 7,
        20,
        22,
        false,
        true
    );

    return grid;
}

// ============================================================
// MAIN
// ============================================================

int main() {

    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout
        << "============================================================\n"
        << "     INTELLIGENT POWER GRID FAILURE RECOVERY SYSTEM\n"
        << "============================================================\n";

    cout << "\nAlgorithms:\n";

    cout
        << "  * BFS Graph Traversal\n"
        << "  * DSU / Disjoint Set Union\n"
        << "  * Kruskal Algorithm\n"
        << "  * Dinic Maximum Flow\n"
        << "  * Greedy Load Restoration\n"
        << "  * Priority-Based Load Shedding\n";

    cout << "\nStarting...\n";

    cout.flush();

    PowerGrid grid =
        createGrid();

    grid.menu();

    return 0;
}