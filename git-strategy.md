# ORE Fork 유지보수 전략 (git branch / merge 전략)

> `todo.md`의 요청사항에 대한 검토 결과 문서.
> 대상 저장소: `nzmars/Engine` (fork) ← `OpenSourceRisk/Engine` (upstream)
> 작성 기준일: 2026-09-02 / upstream 추종 릴리스: v1.8.16.0

---

## 1. 현재 상태 진단

### 1.1 저장소 구성
| 항목 | 값 |
|---|---|
| upstream | `https://github.com/OpenSourceRisk/Engine.git` (remote 이름 `upstream`) |
| origin | `nzmars/Engine` (개인 fork) |
| 기본 브랜치 | `master` (= `origin/master`) |
| 릴리스 주기 | 약 1~4개월. 태그 형식 `v1.8.X.Y` (예: v1.8.16.0 = 2026-05, v1.8.15.0 = 2026-02) |
| 서브모듈 | `QuantLib`, `ORE-SWIG/QuantLib-SWIG` (OSR fork, upstream 병합 시 함께 이동, `.gitmodules`에 `ignore = dirty`) |

### 1.2 커스터마이징 실측 (footprint)

**커밋된 순수 차이 (`git diff upstream/master...master`)** — 놀랍도록 작음:

```
scripts/  (신규 파일 7개, +306줄)   ← 전부 신규 추가, 충돌 위험 없음
```

과거에 있던 컨벤션 수정(`b2f8ad7ae` KRW ibor), `commonSettings.cmake` 수정은
v1.8.16.0 병합 과정에서 upstream 버전으로 흡수됨 → **현재 커밋된 커스터마이징은 `scripts/`가 전부.**

**아직 커밋되지 않은 작업 (working tree, 장기 방치 중 — 위험):**

| 종류 | 파일 | 충돌 위험 |
|---|---|---|
| 신규 SWIG 인터페이스 | `qle_interpolation.i`, `qle_zerocurve.i`, `qle_piecewiseyieldcurve.i` | 없음 (신규 파일) |
| 기존 SWIG 파일 수정 | `qle.i` (+3줄 `%include`), `qle_common.i` (typemap 추가), `ored_conventions.i` (`CrossCcyBasisSwapConvention` 인자 추가), `ORE-SWIG/setup.py` (링크 라이브러리), `ORE-SWIG/CMakeLists.txt` (공백만) | **중** — upstream이 가끔 건드리는 파일 |
| 빌드 스크립트 재수정 | `scripts/config.cmd`, `build_msvc.bat`, `build_swig.bat` | 낮음 (내 소유 파일) |
| 문서/도구 | `CLAUDE.md`, `todo.md`, `.omc/` | 없음 |

### 1.3 현재 방식의 문제점

1. **커스터마이징이 `master`에 merge 커밋과 뒤섞여 있음.** "내가 바꾼 것"을 한눈에 볼 수 없고, rebase·재적용·upstream 기여가 어려움.
2. **커스터마이징이 수개월째 working tree에 미커밋 상태.** 백업 안 됨, 유실 위험, 다른 머신(Windows/Linux 병행 빌드)과 공유 불가.
3. **`upstream/master`(움직이는 브랜치)를 직접 merge.** 릴리스 태그가 아니라 개발 중간 상태를 받게 됨 → 동기화 시점이 비결정적.
4. **`origin/upstream` 미러 브랜치가 죽어 있음** (`upstream/master` 대비 14,513 커밋 뒤처짐). 방치된 잔재.
5. **`origin` 원격 URL에 GitHub PAT(`ghp_...`)가 평문으로 박혀 있음.** `.git/config`는 추적되지 않지만 자격증명-in-URL 안티패턴 → 로그·백업·화면 공유로 유출 위험.

---

## 2. 목표와 제약

**목표 (todo.md 기준)**
- upstream ORE를 지속적으로 추종(following)한다.
- 내 커스터마이징(`scripts/`, SWIG 바인딩 확장, 일부 컨벤션)을 유지한다.
- 주기적으로 upstream 릴리스를 반영하면서 유지보수 가능해야 한다.

**제약**
- Windows(MSVC)와 Linux 양쪽에서 빌드 → 여러 클론/머신에서 pull. **force-push 기반 워크플로는 마찰이 큼.**
- 커스터마이징 규모가 작고 대부분 **파일 추가(additive)** → 충돌 표면이 좁다.
- 혼자 유지보수 → 복잡한 팀 브랜치 모델 불필요.

---

## 3. 권장 브랜치 구조

```
upstream/master           (remote-tracking, 건드리지 않음)
        │
        ▼  fetch --tags
vendor                     ← upstream 릴리스 태그의 순수 미러. FF-only. 절대 커밋 금지.
        │
        ▼  merge (또는 rebase --onto)
master                     ← 통합 브랜치 = vendor(태그) + 내 커스터마이징. origin/master로 push.
        │
        ├── custom/scripts        (선택) 빌드 스크립트 토픽 브랜치
        └── custom/swig-curves    (선택) SWIG 커브/보간 바인딩 토픽 브랜치
```

| 브랜치 | 역할 | 갱신 방법 |
|---|---|---|
| `vendor` | upstream 릴리스의 **순수 스냅샷**. diff 기준선. | `git fetch upstream --tags` → `git branch -f vendor v1.8.X.Y` (또는 checkout 후 `merge --ff-only`) |
| `master` | 실제 사용/빌드 브랜치. vendor + 커스터마이징. | 아래 §5 절차 |
| `custom/*` | (선택) 커스터마이징을 종류별로 격리. `master`는 이들을 merge만. | 커스터마이징 작업 시 여기서 커밋 |
| `origin/upstream` | **삭제** 또는 `vendor`로 대체. 현재는 죽은 브랜치. | `git push origin --delete upstream` |

> **토픽 브랜치(`custom/*`)는 선택 사항이다.** 커스터마이징이 지금처럼 작으면 `master`에 직접 커밋해도 충분하다.
> 단, "내 패치 = `git log vendor..master`"가 항상 성립하도록 **merge 커밋 외의 upstream 코드는 master에 직접 만들지 않는다.**

---

## 4. 병합 전략: 두 가지 옵션

### 옵션 A — Merge 기반 (권장, 현행 방식의 정식화)

`vendor`(릴리스 태그)를 `master`에 **merge**한다.

```bash
git fetch upstream --tags
git branch -f vendor v1.8.17.0          # 새 릴리스 태그로 vendor 이동
git checkout master
git merge vendor                        # 충돌 해결 → 커밋
```

- **장점**: force-push 불필요. 여러 머신에서 안전하게 pull. 이력이 사실 그대로 보존됨. 현재 하던 것과 거의 동일.
- **단점**: 시간이 지나면 merge 커밋이 쌓여 이력이 지저분함. "내 패치"를 보려면 `git log vendor..master --no-merges` 필요.
- **적합**: 이 저장소. 커스터마이징이 작고, 혼자 쓰고, 멀티 머신이며, upstream 기여 계획이 없을 때.

### 옵션 B — Rebase 기반 (이력은 깨끗, 동기화마다 수작업)

커스터마이징을 최신 태그 위에 **선형 커밋열**로 얹는다.

```bash
git fetch upstream --tags
git rebase --onto v1.8.17.0 v1.8.16.0 master
git push --force-with-lease origin master
```

- **장점**: `git log v1.8.17.0..master`가 정확히 내 패치 세트. upstream PR로 제출하기 쉬움. 이력이 깔끔.
- **단점**: **force-push 필수** → 다른 클론에서 `git reset --hard origin/master` 필요, 멀티 머신 마찰. 동기화 때마다 rebase 충돌.
- **적합**: 커스터마이징을 upstream에 기여하려 하거나, 단일 머신에서만 작업할 때.

### 권장

> **옵션 A(merge)를 기본으로 채택.**
> 이력이 너무 지저분해지면, 그때 한 번씩 옵션 B로 `master`를 `vendor` 위에 rebase하여 정리(squash 포함)하는 "하이브리드"를 쓴다.
> SWIG `.i` 수정처럼 upstream과 겹칠 수 있는 부분은 §7의 격리 기법으로 충돌을 줄인다.

---

## 5. 주기적 upstream 동기화 절차 (체크리스트)

upstream이 새 릴리스 태그(`v1.8.17.0` 등)를 내면:

```bash
# 0. 작업 트리 깨끗하게: 진행 중 커스터마이징은 먼저 커밋
git status --porcelain          # 비어 있어야 함

# 1. upstream 태그 가져오기
git fetch upstream --tags
git tag -l 'v1.8.*' --sort=-creatordate | head    # 새 태그 확인

# 2. 병합 전 스냅샷 (되돌리기용)
git branch backup/pre-v1.8.17.0 master

# 3. vendor를 새 릴리스로 이동
git branch -f vendor v1.8.17.0

# 4. master에 병합
git checkout master
git merge vendor

# 5. 충돌 해결 — 예상 지점은 §6
#    해결 후:
git submodule update --init --recursive   # QuantLib 서브모듈 포인터 동기화
git commit                                 # merge 커밋 메시지에 "merge upstream v1.8.17.0"

# 6. 빌드 검증 (실제 검증은 여기서!)
scripts\build_msvc.bat          # C++ + ore.exe
scripts\build_swig.bat          # Python wheel
#   Linux 머신에서: scripts/build_linux.sh && scripts/build_swig.sh

# 7. 문제 없으면 push
git push origin master vendor
git push origin --tags          # 필요 시

# 8. 백업 브랜치 정리 (2~3 릴리스 후)
git branch -D backup/pre-v1.8.17.0
```

**병합 실패/빌드 깨짐 시 롤백:**
```bash
git merge --abort                          # 병합 도중이면
git reset --hard backup/pre-v1.8.17.0      # 이미 커밋했으면
```

**주기**: upstream 릴리스마다(1~4개월). `master` 개발 브랜치를 따라갈 필요 없음 — 태그만 추종.

---

## 6. 예상 충돌 지점과 대응

| 파일 | 충돌 원인 | 대응 |
|---|---|---|
| `scripts/*` | 없음 (upstream에 없는 파일) | 그대로 유지 |
| `ORE-SWIG/setup.py` | upstream이 버전 문자열·빌드 로직 수정, 나는 링크 라이브러리 목록 수정 | 병합 시 **upstream 로직 + 내 `self.libraries` 블록** 유지. §7처럼 최소 diff로. |
| `ORE-SWIG/QuantExt-SWIG/SWIG/qle.i` | upstream이 `%include` 목록에 새 항목 추가 | 양쪽 `%include` 모두 채택 (거의 항상 자동 병합됨) |
| `qle_common.i` | upstream이 typemap 블록 수정 | 내 `boost::optional<Period>` typemap을 **별도 파일**로 분리(§7) 후 이 파일 원복 |
| `ored_conventions.i` | upstream이 `CrossCcyBasisSwapConvention` 시그니처를 바꿈 (이미 upstream에 유사 기능 들어왔을 가능성 높음) | 병합 시 upstream 버전과 비교. **upstream이 같은 기능을 넣었으면 내 수정 폐기** (KRW 컨벤션처럼 흡수시킴) |
| `QuantLib` 서브모듈 | upstream merge가 서브모듈 SHA 이동 | 항상 upstream 쪽 SHA 채택: `git checkout --theirs QuantLib && git submodule update` |
| `.gitmodules` | URL 변경 (upstream에 `update_submodule_urls.yaml` 워크플로 있음) | upstream 버전 채택 |

**원칙**: upstream이 내 커스터마이징과 동등하거나 더 나은 것을 도입하면 **내 것을 버린다.** 커스터마이징은 최소로 유지하는 게 유지보수 비용을 낮춘다.

---

## 7. 커스터마이징 관리 원칙

### 7.1 충돌 표면 최소화 — "추가는 새 파일로"

기존 upstream 파일을 수정하는 대신 **새 파일 + 최소 훅**으로:

- ✅ `qle_interpolation.i`, `qle_zerocurve.i`, `qle_piecewiseyieldcurve.i` — 이미 잘 하고 있음 (신규 파일)
- ⚠️ `qle_common.i`의 `boost::optional<Period>` typemap → `qle_custom_typemaps.i` 같은 **새 파일로 이동**하고 `qle.i`에서 한 줄 `%include`. 그러면 `qle_common.i`는 원복되어 충돌 사라짐.
- `qle.i`에 `%include` 한 줄 추가 정도는 자동 병합되므로 OK.

### 7.2 커스터마이징 매니페스트

`CUSTOMIZATIONS.md`(신규)에 표로 관리 — 무엇을·왜·어디를 바꿨는지, upstream 흡수 여부 추적.
현재 `CLAUDE.md`에 이미 목록이 있으니, 그걸 단일 출처로 삼고 매 릴리스 병합 후 갱신한다.

### 7.3 커밋 위생

- 커스터마이징은 **작고 의미 단위**로 즉시 커밋 (working tree 방치 금지).
- 커밋 메시지 접두어로 구분: `custom: ...`, `build: ...`, `merge upstream vX.Y.Z`.
- `git log --no-merges vendor..master`가 항상 "내 변경 전체"가 되도록 유지.

---

## 8. 즉시 조치 사항 (우선순위 순)

1. **[보안] `origin` 원격 URL에서 PAT 제거.** ✅ 완료
   ```bash
   git remote set-url origin https://github.com/nzmars/Engine.git   # 실행됨
   ```
   `.git/config`에서 `ghp_...` 토큰이 제거됨. 인증은 이제 Git Credential Manager
   (`credential.helper=manager`)가 처리 — 다음 push 때 브라우저 로그인 창이 한 번 뜸.
   **남은 수동 작업 (브라우저):** GitHub → Settings → Developer settings →
   Personal access tokens → 노출됐던 `ghp_OQsm...` 토큰을 **Delete/Revoke**.
   자세한 인증 옵션은 §9 참고.

2. **[유실 방지] 진행 중 커스터마이징을 지금 커밋.**
   ```bash
   git checkout -b custom/swig-curves
   git add ORE-SWIG/QuantExt-SWIG/SWIG/qle_*.i ORE-SWIG/QuantExt-SWIG/SWIG/qle.i \
           ORE-SWIG/QuantExt-SWIG/SWIG/qle_common.i ORE-SWIG/OREData-SWIG/SWIG/ored_conventions.i \
           ORE-SWIG/setup.py ORE-SWIG/CMakeLists.txt
   git commit -m "custom: expose QuantExt curve/interpolation classes to Python SWIG"
   git add scripts/ CLAUDE.md git-strategy.md
   git commit -m "build: script tweaks + repo docs"
   git checkout master && git merge custom/swig-curves --no-ff
   git push origin master
   ```
   (토픽 브랜치가 부담되면 `master`에 직접 2개 커밋으로 해도 됨.)

3. **[정리] 죽은 미러 브랜치 처리.**
   ```bash
   git push origin --delete upstream        # origin/upstream (14513 커밋 뒤처짐)
   git branch -f vendor v1.8.16.0           # vendor 브랜치 신설 = 현재 추종 태그
   git push origin vendor
   ```

4. **[.gitignore] 로컬 도구 산출물 무시.**
   `.omc/`, `.todo.md.swp`, `bin/`(빌드 산출물이면), `build/` 등이 추적되지 않도록 `.gitignore` 확인·추가.
   `CLAUDE.md` / `git-strategy.md` / `CUSTOMIZATIONS.md`는 **커밋 대상**(fork 유지보수 문서).

5. **[문서화] 이 문서를 저장소에 커밋**하고, `todo.md`는 이 문서를 가리키도록 축약하거나 삭제.

---

## 9. 인증 및 접근 (Authentication & Access)

**`nzmars/Engine`는 public 저장소다.** 따라서 역할별로 필요한 것이 완전히 다르다.

| 역할 | 필요한 인증 | 방법 |
|---|---|---|
| **코드를 받아 빌드/패키징만 하는 사람** | **없음** | `git clone --recurse-submodules https://github.com/nzmars/Engine.git` — 자격증명 불필요. 서브모듈(`QuantLib`, `QuantLib-SWIG`)도 public https URL. §10 참고. |
| **주 개발자(나, push)** | 개인 자격증명 1개 | 아래 A 또는 B 중 하나 |
| **가끔 기여하는 사람(push)** | 각자 개인 자격증명 | 각자 fork 후 PR, 또는 소유자가 collaborator로 추가 후 각자 A/B 설정 |
| **CI / 빌드 서버** | 봇 자격증명 | 저장소 전용 **deploy key**(가능하면 read-only) 또는 CI 시크릿의 fine-grained 토큰. 개인 토큰 금지 |

> **원칙: 자격증명(PAT·SSH 키)을 여러 사람이 공유하지 않는다.** 각자 자기 것으로 인증한다.
> 이전 셋업(하나의 `ghp_...`를 원격 URL에 박아 공유)은 이 원칙 위반이었다.

### 옵션 A — HTTPS + Git Credential Manager (현재 상태, Windows 권장)

```bash
git remote set-url origin https://github.com/nzmars/Engine.git   # 이미 적용됨
```

- 다음 push 때 GCM이 브라우저 로그인 창을 띄우고, 이후 자격증명을 OS 자격증명
  저장소에 안전하게 보관. 2FA·토큰 갱신 자동 처리.
- Git for Windows에 GCM이 기본 포함되어 별도 설치 불필요. `git config --get credential.helper` → `manager` 이면 준비 완료.

### 옵션 B — SSH (멀티 OS·서버에서 균일, 장기적으로 권장)

이미 `~/.ssh/id_ed25519`(+ `.pub`)가 있음. **공개키만 GitHub에 등록하면 됨:**

1. `~/.ssh/id_ed25519.pub` 내용 복사
2. GitHub → Settings → **SSH and GPG keys** → New SSH key → 붙여넣기
3. 원격 전환:
   ```bash
   git remote set-url origin git@github.com:nzmars/Engine.git
   ssh -T git@github.com          # "Hi nzmars!" 나오면 성공
   ```

- 만료 없음, 머신마다 1회 설정. Linux/WSL/headless에서 가장 단순.
- 팀원이 SSH를 부담스러워하면 그 사람만 옵션 A를 써도 됨 (원격 URL은 클론마다
  독립적이고 커밋되지 않음).

---

## 10. 새 체크아웃에서 빌드하기 (다른 사람용)

목표: 아무나 clone → 빌드 → **Python wheel** 생성까지 스크립트 편집 없이.

### 사전 요구사항

| 도구 | 버전 | 비고 |
|---|---|---|
| C++ 컴파일러 | C++20 | Windows: Visual Studio 2022 (v143) / Linux: gcc 12+ 또는 clang 15+ |
| CMake | ≥ 3.15 | |
| Python | 3.x (3.12 테스트됨) | wheel 빌드용. `python3-dev` (Linux) |
| SWIG | 4.x (Windows는 4.3+ 권장) | wheel 빌드에만 필요 |
| Boost | 1.86, **static 빌드** | 또는 Linux에서 vcpkg |
| Ninja | 아무 최신 | Linux 기본 제너레이터 |

### 스크립트가 읽는 환경변수 (전부 선택 — 기본값 있음)

| 변수 | 기본값 | 언제 설정하나 |
|---|---|---|
| `BOOST_ROOT` | `<repo>/../boost_1_86_0` | Boost가 다른 위치일 때 (**보통 이것만 설정**) |
| `SWIG_DIR` | `D:\code\util\swigwin-4.2.1` (Win) | swig가 PATH에 없을 때 (Windows) |
| `CMAKE_DIR` | 관리자 경로 (Win) | cmake가 PATH에 없을 때 (Windows) |
| `CC` / `CXX` | `gcc` / `g++` | 다른 컴파일러를 쓸 때 (Linux) |
| `BUILD_TYPE` | `Release` | `Debug` / `RelWithDebInfo` 원할 때 |
| `BUILD` | `build` | 빌드 디렉터리명 |
| `GENERATOR` | `Visual Studio 17 2022` (Win) / `Ninja` (Linux) | |
| `CPU_N` | 코어 수(Win) / 코어÷2(Linux) | 병렬 빌드 잡 수 |
| `flag_use_vcpkg` | `0` | Linux에서 vcpkg(`../vcpkg`)로 Boost를 쓸 때 `1` |

> 기본 경로(`SWIG_DIR`, `CMAKE_DIR`)는 관리자 머신 값이지만, **해당 디렉터리가
> 없으면 스크립트가 무시하고 PATH의 도구를 쓴다.** 도구가 PATH에 있으면 아무것도
> 설정할 필요 없음.

### Windows

```bat
git clone --recurse-submodules https://github.com/nzmars/Engine.git
cd Engine

REM  Boost가 ..\boost_1_86_0 이 아니면:
set BOOST_ROOT=C:\libs\boost_1_86_0

scripts\build_msvc.bat      REM  C++ 라이브러리 + ore.exe  ->  build\
scripts\build_swig.bat      REM  Python wheel             ->  ORE-SWIG\dist\*.whl
```

### Linux

```bash
git clone --recurse-submodules https://github.com/nzmars/Engine.git
cd Engine

export BOOST_ROOT=/opt/boost_1_86_0        # 필요 시
./scripts/build_linux.sh                   # C++ 라이브러리 + ore
./scripts/build_swig.sh                    # Python wheel -> ORE-SWIG/dist/
```

### 결과물

- `build/` — 정적 라이브러리(`QuantLib`, `QuantExt`, `OREData`, `OREAnalytics`) + `ore` 실행파일
- `ORE-SWIG/dist/open_source_risk_engine-<버전>-cp3XX-*.whl` — `pip install` 가능한 wheel

### 이미 만든 클론이 있다면 (서브모듈 누락 시)

```bash
git submodule update --init --recursive
```

---

## 11. 요약 (TL;DR)

- **브랜치**: `vendor`(upstream 릴리스 태그 순수 미러, 커밋 금지) + `master`(vendor + 커스터마이징, push 대상). 필요 시 `custom/*` 토픽 브랜치.
- **병합**: `upstream/master`가 아니라 **릴리스 태그**를 `vendor`로 받아 `master`에 **merge**(옵션 A). force-push 없음. 이력이 지저분해지면 가끔 rebase로 정리.
- **주기 동기화**: upstream 릴리스마다 §5 체크리스트 (백업 브랜치 → vendor 이동 → merge → 서브모듈 동기화 → 양 OS 빌드 검증 → push).
- **커스터마이징**: 최대한 **신규 파일**로. 기존 파일 수정은 최소 훅만. upstream이 동등 기능 도입하면 내 것 폐기. `CLAUDE.md`를 매니페스트로.
- **인증**: public 저장소라 **빌드만 하는 사람은 인증 불필요**(https clone). push하는 사람만 개인 SSH 키(옵션 B) 또는 HTTPS+GCM(옵션 A). 자격증명 공유 금지.
- **외부 빌드**: 스크립트는 전부 env 오버라이드 가능. 보통 `BOOST_ROOT`만 설정하면 `build_msvc.bat`/`build_linux.sh` → `build_swig.*` 로 wheel 생성. §10.
- **지금 당장**: ① ~~원격 URL PAT 제거~~ ✅ + GitHub에서 토큰 revoke(수동) ② 미커밋 커스터마이징 커밋 ③ `origin/upstream` 삭제 + `vendor` 생성 ④ `.gitignore` 정비.
