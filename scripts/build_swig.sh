#!/bin/bash
# Build the ORE Python wheel (Linux / macOS). Mirror of scripts\build_swig.bat.
# Requires the C++ libraries to be built first (scripts/build_linux.sh) so the
# static libs exist under <repo>/build/.
set -e

_HERE="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd -P)"
. "$_HERE/config.sh"

# macOS ships only 'python3'; allow override with PYTHON=...
PYTHON="${PYTHON:-python3}"
command -v "$PYTHON" >/dev/null || { echo "ERROR: '$PYTHON' not found (set PYTHON=...)" >&2; exit 1; }
command -v swig      >/dev/null || { echo "ERROR: swig not found on PATH" >&2; exit 1; }

cd "$ORE_SWIG_DIR"
echo "BUILD ORESWIG  ($("$PYTHON" --version 2>&1),  $(swig -version | sed -n 2p))"

# setup.py's unix branch calls ./oreanalytics-config, which needs these:
export ORE BOOST_INC BOOST_LIB

"$PYTHON" setup.py wrap
"$PYTHON" setup.py build
"$PYTHON" setup.py bdist_wheel

echo "wheel(s):"
ls -1 "$ORE_SWIG_DIR"/dist/*.whl 2>/dev/null || echo "  (none produced - check output above)"
