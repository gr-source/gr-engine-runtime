#pragma once

struct BoneWeight {
    BoneWeight();

    template <typename T>
    void insert(int idx, T value);
    
// private:
    float m_weight[4];
    int m_index[4];
};

