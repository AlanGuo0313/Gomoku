#define UNICODE
#define _UNICODE
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <gdiplus.h>

#include <cmath>
#include <string>

#include "gomoku_game.h"

namespace {

// ── 版面設定：棋盤座標與滑鼠座標共用，調整版面時只需改這裡。 ──
constexpr int GRID_SPACING = 36;
constexpr int BOARD_LEFT = 64;
constexpr int BOARD_TOP = 144;
constexpr int CLIENT_WIDTH = 952;
constexpr int CLIENT_HEIGHT = 730;
constexpr int HIT_RADIUS = 16;
constexpr int BUTTON_RESTART = 101;
constexpr int BUTTON_UNDO = 102;
constexpr int BUTTON_EXIT = 103;

Gomoku game;
HFONT titleFont = nullptr;
HFONT bodyFont = nullptr;
HWND undoButton = nullptr;
int hoverRow = -1;
int hoverCol = -1;

int boardX(int col) {
    return BOARD_LEFT + col * GRID_SPACING;
}

int boardY(int row) {
    return BOARD_TOP + row * GRID_SPACING;
}

// ── 基本繪圖工具 ──

void drawText(HDC dc, const std::wstring& value, RECT rect, HFONT font) {
    HGDIOBJ previousFont = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(48, 42, 35));
    DrawTextW(dc, value.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(dc, previousFont);
}

Gdiplus::Color toGraphicsColor(COLORREF color) {
    return Gdiplus::Color(255, GetRValue(color), GetGValue(color), GetBValue(color));
}

void drawCircle(HDC dc, int x, int y, int radius, COLORREF fill, COLORREF border) {
    Gdiplus::Graphics graphics(dc);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    Gdiplus::SolidBrush brush(toGraphicsColor(fill));
    Gdiplus::Pen pen(toGraphicsColor(border));

    graphics.FillEllipse(&brush, x - radius, y - radius, radius * 2, radius * 2);
    graphics.DrawEllipse(&pen, x - radius, y - radius, radius * 2, radius * 2);
}

void drawPanel(HDC dc, int x, int y, int width, int height, COLORREF fill) {
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, fill);
    HGDIOBJ previousBrush = SelectObject(dc, brush);
    HGDIOBJ previousPen = SelectObject(dc, pen);

    RoundRect(dc, x, y, x + width, y + height, 20, 20);

    SelectObject(dc, previousBrush);
    SelectObject(dc, previousPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void drawStone(HDC dc, int x, int y, int player) {
    using namespace Gdiplus;
    const bool isBlack = player == Gomoku::BLACK;
    Graphics graphics(dc);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    SolidBrush shadow(Color(55, 55, 35, 18));
    graphics.FillEllipse(&shadow, x - 14, y - 12, 32, 32);

    LinearGradientBrush fill(
        Point(x - 10, y - 15), Point(x + 12, y + 16),
        isBlack ? Color(255, 92, 99, 106) : Color(255, 255, 255, 255),
        isBlack ? Color(255, 17, 22, 27) : Color(255, 211, 216, 211)
    );
    Pen border(isBlack ? Color(255, 22, 28, 33) : Color(255, 171, 177, 169));
    graphics.FillEllipse(&fill, x - 15, y - 15, 30, 30);
    graphics.DrawEllipse(&border, x - 15, y - 15, 30, 30);
}

// ── 畫面內容：棋盤、棋子、右側資訊分開繪製。 ──

void drawBoard(HDC dc) {
    drawPanel(dc, 32, 116, 568, 570, RGB(211, 217, 204)); // 棋盤陰影
    drawPanel(dc, 28, 110, 568, 570, RGB(191, 146, 89));  // 木色外框
    drawPanel(dc, 36, 118, 552, 552, RGB(229, 195, 144));

    HPEN grainPen = CreatePen(PS_SOLID, 1, RGB(225, 189, 136));
    HGDIOBJ previousPen = SelectObject(dc, grainPen);
    for (int y = 124; y < 668; y += 7) {
        MoveToEx(dc, 42, y, nullptr);
        LineTo(dc, 582, y + 2);
    }
    SelectObject(dc, previousPen);
    DeleteObject(grainPen);

    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(151, 115, 69));
    previousPen = SelectObject(dc, gridPen);
    for (int index = 0; index < Gomoku::BOARD_SIZE; ++index) {
        MoveToEx(dc, boardX(0), boardY(index), nullptr);
        LineTo(dc, boardX(Gomoku::BOARD_SIZE - 1) + 1, boardY(index));
        MoveToEx(dc, boardX(index), boardY(0), nullptr);
        LineTo(dc, boardX(index), boardY(Gomoku::BOARD_SIZE - 1) + 1);
    }
    SelectObject(dc, previousPen);
    DeleteObject(gridPen);

    for (int index = 0; index < Gomoku::BOARD_SIZE; ++index) {
        const std::wstring columnLabel(1, static_cast<wchar_t>(L'A' + index));
        drawText(dc, columnLabel, {boardX(index) - 6, 120, boardX(index) + 15, 140}, bodyFont);
        drawText(dc, std::to_wstring(index + 1),
                 {39, boardY(index) - 12, 63, boardY(index) + 12}, bodyFont);
    }

    constexpr int starPoints[5][2] = {{3, 3}, {3, 11}, {7, 7}, {11, 3}, {11, 11}};
    for (const auto& point : starPoints) {
        drawCircle(dc, boardX(point[1]), boardY(point[0]), 3,
                   RGB(115, 83, 43), RGB(115, 83, 43));
    }
}

void drawStones(HDC dc) {
    if (!game.isFinished() && game.isValidMove(hoverRow, hoverCol)) {
        drawCircle(dc, boardX(hoverCol), boardY(hoverRow), 12,
                   RGB(202, 180, 137), RGB(115, 137, 111));
    }

    for (int row = 0; row < Gomoku::BOARD_SIZE; ++row) {
        for (int col = 0; col < Gomoku::BOARD_SIZE; ++col) {
            const int player = game.stoneAt(row, col);
            if (player != Gomoku::EMPTY) {
                drawStone(dc, boardX(col), boardY(row), player);
            }
        }
    }

    if (!game.history().empty()) {
        const auto& lastMove = game.history().back();
        drawCircle(dc, boardX(lastMove.col), boardY(lastMove.row), 3,
                   RGB(203, 91, 64), RGB(203, 91, 64));
    }
}

std::wstring gameStatus() {
    if (!game.isFinished()) {
        return game.currentPlayer() == Gomoku::BLACK ? L"黑棋落子" : L"白棋落子";
    }
    if (game.winner() == Gomoku::BLACK) {
        return L"黑棋獲勝";
    }
    return game.winner() == Gomoku::WHITE ? L"白棋獲勝" : L"本局平手";
}

void drawSidebar(HDC dc) {
    drawPanel(dc, 624, 110, 300, 190, RGB(255, 255, 251));
    drawText(dc, game.isFinished() ? L"本局結果" : L"目前回合",
             {648, 126, 900, 158}, bodyFont);

    if (!game.isFinished() || game.winner() != Gomoku::EMPTY) {
        const int player = game.winner() != Gomoku::EMPTY
            ? game.winner() : game.currentPlayer();
        drawStone(dc, 668, 192, player);
    }
    drawText(dc, gameStatus(), {700, 166, 910, 216}, titleFont);
    drawText(dc, L"已下 " + std::to_wstring(game.history().size()) + L" 手",
             {648, 228, 900, 258}, bodyFont);

    std::wstring lastMoveText = L"黑棋先行，點選交叉點開始";
    if (!game.history().empty()) {
        const auto& lastMove = game.history().back();
        lastMoveText = L"最後落子：" +
            std::wstring(1, static_cast<wchar_t>(L'A' + lastMove.col)) +
            L"・" + std::to_wstring(lastMove.row + 1);
    }
    drawText(dc, lastMoveText, {648, 262, 912, 288}, bodyFont);

    drawPanel(dc, 624, 318, 300, 150, RGB(255, 255, 251));
    drawText(dc, L"對弈小提示", {648, 332, 900, 364}, bodyFont);
    drawText(dc, L"橫、直或斜線連成五子即獲勝", {648, 370, 912, 400}, bodyFont);
    drawText(dc, L"紅點標示最後一手", {648, 400, 912, 430}, bodyFont);
    drawText(dc, L"悔棋後由上一位玩家重新落子", {648, 430, 912, 460}, bodyFont);
    drawText(dc, L"遊戲操作", {636, 486, 910, 518}, bodyFont);
    drawText(dc, L"自由五子棋 · 無禁手規則", {636, 654, 920, 687}, bodyFont);
}

void drawGame(HDC dc, RECT client) {
    HBRUSH background = CreateSolidBrush(RGB(238, 242, 237));
    FillRect(dc, &client, background);
    DeleteObject(background);

    drawText(dc, L"五子棋", {36, 20, 600, 62}, titleFont);
    drawText(dc, L"本機雙人  /  15 × 15", {650, 42, 925, 78}, bodyFont);
    drawBoard(dc);
    drawStones(dc);
    drawSidebar(dc);
    drawText(dc, L"GOMOKU", {36, 686, 610, 718}, bodyFont);
}

// ── 控制項與輸入處理 ──

void refreshWindow(HWND window) {
    EnableWindow(undoButton, !game.history().empty());
    InvalidateRect(window, nullptr, FALSE);
}

HFONT createFont(int height, int weight) {
    return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft JhengHei");
}

HWND createButton(HWND window, const wchar_t* label, int id, RECT bounds) {
    HWND button = CreateWindowW(
        L"BUTTON", label, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
        bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top,
        window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        GetModuleHandleW(nullptr), nullptr
    );
    SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(bodyFont), TRUE);
    return button;
}

void createControls(HWND window) {
    titleFont = createFont(-30, FW_BOLD);
    bodyFont = createFont(-16, FW_NORMAL);
    createButton(window, L"重新開始", BUTTON_RESTART, {642, 530, 766, 576});
    undoButton = createButton(window, L"悔棋", BUTTON_UNDO, {782, 530, 906, 576});
    createButton(window, L"離開遊戲", BUTTON_EXIT, {642, 594, 906, 640});
    EnableWindow(undoButton, FALSE);
}

void drawButton(const DRAWITEMSTRUCT& item) {
    const bool disabled = (item.itemState & ODS_DISABLED) != 0;
    const bool pressed = (item.itemState & ODS_SELECTED) != 0;
    const bool primary = item.CtlID == BUTTON_RESTART;

    COLORREF background = primary ? RGB(53, 88, 69) : RGB(248, 250, 246);
    COLORREF foreground = primary ? RGB(255, 255, 255) : RGB(48, 70, 53);
    if (disabled) {
        background = RGB(224, 228, 222);
        foreground = RGB(145, 154, 143);
    } else if (pressed) {
        background = RGB(174, 193, 173);
        foreground = RGB(48, 70, 53);
    }

    RECT bounds = item.rcItem;
    HBRUSH surround = CreateSolidBrush(RGB(238, 242, 237));
    FillRect(item.hDC, &bounds, surround);
    DeleteObject(surround);
    drawPanel(item.hDC, bounds.left, bounds.top,
              bounds.right - bounds.left - 1, bounds.bottom - bounds.top - 1, background);

    wchar_t label[64]{};
    GetWindowTextW(item.hwndItem, label, 64);
    HGDIOBJ previousFont = SelectObject(item.hDC, bodyFont);
    SetBkMode(item.hDC, TRANSPARENT);
    SetTextColor(item.hDC, foreground);
    DrawTextW(item.hDC, label, -1, &bounds, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(item.hDC, previousFont);

    if (item.itemState & ODS_FOCUS) {
        InflateRect(&bounds, -5, -5);
        DrawFocusRect(item.hDC, &bounds);
    }
}

// 將滑鼠座標轉為交叉點；預覽與真正落子使用相同的命中判斷。
bool hitTestBoard(int x, int y, int& row, int& col) {
    col = static_cast<int>(std::lround((x - BOARD_LEFT) / double(GRID_SPACING)));
    row = static_cast<int>(std::lround((y - BOARD_TOP) / double(GRID_SPACING)));
    return game.isValidMove(row, col) &&
           std::abs(x - boardX(col)) <= HIT_RADIUS &&
           std::abs(y - boardY(row)) <= HIT_RADIUS;
}

void updateHover(HWND window, int x, int y) {
    int row = -1;
    int col = -1;
    if (game.isFinished() || !hitTestBoard(x, y, row, col)) {
        row = -1;
        col = -1;
    }
    if (row != hoverRow || col != hoverCol) {
        hoverRow = row;
        hoverCol = col;
        InvalidateRect(window, nullptr, FALSE);
    }

    TRACKMOUSEEVENT tracking = {sizeof(TRACKMOUSEEVENT), TME_LEAVE, window, 0};
    TrackMouseEvent(&tracking);
}

void paintWindow(HWND window) {
    PAINTSTRUCT paintInfo;
    HDC dc = BeginPaint(window, &paintInfo);
    RECT client;
    GetClientRect(window, &client);

    // 先畫到記憶體，再一次複製到視窗，避免滑鼠移動時畫面閃爍。
    HDC buffer = CreateCompatibleDC(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, client.right, client.bottom);
    HGDIOBJ previousBitmap = SelectObject(buffer, bitmap);
    drawGame(buffer, client);
    BitBlt(dc, 0, 0, client.right, client.bottom, buffer, 0, 0, SRCCOPY);

    SelectObject(buffer, previousBitmap);
    DeleteObject(bitmap);
    DeleteDC(buffer);
    EndPaint(window, &paintInfo);
}

// ── Windows 事件分派：各事件只負責呼叫對應功能。 ──

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        createControls(window);
        return 0;

    case WM_DRAWITEM:
        drawButton(*reinterpret_cast<DRAWITEMSTRUCT*>(lParam));
        return TRUE;

    case WM_MOUSEMOVE:
        updateHover(window, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSELEAVE:
        hoverRow = -1;
        hoverCol = -1;
        InvalidateRect(window, nullptr, FALSE);
        return 0;

    case WM_LBUTTONUP: {
        int row;
        int col;
        if (hitTestBoard(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), row, col) &&
            game.makeMove(row, col)) {
            refreshWindow(window);
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case BUTTON_RESTART:
            game.resetGame();
            refreshWindow(window);
            return 0;
        case BUTTON_UNDO:
            game.undoMove();
            refreshWindow(window);
            return 0;
        case BUTTON_EXIT:
            DestroyWindow(window);
            return 0;
        }
        break;

    case WM_ERASEBKGND:
        return 1; // 背景已由雙緩衝一起繪製。

    case WM_PAINT:
        paintWindow(window);
        return 0;

    case WM_DESTROY:
        DeleteObject(titleFont);
        DeleteObject(bodyFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

// 用物件生命週期管理 GDI+，即使提早結束也能釋放資源。
struct GraphicsSession {
    ULONG_PTR token = 0;
    Gdiplus::Status status;

    GraphicsSession() {
        Gdiplus::GdiplusStartupInput input;
        status = Gdiplus::GdiplusStartup(&token, &input, nullptr);
    }

    ~GraphicsSession() {
        if (status == Gdiplus::Ok) {
            Gdiplus::GdiplusShutdown(token);
        }
    }
};

} // namespace

// ── 程式入口：初始化繪圖、建立視窗、持續處理事件。 ──

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand) {
    GraphicsSession graphics;
    if (graphics.status != Gdiplus::Ok) {
        return 1;
    }
    SetProcessDPIAware();

    WNDCLASSW windowClass{};
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = L"GomokuWindow";
    if (!RegisterClassW(&windowClass)) {
        return 1;
    }

    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
                        WS_MINIMIZEBOX | WS_CLIPCHILDREN;
    RECT size = {0, 0, CLIENT_WIDTH, CLIENT_HEIGHT};
    AdjustWindowRect(&size, style, FALSE);

    HWND window = CreateWindowW(
        windowClass.lpszClassName, L"五子棋｜雙人對弈", style,
        CW_USEDEFAULT, CW_USEDEFAULT, size.right - size.left, size.bottom - size.top,
        nullptr, nullptr, instance, nullptr
    );
    if (!window) {
        return 1;
    }
    ShowWindow(window, showCommand);
    UpdateWindow(window);

    MSG message{};
    int result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        // 支援用 Tab 選取按鈕，再用空白鍵執行。
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
