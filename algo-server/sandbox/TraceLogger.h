#ifndef TRACELOGGER_H
#define TRACELOGGER_H

#include <vector>
#include <string>
#include <functional>

class TraceLogger {
private:
    std::vector<std::string> frames;
    std::string _family = "UNKNOWN";

    // ── JSON 빌더 헬퍼 (내부 전용) ──────────────────────────
    // 문자열 값  : "key":"value"
    std::string _s(const std::string& key, const std::string& val) const;
    // 정수 값    : "key":123
    std::string _n(const std::string& key, int val) const;
    // 실수 값    : "key":1.500000
    std::string _d(const std::string& key, double val) const;
    // bool 값   : "key":true / false
    std::string _b(const std::string& key, bool val) const;
    // vector<int> → "[1,2,3]"
    std::string toJsonArray(const std::vector<int>& v) const;
    // vector<int> → "\"key\":[1,2,3]"
    std::string _a(const std::string& key, const std::vector<int>& v) const;

    // ── 내부 재귀 헬퍼 ──────────────────────────────────────
    void _quickSort   (const std::string& id, std::vector<int>& arr, int lo, int hi);
    int  _qPartition  (const std::string& id, std::vector<int>& arr, int lo, int hi);
    void _mergeSort   (const std::string& id, std::vector<int>& arr, int lo, int hi);
    void _merge       (const std::string& id, std::vector<int>& arr, int lo, int mid, int hi);
    void _quickSelect (const std::string& id, std::vector<int>& arr, int lo, int hi, int k);

public:
    TraceLogger() = default;

    // ============================================
    // 1. 1차원 배열 (Array)
    // ============================================
    void setArray        (const std::string& id, const std::vector<int>& arr);
    void swapArray       (const std::string& id, int i, int j);
    void updateArrayValue(const std::string& id, int index, int value);
    void pushBack        (const std::string& id, int value);
    void popBack         (const std::string& id);
    void insertArray     (const std::string& id, int index, int value);
    void eraseArray      (const std::string& id, int index);
    void colorArray      (const std::string& id, const std::vector<int>& indices, const std::string& color);
    void rangeColorArray (const std::string& id, int from, int to, const std::string& color);
    void resetColorArray (const std::string& id);

    // ============================================
    // 2. 2차원 격자 (Grid)
    // ============================================
    void createGrid    (const std::string& id, int rows, int cols, int defaultVal = 0);
    void setGridValue  (const std::string& id, int row, int col, int value);
    void colorGrid     (const std::string& id, int row, int col, const std::string& color);
    void rangeColorGrid(const std::string& id, int r1, int c1, int r2, int c2, const std::string& color);
    void resetColorGrid(const std::string& id);

    // ============================================
    // 3. 스택 (Stack)
    // ============================================
    void createStack(const std::string& id);
    void pushStack  (const std::string& id, int value);
    void popStack   (const std::string& id);
    void colorStack (const std::string& id, int index, const std::string& color);

    // ============================================
    // 4. 큐 (Queue)
    // ============================================
    void createQueue(const std::string& id);
    void pushQueue  (const std::string& id, int value);
    void popQueue   (const std::string& id);
    void colorQueue (const std::string& id, int index, const std::string& color);

    // ============================================
    // 5. 덱 (Deque)
    // ============================================
    void createDeque   (const std::string& id);
    void pushFrontDeque(const std::string& id, int value);
    void popFrontDeque (const std::string& id);
    void pushBackDeque (const std::string& id, int value);
    void popBackDeque  (const std::string& id);
    void colorDeque    (const std::string& id, int index, const std::string& color);

    // ============================================
    // 6. 우선순위 큐 (Priority Queue)
    // ============================================
    void createPQ(const std::string& id);
    void pushPQ  (const std::string& id, int value);
    void popPQ   (const std::string& id);
    void colorPQ (const std::string& id, int index, const std::string& color);

    // ============================================
    // 7. 단일 변수 패널 (Variable)
    // ============================================
    void setVariable      (const std::string& name, int value);
    void setVariableStr   (const std::string& name, const std::string& value);
    void updateVariable   (const std::string& name, int value);
    void updateVariableStr(const std::string& name, const std::string& value);
    void colorVariable    (const std::string& name, const std::string& color);

    // ============================================
    // 8. Union-Find
    // ============================================
    void createUF    (const std::string& id, int n);
    void ufUnion     (const std::string& id, int a, int b);
    void ufFind      (const std::string& id, int a, int root);
    void ufColorGroup(const std::string& id, int a, const std::string& color);

    // ============================================
    // 9. 맵 (Map)
    // ============================================
    void createMap      (const std::string& id);
    void setMapValue    (const std::string& id, int key, int value);
    void setMapValueStr (const std::string& id, const std::string& key, int value);
    void eraseMap       (const std::string& id, int key);
    void eraseMapStr    (const std::string& id, const std::string& key);
    void colorMapKey    (const std::string& id, int key, const std::string& color);
    void colorMapKeyStr (const std::string& id, const std::string& key, const std::string& color);

    // ============================================
    // 10. 셋 (Set)
    // ============================================
    void createSet(const std::string& id);
    void insertSet(const std::string& id, int value);
    void eraseSet (const std::string& id, int value);
    void colorSet (const std::string& id, int value, const std::string& color);

    // ============================================
    // 11. 그래프 / 트리
    // ============================================
    void createNode     (int id, const std::string& valueText);
    void updateNodeValue(int id, const std::string& valueText);
    void colorNode      (int id, const std::string& color);
    void setNodePos     (int id, double x, double y);
    void createEdge     (int u, int v, int weight = 0, bool directed = false);
    void removeEdge     (int u, int v);
    void colorEdge      (int u, int v, const std::string& color);
    void updateEdgeWeight(int u, int v, int weight);
    void updateEdgeText (int u, int v, const std::string& text);
    void updateEdgeStyle(int u, int v, const std::string& style);
    void highlightPath  (const std::vector<int>& nodeIds, const std::string& color);

    // ============================================
    // 12. 포인터 (Floating Pointer)
    // ============================================
    void setPointer   (const std::string& name, const std::string& targetId, int targetIndex);
    void removePointer(const std::string& name);
    void colorPointer (const std::string& name, const std::string& color);

    // ============================================
    // 13. 문자열 (String)
    // ============================================
    void setString      (const std::string& id, const std::string& str);
    void colorChar      (const std::string& id, int index, const std::string& color);
    void rangeColorChar (const std::string& id, int from, int to, const std::string& color);
    void updateChar     (const std::string& id, int index, char ch);

    // ============================================
    // 14. 비트셋 (Bitset)
    // ============================================
    void createBitset  (const std::string& id, int n);
    void setBit        (const std::string& id, int index, int bitValue);
    void colorBit      (const std::string& id, int index, const std::string& color);
    void rangeColorBit (const std::string& id, int from, int to, const std::string& color);

    // ============================================
    // LogText
    // ============================================
    void log(const std::string& message);

    // ============================================
    // Algorithm family metadata
    // ============================================
    void setAlgoFamily(const std::string& family);

    // ============================================
    // 16. STL Wrapper 1
    // ============================================
    void sortArr      (const std::string& id, std::vector<int>& arr);
    void reverseArr   (const std::string& id, std::vector<int>& arr);
    void fillArr      (const std::string& id, std::vector<int>& arr, int value);
    void copyArr      (const std::string& srcId, const std::vector<int>& src,
                       const std::string& dstId, std::vector<int>& dst);
    int  minVal       (const std::string& id, std::vector<int>& arr, int idxA, int idxB);
    int  maxVal       (const std::string& id, std::vector<int>& arr, int idxA, int idxB);
    int  absVal       (const std::string& varName, int value);
    int  accumulateArr(const std::string& id, std::vector<int>& arr, int init = 0);
    int  countVal     (const std::string& id, std::vector<int>& arr, int target);
    int  findVal      (const std::string& id, std::vector<int>& arr, int target);

    // ============================================
    // 17. STL Wrapper 2
    // ============================================
    int  lowerBound      (const std::string& id, std::vector<int>& arr, int target);
    int  upperBound      (const std::string& id, std::vector<int>& arr, int target);
    bool binarySearch    (const std::string& id, std::vector<int>& arr, int target);
    bool nextPermutation (const std::string& id, std::vector<int>& arr);
    bool prevPermutation (const std::string& id, std::vector<int>& arr);
    int  uniqueArr       (const std::string& id, std::vector<int>& arr);
    int  minElement      (const std::string& id, std::vector<int>& arr);
    int  maxElement      (const std::string& id, std::vector<int>& arr);
    void rotateArr       (const std::string& id, std::vector<int>& arr, int pivot);

    // ============================================
    // 18. STL Wrapper
    // ============================================
    void stableSort    (const std::string& id, std::vector<int>& arr);
    void partialSort   (const std::string& id, std::vector<int>& arr, int k);
    void nthElement    (const std::string& id, std::vector<int>& arr, int n);
    int  partitionArr  (const std::string& id, std::vector<int>& arr,
                        std::function<bool(int)> pred);
    int  stablePartition(const std::string& id, std::vector<int>& arr,
                         std::function<bool(int)> pred);
    bool isSorted      (const std::string& id, std::vector<int>& arr);
    int  adjacentFind  (const std::string& id, std::vector<int>& arr);
    void inplaceMerge  (const std::string& id, std::vector<int>& arr, int mid);
    void setUnion      (const std::string& aId, const std::vector<int>& a,
                        const std::string& bId, const std::vector<int>& b,
                        const std::string& dstId, std::vector<int>& dst);
    void setIntersection(const std::string& aId, const std::vector<int>& a,
                         const std::string& bId, const std::vector<int>& b,
                         const std::string& dstId, std::vector<int>& dst);
    void setDifference (const std::string& aId, const std::vector<int>& a,
                        const std::string& bId, const std::vector<int>& b,
                        const std::string& dstId, std::vector<int>& dst);

    // ============================================
    // 19. STL Wrapper 3
    // ============================================
    int  gcd      (const std::string& varName, int a, int b);
    int  lcm      (const std::string& varName, int a, int b);
    int  popcount (const std::string& id, int x);
    int  clz      (const std::string& id, int x);

    // ============================================
    // 20. 단순 연결리스트 (SLL)
    // ============================================
    void createSLL     (const std::string& id);
    void sllCreateNode (const std::string& id, int nodeId, int value);
    void sllDeleteNode (const std::string& id, int nodeId);
    void sllSetNext    (const std::string& id, int nodeId, int nextId);  // nextId=-1: nullptr
    void sllColorNode  (const std::string& id, int nodeId, const std::string& color);
    void sllUpdateValue(const std::string& id, int nodeId, int value);

    // ============================================
    // 21. 이중 연결리스트 (DLL)
    // ============================================
    void createDLL     (const std::string& id);
    void dllCreateNode (const std::string& id, int nodeId, int value);
    void dllDeleteNode (const std::string& id, int nodeId);
    void dllSetNext    (const std::string& id, int nodeId, int nextId);
    void dllSetPrev    (const std::string& id, int nodeId, int prevId);
    void dllColorNode  (const std::string& id, int nodeId, const std::string& color);
    void dllUpdateValue(const std::string& id, int nodeId, int value);

    // ============================================
    // 22. 이진 탐색 트리 (BST)
    // ============================================
    void createBST     (const std::string& id);
    void bstCreateNode (const std::string& id, int nodeId, int value);
    void bstDeleteNode (const std::string& id, int nodeId);
    void bstSetLeft    (const std::string& id, int nodeId, int leftId);   // leftId=-1: nullptr
    void bstSetRight   (const std::string& id, int nodeId, int rightId);  // rightId=-1: nullptr
    void bstColorNode  (const std::string& id, int nodeId, const std::string& color);
    void bstUpdateValue(const std::string& id, int nodeId, int value);

    // ============================================
    // save
    // ============================================
    void save(const std::string& filename = "trace.json");
};

#endif // TRACELOGGER_H
