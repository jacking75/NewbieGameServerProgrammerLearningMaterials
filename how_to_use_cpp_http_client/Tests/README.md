# 테스트 서버 사양

OS          | CPU      | Memory   | HDD      | SSD  |
:-------:   |:-------: | :------: | :------: | :--: |
Rocky 8.7   | 2        | 8        | 100      | 0    |

테스트 서버 프로세스는 `FakeServer`를 사용한다.

# 테스트 항목
1. 지정한 시간 동안 몇 번의 요청 송신과 응답 수신을 완료했는가.
2. 라이브러리에 메모리 누수가 있는가?
3. 라이브러리가 멀티 스레드에 안전한가?

# 테스트 시나리오
0. 설정한 개수 만큼 스레드 생성 (*멀티 스레드 테스트 전용. 실제 성능 테스트시에는 스레드 1개로 진행*)
1. 설정한 시간 동안 요청을 송신하고 응답을 수신받는다.
2. 테스트 종료 후 [CRT 라이브러리](https://learn.microsoft.com/ko-kr/cpp/c-runtime-library/find-memory-leaks-using-the-crt-library?view=msvc-170)를 통해 메모리 누수 확인. (*메모리 누수 테스트 전용. 실제 성능 테스트시에는 사용되지 않는다.*)

# 테스트 결과

## 메모리 누수 테스트

Library     | Memory Leak |
:-------    | :---------: |
`HappyHTTP` | No          |
`libcurl`   | No          |
`WNetWap`   | Yes         |

## 멀티 스레드 안전성 테스트

Library     | Thread Safe |
:-------    | :---------: |
`HappyHTTP` | Yes         |
`libcurl`   | Yes         | 
`WNetWap`   | No          |

## 성능 테스트

API         | Library    | Test Time(sec)  | Total Count  |
:-------    |:-------    | :-------------: | :----------: |
`AuthCheck` |`HappyHTTP` | 60              | 1974         |
`AuthCheck` |`libcurl`   | 60              | 1533         |
`AuthCheck` |`WNetWap`   | 60              | 1890         |

API          | Library    | Test Time(sec)  | Total Count  |
:-------     |:-------    | :-------------: | :----------: |
`InAppCheck` |`HappyHTTP` | 60              | 1877         |
`InAppCheck` |`libcurl`   | 60              | 1479         |
`InAppCheck` |`WNetWap`   | 60              | 1963         |

## 테스트 프로그램 명령어

### HappyHTTP

#### AuthCheck
```cmd
HappyHTTPTest.exe 183.111.164.120 9090 60 1 1101
```

#### InAppCheck
```cmd
HappyHTTPTest.exe 183.111.164.120 9090 60 1 1102
```

### libcurl

#### AuthCheck
```cmd
libcurlTest.exe http://183.111.164.120:9090/AuthCheck 60 1 1101
```

#### InAppCheck
```cmd
libcurlTest.exe http://183.111.164.120:9090/InAppCheck 60 1 1101
```

### WNetWrap

#### AuthCheck
```cmd
WNetWrapTest.exe http://183.111.164.120:9090/AuthCheck 60 1 1101
```

#### InAppCheck
```cmd
WNetWrapTest.exe http://183.111.164.120:9090/InAppCheck 60 1 1101
```