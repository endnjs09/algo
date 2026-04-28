import requests
import json

URL = "http://localhost:8000/api/v1/visualize"

test_code = """
#include <iostream>
#include <vector>
using namespace std;

int main() {
    vector<int> dist(5, 100); 
    
    dist[0] = 0;
    dist[1] = 50;
    dist[2] = 20; // 값이 작아지면 shadow_containers.h가 green 로그를 남김
    
    cout << "Test Finished" << endl;
    return 0;
}
"""

def test():
    payload = {
        "code": test_code,
        "stdin": ""
    }

    response = requests.post(URL, json=payload)

    if response.status_code == 200:
        print("테스트 성공")
        trace_data = response.json()

        if isinstance(trace_data, list):
            print(f"로그 프레임 수: {len(trace_data)}")
        else:
            print(f"로그 프레임 수: {len(trace_data.get('frames', []))}")

        with open("last_test_result.json", "w") as f:
            json.dump(trace_data, f, indent=2)
        print("결과가 'last_test_result.json'에 저장되었습니다.")
    else:
        print(f"테스트 실패 (상태 코드: {response.status_code})")
        print(f"상세 내용: {response.text}")

if __name__ == "__main__":
    test()