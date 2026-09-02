# Upstream 동기화 런북 (copy-paste)

`vendor`를 upstream 최신으로 올리고 → `master`에 병합 → push 하는 절차.
배경/전략은 [`git-strategy.md`](git-strategy.md) 참고.

- `upstream` = `github.com/OpenSourceRisk/Engine` (읽기 전용)
- `vendor`  = upstream의 순수 미러 (여기엔 절대 커밋 안 함)
- `master`  = `vendor` + 내 커스터마이징 (= `origin/master`, push 대상)

마지막 실행: 2026-09-03, `vendor` → `upstream/master @ 3b62ba248` (post-v1.8.16.0).

---

## 0. 사전 준비 (1회)

git push 인증이 깨져 있으면(`Invalid username or token`), `gh` CLI 토큰을 git이
쓰도록 한 번만 설정:

```bash
gh auth status            # 'Logged in ... nzmars' 확인
gh auth setup-git         # git이 gh 토큰으로 push하도록 영구 설정
```

> 설정 없이 1회성으로만 쓰려면 아래 모든 `git push` 앞에 이 옵션을 붙인다:
> `git -c credential.helper= -c credential.helper='!gh auth git-credential' push ...`

remote 확인:

```bash
git remote -v
#  origin    https://github.com/nzmars/Engine.git
#  upstream  https://github.com/OpenSourceRisk/Engine.git
# 없으면:  git remote add upstream https://github.com/OpenSourceRisk/Engine.git
```

---

## 1. 동기화 절차

```bash
# ── 1) 작업 트리 깨끗한지 확인 (아무 출력 없어야 함) ───────────────────
git status --porcelain

# ── 2) upstream 받아오기 ─────────────────────────────────────────────
git fetch upstream --tags --prune
git log --oneline vendor..upstream/master        # 뭐가 새로 들어오는지 확인
git tag -l 'v1.8.*' --sort=-creatordate | head   # 새 릴리스 태그 있나 확인

#    태그가 "would clobber existing tag"로 거부되면 (upstream이 태그를 옮긴 경우):
#    git fetch upstream --tags --force

# ── 3) 되돌리기용 백업 브랜치 ────────────────────────────────────────
git branch backup/pre-upstream-$(date +%Y%m%d) master

# ── 4) vendor를 upstream 최신으로 이동 ──────────────────────────────
#    (A) upstream/master HEAD를 따라감 (지금까지의 방식):
git branch -f vendor upstream/master
#    (B) 새 릴리스 태그로 고정하고 싶으면 대신:
#    git branch -f vendor v1.8.17.0

git push origin vendor                            # origin/vendor 갱신 (백업/참조용)

# ── 5) master에 병합 ────────────────────────────────────────────────
git checkout master
git merge --no-edit vendor
#    커밋 메시지는 나중에 정리하고 싶으면:
#    git commit --amend -m "merge upstream (upstream/master @ <sha>, post-vX.Y.Z)"

#    충돌 나면 → 아래 "충돌 처리" 참고, 해결 후 git add <파일> && git commit

# ── 6) 서브모듈 포인터 동기화 (upstream이 QuantLib을 올렸을 수 있음) ──
git submodule update --init --recursive
git submodule status                              # QuantLib SHA가 병합된 값과 일치하는지

# ── 7) 빌드 검증 (여기서 실제로 확인!) ──────────────────────────────
#    Windows:
scripts\build_msvc.bat
scripts\build_swig.bat
#    Linux/macOS:
./scripts/build_linux.sh && ./scripts/build_swig.sh

# ── 8) 문제 없으면 push ─────────────────────────────────────────────
git push origin master

# ── 9) 2~3 릴리스 지난 뒤 오래된 백업 정리 ─────────────────────────
git branch -D backup/pre-upstream-YYYYMMDD
```

---

## 2. 충돌 처리

내 커스터마이징은 대부분 신규 파일이라 충돌은 드물다. 나는 경우:

| 파일 | 처리 |
|---|---|
| `scripts/*` | 내 것 유지 (upstream에 없음) |
| `ORE-SWIG/setup.py` | upstream 로직 + 내 `self.libraries` 블록 둘 다 살림 |
| `ORE-SWIG/QuantExt-SWIG/SWIG/qle.i` | 양쪽 `%include` 모두 채택 (보통 자동 병합) |
| `qle_common.i` | 내 typemap을 별도 파일로 빼두면 충돌 안 남 (`git-strategy.md` §7) |
| `ored_conventions.i` | upstream이 같은 기능 넣었으면 내 수정 폐기 |
| `QuantLib` (서브모듈) | 항상 upstream 것: `git checkout --theirs QuantLib && git add QuantLib` |
| `.gitmodules` | upstream 버전 채택 |

원칙: **upstream이 내 커스터마이징과 같거나 나은 걸 도입하면 내 것을 버린다.**

---

## 3. 롤백

```bash
# 병합 도중 (아직 커밋 안 함)
git merge --abort

# 이미 커밋했지만 push 전
git reset --hard backup/pre-upstream-YYYYMMDD
git submodule update --init --recursive

# 이미 push한 뒤 되돌리려면 (강제 push 필요, 다른 클론 주의)
git reset --hard backup/pre-upstream-YYYYMMDD
git push --force-with-lease origin master
```

---

## 4. 병합 후 확인용 명령

```bash
git log --oneline --no-merges vendor..master   # = upstream 대비 내 변경 전체
git rev-list --left-right --count vendor...master
git submodule status
```
