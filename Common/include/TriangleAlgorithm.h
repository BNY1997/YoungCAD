#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

#include <gp_Pnt.hxx>

using namespace std;

inline bool TriangleAlgorithmPointEqual2D(const gp_Pnt& lhs, const gp_Pnt& rhs) {
    return lhs.X() == rhs.X() && lhs.Y() == rhs.Y();
}

inline bool TriangleAlgorithmPointLess2D(const gp_Pnt& lhs, const gp_Pnt& rhs) {
    if (lhs.X() != rhs.X()) return lhs.X() < rhs.X();
    return lhs.Y() < rhs.Y();
}

struct Edge {
    gp_Pnt p1, p2;

    Edge(const gp_Pnt& p1 = gp_Pnt(), const gp_Pnt& p2 = gp_Pnt()) : p1(p1), p2(p2) {
        if (TriangleAlgorithmPointLess2D(this->p2, this->p1)) {
            std::swap(this->p1, this->p2);
        }
    }

    bool operator==(const Edge& other) const {
        return TriangleAlgorithmPointEqual2D(p1, other.p1) && TriangleAlgorithmPointEqual2D(p2, other.p2);
    }

    bool operator<(const Edge& other) const {
        if (TriangleAlgorithmPointLess2D(p1, other.p1)) return true;
        if (TriangleAlgorithmPointLess2D(other.p1, p1)) return false;
        return TriangleAlgorithmPointLess2D(p2, other.p2);
    }
};

struct EdgeHash {
    size_t operator()(const Edge& edge) const {
        const size_t h1 = std::hash<double>{}(edge.p1.X());
        const size_t h2 = std::hash<double>{}(edge.p1.Y());
        const size_t h3 = std::hash<double>{}(edge.p2.X());
        const size_t h4 = std::hash<double>{}(edge.p2.Y());
        return (((h1 * 1315423911u) ^ h2) * 1315423911u ^ h3) * 1315423911u ^ h4;
    }
};

struct Triangle {
    gp_Pnt p1, p2, p3;

    Triangle(const gp_Pnt& p1, const gp_Pnt& p2, const gp_Pnt& p3) : p1(p1), p2(p2), p3(p3) {}

    std::array<Edge, 3> getEdges() const {
        return { Edge(p1, p2), Edge(p2, p3), Edge(p3, p1) };
    }

    double orientation() const {
        return (p2.X() - p1.X()) * (p3.Y() - p1.Y()) - (p2.Y() - p1.Y()) * (p3.X() - p1.X());
    }

    bool isPointInCircumcircle(const gp_Pnt& p) const {
        const double ax = p1.X() - p.X();
        const double ay = p1.Y() - p.Y();
        const double bx = p2.X() - p.X();
        const double by = p2.Y() - p.Y();
        const double cx = p3.X() - p.X();
        const double cy = p3.Y() - p.Y();

        const double a2 = ax * ax + ay * ay;
        const double b2 = bx * bx + by * by;
        const double c2 = cx * cx + cy * cy;

        const double det = ax * (by * c2 - cy * b2)
            - ay * (bx * c2 - cx * b2)
            + a2 * (bx * cy - cx * by);

        const double eps = std::numeric_limits<double>::epsilon() * 16.0;
        return orientation() > 0.0 ? det > eps : det < -eps;
    }

    bool containsSuperVertex(const gp_Pnt& super1, const gp_Pnt& super2, const gp_Pnt& super3) const {
        return TriangleAlgorithmPointEqual2D(p1, super1) || TriangleAlgorithmPointEqual2D(p1, super2) || TriangleAlgorithmPointEqual2D(p1, super3) ||
            TriangleAlgorithmPointEqual2D(p2, super1) || TriangleAlgorithmPointEqual2D(p2, super2) || TriangleAlgorithmPointEqual2D(p2, super3) ||
            TriangleAlgorithmPointEqual2D(p3, super1) || TriangleAlgorithmPointEqual2D(p3, super2) || TriangleAlgorithmPointEqual2D(p3, super3);
    }
};

inline void BowyerWatson(const vector<gp_Pnt>& pointListOrigin, vector<Triangle>& triangles) {
    vector<Triangle> triangulation;

    auto pointList = pointListOrigin;

    if (pointList.size() < 3) {
        triangles = std::move(triangulation);
        return;
    }

    sort(pointList.begin(), pointList.end(), [](const gp_Pnt& lhs, const gp_Pnt& rhs) {
        return TriangleAlgorithmPointLess2D(lhs, rhs);
        });
    pointList.erase(unique(pointList.begin(), pointList.end(), [](const gp_Pnt& lhs, const gp_Pnt& rhs) {
        return TriangleAlgorithmPointEqual2D(lhs, rhs);
        }), pointList.end());

    if (pointList.size() < 3) {
        triangles = std::move(triangulation);
        return;
    }

    double minX = pointList.front().X();
    double minY = pointList.front().Y();
    double maxX = pointList.front().X();
    double maxY = pointList.front().Y();

    for (const auto& p : pointList) {
        minX = min(minX, p.X());
        minY = min(minY, p.Y());
        maxX = max(maxX, p.X());
        maxY = max(maxY, p.Y());
    }

    const double dx = maxX - minX;
    const double dy = maxY - minY;
    const double deltaMax = max(dx, dy);

    if (deltaMax <= numeric_limits<double>::epsilon()) {
        triangles = std::move(triangulation);
        return;
    }

    const gp_Pnt super1(minX - 20.0 * deltaMax, minY - deltaMax, 0.0);
    const gp_Pnt super2(maxX + 20.0 * deltaMax, minY - deltaMax, 0.0);
    const gp_Pnt super3((minX + maxX) * 0.5, maxY + 20.0 * deltaMax, 0.0);

    triangulation.reserve(pointList.size() * 2 + 1);
    triangulation.emplace_back(super1, super2, super3);

    for (const auto& p : pointList) {
        vector<Triangle> remainingTriangles;
        remainingTriangles.reserve(triangulation.size());

        unordered_map<Edge, size_t, EdgeHash> edgeCounter;
        edgeCounter.reserve(triangulation.size() * 3);

        for (const auto& tri : triangulation) {
            if (tri.isPointInCircumcircle(p)) {
                for (const auto& edge : tri.getEdges()) {
                    ++edgeCounter[edge];
                }
            }
            else {
                remainingTriangles.push_back(tri);
            }
        }

        size_t boundaryEdgeCount = 0;
        for (const auto& item : edgeCounter) {
            if (item.second == 1) {
                ++boundaryEdgeCount;
            }
        }

        remainingTriangles.reserve(remainingTriangles.size() + boundaryEdgeCount);
        for (const auto& item : edgeCounter) {
            if (item.second == 1) {
                remainingTriangles.emplace_back(item.first.p1, item.first.p2, p);
            }
        }

        triangulation.swap(remainingTriangles);
    }

    triangulation.erase(
        remove_if(triangulation.begin(), triangulation.end(),
            [&](const Triangle& tri) {
                return tri.containsSuperVertex(super1, super2, super3);
            }),
        triangulation.end());

    triangles = std::move(triangulation);
}
