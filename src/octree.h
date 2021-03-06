#pragma once

#include "csg.h"
#include "stdlib.h"
#include "circular_queue.h"
#include <mutex>
#include "mesh.h"

namespace oct {

struct OctScene;

struct OctNode {
    static constexpr u32 LEAF_DEPTH = 7;

    glm::vec3 center;
    float radius;
    u32 children[8];
    u32 leaf_id;
    u32 depth;
    bool hasChildren;

    OctNode(const glm::vec3& c=glm::vec3(0.0f), float _radius=16.0f, u32 d=0)
        : center(c),
        radius(_radius),
        leaf_id(-1),
        depth(d),
        hasChildren(false)
    {}
        
    inline float qlen(){ return 1.732051f * radius; }
    inline bool isLeaf(){return depth == LEAF_DEPTH;}
    inline bool isRoot(){return !depth;}
    inline void makeChildren(OctScene& scene);
    inline void insert(const CSG& item, u32 csg_id, int thread_id, OctScene& scene);
};

struct leafData {
    static constexpr u32 capacity = 1 << (3 * 6);
    static constexpr u32 max_threads = 4;
    
    struct Leaf {
        CSGIndices indices;
        VertexBuffer vb;
        const OctNode* node;
        Mesh mesh;

        inline void remesh(const CSGSet& set){
            fillCells(vb, set, indices, node->center, node->radius);
        }
        // gl thread only
        inline void update(){
            mesh.update(vb);
        }
		// gl thread only
        inline void draw(){
            mesh.draw();
        }
    };

    Array<Leaf, capacity> leaves;
    CircularQueue<u32, capacity> n_remesh, n_update;

    inline void remesh(const CSGSet& set, int thread_id){
        while(n_remesh.empty() == false){
            u32 i = n_remesh.pop();
            leaves[i].remesh(set);

            while(n_update.full()){};
            n_update.push(i);
        }
    }
    // gl thread only
    inline void update(){
        while(n_update.empty() == false){
            u32 i = n_update.pop();
            leaves[i].update();
        }
    }
    // gl thread only
    inline void draw(){
        for(auto& i : leaves){
            i.draw();
        }
    }

    inline u32 append(const OctNode* node){
        if(leaves.full()){
            puts("Ran out of capacity in leafData::append()");
            return 0;
        }
        leaves.grow().node = node;
        return leaves.count() - 1;
    }

    inline void insert(u32 i, u32 csg, int thread_id){
        Leaf& leaf = leaves[i];
        if(leaf.indices.push_back(csg)){
            while(n_remesh.full()){};
            n_remesh.push(i);
        }
    }
};

struct OctScene {
    static constexpr u32 max_octnodes = 1 << (3 * 6);

    leafData m_leafData;
    Array<OctNode, max_octnodes> m_octNodes;
    CSGSet csgs;
    std::mutex m_octNodes_mut;

    OctScene(){
        m_octNodes.grow() = OctNode();
    }
    inline void insert(const CSG& csg, int thread_id){
		m_octNodes[0].insert(csg, 0, thread_id, *this);
    }
    inline void remesh(int thread_id){
        m_leafData.remesh(csgs, thread_id);
    }
    // main thread only
    inline void update(){
        m_leafData.update();
    }
    // main thread only
    inline void draw(){
        m_leafData.draw();
    }
    inline bool hasWork(){
        return m_leafData.n_remesh.empty() == false;
    }
};

void OctNode::makeChildren(OctScene& scene){
    std::lock_guard<std::mutex> lock(scene.m_octNodes_mut);
    const float nlen = radius * 0.5f;
    for(int i = 0; i < 8; i++){
        if(scene.m_octNodes.full()){
            puts("Ran out of octnodes in makeChildren");
            return;
        }

        glm::vec3 n_c(center);
        n_c.x += (i&4) ? nlen : -nlen;
        n_c.y += (i&2) ? nlen : -nlen;
        n_c.z += (i&1) ? nlen : -nlen;

        children[i] = scene.m_octNodes.count();
        scene.m_octNodes.grow();
        OctNode& child = scene.m_octNodes[children[i]];
        child = OctNode(n_c, nlen, depth + 1);

        if(child.isLeaf()){
            child.leaf_id = scene.m_leafData.append(&child);
        }
    }
    hasChildren = true;
}

void OctNode::insert(const CSG& item, u32 csg_id, int thread_id, OctScene& scene){
    if(item.func(center) >= qlen() + item.param.smoothness){
        return;
    }
    if(isLeaf()){
        scene.m_leafData.insert(leaf_id, csg_id, thread_id);
        return;
    }
    if(isRoot()){
        csg_id = scene.csgs.count();
        scene.csgs.grow() = item;
    }
    if(!hasChildren){
        makeChildren(scene);
    }
    for(int i = 0; i < 8; i++){
        scene.m_octNodes[children[i]].insert(item, csg_id, thread_id, scene);
    }
}

};