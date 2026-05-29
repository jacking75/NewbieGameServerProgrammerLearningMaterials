# 바이브 코딩을 위한 Git Work Tree 배우기  

저자: 최흥배, Claude AI   
    
-----  

# Git Worktree에 대한 완전 가이드
Git Worktree는 하나의 저장소에서 여러 작업 디렉토리를 동시에 관리할 수 있게 해주는 강력한 기능이다. 특히 바이브 코딩(Vibe Coding)처럼 빠른 실험과 프로토타이핑이 필요한 작업에 매우 유용하다.

## 🎯 Git Worktree란?
**전통적인 방식의 문제점:**
- 브랜치를 전환할 때마다 `git checkout`으로 파일들이 바뀜
- 여러 기능을 동시에 작업하려면 매번 stash하거나 commit해야 함
- 빌드나 테스트를 실행 중인데 다른 브랜치로 전환할 수 없음

**Worktree의 해결책:**
- 하나의 Git 저장소를 여러 폴더에서 동시에 체크아웃
- 각 폴더는 서로 다른 브랜치를 가질 수 있음
- `.git` 디렉토리는 공유하지만 작업 디렉토리는 독립적
  

## 실습 1: 첫 Worktree 만들기
```bash
# 기존 저장소에서
cd my-project

# 새로운 worktree 생성 (새 브랜치와 함께)
git worktree add ../my-project-feature feature/new-feature

# 기존 브랜치로 worktree 생성
git worktree add ../my-project-hotfix hotfix/bug-123

# 생성된 worktree 확인
git worktree list
```

명령어별 상세 설명  

### 1. cd my-project

```bash
cd my-project
```

이 명령어는 기존 Git 저장소 디렉토리로 이동하는 기본적인 쉘 명령어이다. `my-project`라는 이름의 디렉토리가 이미 Git 저장소로 초기화되어 있어야 하며, 이 디렉토리가 **메인 worktree**(주 작업 트리)가 된다. 여기서 새로운 worktree들을 생성하게 된다.
  

### 2. git worktree add ../my-project-feature feature/new-feature

```bash
git worktree add ../my-project-feature feature/new-feature
```

이 명령어는 새로운 worktree를 생성하는 핵심 명령어이다. 각 부분을 분해해서 살펴보겠다:

- **git worktree add**: worktree를 추가하는 Git 명령어이다.

- **../my-project-feature**: 새로운 worktree가 생성될 디렉토리 경로이다. `..`는 상위 디렉토리를 의미하므로, 현재 `my-project` 디렉토리의 형제 디렉토리로 `my-project-feature`가 생성된다.

- **feature/new-feature**: 이 worktree에서 체크아웃할 브랜치 이름이다.

**동작 방식:**  
이 명령어를 실행하면 Git은 다음과 같은 작업을 수행한다:

1. `../my-project-feature` 경로에 새로운 디렉토리를 생성한다.
2. `feature/new-feature`라는 이름의 브랜치가 이미 존재하면 해당 브랜치를 체크아웃한다.
3. 만약 `feature/new-feature` 브랜치가 존재하지 않으면, 현재 브랜치(HEAD)를 기준으로 새로운 브랜치를 생성하고 체크아웃한다.
4. 해당 디렉토리에서 즉시 작업을 시작할 수 있다.

**디렉토리 구조 예시:**

```
parent-directory/
├── my-project/              # 메인 worktree (기존 저장소)
│   ├── .git/               # Git 저장소 데이터
│   └── ...
└── my-project-feature/      # 새로 생성된 worktree
    ├── .git                # worktree 링크 파일 (디렉토리 아님)
    └── ...
```

### 3. git worktree add ../my-project-hotfix hotfix/bug-123

```bash
git worktree add ../my-project-hotfix hotfix/bug-123
```

이 명령어는 위의 명령어와 동일한 방식으로 작동하지만, 다른 브랜치와 경로를 사용한다:

- **../my-project-hotfix**: 핫픽스 작업을 위한 별도의 worktree 디렉토리가 생성된다.

- **hotfix/bug-123**: 버그 수정을 위한 브랜치이다. 이미 원격 저장소에 `hotfix/bug-123` 브랜치가 존재한다면 해당 브랜치를 체크아웃하고, 없다면 새로 생성한다.

**최종 디렉토리 구조:**

```
parent-directory/
├── my-project/              # 메인 worktree
├── my-project-feature/      # feature 브랜치 worktree
└── my-project-hotfix/       # hotfix 브랜치 worktree
```

  
## 실습 2: Worktree 구조 이해하기
```bash
# 메인 저장소
ls -la my-project/.git  # 실제 .git 디렉토리

# Worktree 저장소
ls -la my-project-feature/.git  # 심볼릭 링크 파일
cat my-project-feature/.git  # 실제 .git 위치를 가리킴
```

이 실습 코드는 **메인 저장소와 worktree의 `.git`이 근본적으로 다르다**는 것을 보여주는 내용이다.

### 핵심 개념
일반적인 Git 저장소에서 `.git`은 **디렉토리(폴더)** 이다. 하지만 worktree에서 `.git`은 **파일** 이다. 이 차이가 전체 구조를 이해하는 핵심이다.

### 명령어 하나씩 뜯어보기

#### 1. ls -la my-project/.git

```bash
ls -la my-project/.git
```

**의미:** 메인 저장소의 `.git`을 자세히 보여주세요.

**결과:** 많은 파일과 폴더들이 나온다.

```
drwxr-xr-x  HEAD
drwxr-xr-x  objects/
drwxr-xr-x  refs/
drwxr-xr-x  hooks/
drwxr-xr-x  worktrees/
...
```

**의미 해석:**

- 첫 글자가 `d`로 시작하는 것들은 디렉토리(폴더)이다.
- `.git`은 실제 **디렉토리**이며, 안에 Git의 모든 데이터가 저장된다.
- 커밋 히스토리, 브랜치 정보, 설정 등 **진짜 Git 저장소의 모든 것**이 여기 있다.

#### 2. ls -la my-project-feature/.git

```bash
ls -la my-project-feature/.git
```

**의미:** worktree의 `.git`을 자세히 보여주세요.

**결과:** 딱 하나의 파일만 나온다.

```
-rw-r--r--  1 user  staff  58 Dec 30 10:00 .git
```

**의미 해석:**

- 첫 글자가 `-`로 시작한다. 이것은 **파일**이라는 뜻이다.
- `.git`이 디렉토리가 아니라 **단순한 텍스트 파일**이다.
- 실제 Git 데이터가 여기 없고, 어딘가를 **가리키는 링크** 역할만 한다.

#### 3. cat my-project-feature/.git

```bash
cat my-project-feature/.git
```

**의미:** worktree의 `.git` 파일 내용을 읽어서 화면에 출력하세요.

**결과:** 텍스트 한 줄이 나온다.

```
gitdir: /absolute/path/to/my-project/.git/worktrees/my-project-feature
```

**의미 해석:**

- 이 파일 안에는 딱 한 줄의 텍스트만 들어있다.
- `gitdir:` 다음에 나오는 경로는 **진짜 Git 데이터가 있는 위치**를 알려준다.
- 즉, "실제 Git 정보는 메인 저장소의 `.git/worktrees/` 안에 있으니 거기를 봐라"는 뜻이다.

### 전체 구조 시각화
실제로 어떻게 연결되어 있는지 그림으로 이해해보자:

```
parent-directory/
│
├── my-project/                          # 메인 저장소
│   ├── .git/                           # 진짜 디렉토리 ✓
│   │   ├── objects/                    # 모든 커밋 데이터
│   │   ├── refs/                       # 모든 브랜치 정보
│   │   ├── HEAD                        # 메인의 현재 브랜치
│   │   ├── config                      # 설정
│   │   └── worktrees/                  # worktree 관리 폴더
│   │       └── my-project-feature/     # feature worktree 정보
│   │           ├── HEAD                # feature의 현재 브랜치
│   │           ├── gitdir              # worktree 위치 정보
│   │           └── ...
│   └── (소스 코드 파일들)
│
└── my-project-feature/                  # Worktree
    ├── .git                            # 단순한 텍스트 파일 ✓
    │                                   # 내용: "gitdir: ../my-project/.git/worktrees/my-project-feature"
    └── (소스 코드 파일들)
```

### 왜 이렇게 만들었을까?
Git이 이런 구조를 사용하는 이유는 **효율성과 일관성** 때문이다:

#### 데이터 중복 방지
만약 worktree마다 완전히 독립적인 `.git` 디렉토리를 만든다면, 커밋 히스토리, 객체 데이터 등이 모두 복사되어 엄청난 디스크 공간을 낭비하게 된다. 대신 모든 worktree가 하나의 메인 `.git` 디렉토리를 공유하므로, 데이터는 한 곳에만 저장된다.

#### 동기화 문제 해결
한 worktree에서 커밋을 만들면, 그 정보가 즉시 다른 모든 worktree에서도 보인다. 왜냐하면 모두 같은 데이터베이스(메인 `.git`)를 바라보고 있기 때문이다.

#### 간단한 참조 방식
worktree의 `.git` 파일은 단순히 "진짜 데이터는 저기 있어"라고 가리키기만 하면 된다. Git 명령어들은 이 파일을 읽고 자동으로 올바른 위치를 찾아간다.

### 실제로 확인해보기   
직접 실습해보면 더 명확하게 이해할 수 있다:

```bash
# 메인 저장소의 .git 크기 확인 (디렉토리)
du -sh my-project/.git
# 결과: 15M  (실제 Git 데이터 크기)

# worktree의 .git 크기 확인 (파일)
du -sh my-project-feature/.git
# 결과: 4K   (텍스트 파일 하나의 크기)

# worktree의 .git 내용 직접 읽기
cat my-project-feature/.git
# 결과: gitdir: /Users/username/parent-directory/my-project/.git/worktrees/my-project-feature
```

### 요약  
이 실습이 보여주는 핵심은 다음과 같다:

**메인 저장소 (my-project):**
- `.git`은 **디렉토리**
- 모든 Git 데이터가 실제로 저장되는 곳
- 용량이 큼

**Worktree (my-project-feature):**
- `.git`은 **파일**
- 메인 저장소의 `.git/worktrees/` 경로를 가리키는 링크
- 용량이 매우 작음 (수십 바이트)

이런 구조 덕분에 여러 worktree를 만들어도 디스크 공간을 거의 사용하지 않으면서, 모든 worktree가 동일한 Git 히스토리와 데이터를 공유할 수 있다.  


## 패턴 1: 동시 다발적 기능 개발
```bash
# 메인 개발
cd my-project
git checkout main

# 기능 A 개발 (별도 폴더)
git worktree add ../feature-a feature/user-auth

# 기능 B 개발 (별도 폴더)
git worktree add ../feature-b feature/payment

# 이제 3개 폴더에서 동시 작업 가능!
# my-project (main)
# feature-a (feature/user-auth)
# feature-b (feature/payment)
```

## 패턴 2: 급한 핫픽스 처리
```bash
# 현재 feature 브랜치에서 작업 중
cd my-project-feature
# 빌드 실행 중... 전환할 수 없음!

# 새 worktree로 즉시 핫픽스 작업
cd ../my-project
git worktree add ../hotfix hotfix/critical-bug
cd ../hotfix
# 핫픽스 작업 후 배포
git add . && git commit -m "Fix critical bug"
git push origin hotfix/critical-bug
```

## 패턴 3: 리뷰와 테스트
```bash
# PR 리뷰를 위한 임시 worktree
git worktree add ../review-pr-123 feature/pull-request-123

# 테스트 실행
cd ../review-pr-123
npm install
npm test

# 리뷰 완료 후 제거
cd ..
git worktree remove review-pr-123
```

## Worktree 관리 명령어
```bash
# 모든 worktree 상태 확인
git worktree list

# Worktree 제거
git worktree remove ../feature-a
# 또는
rm -rf ../feature-a
git worktree prune  # 고아 worktree 정리

# Worktree 이동
git worktree move ../feature-a ../new-location/feature-a

# 잠긴 worktree (실수로 삭제 방지)
git worktree lock ../important-feature
git worktree unlock ../important-feature
```

## 바이브 코딩을 위한 워크플로우
```bash
# 1. 실험용 worktree 빠르게 생성
git worktree add -b experiment/quick-test ../exp-$(date +%s) main

# 2. 작업 후 결과가 좋으면
cd ../exp-xxxxx
git add . && git commit -m "Successful experiment"
git push origin experiment/quick-test

# 3. 결과가 나쁘면
cd ..
git worktree remove exp-xxxxx  # 그냥 버리기
```

## 실습 프로젝트: 간단한 웹 앱 동시 개발

```bash
# 초기 설정
mkdir vibe-coding-demo && cd vibe-coding-demo
git init
echo "# Demo Project" > README.md
git add . && git commit -m "Initial commit"

# 프론트엔드 작업
git worktree add ../frontend feature/frontend
cd ../frontend
# React/Vue 개발...

# 백엔드 작업
cd ../vibe-coding-demo
git worktree add ../backend feature/backend
cd ../backend
# Node.js/Python 개발...

# 문서 작업
cd ../vibe-coding-demo
git worktree add ../docs feature/documentation
cd ../docs
# 문서 작성...

# 각 터미널에서 동시 실행
# 터미널 1: cd frontend && npm run dev
# 터미널 2: cd backend && npm start
# 터미널 3: cd docs && mkdocs serve
```
  
## ⚠️ 주의사항

1. **같은 브랜치는 한 번만 체크아웃 가능**
   ```bash
   # 에러 발생
   git worktree add ../test main  # main이 이미 체크아웃됨
   ```

2. **디스크 공간 고려**
   - 각 worktree는 전체 소스코드 복사본을 가짐
   - node_modules 등은 각각 설치 필요

3. **Worktree 제거 전 커밋 확인**
   ```bash
   cd my-worktree
   git status  # uncommitted 변경사항 확인
   ```  