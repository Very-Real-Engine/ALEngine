# Physics Engine
## Physics Main Loop
### 1) Integrate
- 각 Rigidbody의 누적된 힘과 토크를 통해 물리 속성을 결정
### 2) Broad Phase
- Integrate 단계에서 움직인 물체들의 1차 충돌 감지 수행
### 3) Narrow Phase
- Boroad Phase 단계에서 감지된 충돌들의 실제 충돌 감지 및 충돌 정보 생성
### 4) Solve
- Narrow Phase 단계에서 생성된 충돌 정보를 바탕으로 충돌 해소

## Integrate
### 누적 힘과 토크로 가속도 계산
- Rigidbody에 누적 된 힘과 토크를 중심으로 가속도 계산
- 가속도를 통해 속도와 위치 결정
### 중력 적용
- Rigidbody의 useGravity가 true인 경우 중력 적용
- Rigidbody의 gravityScale에 따른 중력 적용
### 속도 감쇠 적용
- 공기 저항을 구현하기 위한 Damping 설정
## Broad Phase
### BVH (Bounding Volume Hierarchy)를 통한 1차 충돌 감지
- **BVH** : 위치 기반으로 구성된 계층적 이진 트리
- 물체 이동 시 동적으로 트리 구조 변경
- 트리 구조가 비대칭일 경우 Balance 처리
### AABB (Axis-Aligned bounding box) 사용
- BVH 에서 Bounding Volume으로 AABB 사용 
## Narrow Phase
### GJK 알고리즘 사용
- GJK 알고리즘을 통한 실제 충돌 감지
### EPA 알고리즘 사용
- GJK 에서 감지된 충돌에 EPA 알고리즘 적용 
- EPA 알고리즘을 통한 충돌 방향 벡터, 충돌 깊이 계산
### 충돌면 Clipping 
- EPA 알고리즘에서 계산한 충돌 방향 벡터, 충돌 깊이를 이용해 충돌면 설정
- 충돌면끼리의 Clipping을 통한 충돌점 생성
## Solve
### Island 단위의 충돌 처리
- **Island**: 서로 영향을 주는 Rigidbody 들의 묶음
- Island 별로 Solve 진행
### Velocity Constraint Solve (충돌 물체 속도 변경)
- 충격량을 구해 충돌 물체의 속도 변경
- velocity constraint solve를 N번 반복하여 올바른 결과에 수렴 
### Position Constraint Solve (충돌 물체 침투 해소)
- 충돌 깊이와 물체의 질량을 통해 물체 간 침투 해소
- position constraint solve를 N번 반복하여 올바른 결과에 수렴
### Sleep/Awake
- 정지 접촉이 특정 시간 동안 유지 된 경우 물체 Sleep 처리
- Sleep 상태의 물체는 물리연산 적용을 하지 않고, 충돌의 주체가 되지 않음

## Physics Engine Optimization
### Memory Pool
- Stack Memory Pool 구현
- Block Memory Pool 구현