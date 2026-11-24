// Single-file launcher: payload (ruffle.exe + game.swf) is appended to this exe.
#include <windows.h>
#include <stdint.h>
#include <string.h>

#define APPDIR L"BattleForDarkness"

#pragma pack(push,1)
typedef struct {
    char     magic[8];      // "BFDPAYLD"
    uint64_t ruffle_off, ruffle_size;
    uint64_t swf_off, swf_size;
} Trailer;
#pragma pack(pop)

static void die(const wchar_t *msg) {
    MessageBoxW(NULL, msg, L"Battle for Darkness", MB_ICONERROR | MB_OK);
    ExitProcess(1);
}

static int same_size(const wchar_t *path, uint64_t size) {
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (!GetFileAttributesExW(path, GetFileExInfoStandard, &d)) return 0;
    uint64_t s = ((uint64_t)d.nFileSizeHigh << 32) | d.nFileSizeLow;
    return s == size;
}

static void extract(HANDLE self, uint64_t off, uint64_t size, const wchar_t *dest) {
    if (same_size(dest, size)) return;
    HANDLE out = CreateFileW(dest, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (out == INVALID_HANDLE_VALUE) die(L"Tidak bisa menulis file ke folder data aplikasi.");
    const DWORD CH = 1 << 20;
    BYTE *buf = (BYTE *)HeapAlloc(GetProcessHeap(), 0, CH);
    uint64_t left = size, pos = off;
    while (left) {
        DWORD want = left > CH ? CH : (DWORD)left, got = 0, wr = 0;
        LARGE_INTEGER li; li.QuadPart = (LONGLONG)pos;
        SetFilePointerEx(self, li, NULL, FILE_BEGIN);
        if (!ReadFile(self, buf, want, &got, NULL) || got != want) die(L"File rusak (payload tidak lengkap).");
        if (!WriteFile(out, buf, got, &wr, NULL) || wr != got) die(L"Gagal menulis file (disk penuh?).");
        left -= got; pos += got;
    }
    HeapFree(GetProcessHeap(), 0, buf);
    CloseHandle(out);
}

int WINAPI wWinMain(HINSTANCE h, HINSTANCE p, LPWSTR cmd, int show) {
    wchar_t self_path[MAX_PATH];
    GetModuleFileNameW(NULL, self_path, MAX_PATH);
    HANDLE self = CreateFileW(self_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (self == INVALID_HANDLE_VALUE) die(L"Tidak bisa membuka file program.");

    LARGE_INTEGER sz; GetFileSizeEx(self, &sz);
    Trailer t; DWORD got = 0;
    LARGE_INTEGER li; li.QuadPart = sz.QuadPart - (LONGLONG)sizeof(Trailer);
    SetFilePointerEx(self, li, NULL, FILE_BEGIN);
    if (!ReadFile(self, &t, sizeof t, &got, NULL) || got != sizeof t || memcmp(t.magic, "BFDPAYLD", 8) != 0)
        die(L"Payload tidak ditemukan di dalam program.");

    wchar_t dir[MAX_PATH];
    DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", dir, MAX_PATH);
    if (!n || n >= MAX_PATH) n = GetTempPathW(MAX_PATH, dir);
    wcscat(dir, L"\\" APPDIR);
    CreateDirectoryW(dir, NULL);

    wchar_t exe[MAX_PATH], swf[MAX_PATH];
    wcscpy(exe, dir); wcscat(exe, L"\\ruffle.exe");
    wcscpy(swf, dir); wcscat(swf, L"\\game.swf");
    extract(self, t.ruffle_off, t.ruffle_size, exe);
    extract(self, t.swf_off, t.swf_size, swf);
    CloseHandle(self);

    wchar_t cl[2 * MAX_PATH + 8];
    wcscpy(cl, L"\""); wcscat(cl, exe); wcscat(cl, L"\" --open-url-mode deny \""); wcscat(cl, swf); wcscat(cl, L"\"");
    STARTUPINFOW si = { sizeof si }; PROCESS_INFORMATION pi;
    if (!CreateProcessW(exe, cl, NULL, NULL, FALSE, 0, NULL, dir, &si, &pi))
        die(L"Gagal menjalankan emulator (ruffle.exe).");
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    return 0;
}
