#include "TraceLogger.h"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <sstream>

// ============================================================
// JSON 빌더 헬퍼
// ============================================================

std::string TraceLogger::_s(const std::string& key, const std::string& val) const {
    return "\"" + key + "\":\"" + val + "\"";
}
std::string TraceLogger::_n(const std::string& key, int val) const {
    return "\"" + key + "\":" + std::to_string(val);
}
std::string TraceLogger::_d(const std::string& key, double val) const {
    std::ostringstream oss;
    oss << "\"" << key << "\":" << val;
    return oss.str();
}
std::string TraceLogger::_b(const std::string& key, bool val) const {
    return "\"" + key + "\":" + (val ? "true" : "false");
}
std::string TraceLogger::toJsonArray(const std::vector<int>& v) const {
    std::string r = "[";
    for (int i = 0; i < (int)v.size(); ++i) {
        if (i > 0) r += ",";
        r += std::to_string(v[i]);
    }
    return r + "]";
}
std::string TraceLogger::_a(const std::string& key, const std::vector<int>& v) const {
    return "\"" + key + "\":" + toJsonArray(v);
}

// ============================================================
// 1. 배열
// ============================================================

void TraceLogger::setArray(const std::string& id, const std::vector<int>& arr) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","set") + "," + _a("values",arr) + "}");
}
void TraceLogger::swapArray(const std::string& id, int i, int j) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","swap") + "," + _n("i",i) + "," + _n("j",j) + "}");
}
void TraceLogger::updateArrayValue(const std::string& id, int index, int value) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","update_value") + "," + _n("index",index) + "," + _n("value",value) + "}");
}
void TraceLogger::pushBack(const std::string& id, int value) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","push_back") + "," + _n("value",value) + "}");
}
void TraceLogger::popBack(const std::string& id) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","pop_back") + "}");
}
void TraceLogger::insertArray(const std::string& id, int index, int value) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","insert") + "," + _n("index",index) + "," + _n("value",value) + "}");
}
void TraceLogger::eraseArray(const std::string& id, int index) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","erase") + "," + _n("index",index) + "}");
}
void TraceLogger::colorArray(const std::string& id, const std::vector<int>& indices, const std::string& color) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","color") + "," + _s("color",color) + "," + _a("indices",indices) + "}");
}
void TraceLogger::rangeColorArray(const std::string& id, int from, int to, const std::string& color) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","range_color") + "," + _n("from",from) + "," + _n("to",to) + "," + _s("color",color) + "}");
}
void TraceLogger::resetColorArray(const std::string& id) {
    frames.push_back("{" + _s("target","array") + "," + _s("id",id) + "," + _s("action","reset_color") + "}");
}

// ============================================================
// 2. 그리드
// ============================================================

void TraceLogger::createGrid(const std::string& id, int rows, int cols, int defaultVal) {
    frames.push_back("{" + _s("target","grid") + "," + _s("id",id) + "," + _s("action","create") + "," + _n("rows",rows) + "," + _n("cols",cols) + "," + _n("default_val",defaultVal) + "}");
}
void TraceLogger::setGridValue(const std::string& id, int row, int col, int value) {
    frames.push_back("{" + _s("target","grid") + "," + _s("id",id) + "," + _s("action","set_value") + "," + _n("row",row) + "," + _n("col",col) + "," + _n("value",value) + "}");
}
void TraceLogger::colorGrid(const std::string& id, int row, int col, const std::string& color) {
    frames.push_back("{" + _s("target","grid") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("row",row) + "," + _n("col",col) + "," + _s("color",color) + "}");
}
void TraceLogger::rangeColorGrid(const std::string& id, int r1, int c1, int r2, int c2, const std::string& color) {
    frames.push_back("{" + _s("target","grid") + "," + _s("id",id) + "," + _s("action","range_color") + "," + _n("r1",r1) + "," + _n("c1",c1) + "," + _n("r2",r2) + "," + _n("c2",c2) + "," + _s("color",color) + "}");
}
void TraceLogger::resetColorGrid(const std::string& id) {
    frames.push_back("{" + _s("target","grid") + "," + _s("id",id) + "," + _s("action","reset_color") + "}");
}

// ============================================================
// 3. 스택
// ============================================================

void TraceLogger::createStack(const std::string& id) {
    frames.push_back("{" + _s("target","stack") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::pushStack(const std::string& id, int value) {
    frames.push_back("{" + _s("target","stack") + "," + _s("id",id) + "," + _s("action","push") + "," + _n("value",value) + "}");
}
void TraceLogger::popStack(const std::string& id) {
    frames.push_back("{" + _s("target","stack") + "," + _s("id",id) + "," + _s("action","pop") + "}");
}
void TraceLogger::colorStack(const std::string& id, int index, const std::string& color) {
    frames.push_back("{" + _s("target","stack") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("index",index) + "," + _s("color",color) + "}");
}

// ============================================================
// 4. 큐
// ============================================================

void TraceLogger::createQueue(const std::string& id) {
    frames.push_back("{" + _s("target","queue") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::pushQueue(const std::string& id, int value) {
    frames.push_back("{" + _s("target","queue") + "," + _s("id",id) + "," + _s("action","push") + "," + _n("value",value) + "}");
}
void TraceLogger::popQueue(const std::string& id) {
    frames.push_back("{" + _s("target","queue") + "," + _s("id",id) + "," + _s("action","pop") + "}");
}
void TraceLogger::colorQueue(const std::string& id, int index, const std::string& color) {
    frames.push_back("{" + _s("target","queue") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("index",index) + "," + _s("color",color) + "}");
}

// ============================================================
// 5. 덱
// ============================================================

void TraceLogger::createDeque(const std::string& id) {
    frames.push_back("{" + _s("target","deque") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::pushFrontDeque(const std::string& id, int value) {
    frames.push_back("{" + _s("target","deque") + "," + _s("id",id) + "," + _s("action","push_front") + "," + _n("value",value) + "}");
}
void TraceLogger::popFrontDeque(const std::string& id) {
    frames.push_back("{" + _s("target","deque") + "," + _s("id",id) + "," + _s("action","pop_front") + "}");
}
void TraceLogger::pushBackDeque(const std::string& id, int value) {
    frames.push_back("{" + _s("target","deque") + "," + _s("id",id) + "," + _s("action","push_back") + "," + _n("value",value) + "}");
}
void TraceLogger::popBackDeque(const std::string& id) {
    frames.push_back("{" + _s("target","deque") + "," + _s("id",id) + "," + _s("action","pop_back") + "}");
}
void TraceLogger::colorDeque(const std::string& id, int index, const std::string& color) {
    frames.push_back("{" + _s("target","deque") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("index",index) + "," + _s("color",color) + "}");
}

// ============================================================
// 6. 우선순위 큐
// ============================================================

void TraceLogger::createPQ(const std::string& id) {
    frames.push_back("{" + _s("target","pq") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::pushPQ(const std::string& id, int value) {
    frames.push_back("{" + _s("target","pq") + "," + _s("id",id) + "," + _s("action","push") + "," + _n("value",value) + "}");
}
void TraceLogger::popPQ(const std::string& id) {
    frames.push_back("{" + _s("target","pq") + "," + _s("id",id) + "," + _s("action","pop") + "}");
}
void TraceLogger::colorPQ(const std::string& id, int index, const std::string& color) {
    frames.push_back("{" + _s("target","pq") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("index",index) + "," + _s("color",color) + "}");
}

// ============================================================
// 7. 변수 패널
// ============================================================

void TraceLogger::setVariable(const std::string& name, int value) {
    frames.push_back("{" + _s("target","variable") + "," + _s("action","set") + "," + _s("name",name) + "," + _n("value",value) + "}");
}
void TraceLogger::setVariableStr(const std::string& name, const std::string& value) {
    frames.push_back("{" + _s("target","variable") + "," + _s("action","set") + "," + _s("name",name) + "," + _s("value",value) + "}");
}
void TraceLogger::updateVariable(const std::string& name, int value) {
    frames.push_back("{" + _s("target","variable") + "," + _s("action","update") + "," + _s("name",name) + "," + _n("value",value) + "}");
}
void TraceLogger::updateVariableStr(const std::string& name, const std::string& value) {
    frames.push_back("{" + _s("target","variable") + "," + _s("action","update") + "," + _s("name",name) + "," + _s("value",value) + "}");
}
void TraceLogger::colorVariable(const std::string& name, const std::string& color) {
    frames.push_back("{" + _s("target","variable") + "," + _s("action","color") + "," + _s("name",name) + "," + _s("color",color) + "}");
}

// ============================================================
// 8. Union-Find
// ============================================================

void TraceLogger::createUF(const std::string& id, int n) {
    frames.push_back("{" + _s("target","uf") + "," + _s("id",id) + "," + _s("action","create") + "," + _n("n",n) + "}");
}
void TraceLogger::ufUnion(const std::string& id, int a, int b) {
    frames.push_back("{" + _s("target","uf") + "," + _s("id",id) + "," + _s("action","union") + "," + _n("a",a) + "," + _n("b",b) + "}");
}
void TraceLogger::ufFind(const std::string& id, int a, int root) {
    frames.push_back("{" + _s("target","uf") + "," + _s("id",id) + "," + _s("action","find") + "," + _n("a",a) + "," + _n("root",root) + "}");
}
void TraceLogger::ufColorGroup(const std::string& id, int a, const std::string& color) {
    frames.push_back("{" + _s("target","uf") + "," + _s("id",id) + "," + _s("action","color_group") + "," + _n("a",a) + "," + _s("color",color) + "}");
}

// ============================================================
// 9. 맵
// ============================================================

void TraceLogger::createMap(const std::string& id) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::setMapValue(const std::string& id, int key, int value) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","set") + "," + _n("key",key) + "," + _n("value",value) + "}");
}
void TraceLogger::setMapValueStr(const std::string& id, const std::string& key, int value) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","set") + "," + _s("key",key) + "," + _n("value",value) + "}");
}
void TraceLogger::eraseMap(const std::string& id, int key) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","erase") + "," + _n("key",key) + "}");
}
void TraceLogger::eraseMapStr(const std::string& id, const std::string& key) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","erase") + "," + _s("key",key) + "}");
}
void TraceLogger::colorMapKey(const std::string& id, int key, const std::string& color) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("key",key) + "," + _s("color",color) + "}");
}
void TraceLogger::colorMapKeyStr(const std::string& id, const std::string& key, const std::string& color) {
    frames.push_back("{" + _s("target","map") + "," + _s("id",id) + "," + _s("action","color") + "," + _s("key",key) + "," + _s("color",color) + "}");
}

// ============================================================
// 10. 셋
// ============================================================

void TraceLogger::createSet(const std::string& id) {
    frames.push_back("{" + _s("target","set") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::insertSet(const std::string& id, int value) {
    frames.push_back("{" + _s("target","set") + "," + _s("id",id) + "," + _s("action","insert") + "," + _n("value",value) + "}");
}
void TraceLogger::eraseSet(const std::string& id, int value) {
    frames.push_back("{" + _s("target","set") + "," + _s("id",id) + "," + _s("action","erase") + "," + _n("value",value) + "}");
}
void TraceLogger::colorSet(const std::string& id, int value, const std::string& color) {
    frames.push_back("{" + _s("target","set") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("value",value) + "," + _s("color",color) + "}");
}

// ============================================================
// 11. 그래프 / 트리
// ============================================================

void TraceLogger::createNode(int id, const std::string& valueText) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","create_node") + "," + _n("id",id) + "," + _s("value",valueText) + "}");
}
void TraceLogger::updateNodeValue(int id, const std::string& valueText) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","update_node") + "," + _n("id",id) + "," + _s("value",valueText) + "}");
}
void TraceLogger::colorNode(int id, const std::string& color) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","color_node") + "," + _n("id",id) + "," + _s("color",color) + "}");
}
void TraceLogger::setNodePos(int id, double x, double y) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","set_node_pos") + "," + _n("id",id) + "," + _d("x",x) + "," + _d("y",y) + "}");
}
void TraceLogger::createEdge(int u, int v, int weight, bool directed) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","create_edge") + "," + _n("u",u) + "," + _n("v",v) + "," + _n("weight",weight) + "," + _b("directed",directed) + "}");
}
void TraceLogger::removeEdge(int u, int v) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","remove_edge") + "," + _n("u",u) + "," + _n("v",v) + "}");
}
void TraceLogger::colorEdge(int u, int v, const std::string& color) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","color_edge") + "," + _n("u",u) + "," + _n("v",v) + "," + _s("color",color) + "}");
}
void TraceLogger::updateEdgeWeight(int u, int v, int weight) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","update_edge_weight") + "," + _n("u",u) + "," + _n("v",v) + "," + _n("weight",weight) + "}");
}
void TraceLogger::updateEdgeText(int u, int v, const std::string& text) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","update_edge_text") + "," + _n("u",u) + "," + _n("v",v) + "," + _s("text",text) + "}");
}
void TraceLogger::updateEdgeStyle(int u, int v, const std::string& style) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","update_edge_style") + "," + _n("u",u) + "," + _n("v",v) + "," + _s("style",style) + "}");
}
void TraceLogger::highlightPath(const std::vector<int>& nodeIds, const std::string& color) {
    frames.push_back("{" + _s("target","graph") + "," + _s("action","highlight_path") + "," + _s("color",color) + "," + _a("nodes",nodeIds) + "}");
}

// ============================================================
// 12. 포인터
// ============================================================

void TraceLogger::setPointer(const std::string& name, const std::string& targetId, int targetIndex) {
    frames.push_back("{" + _s("target","pointer") + "," + _s("action","set") + "," + _s("name",name) + "," + _s("target_obj",targetId) + "," + _n("target_id",targetIndex) + "}");
}
void TraceLogger::removePointer(const std::string& name) {
    frames.push_back("{" + _s("target","pointer") + "," + _s("action","remove") + "," + _s("name",name) + "}");
}
void TraceLogger::colorPointer(const std::string& name, const std::string& color) {
    frames.push_back("{" + _s("target","pointer") + "," + _s("action","color") + "," + _s("name",name) + "," + _s("color",color) + "}");
}

// ============================================================
// 13. 문자열
// ============================================================

void TraceLogger::setString(const std::string& id, const std::string& str) {
    frames.push_back("{" + _s("target","string") + "," + _s("id",id) + "," + _s("action","set") + "," + _s("value",str) + "}");
}
void TraceLogger::colorChar(const std::string& id, int index, const std::string& color) {
    frames.push_back("{" + _s("target","string") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("index",index) + "," + _s("color",color) + "}");
}
void TraceLogger::rangeColorChar(const std::string& id, int from, int to, const std::string& color) {
    frames.push_back("{" + _s("target","string") + "," + _s("id",id) + "," + _s("action","range_color") + "," + _n("from",from) + "," + _n("to",to) + "," + _s("color",color) + "}");
}
void TraceLogger::updateChar(const std::string& id, int index, char ch) {
    frames.push_back("{" + _s("target","string") + "," + _s("id",id) + "," + _s("action","update_char") + "," + _n("index",index) + "," + _s("char", std::string(1, ch)) + "}");
}

// ============================================================
// 14. 비트셋
// ============================================================

void TraceLogger::createBitset(const std::string& id, int n) {
    frames.push_back("{" + _s("target","bitset") + "," + _s("id",id) + "," + _s("action","create") + "," + _n("n",n) + "}");
}
void TraceLogger::setBit(const std::string& id, int index, int bitValue) {
    frames.push_back("{" + _s("target","bitset") + "," + _s("id",id) + "," + _s("action","set_bit") + "," + _n("index",index) + "," + _n("bit",bitValue) + "}");
}
void TraceLogger::colorBit(const std::string& id, int index, const std::string& color) {
    frames.push_back("{" + _s("target","bitset") + "," + _s("id",id) + "," + _s("action","color") + "," + _n("index",index) + "," + _s("color",color) + "}");
}
void TraceLogger::rangeColorBit(const std::string& id, int from, int to, const std::string& color) {
    frames.push_back("{" + _s("target","bitset") + "," + _s("id",id) + "," + _s("action","range_color") + "," + _n("from",from) + "," + _n("to",to) + "," + _s("color",color) + "}");
}

// ============================================================
// 15. 로그
// ============================================================

void TraceLogger::log(const std::string& message) {
    frames.push_back("{" + _s("target","log") + "," + _s("action","message") + "," + _s("text",message) + "}");
}

// ============================================================
// 알고리즘 패밀리
// ============================================================

void TraceLogger::setAlgoFamily(const std::string& family) {
    _family = family;
}

// ============================================================
// 내부 헬퍼
// ============================================================

int TraceLogger::_qPartition(const std::string& id, std::vector<int>& arr, int lo, int hi) {
    int pivot = arr[hi], i = lo - 1;
    for (int j = lo; j < hi; j++) {
        colorArray(id, {j, hi}, "red");
        if (arr[j] <= pivot) {
            i++;
            std::swap(arr[i], arr[j]);
            swapArray(id, i, j);
        }
    }
    std::swap(arr[i + 1], arr[hi]);
    swapArray(id, i + 1, hi);
    return i + 1;
}
void TraceLogger::_quickSort(const std::string& id, std::vector<int>& arr, int lo, int hi) {
    if (lo >= hi) return;
    int p = _qPartition(id, arr, lo, hi);
    _quickSort(id, arr, lo, p - 1);
    _quickSort(id, arr, p + 1, hi);
}
void TraceLogger::_merge(const std::string& id, std::vector<int>& arr, int lo, int mid, int hi) {
    std::vector<int> tmp(hi - lo + 1);
    int i = lo, j = mid + 1, k = 0;
    while (i <= mid && j <= hi) {
        colorArray(id, {i, j}, "blue");
        if (arr[i] <= arr[j]) tmp[k++] = arr[i++];
        else                  tmp[k++] = arr[j++];
    }
    while (i <= mid) tmp[k++] = arr[i++];
    while (j <= hi)  tmp[k++] = arr[j++];
    for (int x = 0; x < k; x++) {
        arr[lo + x] = tmp[x];
        updateArrayValue(id, lo + x, tmp[x]);
    }
}
void TraceLogger::_mergeSort(const std::string& id, std::vector<int>& arr, int lo, int hi) {
    if (lo >= hi) return;
    int mid = (lo + hi) / 2;
    _mergeSort(id, arr, lo, mid);
    _mergeSort(id, arr, mid + 1, hi);
    _merge(id, arr, lo, mid, hi);
}
void TraceLogger::_quickSelect(const std::string& id, std::vector<int>& arr, int lo, int hi, int k) {
    if (lo >= hi) return;
    int p = _qPartition(id, arr, lo, hi);
    if (p == k)      return;
    else if (p < k)  _quickSelect(id, arr, p + 1, hi, k);
    else             _quickSelect(id, arr, lo, p - 1, k);
}

// ============================================================
// 16. STL 래퍼 — 브론즈~실버
// ============================================================

void TraceLogger::sortArr(const std::string& id, std::vector<int>& arr) {
    if (arr.empty()) return;
    _quickSort(id, arr, 0, (int)arr.size() - 1);
}

void TraceLogger::reverseArr(const std::string& id, std::vector<int>& arr) {
    int lo = 0, hi = (int)arr.size() - 1;
    while (lo < hi) {
        colorArray(id, {lo, hi}, "red");
        std::swap(arr[lo], arr[hi]);
        swapArray(id, lo, hi);
        lo++; hi--;
    }
}

void TraceLogger::fillArr(const std::string& id, std::vector<int>& arr, int value) {
    for (int i = 0; i < (int)arr.size(); i++) {
        arr[i] = value;
        colorArray(id, {i}, "red");
        updateArrayValue(id, i, value);
    }
}

void TraceLogger::copyArr(const std::string& srcId, const std::vector<int>& src,
                          const std::string& dstId, std::vector<int>& dst) {
    dst.resize(src.size());
    setArray(dstId, dst);
    for (int i = 0; i < (int)src.size(); i++) {
        colorArray(srcId, {i}, "red");
        dst[i] = src[i];
        updateArrayValue(dstId, i, dst[i]);
    }
}

int TraceLogger::minVal(const std::string& id, std::vector<int>& arr, int idxA, int idxB) {
    colorArray(id, {idxA, idxB}, "red");
    return arr[idxA] < arr[idxB] ? arr[idxA] : arr[idxB];
}

int TraceLogger::maxVal(const std::string& id, std::vector<int>& arr, int idxA, int idxB) {
    colorArray(id, {idxA, idxB}, "red");
    return arr[idxA] > arr[idxB] ? arr[idxA] : arr[idxB];
}

int TraceLogger::absVal(const std::string& varName, int value) {
    int result = value < 0 ? -value : value;
    setVariable(varName, result);
    return result;
}

int TraceLogger::accumulateArr(const std::string& id, std::vector<int>& arr, int init) {
    int sum = init;
    setVariable("acc", sum);
    for (int i = 0; i < (int)arr.size(); i++) {
        colorArray(id, {i}, "red");
        sum += arr[i];
        updateVariable("acc", sum);
    }
    return sum;
}

int TraceLogger::countVal(const std::string& id, std::vector<int>& arr, int target) {
    int cnt = 0;
    setVariable("count", cnt);
    for (int i = 0; i < (int)arr.size(); i++) {
        colorArray(id, {i}, "red");
        if (arr[i] == target) {
            cnt++;
            colorArray(id, {i}, "green");
            updateVariable("count", cnt);
        }
    }
    return cnt;
}

int TraceLogger::findVal(const std::string& id, std::vector<int>& arr, int target) {
    setVariable("target", target);
    for (int i = 0; i < (int)arr.size(); i++) {
        colorArray(id, {i}, "red");
        if (arr[i] == target) {
            colorArray(id, {i}, "green");
            setVariable("found_idx", i);
            return i;
        }
    }
    setVariable("found_idx", -1);
    return -1;
}

// ============================================================
// 17. STL 래퍼 — 실버~골드
// ============================================================

int TraceLogger::lowerBound(const std::string& id, std::vector<int>& arr, int target) {
    setVariable("target", target);
    int lo = 0, hi = (int)arr.size();
    setPointer("lo", id, lo);
    setPointer("hi", id, hi - 1);
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        colorArray(id, {mid}, "red");
        setPointer("mid", id, mid);
        if (arr[mid] < target) {
            lo = mid + 1;
            setPointer("lo", id, lo);
        } else {
            hi = mid;
            setPointer("hi", id, hi - 1 < 0 ? 0 : hi - 1);
        }
    }
    setVariable("result", lo);
    removePointer("lo"); removePointer("hi"); removePointer("mid");
    return lo;
}

int TraceLogger::upperBound(const std::string& id, std::vector<int>& arr, int target) {
    setVariable("target", target);
    int lo = 0, hi = (int)arr.size();
    setPointer("lo", id, lo);
    setPointer("hi", id, hi - 1);
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        colorArray(id, {mid}, "red");
        setPointer("mid", id, mid);
        if (arr[mid] <= target) {
            lo = mid + 1;
            setPointer("lo", id, lo);
        } else {
            hi = mid;
            setPointer("hi", id, hi - 1 < 0 ? 0 : hi - 1);
        }
    }
    setVariable("result", lo);
    removePointer("lo"); removePointer("hi"); removePointer("mid");
    return lo;
}

bool TraceLogger::binarySearch(const std::string& id, std::vector<int>& arr, int target) {
    setVariable("target", target);
    int lo = 0, hi = (int)arr.size() - 1;
    setPointer("lo", id, lo);
    setPointer("hi", id, hi);
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        colorArray(id, {mid}, "red");
        setPointer("mid", id, mid);
        if (arr[mid] == target) {
            colorArray(id, {mid}, "green");
            setVariableStr("found", "true");
            removePointer("lo"); removePointer("hi"); removePointer("mid");
            return true;
        } else if (arr[mid] < target) {
            lo = mid + 1;
            setPointer("lo", id, lo);
        } else {
            hi = mid - 1;
            setPointer("hi", id, hi);
        }
    }
    setVariableStr("found", "false");
    removePointer("lo"); removePointer("hi"); removePointer("mid");
    return false;
}

bool TraceLogger::nextPermutation(const std::string& id, std::vector<int>& arr) {
    int n = (int)arr.size();
    int i = n - 2;
    while (i >= 0 && arr[i] >= arr[i + 1]) {
        colorArray(id, {i, i + 1}, "red");
        i--;
    }
    if (i < 0) {
        reverseArr(id, arr);
        return false;
    }
    colorArray(id, {i}, "green");
    int j = n - 1;
    while (arr[j] <= arr[i]) {
        colorArray(id, {i, j}, "red");
        j--;
    }
    colorArray(id, {i, j}, "green");
    std::swap(arr[i], arr[j]);
    swapArray(id, i, j);
    int lo = i + 1, hi = n - 1;
    while (lo < hi) {
        colorArray(id, {lo, hi}, "red");
        std::swap(arr[lo], arr[hi]);
        swapArray(id, lo, hi);
        lo++; hi--;
    }
    return true;
}

bool TraceLogger::prevPermutation(const std::string& id, std::vector<int>& arr) {
    int n = (int)arr.size();
    int i = n - 2;
    while (i >= 0 && arr[i] <= arr[i + 1]) {
        colorArray(id, {i, i + 1}, "red");
        i--;
    }
    if (i < 0) {
        reverseArr(id, arr);
        return false;
    }
    colorArray(id, {i}, "green");
    int j = n - 1;
    while (arr[j] >= arr[i]) {
        colorArray(id, {i, j}, "red");
        j--;
    }
    colorArray(id, {i, j}, "green");
    std::swap(arr[i], arr[j]);
    swapArray(id, i, j);
    int lo = i + 1, hi = n - 1;
    while (lo < hi) {
        colorArray(id, {lo, hi}, "red");
        std::swap(arr[lo], arr[hi]);
        swapArray(id, lo, hi);
        lo++; hi--;
    }
    return true;
}

int TraceLogger::uniqueArr(const std::string& id, std::vector<int>& arr) {
    if (arr.empty()) return 0;
    int w = 1;
    setPointer("write", id, 0);
    for (int r = 1; r < (int)arr.size(); r++) {
        colorArray(id, {r, w - 1}, "red");
        setPointer("read", id, r);
        if (arr[r] != arr[w - 1]) {
            arr[w] = arr[r];
            updateArrayValue(id, w, arr[w]);
            colorArray(id, {w}, "green");
            setPointer("write", id, w);
            w++;
        }
    }
    removePointer("read"); removePointer("write");
    setVariable("new_size", w);
    return w;
}

int TraceLogger::minElement(const std::string& id, std::vector<int>& arr) {
    if (arr.empty()) return -1;
    int minIdx = 0;
    setPointer("min", id, 0);
    for (int i = 1; i < (int)arr.size(); i++) {
        colorArray(id, {i, minIdx}, "red");
        if (arr[i] < arr[minIdx]) {
            minIdx = i;
            setPointer("min", id, minIdx);
            colorArray(id, {minIdx}, "green");
        }
    }
    removePointer("min");
    setVariable("min_idx", minIdx);
    return minIdx;
}

int TraceLogger::maxElement(const std::string& id, std::vector<int>& arr) {
    if (arr.empty()) return -1;
    int maxIdx = 0;
    setPointer("max", id, 0);
    for (int i = 1; i < (int)arr.size(); i++) {
        colorArray(id, {i, maxIdx}, "red");
        if (arr[i] > arr[maxIdx]) {
            maxIdx = i;
            setPointer("max", id, maxIdx);
            colorArray(id, {maxIdx}, "green");
        }
    }
    removePointer("max");
    setVariable("max_idx", maxIdx);
    return maxIdx;
}

void TraceLogger::rotateArr(const std::string& id, std::vector<int>& arr, int pivot) {
    int n = (int)arr.size();
    auto rev = [&](int lo, int hi) {
        while (lo < hi) {
            colorArray(id, {lo, hi}, "red");
            std::swap(arr[lo], arr[hi]);
            swapArray(id, lo, hi);
            lo++; hi--;
        }
    };
    rev(0, pivot - 1);
    rev(pivot, n - 1);
    rev(0, n - 1);
}

// ============================================================
// 18. STL 래퍼 — 골드~플래티넘
// ============================================================

void TraceLogger::stableSort(const std::string& id, std::vector<int>& arr) {
    if (arr.empty()) return;
    _mergeSort(id, arr, 0, (int)arr.size() - 1);
}

void TraceLogger::partialSort(const std::string& id, std::vector<int>& arr, int k) {
    int n = (int)arr.size();
    for (int i = 0; i < k; i++) {
        int minIdx = i;
        setPointer("min", id, minIdx);
        for (int j = i + 1; j < n; j++) {
            colorArray(id, {j, minIdx}, "red");
            if (arr[j] < arr[minIdx]) {
                minIdx = j;
                setPointer("min", id, minIdx);
            }
        }
        if (minIdx != i) {
            std::swap(arr[i], arr[minIdx]);
            swapArray(id, i, minIdx);
        }
    }
    removePointer("min");
    rangeColorArray(id, 0, k - 1, "green");
}

void TraceLogger::nthElement(const std::string& id, std::vector<int>& arr, int n) {
    _quickSelect(id, arr, 0, (int)arr.size() - 1, n);
    colorArray(id, {n}, "green");
    setVariable("nth_idx", n);
    setVariable("nth_val", arr[n]);
}

int TraceLogger::partitionArr(const std::string& id, std::vector<int>& arr,
                               std::function<bool(int)> pred) {
    int lo = 0, hi = (int)arr.size() - 1;
    setPointer("lo", id, lo);
    setPointer("hi", id, hi);
    while (lo <= hi) {
        while (lo <= hi && pred(arr[lo]))  { colorArray(id, {lo}, "green"); lo++; setPointer("lo", id, lo); }
        while (lo <= hi && !pred(arr[hi])) { colorArray(id, {hi}, "red");   hi--; setPointer("hi", id, hi); }
        if (lo < hi) {
            colorArray(id, {lo, hi}, "red");
            std::swap(arr[lo], arr[hi]);
            swapArray(id, lo, hi);
            lo++; hi--;
            setPointer("lo", id, lo);
            setPointer("hi", id, hi);
        }
    }
    removePointer("lo"); removePointer("hi");
    setVariable("boundary", lo);
    return lo;
}

int TraceLogger::stablePartition(const std::string& id, std::vector<int>& arr,
                                  std::function<bool(int)> pred) {
    std::vector<int> trueVec, falseVec;
    for (int i = 0; i < (int)arr.size(); i++) {
        colorArray(id, {i}, "red");
        if (pred(arr[i])) { trueVec.push_back(arr[i]); colorArray(id, {i}, "green"); }
        else              { falseVec.push_back(arr[i]); }
    }
    int w = 0;
    for (int v : trueVec)  { arr[w] = v; updateArrayValue(id, w, v); w++; }
    for (int v : falseVec) { arr[w] = v; updateArrayValue(id, w, v); w++; }
    setVariable("boundary", (int)trueVec.size());
    rangeColorArray(id, 0, (int)trueVec.size() - 1, "green");
    return (int)trueVec.size();
}

bool TraceLogger::isSorted(const std::string& id, std::vector<int>& arr) {
    for (int i = 0; i + 1 < (int)arr.size(); i++) {
        colorArray(id, {i, i + 1}, "red");
        if (arr[i] > arr[i + 1]) {
            colorArray(id, {i, i + 1}, "red");
            setVariableStr("is_sorted", "false");
            return false;
        }
        colorArray(id, {i, i + 1}, "green");
    }
    setVariableStr("is_sorted", "true");
    return true;
}

int TraceLogger::adjacentFind(const std::string& id, std::vector<int>& arr) {
    for (int i = 0; i + 1 < (int)arr.size(); i++) {
        colorArray(id, {i, i + 1}, "red");
        if (arr[i] == arr[i + 1]) {
            colorArray(id, {i, i + 1}, "green");
            setVariable("adj_idx", i);
            return i;
        }
    }
    setVariable("adj_idx", -1);
    return -1;
}

void TraceLogger::inplaceMerge(const std::string& id, std::vector<int>& arr, int mid) {
    _merge(id, arr, 0, mid - 1, (int)arr.size() - 1);
}

void TraceLogger::setUnion(const std::string& aId, const std::vector<int>& a,
                            const std::string& bId, const std::vector<int>& b,
                            const std::string& dstId, std::vector<int>& dst) {
    dst.clear();
    setArray(dstId, dst);
    int i = 0, j = 0;
    setPointer("i", aId, 0);
    setPointer("j", bId, 0);
    while (i < (int)a.size() && j < (int)b.size()) {
        colorArray(aId, {i}, "red"); colorArray(bId, {j}, "red");
        if (a[i] < b[j])      { dst.push_back(a[i]); pushBack(dstId, a[i]); i++; setPointer("i", aId, i); }
        else if (a[i] > b[j]) { dst.push_back(b[j]); pushBack(dstId, b[j]); j++; setPointer("j", bId, j); }
        else                  { dst.push_back(a[i]); pushBack(dstId, a[i]); i++; j++; setPointer("i", aId, i); setPointer("j", bId, j); }
    }
    while (i < (int)a.size()) { colorArray(aId, {i}, "red"); dst.push_back(a[i]); pushBack(dstId, a[i]); i++; }
    while (j < (int)b.size()) { colorArray(bId, {j}, "red"); dst.push_back(b[j]); pushBack(dstId, b[j]); j++; }
    removePointer("i"); removePointer("j");
}

void TraceLogger::setIntersection(const std::string& aId, const std::vector<int>& a,
                                   const std::string& bId, const std::vector<int>& b,
                                   const std::string& dstId, std::vector<int>& dst) {
    dst.clear();
    setArray(dstId, dst);
    int i = 0, j = 0;
    setPointer("i", aId, 0); setPointer("j", bId, 0);
    while (i < (int)a.size() && j < (int)b.size()) {
        colorArray(aId, {i}, "red"); colorArray(bId, {j}, "red");
        if (a[i] < b[j])      { i++; setPointer("i", aId, i); }
        else if (a[i] > b[j]) { j++; setPointer("j", bId, j); }
        else { dst.push_back(a[i]); pushBack(dstId, a[i]); colorArray(aId, {i}, "green"); i++; j++; setPointer("i", aId, i); setPointer("j", bId, j); }
    }
    removePointer("i"); removePointer("j");
}

void TraceLogger::setDifference(const std::string& aId, const std::vector<int>& a,
                                 const std::string& bId, const std::vector<int>& b,
                                 const std::string& dstId, std::vector<int>& dst) {
    dst.clear();
    setArray(dstId, dst);
    int i = 0, j = 0;
    setPointer("i", aId, 0); setPointer("j", bId, 0);
    while (i < (int)a.size() && j < (int)b.size()) {
        colorArray(aId, {i}, "red"); colorArray(bId, {j}, "red");
        if (a[i] < b[j])      { dst.push_back(a[i]); pushBack(dstId, a[i]); colorArray(aId, {i}, "green"); i++; setPointer("i", aId, i); }
        else if (a[i] > b[j]) { j++; setPointer("j", bId, j); }
        else                  { i++; j++; setPointer("i", aId, i); setPointer("j", bId, j); }
    }
    while (i < (int)a.size()) { colorArray(aId, {i}, "green"); dst.push_back(a[i]); pushBack(dstId, a[i]); i++; }
    removePointer("i"); removePointer("j");
}

// ============================================================
// 19. STL 래퍼 — 플래티넘~다이아
// ============================================================

int TraceLogger::gcd(const std::string& varName, int a, int b) {
    setVariable(varName + "_a", a);
    setVariable(varName + "_b", b);
    while (b != 0) {
        colorVariable(varName + "_a", "red");
        colorVariable(varName + "_b", "red");
        int t = b;
        b = a % b;
        a = t;
        updateVariable(varName + "_a", a);
        updateVariable(varName + "_b", b);
    }
    setVariable(varName + "_result", a);
    colorVariable(varName + "_result", "green");
    return a;
}

int TraceLogger::lcm(const std::string& varName, int a, int b) {
    int g = gcd(varName + "_gcd", a, b);
    int result = a / g * b;
    setVariable(varName + "_result", result);
    colorVariable(varName + "_result", "green");
    return result;
}

int TraceLogger::popcount(const std::string& id, int x) {
    createBitset(id, 32);
    int cnt = 0;
    for (int i = 0; i < 32; i++) {
        int bit = (x >> i) & 1;
        setBit(id, i, bit);
        if (bit) { colorBit(id, i, "green"); cnt++; }
        else     { colorBit(id, i, "red"); }
    }
    setVariable(id + "_count", cnt);
    return cnt;
}

int TraceLogger::clz(const std::string& id, int x) {
    createBitset(id, 32);
    int cnt = 0;
    bool found = false;
    for (int i = 31; i >= 0; i--) {
        int bit = (x >> i) & 1;
        setBit(id, i, bit);
        if (!found && bit == 0) { colorBit(id, i, "red"); cnt++; }
        else { found = true; colorBit(id, i, "green"); }
    }
    setVariable(id + "_clz", cnt);
    return cnt;
}

// ============================================================
// 20. 단순 연결리스트 (SLL)
// ============================================================

void TraceLogger::createSLL(const std::string& id) {
    frames.push_back("{" + _s("target","sll") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::sllCreateNode(const std::string& id, int nodeId, int value) {
    frames.push_back("{" + _s("target","sll") + "," + _s("id",id) + "," + _s("action","create_node") + "," + _n("node_id",nodeId) + "," + _n("value",value) + "}");
}
void TraceLogger::sllDeleteNode(const std::string& id, int nodeId) {
    frames.push_back("{" + _s("target","sll") + "," + _s("id",id) + "," + _s("action","delete_node") + "," + _n("node_id",nodeId) + "}");
}
void TraceLogger::sllSetNext(const std::string& id, int nodeId, int nextId) {
    frames.push_back("{" + _s("target","sll") + "," + _s("id",id) + "," + _s("action","set_next") + "," + _n("node_id",nodeId) + "," + _n("next_id",nextId) + "}");
}
void TraceLogger::sllColorNode(const std::string& id, int nodeId, const std::string& color) {
    frames.push_back("{" + _s("target","sll") + "," + _s("id",id) + "," + _s("action","color_node") + "," + _n("node_id",nodeId) + "," + _s("color",color) + "}");
}
void TraceLogger::sllUpdateValue(const std::string& id, int nodeId, int value) {
    frames.push_back("{" + _s("target","sll") + "," + _s("id",id) + "," + _s("action","update_value") + "," + _n("node_id",nodeId) + "," + _n("value",value) + "}");
}

// ============================================================
// 21. 이중 연결리스트 (DLL)
// ============================================================

void TraceLogger::createDLL(const std::string& id) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::dllCreateNode(const std::string& id, int nodeId, int value) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","create_node") + "," + _n("node_id",nodeId) + "," + _n("value",value) + "}");
}
void TraceLogger::dllDeleteNode(const std::string& id, int nodeId) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","delete_node") + "," + _n("node_id",nodeId) + "}");
}
void TraceLogger::dllSetNext(const std::string& id, int nodeId, int nextId) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","set_next") + "," + _n("node_id",nodeId) + "," + _n("next_id",nextId) + "}");
}
void TraceLogger::dllSetPrev(const std::string& id, int nodeId, int prevId) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","set_prev") + "," + _n("node_id",nodeId) + "," + _n("prev_id",prevId) + "}");
}
void TraceLogger::dllColorNode(const std::string& id, int nodeId, const std::string& color) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","color_node") + "," + _n("node_id",nodeId) + "," + _s("color",color) + "}");
}
void TraceLogger::dllUpdateValue(const std::string& id, int nodeId, int value) {
    frames.push_back("{" + _s("target","dll") + "," + _s("id",id) + "," + _s("action","update_value") + "," + _n("node_id",nodeId) + "," + _n("value",value) + "}");
}

// ============================================================
// 22. 이진 탐색 트리 (BST)
// ============================================================

void TraceLogger::createBST(const std::string& id) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","create") + "}");
}
void TraceLogger::bstCreateNode(const std::string& id, int nodeId, int value) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","create_node") + "," + _n("node_id",nodeId) + "," + _n("value",value) + "}");
}
void TraceLogger::bstDeleteNode(const std::string& id, int nodeId) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","delete_node") + "," + _n("node_id",nodeId) + "}");
}
void TraceLogger::bstSetLeft(const std::string& id, int nodeId, int leftId) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","set_left") + "," + _n("node_id",nodeId) + "," + _n("left_id",leftId) + "}");
}
void TraceLogger::bstSetRight(const std::string& id, int nodeId, int rightId) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","set_right") + "," + _n("node_id",nodeId) + "," + _n("right_id",rightId) + "}");
}
void TraceLogger::bstColorNode(const std::string& id, int nodeId, const std::string& color) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","color_node") + "," + _n("node_id",nodeId) + "," + _s("color",color) + "}");
}
void TraceLogger::bstUpdateValue(const std::string& id, int nodeId, int value) {
    frames.push_back("{" + _s("target","bst") + "," + _s("id",id) + "," + _s("action","update_value") + "," + _n("node_id",nodeId) + "," + _n("value",value) + "}");
}

// ============================================================
// 저장
// ============================================================

void TraceLogger::save(const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs.is_open()) {
        fprintf(stderr, "TraceLogger: Failed to save '%s'\n", filename.c_str());
        return;
    }
    ofs << "{\n";
    ofs << "  \"meta\":{\"family\":\"" << _family << "\"},\n";
    ofs << "  \"frames\":[\n";
    for (int i = 0; i < (int)frames.size(); ++i) {
        ofs << "    " << frames[i];
        if (i + 1 < (int)frames.size()) ofs << ",";
        ofs << "\n";
    }
    ofs << "  ]\n";
    ofs << "}\n";
    ofs.close();
    fprintf(stdout, "TraceLogger: Saved %d frames to '%s'\n", (int)frames.size(), filename.c_str());
}
