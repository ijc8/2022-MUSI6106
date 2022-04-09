#include <array>
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
    rows = iNumRows;
    cols = iNumCols;

    costMatrix = new float[rows * cols];
    parentMatrix = new int[rows * cols];

    memset(costMatrix, 0, sizeof(float) * rows * cols);
    memset(parentMatrix, 0, sizeof(int) * rows * cols);

    // Initialize parent matrix.
    for (int col = 0; col < cols; col++) parentMatrix[cols] = 2; // (0, -1)
    for (int row = 0; row < rows; row++) parentMatrix[row*cols] = 1; // (-1, 0)
    parentMatrix[0] = 0; // (-1, -1)

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
        costMatrix[idx(row, 0)] = 0;
    }

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
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

    return Error_t::kNoError;
}

inline int CDtw::idx(int row, int col) const {
    return row*cols + col;
}

int CDtw::getPathLength() {
    // TODO
    return -1;
}

float CDtw::getPathCost() const {
    // TODO
    return -1.F;
}

Error_t CDtw::getPath(int **ppiPathResult) const {
    // TODO: Port this.
    // p = np.asarray(D.shape, dtype=int) - 1  # start with the last element
    // n = p

    // while (n[0] >= 0) or (n[1] >= 0):
    //     n = n + iDec[DeltaP[n[0], n[1]], :]

    //     # update path
    //     tmp = np.vstack([n, p])
    //     p = tmp

    // return p[1:, :], C
    return Error_t::kNoError;
}
