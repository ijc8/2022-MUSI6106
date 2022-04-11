#include <algorithm>

#include "Vector.h"
#include "Util.h"

#include "Dtw.h"

static const int DIRECTIONS[][2] = {
    {-1, -1},
    {-1,  0},
    { 0, -1},
};

CDtw::CDtw() : m_bIsInitialized(false) {
    reset();
}

CDtw::~CDtw() {
    reset();
}

Error_t CDtw::init(int iNumRows, int iNumCols) {
    if (iNumRows <= 0 || iNumCols <= 0) {
        return Error_t::kFunctionInvalidArgsError;
    }
    rows = iNumRows;
    cols = iNumCols;

    costMatrix = new float[rows * cols];
    parentMatrix = new int[rows * cols];

    memset(costMatrix, 0, sizeof(float) * rows * cols);
    memset(parentMatrix, 0, sizeof(int) * rows * cols);

    // Initialize parent matrix.
    for (int col = 0; col < cols; col++) parentMatrix[idx(0, col)] = 2; // (0, -1)
    for (int row = 0; row < rows; row++) parentMatrix[idx(row, 0)] = 1; // (-1, 0)
    parentMatrix[0] = 0; // (-1, -1)

    m_bIsInitialized = true;

    return Error_t::kNoError;
}

Error_t CDtw::reset() {
    m_bIsInitialized = false;
    if (costMatrix) {
        delete[] costMatrix;
        costMatrix = nullptr;
    }
    if (parentMatrix) {
        delete[] parentMatrix;
        parentMatrix = nullptr;
    }
    path.clear();
    return Error_t::kNoError;
}

Error_t CDtw::process(float **ppfDistanceMatrix) {
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    if (!ppfDistanceMatrix)
        return Error_t::kFunctionInvalidArgsError;

    // Initialize cost matrix.
    float acc = 0;
    for (int col = 0; col < cols; col++) {
        acc += ppfDistanceMatrix[0][col];
        costMatrix[idx(0, col)] = acc;
    }

    acc = 0;
    for (int row = 0; row < rows; row++) {
        acc += ppfDistanceMatrix[row][0];
        costMatrix[idx(row, 0)] = acc;
    }

    for (int row = 1; row < rows; row++) {
        for (int col = 1; col < cols; col++) {
            std::array<float, 3> costs = {
                costMatrix[idx(row - 1, col - 1)],
                costMatrix[idx(row - 1, col)],
                costMatrix[idx(row, col - 1)],
            };
            int argmin = std::min_element(costs.begin(), costs.end()) - costs.begin();
            parentMatrix[idx(row, col)] = argmin;
            int prevRow = row + DIRECTIONS[argmin][0];
            int prevCol = col + DIRECTIONS[argmin][1];
            costMatrix[idx(row, col)] = ppfDistanceMatrix[row][col] + costMatrix[idx(prevRow, prevCol)];
        }
    }


    // Start with the last element.
    int row = rows - 1;
    int col = cols - 1;

    path.clear();
    while (row >= 0 || col >= 0) {
        path.push_back({row, col});
        int dir = parentMatrix[idx(row, col)];
        row += DIRECTIONS[dir][0];
        col += DIRECTIONS[dir][1];
    }
    // We built the path up backwards, so let's fix the order.
    std::reverse(path.begin(), path.end());

    return Error_t::kNoError;
}

inline int CDtw::idx(int row, int col) const {
    return row*cols + col;
}

int CDtw::getPathLength() {
    return path.size();
}

float CDtw::getPathCost() const {
    return costMatrix[idx(rows - 1, cols - 1)];
}

Error_t CDtw::getPath(int **ppiPathResult) const {
    for (int i = 0; i < path.size(); i++) {
        ppiPathResult[0][i] = path[i][0];
        ppiPathResult[1][i] = path[i][1];
    }
    return Error_t::kNoError;
}
