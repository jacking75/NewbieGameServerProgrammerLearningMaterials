# Tailscale을 이용한 로컬PC의 게임 서버 외부 공개

[Tailscale을 이용하여 개발 환경 준비하기](https://docs.google.com/document/d/1u_v1gJZ5kMRGZB7CCwABAn8-rMtVxQDeFkCG7GxH5Cc/edit?usp=sharing )  
  

## 개요
Tailscale은 WireGuard 기반의 **VPN Overlay Network**를 구성해 주는 서비스로, 로컬 PC가 NAT 뒤에 있거나 **공개 IP가 없어도** 외부에서 안전하게 접속할 수 있도록 할 수 있다.  
  
즉, 로컬 PC가 사설망(예: 192.168.x.x, 10.x.x.x)에 있어도
Tailscale이 부여하는 **100.x.x.x (CGNAT range) 또는 fd7:…(IPv6)** 주소로 외부 장치가 접속할 수 있다.  
  

## 네트워크 접속

* ✔ 공개 IP 없이도 외부 접속 가능
* ✔ HTTP 문제 없음
* ✔ TCP 기반 소켓 서버 모두 가능
* ✔ NAT, 포트포워딩 필요 없음
  
### ✔ HTTP 가능
로컬 PC에서 8080 포트로 웹 서버를 띄우고 있다면  
외부 클라이언트에서:  

```
http://100.xx.xx.xx:8080
```

처럼 직접 접근할 수 있다.

### ✔ TCP Socket도 가능
Tailscale은 네트워크 레벨에서 **L3 레이어를 그대로 노출**하므로 다음 모두 가능하다.  
* TCP 서버
* WebSocket
* gRPC
* 데이터베이스 포트(MySQL, PostgreSQL 등)
* Custom TCP 기반 프로토콜

Tailscale은 포트 포워딩이 필요 없고 NAT Traversal을 자동 처리한다.  


### 주의할 점

#### 1. 피어 간 직접 연결 실패 시 릴레이(derp) 사용
대부분 NAT Traversal로 직접 연결되지만 간혹 직접 연결이 안 되면 Tailscale이 운영하는 **DERP 릴레이 서버**를 통한 중계가 된다.  
이 경우 속도가 느려질 수 있다.  

#### 2. ACL에 의해 접근 제어 가능
Tailscale Admin Console에서 접근 가능한 사용자/장치를 조절할 수 있다.

#### 3. Public sharing 옵션도 있음
원하면 특정 서비스만 공개(익명 접근 허용)할 수도 있다.
단, 이 기능은 HTTP(S)에 한정된다.


## 사용하기  

### 1. Tailscale 설치

#### 서버 PC(로컬 PC)에 설치
Windows / macOS / Linux 어디든 설치 가능하다.   
>>> [Windows 다운로드](https://tailscale.com/kb/1022/install-windows )  
>>> Windows에서는 사용할 port 번호를 방화벽에서 허용해야 한다.  
  
설치 후 로그인하면 해당 PC에 Tailscale IP가 부여된다.  

예:

* IPv4: `100.107.15.21`
* IPv6: `fd7a:115c:a1e0:ab12:...`

이 IP가 외부에서 접속하는 주소가 된다.

  
### 2. 서버 PC에서 서비스 실행
예시: HTTP 서버 또는 TCP 소켓 서버

#### HTTP 예시 (예: 8080포트)

```
python3 -m http.server 8080
```

#### TCP 소켓 서버(예: 5000포트)

```
# 예: Python TCP server
import socket

s = socket.socket()
s.bind(("0.0.0.0", 5000))
s.listen(5)
print("listening...")
while True:
    conn, addr = s.accept()
    print("connected:", addr)
    conn.send(b"hello")
    conn.close()
```

이렇게 로컬 PC에서 포트만 열어두면 된다.

  
### 3. 외부 PC(클라이언트 PC)에 Tailscale 설치
동일하게 Tailscale을 설치하고 같은 계정 또는 공유된 Tailnet에 로그인한다.  
그럼 외부 PC에서도 서버 PC의 Tailscale IP가 보인다.  

### 4. 외부 PC에서 접속

#### HTTP 접속
외부 PC 웹 브라우저 또는 curl에서:

```
http://100.107.15.21:8080
```

#### TCP 소켓 접속

```
telnet 100.107.15.21 5000
```

또는 자체 프로그램에서 소켓 연결:

```python
socket.connect(("100.107.15.21", 5000))
```

이렇게 하면 외부인데도 바로 로컬 PC 서버에 접속된다.


### 5. 추가 정보

#### ✔ 포트포워딩 필요 없음
Tailscale이 NAT Traversal을 자동 처리한다.

#### ✔ HTTPS, DB포트도 문제 없음
웹 서버, 게임 서버, WebSocket, gRPC 등 모두 접속 가능하다.

#### ✔ 속도
대부분 PC ↔ PC가 직접 연결되므로 빠르다.  
직접 연결이 안 되면 자동으로 DERP 릴레이로 fallback한다.

#### ✔ 공용 서비스 공개(Public Sharing)도 가능
원한다면 특정 HTTP 서비스만 외부 일반 사용자에게 공개도 할 수 있다(인증 없음).  하지만 이는 선택 사항이다.  

  
### 6. 외부 PC에서 접속이 되지 않는다면  
접속하는 포트 번호가 11500 이라고 가정한다.

#### 1. **Windows 방화벽에서 11500 포트가 허용되지 않음** ← 가장 흔한 원인
Windows 방화벽은 **포트별로 따로 열어야 한다.**

해결 방법(간단)  
PowerShell(관리자)에서:  
```
New-NetFirewallRule -DisplayName "GameServer11500" -Direction Inbound -Protocol TCP -LocalPort 11500 -Action Allow
```
  
또는 “Windows 방화벽 → 고급 설정 → 인바운드 규칙 → 새 규칙”에서 추가하면 된다.

✔ **이 문제 하나만 해결하면 대부분 접속된다.**

#### 2. 서버 애플리케이션이 11500 포트를 실제로 바인딩하지 않음
서버 코드가 다음처럼 되어 있어야 한다.

```
bind("0.0.0.0", 11500)
```

만약:

* `bind("127.0.0.1", 11500)`
* `bind("localhost", 11500)`

으로 되어 있으면, **Tailscale 인터페이스(tailscale0)에서는 접근 불가**다.

윈도우에서는 `netstat -ano | findstr 11500` 으로 19,300 프로세스가 실제로 LISTEN 중인지 확인해보면 된다.

#### 3. Tailscale Funnel 또는 Serve 설정과 충돌
만약 Tailscale의 serve/funnel 기능을 사용한 적이 있다면 특정 포트가 강제로 리디렉션되거나 막히는 경우가 있다.  

확인:

```
tailscale serve status
tailscale funnel status
```

문제 있을 경우:

```
tailscale serve reset
tailscale funnel 11500 off
```

---  
    
    
## Tailscale을 이용한 게임 서버 테스트 환경 구성  
  
### 1. Tailscale을 이용한 게임 서버 테스트 환경 구성 예시
게임 서버 개발자가 흔히 요구하는 구조를 기준으로 설명한다.

#### 1) 서버 PC에 Tailscale 설치
로컬 개발 PC(서버 역할)에 Tailscale을 설치하고 로그인한다.
로그인하면 해당 PC에 Tailscale IP가 부여된다.

예시:
`100.87.12.40`

이 IP가 마치 “퍼블릭 IP”처럼 동작한다.

#### 2) 게임 서버 실행
예를 들어 TCP 기반 게임 서버가 9000 포트를 사용한다고 하자.

```
./GameServer --port 9000
```

또는

```
python game_server.py 9000
```

**중요**: 바인딩 주소를 `0.0.0.0`(또는 시스템 기본)으로 설정해야 한다.

#### 3) 클라이언트(외부 PC 또는 모바일 테스트 디바이스)에 Tailscale 설치
테스트하는 PC/노트북/태블릿/모바일에도 Tailscale을 설치해 같은 Tailnet에 접속한다.  
이제 클라이언트는 서버 PC를 마치 같은 LAN에 있는 것처럼 인식한다.
  
#### 4) 클라이언트에서 게임 서버 접속
클라이언트 게임에서 서버 주소를 다음처럼 설정한다.

```
Server IP = 100.87.12.40
Port = 9000
```

그럼 외부 네트워크에서도 NAT, 방화벽, 포트포워딩 없이 바로 연결된다.  
Tailscale이 WireGuard 터널을 자동으로 구성하기 때문이다.  
  
#### 5) 여러 개발자/QA와 공유도 가능
Tailnet에 초대하거나, ACL로 특정 사용자만 접속하도록 제한할 수 있다.
즉, 팀 내 테스트 서버로 활용하기 적합하다.


### 2. Tailscale Public Sharing(공용 서비스 공개) 기능 설명
Tailscale은 특정 HTTP 서비스를 **인터넷 전체에 공개**할 수 있는 기능도 제공한다.  
단, 이 기능은 “특정 포트 기반 HTTP(s)”에만 적용된다.  
  
예:  
로컬 PC에서 8000 포트로 웹 서버를 띄우고 있다면 이를 Public Link로 웹 전체에 공개할 수 있다.

#### 1) Public Sharing 사용 조건
* Tailscale admin 콘솔에서 Public Sharing이 활성화돼 있어야 한다
* 공개할 서비스는 **HTTP 또는 HTTPS**여야 한다
* TCP 소켓, 게임 서버, DB는 **Public Sharing 불가**
  
즉, 일반 TCP 포트나 게임 서버는 인터넷 전체 공개는 지원하지 않고, HTTP 기반 API나 Web UI 등만 공개할 수 있다.

#### 2) Public Sharing 사용 방법

##### (1) 공개할 서버 실행

예:

```
python3 -m http.server 8000
```

##### (2) Tailscale 관리 콘솔 접속

[https://login.tailscale.com/admin/machines](https://login.tailscale.com/admin/machines)

##### (3) 해당 머신 선택 → *Enable sharing* 또는 *Share this device* 버튼
그러면 “URL 공유” 기능이 나온다.

##### (4) 공개 URL 생성

예:

```
https://example-shared-1234.ts.net/
```

이 URL은 **인터넷 누구나** 접속할 수 있다.
즉, Tailscale 계정 없이도 접근 가능하다.

#### 3) Public Sharing의 특징 및 주의점

##### 장점
* 서버가 NAT 뒤에 있어도 인터넷 공개 가능
* HTTPS 자동 적용
* 깔끔한 URL 제공
* 서버 PC의 IP를 숨긴다

##### 제한
* **HTTP/HTTPS만 가능**
* TCP 게임 서버는 Public Sharing 불가
* 여러 포트를 동시에 공개하려면 각각 별도 설정 필요
* 과도한 트래픽에는 부적합(릴레이 기반)

### 3. 게임 서버 개발자가 Tailscale을 활용하는 대표적 패턴

#### ✔ 로컬 개발 서버를 외부 QA에게 즉시 공유
Tailscale IP로 바로 접속하게 할 수 있다.

#### ✔ NAT/포트포워딩 없는 간단한 테스트 인프라
AWS/GCP 없이도 외부 접속 테스트 가능하다.

#### ✔ 팀 내 개발용 VPN
모든 개발 PC를 하나의 Tailnet에 묶어 서로 SSH, DB 접속 가능하다.

#### ✔ 배포 전 기능 점검(Feature Flag 테스트)
모바일 기기에서도 동일한 접속 경로로 테스트 가능하다.
  
  
  
## 게임 서버용 최적 ACL 구성 예시
게임 서버 개발 조직에서는 다음 2가지 요구가 많다:
  
1. 특정 서버 머신(개발 서버)에만 접속 허용
2. 특정 포트만 제한적으로 열기

Tailscale ACL(JSON) 예시를 아래에 작성한다.

### ✔ 예시 요구
* QA 팀은 게임 서버 포트(9000)만 접근 가능
* 개발자는 SSH, DB, HTTP 등 모두 가능
* 로컬 개발 서버 이름이 `dev-server-01`

### ✔ ACL 예시(JSON)

```json
{
  "groups": {
    "group:dev": ["alice@example.com", "bob@example.com"],
    "group:qa": ["qa1@example.com", "qa2@example.com"]
  },
  "acls": [
    {
      "action": "accept",
      "users": ["group:dev"],
      "ports": ["dev-server-01:22", "dev-server-01:9000", "dev-server-01:5432", "dev-server-01:8000"]
    },
    {
      "action": "accept",
      "users": ["group:qa"],
      "ports": ["dev-server-01:9000"]
    }
  ]
}
```

#### 의미
* 개발자는 서버 전 포트 접근 가능
* QA는 **게임 서버 포트인 9000번만 접근 가능**
* NAT, 포트포워딩 없이 제어된다


## 성능 최적화: Subnet Router / Node Routing
Tailscale은 기본적으로 각 노드가 직접 서로 P2P 연결한다.
그러나 대규모 네트워크나 사내 시스템과 연동해야 할 경우 **Subnet Router** 또는 **Node Routing**을 사용하면 좋다.

### ✔ Subnet Router란?
특정 노드를 “게이트웨이”로 만들어, Tailscale 네트워크에서 사설망(IP 범위)까지 접근할 수 있게 해주는 기능이다.   
  
예:
사내망 `192.168.0.0/24`의 DB, 내부 API에 Tailscale로 접속 가능하게 한다.
  
예시 명령:

```
tailscale up --advertise-routes=192.168.0.0/24
```
  
Admin 콘솔에서 승인하면 다른 Tailscale 장치들이 해당 사설망 자원에 접근할 수 있게 된다.  

#### 게임 개발자에게 유용한 이유
* 로컬 사무실의 빌드 서버, DB 서버 등에 원격으로 Tailscale로 접근 가능
* 사내환경과 개발환경을 하나의 VPN으로 통합
* QA, 외주 개발자가 내부 리소스에 접근 가능(ACL로 제한 가능)


### ✔ Node Routing(Exit Node)
특정 노드를 “인터넷 게이트웨이”로 설정해 모든 트래픽을 그 노드를 거쳐 나가게 하는 기능이다.  
  
예:  
  
```
tailscale up --advertise-exit-node
```
  
그리고 클라이언트에서 이 노드를 Exit Node로 선택한다.  

#### 활용 예
* 회사망 IP에서만 접속 가능한 외부 서비스 테스트
* 특정 국가 IP에서만 접속 가능한 테스트

   
## **Tailscale Funnel** — Public Sharing보다 강력한 공개 기능
Tailscale Funnel은 Public Sharing의 확장판이다. 특징은:  

* **HTTP뿐 아니라 TCP도 외부 인터넷에 공개 가능**
* 도메인 자동 발급
* HTTPS 자동 구성
* 포트 라우팅 기능 제공
* Tailscale Relay 기반이라 NAT, 포트포워딩 필요 없음
  
즉, 게임 서버 개발자에게 **외부 테스트용 공개 서버**를 쉽게 만들 수 있는 기능이다.  
  
### ✔ Funnel 사용 방법
  
#### 1) 서버 머신이 Tailscale에 로그인된 상태에서
Funnel 사용을 허용한다:

```
tailscale funnel 9000 on
```

그러면 자동으로 URL이 생성된다:

```
https://<your-tailnet-name>.ts.net
```

TCP 포트도 지원한다.

예:

```
tailscale serve tcp 9000 tcp://localhost:9000
```

또는

```
tailscale funnel 9000 on
```

#### 2) DNS 자동 설정
Tailscale이 자동으로 다음 형태의 DNS를 발급한다.

```
https://myserver-name.tailnet.ts.net/
```

TCP 포트도 마찬가지로 외부에서 접근 가능하다.
   
### ✔ Funnel vs Public Sharing 차이

| 기능       | Public Sharing | Funnel   |
| -------- | -------------- | -------- |
| HTTP 공개  | 가능             | 가능       |
| HTTPS    | 자동             | 자동       |
| TCP 공개   | 불가             | 가능       |
| 인증 없이 공개 | 가능             | 가능       |
| 도메인 제공   | 있음             | 더 강력한 형태 |
| 포트 라우팅   | 제한적            | 완전 지원    |
  
게임 서버 개발자는 **Funnel이 훨씬 적합**하다.
특히 외부 테스터나 모바일 QA에게 쉽게 링크만 던져주면 된다.  
  
  
## 추천 구성 패턴 (게임 서버 개발자 기준)
  
### 1) 개발 팀 내부용 서버
* Tailscale 기본 P2P
* ACL로 접근 제어
* Subnet Router로 사내 DB 접근 가능

### 2) QA 테스트용 외부 접근 서버
* Basic Tailnet 공유
* 머신 IP(100.x.x.x) 그대로 사용
* 게임 클라이언트에서 해당 주소로 접속

### 3) 외부 테스터/커뮤니티 테스트용 공개 서버
* **Tailscale Funnel 사용**
* TCP 게임 서버도 HTTPS 없이 바로 공개 가능
* 일시적인 이벤트용 서버 운영하기 쉬움

  

## ✔ 모바일에서 Tailscale 설정 순서

### 1) Tailscale 앱 설치

* Android: Play Store
* iPhone: App Store

검색어: **Tailscale**

설치 후 실행하고 로그인한다(구글, MS, 애플 계정 등).

### 2) Tailnet에 연결
로그인하면 VPN 연결 팝업이 뜨는데 “허용”을 선택한다.    
그러면 모바일 기기에도 Tailscale IP가 부여된다.    

예:
`100.120.55.9`

### 3) 로컬 PC(서버 PC)의 Tailscale IP 확인
서버 PC에서 다음 명령을 실행한다:

```
tailscale ip
```

또는 Tailscale 앱 UI에서 확인한다.

예:
`100.87.12.40`

이 주소가 모바일 게임 클라이언트의 서버 주소가 된다.

### 4) 모바일 게임에서 서버 주소 입력

#### (Unity/Unreal 클라이언트, 커스텀 클라이언트 모두 동일)

서버 주소:

```
100.87.12.40
```

포트:

```
9000
```

이렇게 설정하면 **모바일 기기 → Tailscale VPN → 로컬 PC** 로 직접 연결된다.


### ✔ 모바일에서 흔히 하는 실수

#### 1) 모바일 기기의 셀룰러/와이파이가 상관 있을까?
아무 영향 없다.
모바일은 어떤 네트워크를 사용하든 **Tailscale VPN을 통해 통신되기 때문에 항상 같은 경로로 연결**된다.

#### 2) 게임 클라이언트에서 DNS로 접근하면 안 된다?
일반 LAN IP 같은 `192.168.x.x` 주소는 모바일에서 보이지 않는다.
반드시 **Tailscale IP(100.x.x.x)** 로 접속해야 한다.

#### 3) PC 방화벽을 열지 않으면 모바일에서 절대 접속 불가
반드시 방화벽 인바운드 규칙을 추가해야 한다.


### 모바일 환경에서 Tailscale을 이용한 게임 테스트 최적 팁

#### ✔ 화면 꺼짐 방지
Tailscale는 VPN이라 백그라운드 제약이 있지만 일반 게임 플레이 정도는 문제 없다.
그래도 테스트 중 화면이 꺼지지 않도록 설정해두면 안정적이다.

#### ✔ 모바일 기기 여러 개 테스트도 가능
여러 스마트폰에 Tailscale을 설치하면 모두 동일한 서버로 바로 붙일 수 있다.

#### ✔ 외부 QA에게 테스트 배포도 가능
QA 팀에게 Tailscale 로그인만 시키면
로컬 개발 PC가 일시적인 “테스트 서버” 역할을 할 수 있다.


## Tailscale Funnel을 모바일 게임 테스트에 적용하는 실전 사례
Funnel은 TCP, HTTP를 **인터넷 전체에 즉시 공개**할 수 있는 기능이다.  
게임 서버 개발자에게 매우 유용하다.  

### ✔ 실전 사례: 외부 QA 또는 클라이언트에게 일시적 테스트 서버 제공

#### 1) 로컬 PC에서 게임 서버 실행

예:

```
GameServer.exe --port 9000
```

#### 2) Funnel 활성화

```
tailscale funnel 9000 on
```

#### 3) 외부 접속용 URL 또는 TCP 주소 자동 생성

예:

```
tcp://mydevserver.tailnet.ts.net:9000
```

#### 4) 모바일 클라이언트에서 해당 주소를 입력

모바일 기기에는 Tailscale을 설치할 필요가 없다.
**인터넷으로 곧장 접속 가능**하다.


### ✔ Funnel을 게임 QA에서 어떻게 쓰는가?
* 신규 기능을 외부 QA나 클라이언트에게 잠시 테스트할 때
* 개발 PC에서 바로 “퍼블릭 서버”를 만들고 테스트
* 모바일 클라이언트에 고정된 공개 주소가 생기므로 테스트가 쉽다
* 외부 테스트가 끝나면

  ```
  tailscale funnel 9000 off
  ```

  한 줄로 즉시 비활성화한다

Funnel은 실제 프로덕션 환경에서는 적합하지 않지만, **기능 테스트·임시 테스트 서버**로는 매우 강력하다.
  

아래에 요청한 **모든 항목**을 빠짐없이 정리해준다.
구성은 다음과 같다.

1. 모바일 기기에서 멀티 클라이언트 테스트 시 주의사항
2. Tailscale로 배틀서버/매칭 서버 등 멀티 서버 구성 테스트 방법
3. 로컬 PC 여러 대로 멀티 서버 클러스터 구성 예시
4. Tailscale + Docker + WSL2 조합으로 개발 환경 자동화 구축 방법

각 항목은 게임 서버 개발자가 실제로 쓸 수 있는 실전 중심으로 정리한다.

  

## Tailscale로 배틀서버/매칭 서버 등 멀티 서버 테스트 방법
MMO 구조 또는 멀티 서버 구조를 테스트할 때 Tailscale은 매우 유용하다.

### 1) 서버 구성 예시
예를 들어 다음과 같은 구조를 가정한다.

* 로그인 서버: 8000
* 매칭 서버: 8100
* 배틀 서버: 9000
* 채팅 서버: 7000

이 서버들이 PC 여러 대 또는 한 PC의 여러 프로세스로 동작하고 있다고 가정한다.

### 2) Tailscale 사용 시 서버 간 연결 구조
Tailscale 네트워크 내부에서는 모든 장치가 하나의 LAN처럼 동작하므로, 서버 간 통신은 아래처럼 단순해진다.

예:

```
login-server → 100.87.12.10
match-server → 100.87.12.11
battle-server → 100.87.12.12
chat-server → 100.87.12.13
```
  
게임 클라이언트는 로그인 서버에만 접속하면 되고, 서버끼리는 Tailscale IP로 서로 연결하면 된다.  
  
### 3) 대표적인 멀티 서버 테스트 패턴

#### 패턴 A — 각 서버를 개별 PC에 배치
개발자 여러 대가 서버 한 개씩 띄워서 통합 테스트하는 구조다.

#### 패턴 B — 하나의 PC에 여러 서버 실행 → Tailscale 한 번만 설치
한 PC에서 여러 프로세스를 다 띄운 후 포트만 각각 열고 테스트할 수 있다.

#### 패턴 C — Docker 내부 서버끼리 통신
Docker 컨테이너끼리 Tailscale을 직접 붙이거나 Host PC에만 Tailscale 설치하고 컨테이너는 Host의 네트워크 사용도 가능하다.


## 로컬 PC 여러 대로 멀티 서버 클러스터 구성 예시
실제 게임 서버 개발자들이 많이 사용하는 시나리오다.

### 구성 예시

* PC1: 로그인 서버 + 매칭 서버
* PC2: 배틀 서버 전용
* PC3: 데이터베이스 서버(MySQL / PostgreSQL)
* PC4: 툴 서버(API 서버 / GM툴)

각 PC에 Tailscale을 설치하고 각 장치의 100.x.x.x IP를 사용해 통합 테스트한다.

### 장점
* 사무실·집 어디에 있어도 서버 간 통신이 가능하다
* NAT/포트포워딩 필요 없음
* 로컬 PC를 모두 하나의 클러스터처럼 사용할 수 있다
* CPU 요구가 높으면 PC 여러 대로 분산할 수 있다
* AWS처럼 비용 걱정이 없다
   
### 클라이언트에서 접속 흐름
1. 모바일/PC 클라이언트 → 로그인 서버(Tailscale IP)
2. 로그인 서버 → 매칭 서버(Tailscale IP)
3. 매칭 서버 → 배틀 서버(Tailscale IP)
4. 배틀 서버 ↔ 클라이언트(Tailscale IP)

멀티 서버 구조 테스트를 로컬 PC 여러 대로 그대로 재현할 수 있다.



## Tailscale + Docker + WSL2 조합으로 개발 환경 자동화 구축 방법
게임 서버 개발자에게 가장 추천하는 방식이다.

### 1) WSL2 + Docker + Tailscale 기본 구조

```
Windows
 ├─ WSL2 Ubuntu
 │    ├─ Docker
 │    └─ GameServer
 └─ Tailscale for Windows (Host mode)
```

이 구조가 가장 안정적이고 관리가 쉽다.

### 2) 왜 Host(Windows)에 Tailscale을 설치하는가?
* WSL2 내부에 직접 설치하는 것보다 Windows Host에 설치하는 것이 훨씬 안정적이다
* WSL2 컨테이너/서버는 Host의 네트워크를 사용하면 된다
* Tailscale IP는 Host에만 존재하므로 방화벽 제어가 쉽다

### 3) Docker에서 서버 실행 후 Host PC 포트와 매핑

예:

Docker 컨테이너 내부 서버가 9000 포트를 쓴다면:

```
docker run -p 9000:9000 my-game-server
```

그러면 Tailscale IP 기준으로 `100.xx.xx.xx:9000` 으로 접속할 수 있다.

### 4) Docker Compose로 여러 서버 자동화

예:

```yaml
version: "3"
services:
  login:
    image: login-server
    ports:
      - "8000:8000"

  match:
    image: match-server
    ports:
      - "8100:8100"

  battle:
    image: battle-server
    ports:
      - "9000:9000"
```

**한 번에 로그인–매칭–배틀 서버 클러스터가 자동으로 뜨는 구조가 된다.**

### 5) WSL2 성능 최적화
* `.wslconfig`에서 메모리·CPU 제한 최적 조정
* Host 네트워크 모드 사용
* WSL2 내부에서 Docker를 쓰면 파일 I/O 속도가 빨라 개발이 더 편해진다

1. Windows에서 “WSL2 + Docker + Tailscale” 개발 환경 자동 세팅 스크립트 예시
2. Unity 클라이언트에서 자동 서버 선택 로직 예시(C#)
3. Unreal 클라이언트에서 자동 서버 선택 로직 예시(C++ 중심 개념)
4. 모바일 QA용 “자동 연결 앱” 구조/코드 예시

  

### Windows용 완전 자동화 개발 환경 구성 스크립트 예시

#### 1-1. 전제
* Windows 11에서 사용한다고 가정한다
* 목표는 다음과 같다

  * WSL2 켜기
  * Ubuntu 설치
  * Docker Desktop 설치
  * Tailscale 설치
  * 방화벽에 게임 서버 포트/UDP 포트 허용
  * Docker Compose로 멀티 서버 올릴 준비

#### 1-2. PowerShell 스크립트 예시
`setup-dev-env.ps1` 같은 이름으로 만들어 둘 수 있다.

```powershell
# 관리자 권한 PowerShell에서 실행해야 한다

Write-Host "WSL2 및 필수 기능 활성화 중..." -ForegroundColor Cyan

# WSL 및 가상 머신 플랫폼 기능 켜기
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart

# WSL 버전 2를 기본값으로 설정
wsl --set-default-version 2

Write-Host "Ubuntu 설치 (Microsoft Store에서 설치되어 있어야 함) 또는 수동 설치 필요" -ForegroundColor Yellow

# Docker Desktop 설치 (winget 사용, 설치되어 있으면 스킵됨)
Write-Host "Docker Desktop 설치 시도 중..." -ForegroundColor Cyan
winget install -e --id Docker.DockerDesktop -h || Write-Host "이미 설치되어 있거나 실패함" -ForegroundColor Yellow

# Tailscale 설치
Write-Host "Tailscale 설치 시도 중..." -ForegroundColor Cyan
winget install -e --id Tailscale.Tailscale -h || Write-Host "이미 설치되어 있거나 실패함" -ForegroundColor Yellow

# 방화벽 규칙 추가 (예: 게임 서버 포트들)
$ports = @(8000, 8100, 9000, 7000)
foreach ($p in $ports) {
    Write-Host "방화벽에 포트 $p 허용 규칙 추가 중..." -ForegroundColor Cyan
    New-NetFirewallRule -DisplayName "GameServer_$p" -Direction Inbound -Protocol TCP -LocalPort $p -Action Allow -ErrorAction SilentlyContinue
}

# Tailscale UDP 포트 허용
Write-Host "Tailscale UDP 41641 포트 허용 규칙 추가 중..." -ForegroundColor Cyan
New-NetFirewallRule -DisplayName "Tailscale_UDP" -Direction Inbound -Protocol UDP -LocalPort 41641 -Action Allow -ErrorAction SilentlyContinue

Write-Host "기본 설정 완료. 재부팅 후 Ubuntu 초기 설정 및 Docker 설정을 진행해야 한다" -ForegroundColor Green
```

이 스크립트로 기본 골격은 자동으로 깔리게 할 수 있다.

#### 1-3. WSL2 + Docker + Tailscale 조합 구조
추천 구조는 다음과 같다.

* Windows

  * Tailscale 설치 (여기가 100.x.x.x IP를 가짐)
  * 방화벽 규칙 설정
  * Docker Desktop (WSL2 backend 사용)
* WSL2 Ubuntu

  * Docker 엔진
  * 게임 서버 컨테이너들

컨테이너는 다음처럼 포트 매핑을 한다.

```yaml
# docker-compose.yml 예시
version: "3"
services:
  login:
    image: my-login-server
    ports:
      - "8000:8000"

  match:
    image: my-match-server
    ports:
      - "8100:8100"

  battle:
    image: my-battle-server
    ports:
      - "9000:9000"

  chat:
    image: my-chat-server
    ports:
      - "7000:7000"
```

그러면 외부(모바일, 다른 PC, QA)는 모두

* `Tailscale_IP:8000` → 로그인 서버
* `Tailscale_IP:9000` → 배틀 서버

이렇게 붙을 수 있다.


## Unity 클라이언트에서 자동 서버 선택 로직 예시
목표는 다음과 같다.

* 후보 서버 리스트가 여러 개 있다

  * 예: 지역별 / 개발자 PC별 / 환경별
* 클라이언트가

  * 서버들에 Ping 또는 헬스체크를 보내고
  * 응답이 가장 빠른 서버를 선택해서 자동 접속한다

### 1. 데이터 구조 예시

```csharp
[System.Serializable]
public class ServerInfo {
    public string Name;      // 예: "Dev-PC-1"
    public string Host;      // 예: "100.87.12.40"
    public int Port;         // 예: 9000
}
```

### 2. 간단한 자동 선택 스크립트 예시

```csharp
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Networking;

public class AutoServerSelector : MonoBehaviour
{
    public List<ServerInfo> Servers;

    public float TimeoutSeconds = 1.5f;

    void Start()
    {
        StartCoroutine(SelectBestServer());
    }

    IEnumerator SelectBestServer()
    {
        ServerInfo bestServer = null;
        float bestPing = float.MaxValue;

        foreach (var server in Servers)
        {
            string url = $"http://{server.Host}:{server.Port}/health"; 
            // 서버에 간단한 HTTP health 체크 엔드포인트를 두면 좋다

            float startTime = Time.realtimeSinceStartup;

            using (UnityWebRequest req = UnityWebRequest.Get(url))
            {
                req.timeout = Mathf.CeilToInt(TimeoutSeconds);
                yield return req.SendWebRequest();

                if (req.result == UnityWebRequest.Result.Success)
                {
                    float rtt = (Time.realtimeSinceStartup - startTime) * 1000f; // ms
                    Debug.Log($"Server {server.Name} ping = {rtt} ms");

                    if (rtt < bestPing)
                    {
                        bestPing = rtt;
                        bestServer = server;
                    }
                }
                else
                {
                    Debug.LogWarning($"Server {server.Name} check failed: {req.error}");
                }
            }
        }

        if (bestServer != null)
        {
            Debug.Log($"Selected server: {bestServer.Name} {bestServer.Host}:{bestServer.Port} (ping={bestPing}ms)");
            ConnectToGameServer(bestServer);
        }
        else
        {
            Debug.LogError("No reachable server");
        }
    }

    void ConnectToGameServer(ServerInfo server)
    {
        // 여기에 실제 게임 서버 연결 로직을 넣으면 된다
        // 예: TCP 소켓, WebSocket, 커스텀 네트워크 라이브러리 등
    }
}
```

* `Servers` 리스트에 Tailscale IP들을 넣어두면
  개발자 PC가 여러 대일 때도 자동으로 가장 빠른 서버를 선택하게 만들 수 있다.  