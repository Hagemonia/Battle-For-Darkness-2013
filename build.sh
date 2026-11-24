#!/usr/bin/env bash
# Builds BattleForDarkness.exe (Windows) and BattleForDarkness.apk (Android) from game/game.swf.
# Needs Ubuntu/Debian (or WSL). See README.md for the apt packages.
set -euo pipefail
cd "$(dirname "$0")"

RUFFLE_TAG="${RUFFLE_TAG:-nightly-2026-11-24}"          # any tag from github.com/ruffle-rs/ruffle/releases
SWF="${SWF:-game/game.swf}"
KS="keystore/bfd.keystore"
KS_PASS="${KS_PASS:-bfdgame123}"
ANDROID_JAR="${ANDROID_JAR:-/usr/lib/android-sdk/platforms/android-23/android.jar}"

for c in curl unzip python3 javac keytool x86_64-w64-mingw32-gcc aapt zipalign apksigner dalvik-exchange; do
  command -v "$c" >/dev/null || { echo "Missing tool: $c  (see README.md for the apt install line)"; exit 1; }
done
[ -f "$SWF" ]         || { echo "Put your SWF at $SWF first."; exit 1; }
[ -f "$ANDROID_JAR" ] || { echo "android.jar not found at $ANDROID_JAR (apt: android-sdk-platform-23)"; exit 1; }

DATE="${RUFFLE_TAG#nightly-}"; DATE_U="${DATE//-/_}"
BASE="https://github.com/ruffle-rs/ruffle/releases/download/$RUFFLE_TAG"
mkdir -p cache dist build keystore
fetch() { [ -f "cache/$1" ] || { echo "Downloading $1"; curl -fSL -o "cache/$1.part" "$BASE/$2" && mv "cache/$1.part" "cache/$1"; }; }
fetch "ruffle-win-$RUFFLE_TAG.zip" "ruffle-nightly-${DATE_U}-windows-x86_64.zip"
fetch "ruffle-web-$RUFFLE_TAG.zip" "ruffle-nightly-${DATE_U}-web-selfhosted.zip"

rm -rf build/rwin build/rweb build/apk build/win
mkdir -p build/rwin build/rweb
unzip -q -o "cache/ruffle-win-$RUFFLE_TAG.zip" -d build/rwin
unzip -q -o "cache/ruffle-web-$RUFFLE_TAG.zip" -d build/rweb

# ---------- Windows: single-file exe ----------
echo "== Building Windows exe"
mkdir -p build/win
x86_64-w64-mingw32-gcc -O2 -s -mwindows -municode -static -o build/win/stub.exe windows/stub.c
python3 tools/pack_exe.py build/win/stub.exe build/rwin/ruffle.exe "$SWF" dist/BattleForDarkness.exe

# ---------- Android: WebView + Ruffle, fully offline ----------
echo "== Building Android apk"
cp -r android build/apk
cp build/rweb/ruffle.js build/rweb/*.wasm build/apk/assets/
cp "$SWF" build/apk/assets/game.swf
(
  cd build/apk
  mkdir -p out/classes
  javac --release 8 -Xlint:-options -cp "$ANDROID_JAR" -d out/classes src/id/bfd/game/MainActivity.java
  dalvik-exchange --dex --output=out/classes.dex out/classes
  aapt package -f -M AndroidManifest.xml -S res -A assets -I "$ANDROID_JAR" -F out/base.apk
  cp out/base.apk out/unsigned.apk
  (cd out && aapt add unsigned.apk classes.dex >/dev/null)
  zipalign -f -p 4 out/unsigned.apk out/aligned.apk
)
# Same keystore = new builds install over old ones. Keep keystore/ private (it is git-ignored).
[ -f "$KS" ] || keytool -genkeypair -keystore "$KS" -storepass "$KS_PASS" -keypass "$KS_PASS" \
    -alias bfd -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Battle for Darkness" >/dev/null 2>&1
apksigner sign --v4-signing-enabled false --ks "$KS" --ks-pass "pass:$KS_PASS" --key-pass "pass:$KS_PASS" \
    --out dist/BattleForDarkness.apk build/apk/out/aligned.apk
apksigner verify dist/BattleForDarkness.apk >/dev/null && echo "apk signature OK"

echo; ls -lh dist/
