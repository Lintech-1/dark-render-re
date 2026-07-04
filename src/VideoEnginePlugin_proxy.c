typedef unsigned long DWORD;
typedef int BOOL;
typedef void *HANDLE;
typedef void *HMODULE;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef void *LPVOID;

int _fltused = 0;

#ifndef ENABLE_DARK_RENDER_RE_LOGS
#define ENABLE_DARK_RENDER_RE_LOGS 0
#endif

#define TRUE 1
#define FALSE 0
#define WINAPI __stdcall
#define THISCALL __thiscall

typedef struct STARTUPINFOA {
    DWORD cb;
    LPSTR lpReserved;
    LPSTR lpDesktop;
    LPSTR lpTitle;
    DWORD dwX;
    DWORD dwY;
    DWORD dwXSize;
    DWORD dwYSize;
    DWORD dwXCountChars;
    DWORD dwYCountChars;
    DWORD dwFillAttribute;
    DWORD dwFlags;
    unsigned short wShowWindow;
    unsigned short cbReserved2;
    unsigned char *lpReserved2;
    HANDLE hStdInput;
    HANDLE hStdOutput;
    HANDLE hStdError;
} STARTUPINFOA;

typedef struct PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION;

typedef struct SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES;

__declspec(dllimport) HMODULE WINAPI LoadLibraryA(LPCSTR);
__declspec(dllimport) void *WINAPI GetProcAddress(HMODULE, LPCSTR);
__declspec(dllimport) BOOL WINAPI FreeLibrary(HMODULE);
__declspec(dllimport) BOOL WINAPI CreateProcessA(LPCSTR, LPSTR, void *, void *, BOOL, DWORD, LPVOID, LPCSTR, STARTUPINFOA *, PROCESS_INFORMATION *);
__declspec(dllimport) BOOL WINAPI TerminateProcess(HANDLE, unsigned int);
__declspec(dllimport) BOOL WINAPI GetExitCodeProcess(HANDLE, DWORD *);
__declspec(dllimport) DWORD WINAPI WaitForSingleObject(HANDLE, DWORD);
__declspec(dllimport) BOOL WINAPI CloseHandle(HANDLE);
__declspec(dllimport) BOOL WINAPI CreatePipe(HANDLE *, HANDLE *, SECURITY_ATTRIBUTES *, DWORD);
__declspec(dllimport) BOOL WINAPI SetHandleInformation(HANDLE, DWORD, DWORD);
__declspec(dllimport) BOOL WINAPI ReadFile(HANDLE, void *, DWORD, DWORD *, void *);
__declspec(dllimport) HANDLE WINAPI CreateFileA(LPCSTR, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
__declspec(dllimport) BOOL WINAPI WriteFile(HANDLE, const void *, DWORD, DWORD *, void *);
__declspec(dllimport) HANDLE WINAPI CreateThread(void *, DWORD, DWORD (WINAPI *)(void *), void *, DWORD, DWORD *);
__declspec(dllimport) BOOL WINAPI SetPriorityClass(HANDLE, DWORD);
__declspec(dllimport) BOOL WINAPI SetThreadPriority(HANDLE, int);
__declspec(dllimport) HANDLE WINAPI GetCurrentThread(void);
__declspec(dllimport) void WINAPI Sleep(DWORD);
__declspec(dllimport) DWORD WINAPI GetLastError(void);
__declspec(dllimport) void *WINAPI GetProcessHeap(void);
__declspec(dllimport) void *WINAPI HeapAlloc(void *, DWORD, unsigned long);
__declspec(dllimport) BOOL WINAPI HeapFree(void *, DWORD, void *);
__declspec(dllimport) short WINAPI GetAsyncKeyState(int);
__declspec(dllimport) DWORD WINAPI GetTickCount(void);

static HMODULE g_orig;
static char g_last_video[520];
static char g_audio_path[520];
static void *g_tex;
static int g_tex_w;
static int g_tex_h;
static HANDLE g_ffmpeg_process;
static HANDLE g_ffmpeg_thread;
static HANDLE g_ffmpeg_stdout;
static HANDLE g_ffmpeg_stdin;
static HANDLE g_audio_process;
static int g_audio_bypass_started;
static int g_audio_bypass_done;
static int g_audio_bypass_video;
static unsigned long g_audio_bypass_start_tick;
static int g_ffmpeg_prepared;
static int g_ffmpeg_reader_started;
static unsigned char *g_frame;
static unsigned long g_frame_size;
static volatile long g_frame_ready;
static volatile long g_stop_worker;
static volatile long g_ffmpeg_eof;
static volatile long g_frames_read;
static unsigned long g_upload_pause_until;
static unsigned long g_update_failures;
static int g_combo_key_down;
static int g_engine_reset_state;
static unsigned char *g_screen_needs_reset;
static unsigned char *g_rendering_suspended;
static unsigned long g_update_calls;
static unsigned long g_update_ok;

#define GENERIC_READ 0x80000000u
#define GENERIC_WRITE 0x40000000u
#define FILE_APPEND_DATA 0x00000004u
#define FILE_SHARE_READ 0x00000001u
#define FILE_SHARE_WRITE 0x00000002u
#define CREATE_ALWAYS 2u
#define OPEN_EXISTING 3u
#define OPEN_ALWAYS 4u
#define CREATE_NO_WINDOW 0x08000000u
#define IDLE_PRIORITY_CLASS 0x00000040u
#define THREAD_PRIORITY_BELOW_NORMAL -1
#define INVALID_HANDLE_VALUE ((HANDLE)(-1))
#define HANDLE_FLAG_INHERIT 0x00000001u
#define STARTF_USESTDHANDLES 0x00000100u
#define VK_RETURN 0x0du
#define VK_MENU 0x12u

typedef void *(THISCALL *CtorFn)(void *);
typedef void (THISCALL *VoidFn)(void *);
typedef unsigned char (THISCALL *BoolFn)(void *);
typedef unsigned char (THISCALL *InitVideoFn)(void *, char *, float);
typedef unsigned char (THISCALL *SetVideoTargetFn)(void *, void *, int, int, int, int);
typedef void (THISCALL *GetVideoDimensionsFn)(void *, int *, int *);
typedef float (THISCALL *FloatFn)(void *);
typedef void *(__stdcall *CreateObjectFn)(void);
typedef void *(__stdcall *GetClassTypeIdFn)(void);
typedef void *(__stdcall *GetEnginePluginFn)(void);

static void strcopy(char *dst, int cap, const char *src);

static int streq_suffix(const char *s, const char *suffix)
{
    int ls = 0, lf = 0;
    while (s && s[ls]) ls++;
    while (suffix[lf]) lf++;
    if (ls < lf) return 0;
    for (int i = 0; i < lf; ++i) {
        char a = s[ls - lf + i];
        char b = suffix[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (b >= 'A' && b <= 'Z') b += 32;
        if (a != b) return 0;
    }
    return 1;
}

static void audio_path_from_video(char *dst, int cap, const char *video)
{
    int n = 0;
    strcopy(dst, cap, video);
    while (dst[n]) n++;
    if (n >= 4 && streq_suffix(dst, ".wmv")) {
        dst[n - 3] = 'w';
        dst[n - 2] = 'a';
        dst[n - 1] = 'v';
    }
}

static int should_bypass_audio(void)
{
    const char *name = g_last_video;
    const char *p = g_last_video;
    const char exact_intro[] = "intro.wmv";
    int exact = 1;
    int i;
    while (*p) {
        if (*p == '\\' || *p == '/') name = p + 1;
        p++;
    }
    if (!streq_suffix(g_last_video, ".wmv")) return 0;
    for (i = 0; exact_intro[i] || name[i]; ++i) {
        char a = name[i];
        char b = exact_intro[i];
        if (a >= 'A' && a <= 'Z') a += 32;
        if (b >= 'A' && b <= 'Z') b += 32;
        if (a != b) {
            exact = 0;
            break;
        }
    }
    if (exact) return 0;
    return 1;
}

static void strcopy(char *dst, int cap, const char *src)
{
    int i = 0;
    if (!dst || cap <= 0) return;
    if (src) {
        for (; i < cap - 1 && src[i]; ++i) dst[i] = src[i];
    }
    dst[i] = 0;
}

static void strappend(char *dst, int cap, const char *src)
{
    int i = 0;
    while (i < cap && dst[i]) i++;
    if (i >= cap) return;
    for (int j = 0; i < cap - 1 && src && src[j]; ++i, ++j) dst[i] = src[j];
    dst[i] = 0;
}

static void append_hex8(char *dst, int cap, unsigned long value)
{
    static const char h[] = "0123456789abcdef";
    strappend(dst, cap, "0x");
    for (int i = 7; i >= 0; --i) {
        char one[2];
        one[0] = h[(value >> (i * 4)) & 0xf];
        one[1] = 0;
        strappend(dst, cap, one);
    }
}

static void append_u32(char *dst, int cap, unsigned long value)
{
    char tmp[16];
    int n = 0;
    if (value == 0) {
        strappend(dst, cap, "0");
        return;
    }
    while (value && n < (int)sizeof(tmp)) {
        tmp[n++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (n > 0) {
        char one[2];
        one[0] = tmp[--n];
        one[1] = 0;
        strappend(dst, cap, one);
    }
}

static void append_i32(char *dst, int cap, int value)
{
    if (value < 0) {
        strappend(dst, cap, "-");
        append_u32(dst, cap, (unsigned long)(-value));
    } else {
        append_u32(dst, cap, (unsigned long)value);
    }
}

static void zero_mem(void *ptr, int bytes)
{
    unsigned char *p = (unsigned char *)ptr;
    for (int i = 0; i < bytes; ++i) p[i] = 0;
}

static int str_len(const char *s)
{
    int n = 0;
    while (s && s[n]) n++;
    return n;
}

#if ENABLE_DARK_RENDER_RE_LOGS
static void log_line(const char *s)
{
    HANDLE f;
    DWORD written;
    f = CreateFileA("dark_video_proxy.log", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
    if (f == INVALID_HANDLE_VALUE) return;
    WriteFile(f, s, (DWORD)str_len(s), &written, 0);
    WriteFile(f, "\r\n", 2, &written, 0);
    CloseHandle(f);
}

static void log_num(const char *prefix, unsigned long value)
{
    char buf[256];
    strcopy(buf, sizeof(buf), prefix);
    append_u32(buf, sizeof(buf), value);
    log_line(buf);
}

static void log_hex(const char *prefix, unsigned long value)
{
    char buf[256];
    strcopy(buf, sizeof(buf), prefix);
    append_hex8(buf, sizeof(buf), value);
    log_line(buf);
}
#else
static void log_line(const char *s)
{
    (void)s;
}

static void log_num(const char *prefix, unsigned long value)
{
    (void)prefix;
    (void)value;
}

static void log_hex(const char *prefix, unsigned long value)
{
    (void)prefix;
    (void)value;
}
#endif

static HMODULE orig(void)
{
    if (!g_orig) g_orig = LoadLibraryA("VideoEnginePlugin.orig.vPlugin");
    return g_orig;
}

static void *sym(const char *name)
{
    HMODULE m = orig();
    return m ? GetProcAddress(m, name) : 0;
}

typedef unsigned char (THISCALL *UpdateRectFn)(void *, int, int, int, int, int, int, const void *, int, int);

static UpdateRectFn update_rect(void)
{
    HMODULE m = LoadLibraryA("vBase100.dll");
    return m ? (UpdateRectFn)GetProcAddress(m, "?UpdateRect@VTextureObject@@QAE_NHHHHHHPBXHH@Z") : 0;
}

static unsigned char *vbase_bool_symbol(const char *name)
{
    HMODULE m = LoadLibraryA("vBase100.dll");
    return m ? (unsigned char *)GetProcAddress(m, name) : 0;
}

static int engine_reset_active(void)
{
    if (!g_screen_needs_reset) {
        g_screen_needs_reset = vbase_bool_symbol("?m_bScreenNeedsReset@VVideo@@2_NA");
        if (g_screen_needs_reset) log_line("hook m_bScreenNeedsReset OK");
        else log_hex("hook m_bScreenNeedsReset missing err=", GetLastError());
    }
    if (!g_rendering_suspended) {
        g_rendering_suspended = vbase_bool_symbol("?m_bRenderingIsSuspended@VVideo@@1_NA");
        if (g_rendering_suspended) log_line("hook m_bRenderingIsSuspended OK");
        else log_hex("hook m_bRenderingIsSuspended missing err=", GetLastError());
    }
    if (g_screen_needs_reset && *g_screen_needs_reset) return 1;
    if (g_rendering_suspended && *g_rendering_suspended) return 1;
    return 0;
}

static int reset_guard_active(void)
{
    unsigned long now;
    int active = engine_reset_active();
    if (active) {
        now = GetTickCount();
        g_upload_pause_until = now + 120u;
        if (!g_engine_reset_state) log_line("engine reset begin");
        g_engine_reset_state = 1;
        return 1;
    }
    if (g_engine_reset_state) {
        g_engine_reset_state = 0;
        log_line("engine reset end");
    }
    return 0;
}

static DWORD WINAPI ffmpeg_reader(void *unused)
{
    DWORD got;
    DWORD exit_code;
    unsigned long pos;
    unsigned long next_frame_time;
    (void)unused;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    next_frame_time = GetTickCount();
    while (!g_stop_worker && g_ffmpeg_stdout && g_frame && g_frame_size) {
        pos = 0;
        while (pos < g_frame_size && !g_stop_worker) {
            got = 0;
            if (!ReadFile(g_ffmpeg_stdout, g_frame + pos, g_frame_size - pos, &got, 0) || got == 0) {
                log_hex("ReadFile failed/EOF err=", GetLastError());
                log_num("partial pos=", pos);
                exit_code = 0;
                if (g_ffmpeg_process) {
                    DWORD wait = WaitForSingleObject(g_ffmpeg_process, 100);
                    log_hex("ffmpeg wait=", wait);
                }
                if (g_ffmpeg_process && GetExitCodeProcess(g_ffmpeg_process, &exit_code)) {
                    log_hex("ffmpeg exit=", exit_code);
                } else {
                    log_hex("GetExitCodeProcess failed err=", GetLastError());
                }
                g_stop_worker = 1;
                g_ffmpeg_eof = 1;
                break;
            }
            pos += got;
        }
        if (pos == g_frame_size) {
            g_frame_ready = 1;
            g_frames_read++;
            next_frame_time += 40u;
            while (!g_stop_worker && (long)(next_frame_time - GetTickCount()) > 0) {
                Sleep(1);
            }
        }
    }
    log_line("reader exit");
    if (g_ffmpeg_stdout) {
        CloseHandle(g_ffmpeg_stdout);
        g_ffmpeg_stdout = 0;
    }
    if (g_ffmpeg_process) {
        WaitForSingleObject(g_ffmpeg_process, 0);
        exit_code = 0;
        if (GetExitCodeProcess(g_ffmpeg_process, &exit_code)) log_hex("reader final ffmpeg exit=", exit_code);
    }
    return 0;
}

static void stop_ffmpeg(void)
{
    HANDLE thread;
    if (!g_ffmpeg_process && !g_ffmpeg_stdout && !g_ffmpeg_thread && !g_frame) return;
    log_line("stop_ffmpeg");
    g_stop_worker = 1;
    if (g_ffmpeg_process) {
        TerminateProcess(g_ffmpeg_process, 0);
    }
    if (g_ffmpeg_stdout) {
        CloseHandle(g_ffmpeg_stdout);
        g_ffmpeg_stdout = 0;
    }
    if (g_ffmpeg_stdin) {
        CloseHandle(g_ffmpeg_stdin);
        g_ffmpeg_stdin = 0;
    }
    if (g_ffmpeg_thread) {
        thread = g_ffmpeg_thread;
        WaitForSingleObject(thread, 500);
        CloseHandle(thread);
        g_ffmpeg_thread = 0;
    }
    if (g_ffmpeg_process) {
        CloseHandle(g_ffmpeg_process);
        g_ffmpeg_process = 0;
    }
    if (g_frame) {
        HeapFree(GetProcessHeap(), 0, g_frame);
        g_frame = 0;
    }
    g_frame_size = 0;
    g_frame_ready = 0;
    g_ffmpeg_eof = 0;
    g_frames_read = 0;
    g_upload_pause_until = 0;
    g_update_failures = 0;
    g_combo_key_down = 0;
    g_engine_reset_state = 0;
    g_ffmpeg_prepared = 0;
    g_ffmpeg_reader_started = 0;
}

static void poll_display_toggle(void)
{
    unsigned long now;
    short alt = GetAsyncKeyState(VK_MENU);
    short enter = GetAsyncKeyState(VK_RETURN);
    int combo = ((alt & 0x8000) && ((enter & 0x8000) || (enter & 1))) ? 1 : 0;
    if (combo && !g_combo_key_down) {
        engine_reset_active();
        now = GetTickCount();
        if (g_screen_needs_reset || g_rendering_suspended) {
            g_upload_pause_until = now + 250u;
            log_line("upload pause: Alt+Enter combo fast");
        } else {
            g_upload_pause_until = now + 1200u;
            log_line("upload pause: Alt+Enter combo fallback");
        }
    }
    g_combo_key_down = combo;
}

static int upload_paused(void)
{
    unsigned long now;
    if (!g_upload_pause_until) return 0;
    now = GetTickCount();
    if ((long)(g_upload_pause_until - now) > 0) return 1;
    g_upload_pause_until = 0;
    log_line("upload resume");
    return 0;
}

static void start_ffmpeg_reader(void)
{
    DWORD tid = 0;
    if (g_ffmpeg_reader_started || !g_ffmpeg_stdout || !g_frame || !g_frame_size) return;
    g_stop_worker = 0;
    g_ffmpeg_thread = CreateThread(0, 0, ffmpeg_reader, 0, 0, &tid);
    g_ffmpeg_reader_started = 1;
    log_hex("CreateThread handle=", (unsigned long)g_ffmpeg_thread);
}

static void stop_audio_bypass(void)
{
    if (g_audio_process) {
        TerminateProcess(g_audio_process, 0);
        CloseHandle(g_audio_process);
        g_audio_process = 0;
    }
    g_audio_bypass_started = 0;
    g_audio_bypass_done = 0;
    g_audio_bypass_video = 0;
    g_audio_bypass_start_tick = 0;
}

static int audio_bypass_finished(void)
{
    DWORD exit_code;
    if (!g_audio_bypass_started) return 0;
    if (g_audio_bypass_done) return 1;
    if (!g_audio_process) return 0;
    exit_code = 0;
    if (!GetExitCodeProcess(g_audio_process, &exit_code)) return 0;
    if (exit_code == 259u) return 0;
    CloseHandle(g_audio_process);
    g_audio_process = 0;
    g_audio_bypass_done = 1;
    log_hex("ffplay exit=", exit_code);
    return 1;
}

static void start_audio_bypass(void)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    char cmd[900];
    HANDLE nul = 0;
    HANDLE err_file = 0;

    if (!g_audio_bypass_video || g_audio_process) return;

    nul = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS, 0, 0);
#if ENABLE_DARK_RENDER_RE_LOGS
    err_file = CreateFileA("dark_audio_ffplay.log", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
    if (err_file == INVALID_HANDLE_VALUE) err_file = 0;
#endif
    zero_mem(&si, sizeof(si));
    zero_mem(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = 0;
    si.hStdOutput = nul;
    si.hStdError = err_file ? err_file : nul;

    strcopy(cmd, sizeof(cmd), "\"ffmpeg\\bin\\ffplay.exe\" -nodisp -autoexit -loglevel quiet \"");
    strappend(cmd, sizeof(cmd), g_audio_path);
    strappend(cmd, sizeof(cmd), "\"");
    log_line(cmd);
    if (!CreateProcessA(0, cmd, 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
        log_hex("CreateProcess ffplay failed err=", GetLastError());
        if (nul) CloseHandle(nul);
        if (err_file) CloseHandle(err_file);
        return;
    }
    log_line("CreateProcess ffplay OK");
    if (nul) CloseHandle(nul);
    if (err_file) CloseHandle(err_file);
    g_audio_process = pi.hProcess;
    g_audio_bypass_started = 1;
    g_audio_bypass_done = 0;
    g_audio_bypass_start_tick = GetTickCount();
    CloseHandle(pi.hThread);
}

static void prepare_ffmpeg(void)
{
    SECURITY_ATTRIBUTES sa;
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    HANDLE read_pipe = 0;
    HANDLE write_pipe = 0;
    HANDLE stdin_read = 0;
    HANDLE stdin_write = 0;
    HANDLE err_file = 0;
    char cmd[1600];

    if (!g_last_video[0] || !g_tex_w || !g_tex_h) return;
    if (g_ffmpeg_prepared) return;
    log_line("prepare_ffmpeg");
    log_line(g_last_video);
    log_num("tex_w=", (unsigned long)g_tex_w);
    log_num("tex_h=", (unsigned long)g_tex_h);
    stop_ffmpeg();

    g_frame_size = (unsigned long)g_tex_w * (unsigned long)g_tex_h * 4u;
    g_frame = (unsigned char *)HeapAlloc(GetProcessHeap(), 0, g_frame_size);
    if (!g_frame) {
        log_line("HeapAlloc failed");
        return;
    }
    log_num("frame_size=", g_frame_size);

    zero_mem(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) {
        log_hex("CreatePipe failed err=", GetLastError());
        stop_ffmpeg();
        return;
    }
    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
    if (CreatePipe(&stdin_read, &stdin_write, &sa, 0)) {
        SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0);
        g_ffmpeg_stdin = stdin_write;
    } else {
        log_hex("CreatePipe stdin failed err=", GetLastError());
    }
#if ENABLE_DARK_RENDER_RE_LOGS
    err_file = CreateFileA("dark_video_ffmpeg.log", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, CREATE_ALWAYS, 0, 0);
    if (err_file == INVALID_HANDLE_VALUE) {
        log_hex("CreateFile ffmpeg log failed err=", GetLastError());
        err_file = 0;
    }
#else
    err_file = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa, OPEN_ALWAYS, 0, 0);
    if (err_file == INVALID_HANDLE_VALUE) err_file = 0;
#endif

    zero_mem(&si, sizeof(si));
    zero_mem(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdin_read;
    si.hStdOutput = write_pipe;
    si.hStdError = err_file;

    strcopy(cmd, sizeof(cmd), "\"ffmpeg\\bin\\ffmpeg.exe\" -hide_banner -loglevel error -nostdin -threads 1 -filter_threads 1 -filter_complex_threads 1 -i \"");
    strappend(cmd, sizeof(cmd), g_last_video);
    strappend(cmd, sizeof(cmd), "\" -map 0:v:0 -an -sn -dn -vf ");
    if (g_tex_w == 1280 && g_tex_h == 720) {
        strappend(cmd, sizeof(cmd), "fps=25");
    } else {
        strappend(cmd, sizeof(cmd), "scale=");
        append_i32(cmd, sizeof(cmd), g_tex_w);
        strappend(cmd, sizeof(cmd), ":");
        append_i32(cmd, sizeof(cmd), g_tex_h);
        strappend(cmd, sizeof(cmd), ":flags=fast_bilinear,fps=25");
    }
    strappend(cmd, sizeof(cmd), " -pix_fmt bgra -f rawvideo pipe:1");
    log_line(cmd);

    if (!CreateProcessA(0, cmd, 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi)) {
        log_hex("CreateProcess ffmpeg failed err=", GetLastError());
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        if (stdin_read) CloseHandle(stdin_read);
        if (stdin_write) CloseHandle(stdin_write);
        g_ffmpeg_stdin = 0;
        if (err_file) CloseHandle(err_file);
        stop_ffmpeg();
        return;
    }
    log_line("CreateProcess ffmpeg OK");
    SetPriorityClass(pi.hProcess, IDLE_PRIORITY_CLASS);
    CloseHandle(write_pipe);
    if (stdin_read) CloseHandle(stdin_read);
    if (err_file) CloseHandle(err_file);
    g_ffmpeg_stdout = read_pipe;
    g_ffmpeg_process = pi.hProcess;
    CloseHandle(pi.hThread);
    g_ffmpeg_prepared = 1;
}

void *THISCALL Proxy_ctor(void *self)
{
    CtorFn f = (CtorFn)sym("??0VideoEngineRenderer@@QAE@XZ");
    return f ? f(self) : self;
}

void THISCALL Proxy_dtor(void *self)
{
    VoidFn f = (VoidFn)sym("??1VideoEngineRenderer@@UAE@XZ");
    if (f) f(self);
}

void *WINAPI Proxy_CreateObject(void)
{
    CreateObjectFn f = (CreateObjectFn)sym("?CreateObject@VideoEngineRenderer@@SAPAVVTypedObject@@XZ");
    return f ? f() : 0;
}

void THISCALL Proxy_DeInit(void *self)
{
    VoidFn f = (VoidFn)sym("?DeInit@VideoEngineRenderer@@QAEXXZ");
    stop_audio_bypass();
    stop_ffmpeg();
    if (f) f(self);
}

BOOL THISCALL Proxy_FinishedPlaying(void *self)
{
    BoolFn f = (BoolFn)sym("?FinishedPlaying@VideoEngineRenderer@@QAE_NXZ");
    BOOL done = (f && f(self)) ? TRUE : FALSE;
    if (g_audio_bypass_video && g_audio_bypass_started && g_audio_bypass_start_tick) {
        unsigned long elapsed = GetTickCount() - g_audio_bypass_start_tick;
        int audio_done = audio_bypass_finished();
        if (!done && elapsed > 1000u && audio_done) {
            log_line("audio bypass ended: force FinishedPlaying");
            {
                VoidFn stop = (VoidFn)sym("?Stop@VideoEngineRenderer@@QAEXXZ");
                if (stop) {
                    log_line("audio bypass: original Stop before finish");
                    stop(self);
                }
            }
            stop_audio_bypass();
            stop_ffmpeg();
            return TRUE;
        }
    }
    return done;
}

void *WINAPI Proxy_GetClassTypeId(void)
{
    GetClassTypeIdFn f = (GetClassTypeIdFn)sym("?GetClassTypeId@VideoEngineRenderer@@SAPAUVType@@XZ");
    return f ? f() : 0;
}

float THISCALL Proxy_GetCurrentPosition(void *self)
{
    FloatFn f = (FloatFn)sym("?GetCurrentPosition@VideoEngineRenderer@@QAEMXZ");
    return f ? f(self) : 0.0f;
}

float THISCALL Proxy_GetTickCount(void *self)
{
    FloatFn f = (FloatFn)sym("?GetTickCount@VideoEngineRenderer@@QAEMXZ");
    return f ? f(self) : 0.0f;
}

float THISCALL Proxy_GetTotalTime(void *self)
{
    FloatFn f = (FloatFn)sym("?GetTotalTime@VideoEngineRenderer@@QAEMXZ");
    return f ? f(self) : 0.0f;
}

void *THISCALL Proxy_GetTypeId(void *self)
{
    typedef void *(THISCALL *Fn)(void *);
    Fn f = (Fn)sym("?GetTypeId@VideoEngineRenderer@@UBEPAUVType@@XZ");
    return f ? f(self) : 0;
}

void THISCALL Proxy_GetVideoDimensions(void *self, int *w, int *h)
{
    GetVideoDimensionsFn f = (GetVideoDimensionsFn)sym("?GetVideoDimensions@VideoEngineRenderer@@QAEXAAH0@Z");
    if (f) f(self, w, h);
}

BOOL THISCALL Proxy_InitVideo(void *self, char *path, float volume)
{
    InitVideoFn f = (InitVideoFn)sym("?InitVideo@VideoEngineRenderer@@QAE_NPADM@Z");
    g_audio_bypass_done = 0;
    g_audio_bypass_started = 0;
    g_audio_bypass_video = 0;
    g_audio_bypass_start_tick = 0;
    strcopy(g_last_video, sizeof(g_last_video), path);
    if (path && !streq_suffix(path, ".wmv")) strappend(g_last_video, sizeof(g_last_video), ".wmv");
    audio_path_from_video(g_audio_path, sizeof(g_audio_path), g_last_video);
    g_audio_bypass_video = should_bypass_audio();
    log_line("InitVideo");
    log_line(g_last_video);
    return (f && f(self, path, volume)) ? TRUE : FALSE;
}

BOOL THISCALL Proxy_IsPlaying(void *self)
{
    BoolFn f = (BoolFn)sym("?IsPlaying@VideoEngineRenderer@@QAE_NXZ");
    return (f && f(self)) ? TRUE : FALSE;
}

BOOL THISCALL Proxy_SetVideoTarget(void *self, void *tex, int left, int top, int right, int bottom)
{
    SetVideoTargetFn f = (SetVideoTargetFn)sym("?SetVideoTarget@VideoEngineRenderer@@QAE_NPAVVTextureObject@@UtagRECT@@@Z");
    g_tex = tex;
    if (tex) {
        g_tex_w = (int)*(short *)((unsigned char *)tex + 0x4a);
        g_tex_h = (int)*(short *)((unsigned char *)tex + 0x4c);
    }
    if (g_tex_w <= 0) g_tex_w = right - left;
    if (g_tex_h <= 0) g_tex_h = bottom - top;
    log_line("SetVideoTarget");
    log_num("w=", (unsigned long)g_tex_w);
    log_num("h=", (unsigned long)g_tex_h);
    {
        BOOL ok = (f && f(self, tex, left, top, right, bottom)) ? TRUE : FALSE;
        prepare_ffmpeg();
        return ok;
    }
}

void THISCALL Proxy_StartVideo(void *self)
{
    VoidFn f = (VoidFn)sym("?StartVideo@VideoEngineRenderer@@QAEXXZ");
    log_line("StartVideo");
    prepare_ffmpeg();
    start_ffmpeg_reader();
    start_audio_bypass();
    if (f) f(self);
}

void THISCALL Proxy_Stop(void *self)
{
    VoidFn f = (VoidFn)sym("?Stop@VideoEngineRenderer@@QAEXXZ");
    stop_audio_bypass();
    stop_ffmpeg();
    if (f) f(self);
}

BOOL THISCALL Proxy_UpdateTexture(void *self)
{
    (void)self;
    g_update_calls++;
    poll_display_toggle();
    if (reset_guard_active()) return TRUE;
    if (upload_paused()) return TRUE;
    if (g_ffmpeg_eof) return TRUE;
    if (g_tex && g_frame && g_frame_ready && g_tex_w > 0 && g_tex_h > 0) {
        UpdateRectFn fn = update_rect();
        if (fn) {
            BOOL ok = fn(g_tex, 0, 0, 0, 0, 0, -1, g_frame, 1, 0) ? TRUE : FALSE;
            if (!ok) {
                g_update_failures++;
                log_num("UpdateRect failed=", g_update_failures);
                g_upload_pause_until = GetTickCount() + 350u;
                return TRUE;
            }
            else {
                g_update_ok++;
            }
            return ok;
        }
        log_line("UpdateRect sym missing");
    }
    return TRUE;
}

void *classVideoEngineRenderer;

void *WINAPI Proxy_GetEnginePlugin(void)
{
    GetEnginePluginFn f = (GetEnginePluginFn)sym("GetEnginePlugin");
    return f ? f() : 0;
}

void *WINAPI Proxy_GetEnginePlugin_VideoEnginePlugin(void)
{
    GetEnginePluginFn f = (GetEnginePluginFn)sym("GetEnginePlugin_VideoEnginePlugin");
    return f ? f() : 0;
}

BOOL WINAPI DllMain(HMODULE h, DWORD reason, LPVOID reserved)
{
    if (reason == 0) {
        stop_audio_bypass();
        stop_ffmpeg();
        if (g_orig) {
            FreeLibrary(g_orig);
            g_orig = 0;
        }
    }
    return TRUE;
}
