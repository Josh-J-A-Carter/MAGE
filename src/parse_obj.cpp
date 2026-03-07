#include "parse.h"

#include "gmath.h"

#include "mesh.h"

#include <fstream>
#include <istream>
#include <sstream>
#include <string>

#include <vector>
#include <unordered_map>
#include <tuple>

#include <cassert>



using FeatureIndex = int;
using VertexIndex = int;

//using FeatureSet = std::tuple<FeatureIndex, FeatureIndex, FeatureIndex>

struct FeatureSet {
    FeatureIndex pos {};
    FeatureIndex tex {};
    FeatureIndex normal {};

    bool operator==(const FeatureSet& other) const {
        return pos == other.pos && tex == other.tex && normal == other.normal;
    }
};

template <>
struct std::hash<FeatureSet> {
    std::size_t operator()(const FeatureSet& k) const {
        using std::hash;

        return ((hash<FeatureIndex>()(k.pos) ^ (hash<FeatureIndex>()(k.tex) << 1)) >> 1) ^ (hash<FeatureIndex>()(k.normal) << 1);
    }
};

constexpr int VERTEX_NUM_ELEMENTS = 8;
struct Vertex { float data[VERTEX_NUM_ELEMENTS]; };

constexpr int FACE_NUM_ELEMENTS = 3;
struct Face { VertexIndex data[FACE_NUM_ELEMENTS]; };


struct OBJParseState {
    // Raw data
    std::vector<Vec3> v_positions {};
    std::vector<Vec2> v_texcoords {};
    std::vector<Vec3> v_normals {};

    // OpenGL translation
    std::vector<Face> gl_faces {};
    std::vector<Vertex> gl_vertices {};
    std::unordered_map<FeatureSet, VertexIndex> gl_vertex_map {};
};

FeatureSet parse_feature_set(std::string raw) {
    std::istringstream iss { raw };

    int num_tokens = 0;
    constexpr int MAX_TOKENS = 3;
    std::string tokens[MAX_TOKENS];
    while (num_tokens < MAX_TOKENS && std::getline(iss, tokens[num_tokens], '/')) {
        num_tokens += 1;
    }

    return { std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2]) };
}

/// <summary>
/// Map a FeatureSet to its corresponding VertexIndex. For each FeatureSet in a given
/// OBJ file, the corresponding vertex is unique. If the entry does not yet exist, it adds it.
/// </summary>
VertexIndex feature_set_to_vert_index(OBJParseState& state, FeatureSet fs) {
    if (state.gl_vertex_map.contains(fs)) return state.gl_vertex_map[fs];

    VertexIndex index = static_cast<int>(state.gl_vertices.size());
    Vec3 p = state.v_positions[fs.pos - 1];
    Vec2 t = state.v_texcoords[fs.tex - 1];
    Vec3 n = state.v_normals[fs.normal - 1];
    state.gl_vertices.push_back({ p.x, p.y, p.z, t.x, t.y, n.x, n.y, n.z });
    state.gl_vertex_map[fs] = index;

    return index;
}


Mesh* parse_obj(std::string filepath) {

    OBJParseState state {};

    // Parse the file
    std::ifstream file { filepath };

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss { line };

        // Get the keyword
        std::string keyword;
        if (std::getline(iss, keyword, ' ')) {

            // Tokenise the arguments
            int num_tokens = 0;
            constexpr int MAX_TOKENS = 4;
            std::string tokens[MAX_TOKENS];
            while (num_tokens < MAX_TOKENS && std::getline(iss, tokens[num_tokens], ' ')) {
                num_tokens += 1;
            }

            // Parse arguments depending on the keyword
            if (keyword == "v") {
                assert(num_tokens == 3 && "Malformed vertex position in OBJ file");

                state.v_positions.push_back({ std::stof(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]) });
            }

            else if (keyword == "vt") {
                assert(num_tokens == 2 && "Malformed texcoord in OBJ file");

                state.v_texcoords.push_back({ std::stof(tokens[0]), std::stof(tokens[1]) });
            }

            else if (keyword == "vn") {
                assert(num_tokens == 3 && "Malformed vertex normal in OBJ file");

                state.v_normals.push_back({ std::stof(tokens[0]), std::stof(tokens[1]), std::stof(tokens[2]) });
            }

            else if (keyword == "f") {
                assert(num_tokens == 3 && "Non triangular surface defined in OBJ file. Only triangles are supported.");

                FeatureSet fs1 { parse_feature_set(tokens[0]) };
                FeatureSet fs2 { parse_feature_set(tokens[1]) };
                FeatureSet fs3 { parse_feature_set(tokens[2]) };
                state.gl_faces.push_back({
                    feature_set_to_vert_index(state, fs1),
                    feature_set_to_vert_index(state, fs2),
                    feature_set_to_vert_index(state, fs3)
                });
            }

            // Ignore all other keywords
            else;
        }
    }

    std::vector<float> vertices {};
    vertices.reserve(state.gl_vertices.size() * VERTEX_NUM_ELEMENTS);
    for (Vertex& v : state.gl_vertices) {
        for (std::size_t i { 0 } ; i < VERTEX_NUM_ELEMENTS ; i += 1) vertices.push_back(v.data[i]);
    }

    std::vector<int> indices {};
    vertices.reserve(state.gl_faces.size() * FACE_NUM_ELEMENTS);
    for (Face& f : state.gl_faces) {
        for (std::size_t i { 0 }; i < FACE_NUM_ELEMENTS; i += 1) indices.push_back(f.data[i]);
    }
    
    return new Mesh(vertices, indices);
}