#include "dbutils.h"
#include "dbmanage3.h"
#include <iostream>
#include <algorithm>
#include <random>

#include "utils.h"

using namespace std;

map<triple<int, int, int>, vector<vector<uint64_t>>> sortedMap;

int compareIdx(const uint64_t &idx1, const uint64_t &idx2) {
    uint8_t *entry1 = db->getFromIdx(idx1);
    uint8_t *entry2 = db->getFromIdx(idx2);

    if (entry1 == 0 || entry2 == 0) {
        cout << "dbutils compareIndex: entry not found" << endl;
        while (1) {}
    }

    if (*db_get_outcome(entry1) == 0 || *db_get_outcome(entry2) == 0) {
        cout << "dbutils compareIndex: entry not processed" << endl;
        while (1) {}
    }

    uint8_t *g1;
    size_t g1Size;

    uint8_t *g2;
    size_t g2Size;

    makeGame(*db_get_shape(entry1), *db_get_number(entry1), &g1, &g1Size);
    makeGame(*db_get_shape(entry2), *db_get_number(entry2), &g2, &g2Size);

    negateBoard(g2, g2Size);

    size_t g3Size;
    uint8_t *g3 = addGames(g1, g1Size, g2, g2Size, &g3Size);

    int bFirst = solver->solveID(g3, g3Size, 1);
    int wFirst = solver->solveID(g3, g3Size, 2);

    delete[] g1;
    delete[] g2;
    delete[] g3;

    if (bFirst == wFirst) {
        return bFirst;
    }
    if (bFirst == 1) {
        return OC_N;
    }
    return OC_P;
}

uint64_t simplifyIdx(const triple<int, int, int> &mapTriple, const uint64_t &idx) {
    const auto &mapIt = sortedMap.find(mapTriple);
    if (mapIt == sortedMap.end()) {
        return idx;
    }

    const vector<vector<uint64_t>> &listList = mapIt->second;

    for (const vector<uint64_t> &vec : listList) {
        size_t size = vec.size();

        int low = 0;
        int high = size - 1;
        while (low <= high) {
            int mid = (low + high) / 2;

            int comparison = compareIdx(idx, vec[mid]);

            if (comparison == OC_P) {
                return vec[mid];
            }
            if (comparison == OC_N) {
                break; //not in this list
            }
            if (comparison == OC_B) {
                low = mid + 1;
                continue;
            }
            if (comparison == OC_W) {
                high = mid - 1;
                continue;
            }

            cout << "dbutils simplify index search fail..." << endl;
            while (1) {}
        }
    }

    return idx;
}

void makeSortedMap() {
    default_random_engine rng(983);
    

    for (auto &rmapIt : replacementMap) {
        const triple<int, int, int> &mapTriple = rmapIt.first;
        vector<uint64_t> &mapVector = *(rmapIt.second);

        shuffle(mapVector.begin(), mapVector.end(), rng);

        vector<vector<uint64_t>> &listList = sortedMap[mapTriple];

        for (uint64_t idx : mapVector) {
            bool found = false;
            //check every vector for location
            
            for (vector<uint64_t> &vec : listList) {
                int low = 0;
                int high = vec.size() - 1;


                int prevLow = low;
                int prevHigh = high;
                int mid = 0;
                int lastCompare = 0;

                while (low <= high) {
                    mid = (low + high) / 2;
                    prevLow = low;
                    prevHigh = high;

                    lastCompare = compareIdx(idx, vec[mid]);

                    // g1 - g2 > 0 --> g1 > g2
                    if (lastCompare == OC_B) {
                        low = mid + 1;
                        continue;
                    }
                    
                    // g1 - g2 < 0 --> g1 < g2
                    if (lastCompare == OC_W) {
                        high = mid - 1;
                        continue;
                    }

                    break;
                }

                if (lastCompare == OC_N) { //not in this vec
                    continue;
                }

                found = true;
                if (lastCompare == OC_P) {
                    uint8_t *entry1 = db->getFromIdx(idx);
                    uint8_t *entry2 = db->getFromIdx(vec[mid]);

                    if (*db_get_metric(entry1) < *db_get_metric(entry2)) {
                        vec[mid] = idx;
                    }
                    break;
                }
                if (lastCompare == OC_W) {
                    vec.insert(vec.begin() + mid, idx);
                    break;
                }
                if (lastCompare == OC_B) {
                    vec.insert(vec.begin() + mid + 1, idx);
                    break;
                }

            }
            if (!found) {
                listList.push_back(vector<uint64_t>({idx}));
            }
        }
    }

}

void checkSorting() {
    for (auto &mapIt : sortedMap) {
        const triple<int, int, int> &mapTriple = mapIt.first;
        const vector<vector<uint64_t>> &listList = mapIt.second;

        cout << "<" << mapTriple.first << " " << mapTriple.second << " " << mapTriple.third << "> (" << listList.size() << "):" << endl;

        for (const vector<uint64_t> &vec : listList) {
            size_t size = vec.size();
            cout << size << endl;

            for (size_t i = 0; i < size - 1; i++) {
                int comparison = compareIdx(vec[i], vec[i + 1]);
                if (comparison != OC_W) {
                    cout << "Ordering not maintained at i = " << i << endl;

                    uint8_t *entry1 = db->getFromIdx(vec[i]);
                    uint8_t *entry2 = db->getFromIdx(vec[i + 1]);

                    uint8_t *g1, *g2;
                    size_t g1Size, g2Size;

                    makeGame(*db_get_shape(entry1), *db_get_number(entry1), &g1, &g1Size);
                    makeGame(*db_get_shape(entry2), *db_get_number(entry2), &g2, &g2Size);

                    printBoard(g1, g1Size);
                    cout << endl;
                    printBoard(g2, g2Size);
                    cout << endl;

                    delete[] g1;
                    delete[] g2;


                    while (1) {}
                }
            }


        }
    }

}
