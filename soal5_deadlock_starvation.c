/* UTS Sistem Operasi - Soal 5: Deadlock dan Starvation
   Nama: Divya Humairoh | NIM: 25076035 
   X=3, Y=5, D = 100+20X = 160 ms, W = 200+20Y = 300 ms */
   
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NIM "25076035"
DWORD D, W;
HANDLE ev[2];
volatile LONG berhenti;

struct Dl {                 /* data satu thread (deadlock / pencegahan) */
    int id, sinkron;
    HANDLE pertama, kedua;
    char *np, *nk;
    DWORD batas, tunggu;
    int ok1, ok2;
};

DWORD WINAPI dl(LPVOID p) {
    struct Dl *t = (struct Dl *)p;
    DWORD s = GetTickCount();
    if (WaitForSingleObject(t->pertama, 5000) != WAIT_OBJECT_0) return 0;
    t->ok1 = 1;
    t->tunggu = GetTickCount() - s;
    if (t->sinkron) {                      /* pastikan dua thread sudah pegang resource pertama */
        SetEvent(ev[t->id]);
        WaitForSingleObject(ev[1 - t->id], 3000);
    }
    Sleep(D);                              /* bekerja dengan resource pertama */
    s = GetTickCount();
    t->ok2 = (WaitForSingleObject(t->kedua, t->batas) == WAIT_OBJECT_0);
    t->tunggu += GetTickCount() - s;
    if (t->ok2) { Sleep(D); ReleaseMutex(t->kedua); }
    ReleaseMutex(t->pertama);
    return 0;
}

/* pencegahan = 0: Thread2 kunci B lalu A (deadlock). pencegahan = 1: semua kunci A lalu B */
void jalankan(int pencegahan) {
    HANDLE A = CreateMutex(NULL, FALSE, NULL), B = CreateMutex(NULL, FALSE, NULL), th[2];
    struct Dl t[2];
    int i, sukses = 0;
    ev[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    ev[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    memset(t, 0, sizeof(t));
    t[0].id = 0; t[0].pertama = A; t[0].kedua = B; t[0].np = "A"; t[0].nk = "B";
    t[1].id = 1;
    if (pencegahan) { t[1].pertama = A; t[1].kedua = B; t[1].np = "A"; t[1].nk = "B"; }
    else            { t[1].pertama = B; t[1].kedua = A; t[1].np = "B"; t[1].nk = "A"; }
    for (i = 0; i < 2; i++) {
        t[i].sinkron = !pencegahan;
        t[i].batas = pencegahan ? 5000 : W;   /* batas tunggu agar tidak menggantung */
    }
    for (i = 0; i < 2; i++) th[i] = CreateThread(NULL, 0, dl, &t[i], 0, NULL);
    WaitForMultipleObjects(2, th, TRUE, 15000);
    printf("%-7s %-9s %-8s %-8s %-11s %s\n", "Thread", "Urutan", "Dapat1", "Dapat2", "Tunggu(ms)", "Status");
    for (i = 0; i < 2; i++) {
        printf("%-7d %s->%-6s %-8s %-8s %-11lu %s\n", i + 1, t[i].np, t[i].nk,
               t[i].ok1 ? "ya" : "tidak", t[i].ok2 ? "ya" : "tidak", t[i].tunggu,
               t[i].ok2 ? "SELESAI" : "TIMEOUT -> DEADLOCK");
        sukses += t[i].ok2;
        CloseHandle(th[i]);
    }
    printf("Thread selesai: %d dari 2 -> %s\n", sukses, sukses == 2 ? "AMAN (tanpa deadlock)" : "DEADLOCK");
    CloseHandle(A); CloseHandle(B); CloseHandle(ev[0]); CloseHandle(ev[1]);
}

struct St { int id, rakus, sukses, coba; HANDLE res; DWORD tunggu; };

DWORD WINAPI starve(LPVOID p) {
    struct St *t = (struct St *)p;
    DWORD mulai = GetTickCount();
    while (!berhenti) {
        t->coba++;
        if (WaitForSingleObject(t->res, 0) == WAIT_OBJECT_0) {   /* coba ambil tanpa menunggu */
            t->tunggu += GetTickCount() - mulai;
            t->sukses++;
            Sleep(D);
            ReleaseMutex(t->res);
            mulai = GetTickCount();
            if (!t->rakus) Sleep(50);
        } else Sleep(t->rakus ? 0 : 50);   /* rakus langsung coba lagi, sopan tidur dulu */
    }
    t->tunggu += GetTickCount() - mulai;
    return 0;
}

void starvation(void) {
    HANDLE res = CreateMutex(NULL, FALSE, NULL), th[4];
    struct St s[4];
    int i;
    berhenti = 0;
    memset(s, 0, sizeof(s));
    for (i = 0; i < 4; i++) { s[i].id = i + 1; s[i].rakus = (i == 0); s[i].res = res; }
    for (i = 0; i < 4; i++) th[i] = CreateThread(NULL, 0, starve, &s[i], 0, NULL);
    Sleep(W);                              /* waktu pengamatan */
    berhenti = 1;
    WaitForMultipleObjects(4, th, TRUE, 3000);
    printf("%-7s %-7s %-10s %-9s %-11s %s\n", "Thread", "Tipe", "Percobaan", "Berhasil", "Tunggu(ms)", "Status");
    for (i = 0; i < 4; i++) {
        printf("%-7d %-7s %-10d %-9d %-11lu %s\n", s[i].id, s[i].rakus ? "rakus" : "sopan",
               s[i].coba, s[i].sukses, s[i].tunggu, s[i].sukses ? "berjalan" : "STARVATION");
        CloseHandle(th[i]);
    }
    CloseHandle(res);
    printf("Semua thread dihentikan, resource dilepas.\n");
}

int main(void) {
    int X, Y, pilih;
    char buf[16];
    X = NIM[strlen(NIM) - 2] - '0';
    Y = NIM[strlen(NIM) - 1] - '0';
    D = 100 + 20 * X;
    W = 200 + 20 * Y;
    printf("UTS SISTEM OPERASI - SOAL 5\nNama: Divya Humairoh\nNIM : %s\n", NIM);
    printf("X=%d Y=%d D=%lu ms W=%lu ms\n", X, Y, D, W);
    for (;;) {
        printf("\n1. Deadlock\n2. Pencegahan deadlock (urutan kunci A lalu B)\n3. Starvation\n");
        printf("4. Semua + perbandingan\n0. Keluar\nPilih: ");
        if (!fgets(buf, sizeof(buf), stdin)) break;
        pilih = atoi(buf);
        if (pilih == 0) break;
        if (pilih == 1 || pilih == 4) { printf("\n[SEBELUM pencegahan: deadlock]\n"); jalankan(0); }
        if (pilih == 2 || pilih == 4) { printf("\n[SESUDAH pencegahan: urutan kunci konsisten]\n"); jalankan(1); }
        if (pilih == 3 || pilih == 4) { printf("\n[STARVATION, 4 thread 1 resource]\n"); starvation(); }
    }
    return 0;
}
