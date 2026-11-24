# Battle for Darkness: pembungkus Windows & Android

Membungkus game Flash (`.swf`) menjadi aplikasi mandiri memakai emulator [Ruffle](https://ruffle.rs):

- **Windows**: satu file `.exe`. Ruffle dan SWF ada di dalamnya, diekstrak otomatis ke `%LOCALAPPDATA%\BattleForDarkness` saat pertama dibuka.
- **Android**: `.apk` berisi WebView + Ruffle. Berjalan offline, layar landscape penuh.

**Repo ini tidak berisi game-nya.** Taruh file SWF milikmu di `game/game.swf` (sudah di-gitignore).

## Struktur

```
android/    sumber app Android (manifest, MainActivity.java, assets/index.html, ikon)
windows/    stub.c, launcher kecil yang mengekstrak Ruffle + SWF lalu menjalankannya
tools/      pack_exe.py, menempelkan Ruffle + SWF ke stub menjadi satu exe
game/       taruh game.swf di sini (tidak ikut di-commit)
build.sh    membuat dist/BattleForDarkness.exe dan dist/BattleForDarkness.apk
```

## Cara build (Ubuntu / Debian / WSL)

```bash
sudo apt-get install -y curl unzip python3 openjdk-17-jdk-headless gcc-mingw-w64-x86-64 \
  aapt apksigner zipalign android-sdk-platform-23 dalvik-exchange

cp /path/ke/battle-for-darkness.swf game/game.swf
./build.sh
```

Hasil ada di `dist/`. Ruffle diunduh otomatis dari GitHub. Versi bisa diganti:
`RUFFLE_TAG=nightly-2026-11-24 ./build.sh`.

Build pertama membuat `keystore/bfd.keystore`. **Simpan dan jangan di-commit.** APK versi berikutnya hanya bisa
meng-update yang terpasang kalau memakai keystore yang sama. Kalau keystore hilang, uninstall dulu app lamanya.

## Fitur app Android

- Tombol **⚙** di pojok kanan atas:
  - **Layar**: Penuh (stretch), Lebar, atau Asli (rasio 800×550).
  - **Grafik**: Cepat, Seimbang, atau Tajam. Pilih *Cepat* kalau FPS patah-patah. Mengganti grafik memuat ulang game.
- Tombol **🎮**: menampilkan tombol virtual (◀ ▶ SPACE P ESC). Game ini sebenarnya cukup dengan tap dan drag.
- Semua link yang dicoba dibuka game (logo, banner) diblokir.

## Catatan

- SWF ini AS3 dan berjalan di 60 FPS. Di HP lemah game bisa berjalan lebih lambat, karena unit digerakkan per frame.
- Ruffle berlisensi MIT / Apache-2.0. Game-nya milik pembuatnya (QiGames / GamesNitro). Jangan unggah SWF, exe, atau apk
  berisi game ke repo publik tanpa izin pembuatnya. Gunakan repo Private, atau hanya publikasikan kode ini.
- Belum diuji di semua perangkat. Versi Ruffle yang dipakai: lihat `RUFFLE_TAG` di `build.sh`.
