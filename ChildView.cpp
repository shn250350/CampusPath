// ChildView.cpp: CChildView 클래스의 구현
//

#include "pch.h"
#include "framework.h"
#include "CampusPath.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 전역 상수 (인접행렬에서 '없음' 표시용)
const double INF = 1e18;


// CChildView

CChildView::CChildView()
{
    m_selectedForEdge = -1;
    m_startNode = -1;
    m_endNode = -1;

    InitGraph();
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()



// CChildView 메시지 처리기

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
    if (!CWnd::PreCreateWindow(cs))
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
        ::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);

    return TRUE;
}

// ====== 그래프 초기화 ======================================

void CChildView::InitGraph()
{
    for (int i = 0; i < MAX_NODES; ++i) {
        for (int j = 0; j < MAX_NODES; ++j) {
            if (i == j) m_adj[i][j] = 0;
            else        m_adj[i][j] = INF;
        }
    }
}

// ====== OnCreate : 여기서 지도 이미지 로드 =================

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // EXE와 같은 폴더 또는 Debug 폴더에 CampusMap.png 넣어두기
    HRESULT hr = m_mapImg.Load(L"CampusMap.png");
    if (FAILED(hr)) {
        AfxMessageBox(L"CampusMap.png 로드 실패");
    }

    return 0;
}

// ====== 그리기 =============================================

void CChildView::OnPaint()
{
    CPaintDC dc(this); // 그리기를 위한 DC

    DrawMap(&dc);
    DrawEdges(&dc);
    DrawNodes(&dc);
    DrawShortestPath(&dc);
}

void CChildView::DrawMap(CDC* pDC)
{
    if (m_mapImg.IsNull())
        return;

    CRect rc;
    GetClientRect(&rc);   // 현재 창 내부 영역 크기

    int dstW = rc.Width();
    int dstH = rc.Height();

    int srcW = m_mapImg.GetWidth();
    int srcH = m_mapImg.GetHeight();

    // 비율 맞춰서 축소/확대 (옵션, 비율 신경 안 쓰면 이 부분 생략 가능)
    double scaleX = (double)dstW / srcW;
    double scaleY = (double)dstH / srcH;
    double scale = min(scaleX, scaleY);

    int drawW = (int)(srcW * scale);
    int drawH = (int)(srcH * scale);

    int offsetX = (dstW - drawW) / 2;
    int offsetY = (dstH - drawH) / 2;

    m_mapImg.Draw(
        pDC->m_hDC,
        offsetX, offsetY,          // 목적지 왼쪽 위
        drawW, drawH,              // 목적지 크기
        0, 0,                      // 원본 시작 좌표
        srcW, srcH                 // 원본 크기
    );
}

bool CChildView::IsInShortestPath(int idx)
{
    return std::find(m_shortestPath.begin(), m_shortestPath.end(), idx)
        != m_shortestPath.end();
}

void CChildView::DrawNodes(CDC* pDC)
{
    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        COLORREF color = IsInShortestPath(i) ? RGB(255, 0, 0) : RGB(0, 0, 255);

        CBrush brush(color);
        CBrush* oldBrush = pDC->SelectObject(&brush);

        int r = 4;
        int x = m_nodes[i].x;
        int y = m_nodes[i].y;

        pDC->Ellipse(x - r, y - r, x + r, y + r);

        pDC->SelectObject(oldBrush);
    }
}

void CChildView::DrawEdges(CDC* pDC)
{
    CPen pen(PS_SOLID, 1, RGB(0, 0, 255));
    CPen* oldPen = pDC->SelectObject(&pen);

    int n = (int)m_nodes.size();

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (m_adj[i][j] < INF / 2) {
                auto& a = m_nodes[i];
                auto& b = m_nodes[j];

                pDC->MoveTo(a.x, a.y);
                pDC->LineTo(b.x, b.y);

                // 선 중간에 거리 숫자 표시
                int mx = (a.x + b.x) / 2;
                int my = (a.y + b.y) / 2;
                CString s;
                s.Format(L"%.0f", m_adj[i][j]);
                pDC->TextOut(mx, my, s);
            }
        }
    }

    pDC->SelectObject(oldPen);
}

void CChildView::DrawShortestPath(CDC* pDC)
{
    if (m_shortestPath.size() < 2) return;

    CPen pen(PS_SOLID, 3, RGB(255, 0, 0));
    CPen* oldPen = pDC->SelectObject(&pen);

    for (int i = 0; i < (int)m_shortestPath.size() - 1; ++i) {
        auto& a = m_nodes[m_shortestPath[i]];
        auto& b = m_nodes[m_shortestPath[i + 1]];

        pDC->MoveTo(a.x, a.y);
        pDC->LineTo(b.x, b.y);
    }

    pDC->SelectObject(oldPen);
}

// ====== 보조 함수들 ========================================

int CChildView::FindNearestNode(CPoint pt)
{
    const int PICK_RADIUS = 10; // 픽셀 반경
    int idx = -1;
    double best = 1e9;

    for (int i = 0; i < (int)m_nodes.size(); ++i) {
        double dx = m_nodes[i].x - pt.x;
        double dy = m_nodes[i].y - pt.y;
        double d2 = dx * dx + dy * dy;

        if (d2 < best) {
            best = d2;
            idx = i;
        }
    }

    if (best > PICK_RADIUS * PICK_RADIUS)
        return -1;

    return idx;
}

double CChildView::Distance(const Node& a, const Node& b)
{
    return std::sqrt((double)(a.x - b.x) * (a.x - b.x) +
        (double)(a.y - b.y) * (a.y - b.y));
}

std::vector<int> CChildView::RunDijkstra(int start, int goal)
{
    int n = (int)m_nodes.size();
    std::vector<double> dist(n, INF);
    std::vector<int> prev(n, -1);
    std::vector<bool> used(n, false);

    dist[start] = 0;

    for (int i = 0; i < n; ++i) {
        int u = -1;
        double best = INF;
        for (int j = 0; j < n; ++j) {
            if (!used[j] && dist[j] < best) {
                best = dist[j];
                u = j;
            }
        }
        if (u == -1) break;
        used[u] = true;

        for (int v = 0; v < n; ++v) {
            if (m_adj[u][v] < INF / 2) {
                double nd = dist[u] + m_adj[u][v];
                if (nd < dist[v]) {
                    dist[v] = nd;
                    prev[v] = u;
                }
            }
        }
    }

    // ➜ 여기서 경로 유무 체크
    if (dist[goal] >= INF / 2) {
        AfxMessageBox(L"두 점 사이에 연결된 경로가 없습니다.\n(중간 선을 다 만들었는지 확인해 주세요)");
        return {};
    }

    std::vector<int> path;
    int cur = goal;
    while (cur != -1) {
        path.push_back(cur);
        cur = prev[cur];
    }
    std::reverse(path.begin(), path.end());
    return path;
}


// ====== 마우스 입력 ========================================

// 오른쪽 버튼 : 새 점 생성
void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
    if ((int)m_nodes.size() >= MAX_NODES) {
        AfxMessageBox(L"노드 개수 한도 초과");
        return;
    }

    Node node{ point.x, point.y };
    m_nodes.push_back(node);

    Invalidate(); // 다시 그리기
    CWnd::OnRButtonDown(nFlags, point);
}

// 왼쪽 버튼 : Ctrl = 선 생성, Alt = 최단 경로
void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
    int nearest = FindNearestNode(point);
    if (nearest == -1) {
        CWnd::OnLButtonDown(nFlags, point);
        return;
    }

    // Ctrl + Left : Edge 생성
    if (GetKeyState(VK_CONTROL) & 0x8000) {
        if (m_selectedForEdge == -1) {
            m_selectedForEdge = nearest;
        }
        else {
            int a = m_selectedForEdge;
            int b = nearest;
            if (a != b) {
                double dist = Distance(m_nodes[a], m_nodes[b]);
                m_adj[a][b] = dist;
                m_adj[b][a] = dist;
            }
            m_selectedForEdge = -1;
            Invalidate();
        }
    }
    // Alt + Left : 최단 경로
    else if (GetKeyState(VK_MENU) & 0x8000) {
        if (m_startNode == -1) {
            m_shortestPath.clear();  // 이전 경로 초기화
            m_startNode = nearest;
        }
        else {
            m_endNode = nearest;
            if (m_startNode != m_endNode) {
                m_shortestPath = RunDijkstra(m_startNode, m_endNode);
            }
            m_startNode = m_endNode = -1;
            Invalidate();
        }
    }

    CWnd::OnLButtonDown(nFlags, point);
}
